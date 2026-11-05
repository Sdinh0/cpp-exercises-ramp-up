# Sensor driver and lifecycle stack

## Goal

Implement the sensor-specific driver over the project's byte-transport HAL.
Reuse `Codec`, `StreamDecoder`, and `SampleWindow` so the result connects to
the existing project instead of introducing a separate register protocol.

The completed stack is:

```text
SensorLifecycle
    -> SensorDevice / SensorDriver
        -> SensorHal byte transport
            -> Codec and StreamDecoder
        -> decoded samples -> SampleWindow
```

The project provides the interfaces in
[`include/sensor/sensor.hpp`](../include/sensor/sensor.hpp), incomplete starter
implementations in
[`starter/src/sensor_driver.cpp`](../starter/src/sensor_driver.cpp) and
[`starter/src/sensor_lifecycle.cpp`](../starter/src/sensor_lifecycle.cpp).
Tests define the expected behavior. The simulated sensor in
[`tests/sensor_driver_tests.cpp`](../tests/sensor_driver_tests.cpp) decodes
requests, enforces device state, and queues framed responses for buffered
reads. Configurable read chunk sizes exercise partial response delivery. Writes
submit each complete request frame in one call; the HAL handles any lower-level
partial transfers. No board, physical sensor, or real transport is needed.

Implement both the protocol-facing `SensorDriver` and the higher-level
`SensorLifecycle`. The driver handles frames, response validation, protocol
order, and sample decoding. The lifecycle coordinates the driver's startup
operations and owns lifecycle state; it must not parse frames or decode sample
payloads.

## Transport and frame exchange

`SensorHal` exposes buffered `write(data, size, timeout_ms)` and
`read(data, capacity, timeout_ms)` operations. Each returns an `IoResult`
containing an `IoStatus` and the number of bytes transferred. `SensorDriver`
operations return `SensorStatus`, allowing callers to distinguish an invalid
operation order, transport failure, malformed response, and sensor rejection.
`read_sample()` returns `SampleResult`, which contains both a `SensorStatus`
and the sample value.

A successful write means the complete request buffer was transmitted; the HAL
handles any lower-level partial transfers internally. The driver submits the
frame once and never retries a remainder. A write timeout or transport error
fails the transaction, even if the HAL reports that some bytes were sent.

Reads may return fewer bytes than the buffer can hold. The driver feeds each
received chunk to the stream decoder, preserving partial frames between reads.
The transfer count must not exceed the requested size. A successful read with
zero transferred bytes is no progress and must fail rather than loop forever.
A timeout or transport error fails the transaction.

### Status results

The public `SensorDevice` and `SensorDriver` operations return
`SensorStatus`. `read_sample()` returns `SampleResult`, and its `value` is
meaningful only when `status` is `SensorStatus::ok`.

| Status | Meaning |
|---|---|
| `ok` | Transport and framing succeeded in `transact()`; the public operation returns it only when the sensor accepts the command and command-specific validation succeeds. |
| `invalid_state` | Returned by a public driver operation or `SensorLifecycle::begin()` when called in a disallowed state. |
| `invalid_request` | `transact()` could not encode the request frame. Current public commands use fixed, bounded payloads, so this is a defensive result for an invalid internal request. |
| `write_timeout` | `transact()` maps `IoStatus::timeout` from `SensorHal::write()` to this value. |
| `write_error` | `transact()` maps `IoStatus::error` from `SensorHal::write()` to this value. |
| `incomplete_write` | `write()` returned `ok` but reported fewer bytes than requested. This violates the all-or-failure write contract. |
| `read_timeout` | `transact()` maps a read timeout or expiry of the overall response deadline to this value. |
| `read_error` | `transact()` maps `IoStatus::error` from `SensorHal::read()` to this value. |
| `no_progress` | A read or write returned `ok` with zero bytes transferred. |
| `invalid_transfer_count` | A HAL operation reported more bytes transferred than the supplied buffer/request permits. |
| `unexpected_response` | A decoded response frame has a command other than `request_command | response_mask`. |
| `sensor_rejected` | A valid matching response reports a nonzero sensor status byte. |
| `self_test_failed` | The sensor status is successful, but its self-test result says the test failed. |
| `invalid_response` | A response is missing its status byte or has an invalid command-specific payload length/value. |

