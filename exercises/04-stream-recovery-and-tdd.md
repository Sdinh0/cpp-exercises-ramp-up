# Stream recovery and TDD

Continue work in [`src/stream_decoder.cpp`](../src/stream_decoder.cpp), using
the interface and buffer in [`include/sensor/sensor.hpp`](../include/sensor/sensor.hpp).
For the byte layout, expected frame size, and definition of a complete valid
frame, see [Stream framing](03-stream-framing.md). A frame is
`4 + payload length` bytes total: sync, command, length, payload bytes, and
CRC. The permitted payload length is 0-16 bytes.

## Reset behavior

Make `StreamDecoder::reset` discard partial input. After reset, previously
buffered bytes must not contribute to a later frame.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_recovery_tests.cpp`
- **Meaning:** If only a prefix such as sync, command, or length has arrived,
  reset forgets it. Subsequent bytes are interpreted as a fresh stream.

## Empty payload

Treat a frame with a zero-length payload as a complete valid frame. Do not wait
for payload bytes that the length field says are absent.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_recovery_tests.cpp`
- **Meaning:** The complete wire frame is still four bytes: sync, command,
  length `0`, and CRC over command plus length.

## Buffer bound defect

Ensure malformed or repeated partial input cannot grow the receive buffer
without limit. Keep the buffered data within the largest possible protocol
frame, including sync, header, payload, and checksum.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_recovery_tests.cpp`
- **Meaning:** The largest valid candidate is `Codec::max_payload_size + 4`
  bytes (20 with the current 16-byte limit). Discard noise and impossible
  lengths instead of retaining them indefinitely. The test repeatedly feeds
  sync, command, and an impossible length of `0xFF`; the buffer must stay at
  or below the maximum-frame size.

## TDD practice

Before implementing a small variation of parser behavior, add a focused
GoogleTest case that fails for the current behavior. Observe the failure, make
the smallest implementation change, observe the pass, and refactor if needed.
Good variations include a partial frame followed by `reset`, a zero-length
payload, or an invalid length followed by a valid frame.

- **Update:** `tests/stream_recovery_tests.cpp`, then
  `src/stream_decoder.cpp`
- **Verify:** run the `StreamDecoder` test group before and after the change.

## Test quality review

Add at least one assertion for an invalid parser boundary. Run the entire
CTest suite and explain what the failure output identifies.

- **Update:** `tests/stream_recovery_tests.cpp`
- **Verify:** `ctest --test-dir build --output-on-failure`
