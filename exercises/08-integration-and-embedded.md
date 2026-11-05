# Integration and embedded constraints

These tasks connect the existing components and document the boundary between
the host-testable library and target-specific code.

## End-to-end sensor path

Review and extend the existing simulated end-to-end test. It should start a
`SensorDriver` through `SensorLifecycle`, exchange encoded frames through
buffered byte-transport HAL calls and `StreamDecoder`, read signed samples,
and add them to `SampleWindow`.

- **Update:** `tests/sensor_driver_tests.cpp` and `CMakeLists.txt` only if a
  missing integration case requires it
- **Verify:** run
  `SensorStackIntegration.StartsDriverAndFeedsDecodedSamplesToWindow`.

## Target HAL adapter boundary

Document the layers: codec, stream decoder, sample window, driver, and
lifecycle policy are hardware independent. A target-specific `SensorHal`
adapter owns UART/SPI/I2C or other hardware I/O and exposes buffered reads and
writes with transfer status, byte counts, and timeouts to `SensorDriver`.
Each write submits a complete encoded request in one call: on success the HAL
reports the full byte count, and it handles any lower-level partial transfers
internally. Reads may return fewer bytes than requested; the driver feeds
those chunks to `StreamDecoder`, and rejects errors, invalid byte counts, and
zero-progress reads. The HAL transports bytes, not protocol frames;
`SensorDriver` uses `Codec` and the byte/range `StreamDecoder::feed` overloads
above it. The simulated HAL remains the host-test implementation; do not make
normal host tests depend on board hardware or a target toolchain.

- **Update:** `README.md`
- **Verify:** review the project map and ensure host tests require no board or
  target toolchain.

## Memory review

Inspect vector allocations and the stream buffer bound. In your own notes,
describe one viable no-heap alternative for a constrained target without
changing the current public API.

- **Review:** `src/codec.cpp`, `src/stream_decoder.cpp`, and
  `include/sensor/sensor.hpp`
- **Update:** your own notes; do not change the API for this review.

## Error policy

Document whether malformed transport frames should alter lifecycle state, and
why parsing errors and lifecycle transitions remain separate responsibilities.

- **Update:** `README.md`
- **Verify:** compare the documented policy with `src/codec.cpp`,
  `src/stream_decoder.cpp`, and `src/sensor_lifecycle.cpp`.

## Target HAL adapter and smoke check

If a board and target toolchain are available, implement a target-specific
buffered byte-transport `SensorHal` adapter and add an explicitly invoked
target compile or smoke test. Keep the driver and lifecycle hardware
independent and normal host tests runnable without the target. If no target is
available, document the intended adapter boundary and that limitation rather
than making host validation depend on unavailable tooling.

- **Update:** target-specific adapter/build configuration and `README.md`,
  only if a target toolchain is available
- **Verify:** host configure/build/CTest still works independently.
