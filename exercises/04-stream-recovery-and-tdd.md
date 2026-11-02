# Exercises 16-20: Stream recovery and TDD

Continue work in [`src/stream_decoder.cpp`](../src/stream_decoder.cpp), using
the interface and buffer in [`include/sensor/sensor.hpp`](../include/sensor/sensor.hpp).

## 16. Reset behavior

Make `StreamDecoder::reset` discard partial input. After reset, previously
buffered bytes must not contribute to a later frame.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_recovery_tests.cpp`

## 17. Empty payload

Treat a frame with a zero-length payload as a complete valid frame. Do not wait
for payload bytes that the length field says are absent.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_recovery_tests.cpp`

## 18. Buffer bound defect

Ensure malformed or repeated partial input cannot grow the receive buffer
without limit. Keep the buffered data within the largest possible protocol
frame, including sync, header, payload, and checksum.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_recovery_tests.cpp`

## 19. TDD practice

Before implementing a small variation of parser behavior, add a focused
GoogleTest case that fails for the current behavior. Observe the failure, make
the smallest implementation change, observe the pass, and refactor if needed.

- **Update:** `tests/stream_recovery_tests.cpp`, then
  `src/stream_decoder.cpp`
- **Verify:** run the `StreamDecoder` test group before and after the change.

## 20. Test quality review

Add at least one assertion for an invalid parser boundary. Run the entire
CTest suite and explain what the failure output identifies.

- **Update:** `tests/stream_recovery_tests.cpp`
- **Verify:** `ctest --test-dir build --output-on-failure`
