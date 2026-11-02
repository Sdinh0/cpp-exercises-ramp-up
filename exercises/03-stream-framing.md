# Exercises 11-15: Stream framing and parsing

Implement the incremental parser in
[`src/stream_decoder.cpp`](../src/stream_decoder.cpp). `StreamDecoder::feed`
receives one byte per call and returns any complete valid frames found.

## 11. Incremental parsing

Accumulate input until there are enough bytes for a whole frame. Do not emit a
frame while its payload or checksum is still missing.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_framing_tests.cpp`

## 12. Resource cleanup

After emitting a complete valid frame, remove its bytes from the receive
buffer. The decoder should not retain a completed frame.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_framing_tests.cpp`

## 13. Noise recovery

Ignore bytes received before the next sync marker. Once a sync marker appears,
continue parsing from that candidate frame.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_framing_tests.cpp`

## 14. Corrupt-stream recovery

When a candidate frame has a bad checksum, discard the invalid candidate and
continue accepting subsequent input instead of leaving the decoder stuck.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_framing_tests.cpp`

## 15. Multiple frames

Support back-to-back encoded frames. Each complete valid frame must be emitted
once, in input order, without bytes from one frame affecting the next.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_framing_tests.cpp`