`transact()` returns `ok` only when the complete request was sent and a valid,
matching response frame was received. It places the response's status byte and
remaining command data in `SensorResponse`; it does not convert the sensor
status into a transaction error. The public operation checks
`SensorResponse::status`, validates its command-specific data, and returns the
final `SensorStatus`. `SensorLifecycle::begin()` returns `ok` after all startup
operations succeed; otherwise it returns the first failing operation's status
and enters `fault`.

The stream decoder discards noise and malformed frames while searching for a
valid frame. Since its API does not expose a parse error to `SensorDriver`, a
corrupt frame that is never followed by a valid response ends as
`read_timeout`, not as a separate invalid-frame status. A valid frame with a
wrong command is observable and returns `unexpected_response`.

For each driver operation:

- Encode one request `Frame` with `Codec::encode()` and submit the complete
  encoded frame in one `write()` call. The HAL is responsible for sending the
  entire buffer or reporting failure.
- Read response chunks into a buffer and pass each received range to
  `StreamDecoder::feed(data, size)` until the matching response frame is
  available.
- The response command is the request command ORed with `response_mask`.
- The first response payload byte is status: zero means success; nonzero means
  the simulated sensor rejected the request.
- Propagate write/read errors and reject malformed frames, bad checksums,
  and responses for another command. A successful transmit alone is not a
  successful sensor operation.
- Keep transaction validity distinct from the sensor's command status. A
  matching, well-formed response with nonzero status is a successful exchange
  that reports a rejected operation. `transact()` returns `ok` for a valid
  exchange and separates the response's sensor status from command-specific
  payload data. Public operations translate nonzero sensor status into
  `SensorStatus::sensor_rejected`; a self-test failure returns
  `SensorStatus::self_test_failed`.
- Bound response collection with a transaction deadline; do not wait
  indefinitely for bytes. The HAL timeout applies to each read/write call,
  while the driver enforces the overall transaction bound.

## Device protocol

Use the command and configuration constants declared by `SensorDriver`. The
simulated sensor expects these operations in order:

1. **Reset** — send `reset_command` with no payload. On success, the sensor
   clears its configured, self-tested, and running flags.
2. **Configure** — send `configure_command` with
   `{sample_rate_hz, sample_range_g}`. Configuration succeeds only after reset
   and if both settings match.
3. **Self-test** — send `self_test_command` with no payload. The response
   payload is `{status, pass}`. A failed self-test must prevent start.
4. **Start** — send `start_command` with no payload. The sensor accepts it
   only after reset, configuration, and a passing self-test.
5. **Read sample** — send `read_sample_command` with no payload. A successful
   response payload is `{status, sample_high, sample_low}`. The two sample
   bytes form a signed 16-bit big-endian two's-complement value; convert it to
   `std::int32_t` preserving its sign.

Every response frame has the command `request_command | response_mask`. Its
payload begins with the status byte; zero means success, and nonzero means the
sensor rejected the operation. In `SensorDriver::SensorResponse`, that byte is
stored separately as `status`, with any remaining command-specific bytes in
`payload`. Reset, configure, and start have no additional response data in the
simulated protocol.

Reject out-of-order calls locally without transmitting. Advance driver state
only after a valid successful response. After an operation fails, do not allow
later operations to proceed as if startup had succeeded.

## Driver implementation

### Implement framed transactions

Encode requests and submit each complete request frame with one HAL write.
Read response chunks and feed each buffer to the range overload of
`StreamDecoder::feed`. Use it for noise
recovery and frame validation. Check the response command and status, and
report timeout, transport, or protocol errors as failure.

**Verify with:**

- `SensorDriver.BadChecksumResponseIsRejected`
- `SensorDriver.StreamDecoderRecoversFromLeadingTransportNoise`
- `SensorDriver.IgnoresResponseForAnotherCommand`
- `SensorDriver.TransportErrorsAreReported`
- `SensorDriver.ReadTimeoutFailsTheTransaction`

### Implement ordered startup

Implement reset, configure, self-test, and start with local state guards.
Respect the response payload shapes and stop progression after any rejected or
failed operation. Return the specific `SensorStatus` for each outcome and use
`SensorStatus::invalid_state` for a locally rejected out-of-order call.

**Verify with:**

- `SensorDriver.EncodesOrderedStartupCommandsAndParsesResponses`
- `SensorDriver.RejectsOutOfOrderCommandsWithoutSendingFrames`
- `SensorDriver.StopsInitializationWhenTheSensorRejectsConfiguration`
- `SensorDriver.SelfTestFailurePreventsStartingTheSensor`

### Decode and use samples

Implement `read_sample()` only after the driver has started. Decode the signed
16-bit sample from the response without losing its sign. Return it alongside
the status in `SampleResult`; the caller should use the value only when status
is `SensorStatus::ok`. Keep rolling storage and averaging in the existing
`SampleWindow`, not in the driver.

**Verify with:** `SensorDriver.ReadsSignedSamplesAndFeedsTheExistingSampleWindow`.

### Handle chunked reads and failed I/O

Do not split a request across multiple HAL write calls. The HAL completes the
whole write or reports failure. Feed short reads incrementally to the stream
decoder. Treat a successful zero-byte read as no progress and return failure
rather than retrying forever.

**Verify with:**

- `SensorDriver.WritesEachCompleteRequestOnceAndParsesChunkedReads`
- `SensorDriver.ZeroProgressTransportResultsFailInsteadOfLooping`
- `SensorDriver.ReadTimeoutFailsTheTransaction`

## Lifecycle implementation

Implement `SensorLifecycle` in `src/sensor_lifecycle.cpp`. Its constructor
receives a `SensorDevice`; use the fake device in
[`tests/sensor_lifecycle_tests.cpp`](../tests/sensor_lifecycle_tests.cpp) to
develop lifecycle policy independently of transport details.

### Implement initialization state and ordering

Start in `uninitialized`. `begin()` is effective only from that state. Set
`initializing` before calling the device, then call `hardware_reset()`,
`configure()`, `self_test()`, and `start()` in order. Enter `ready` only when
all operations succeed. Return the first failing `SensorStatus` and enter
`fault`; return `SensorStatus::ok` after successful initialization. Calling
`begin()` from any other state returns `SensorStatus::invalid_state`.

**Verify with:** `SensorLifecycle.StartsDeviceInOrder`.

### Handle failures and retries

On the first failed device operation, stop immediately and enter `fault`.
Calling `begin()` while faulted must not retry. `reset_state()` returns the
lifecycle coordinator to `uninitialized` without calling the device; a later
`begin()` starts a fresh initialization attempt, including a hardware reset.

**Verify with:**

- `SensorLifecycle.StopsAtTheFirstFailedStartupStep`
- `SensorLifecycle.ResetStateAllowsACompleteNewInitializationAttempt`
- `SensorLifecycle.ResetStateDoesNotInvokeDevice`

### Preserve lifecycle guards

Do not repeat startup if `begin()` is called while already `ready`. Keep
`state()` as an observation-only operation.

**Verify with:**

- `SensorLifecycle.StartsUninitialized`
- `SensorLifecycle.BeginDoesNotRepeatInitializationWhileReady`

Run the lifecycle checks from the repository root:

```powershell
ctest --test-dir build -R "SensorLifecycle" --output-on-failure
```

## Full-stack integration

The integration test starts the driver through `SensorLifecycle`, reads
simulated samples through buffered framed transport, and checks their average
in `SampleWindow`. Keep lifecycle policy independent from transport details.

**Verify with:**
`SensorStackIntegration.StartsDriverAndFeedsDecodedSamplesToWindow`.

Run the focused checks from the repository root:

```powershell
cmake --build build
ctest --test-dir build -R "SensorDriver|SensorStackIntegration" --output-on-failure
ctest --test-dir build -R "SensorLifecycle" --output-on-failure
```

Both starter implementations are intentionally incomplete. The project and
test executable should compile, while driver and lifecycle behavior checks
fail until the corresponding implementations are completed. The end-to-end
test should pass only when both layers work together. Keep the starter sources
consistent with the public interfaces so copying them over `src/` restores a
buildable exercise starting point.
