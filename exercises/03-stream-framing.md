# Exercises 11-15: Stream framing and parsing

Implement the incremental parser in
[`src/stream_decoder.cpp`](../src/stream_decoder.cpp). `StreamDecoder::feed`
receives exactly one byte per call and returns a vector of zero or more
**complete, valid** `Frame` values found by processing that byte. It must keep
any unfinished candidate internally for later calls. The public frame format
and limits are defined by `Codec` in
[`include/sensor/sensor.hpp`](../include/sensor/sensor.hpp); use
`Codec::decode` to validate a candidate rather than duplicating the codec's
checksum rules.

## What counts as a complete frame?

The wire format is:

| Offset | Field | Size | Meaning |
|---:|---|---:|---|
| 0 | Sync | 1 byte | Must equal `Codec::sync_byte` (`0xA5`) |
| 1 | Command | 1 byte | Copied to `Frame::command` |
| 2 | Payload length | 1 byte | Number of payload bytes; valid values are 0-16 |
| 3 onward | Payload | Length bytes | Copied to `Frame::payload` |
| Last | CRC-8 | 1 byte | Covers command, length, and payload; does not cover sync |

Once the sync byte and length byte have arrived, the candidate's total size
is known: **4 + payload length bytes**. Four bytes is the minimum frame (an
empty payload); twenty bytes is the maximum frame (a 16-byte payload).

A candidate is **complete** when that many bytes have arrived. It is a
**complete valid frame** only if the sync, length, and CRC are valid. Do not
emit a frame just because the expected number of bytes arrived: a bad checksum
must not produce a `Frame`. Do not wait for extra bytes after the expected
frame size; the next byte may begin another frame.

For example, the frame with command `0x10` and payload `{0x01, 0x02}` is:

```text
A5 10 02 01 02 AA
|  |  |  |     |
|  |  |  |     +-- CRC-8 over 10 02 01 02
|  |  |  +-------- two payload bytes
|  |  +----------- payload length (2), so total frame size is 6
|  +-------------- command
+----------------- sync
```

Calling `feed` once for each byte should behave like this:

| Bytes received so far | Expected result |
|---|---|
| `A5` | No frame; keep 1 byte buffered |
| `A5 10` | No frame; command is known, but length has not arrived |
| `A5 10 02` | No frame; expected total size is 6, only 3 arrived |
| `A5 10 02 01 02` | No frame; checksum has not arrived |
| `A5 10 02 01 02 AA` | Emit one frame (`command=0x10`, payload `{0x01,0x02}`); clear its bytes |

An empty-payload frame still has four bytes: sync, command, zero length, and
CRC. The length is zero, not an instruction to wait for a payload byte.

## General parsing behavior

1. Ignore bytes before a sync byte; they are noise, not part of a frame.
2. Starting at sync, wait until the length byte is available. If it is greater
   than `Codec::max_payload_size`, it cannot describe a valid protocol frame;
   discard that candidate and resume searching for sync.
3. For an allowed length, wait for exactly `4 + length` bytes. Keep returning
   no frame while the candidate is incomplete.
4. Validate the candidate with `Codec::decode`. Emit its `Frame` only when
   `DecodeResult::ok` is true.
5. After emitting a frame, remove its bytes and immediately continue parsing
   any already-buffered bytes. This is what allows consecutive frames.
6. If a candidate is invalid, do not emit it. Discard/recover from it so later
   valid frames can still be found; exercise 14 focuses on a bad checksum
   followed by a good frame.

For a simple stream of one valid frame at a time, `feed` returns an empty
vector for every prefix and a one-element vector when the final byte arrives.
With consecutive input, each frame is emitted once, in order, on the call
that supplies its final byte.

## 11. Incremental parsing

Append incoming bytes to the receive buffer and use the sync and length fields
to determine the candidate's expected size. Do not emit a frame while any
payload or the CRC byte is still missing. A candidate is not valid until
`Codec::decode` accepts it.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_framing_tests.cpp`
- **Key check:** `WaitsUntilFrameIsComplete` emits nothing for every prefix;
  `EmitsCompleteFrame` emits the decoded command and payload only after the
  final byte.

## 12. Resource cleanup

After emitting a valid frame, remove all of that frame's bytes from the
receive buffer. The buffer must not retain a completed frame or any bytes
before its next candidate.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_framing_tests.cpp`
- **Key check:** `ClearsBufferAfterEmittingFrame` expects a buffered size of
  zero after the frame is consumed.

## 13. Noise recovery

Bytes received before the next `0xA5` sync marker are noise. Ignore them
without treating them as command, length, payload, or checksum. Begin a
candidate at the sync marker and parse normally from there.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_framing_tests.cpp`
- **Key check:** `IgnoresNoiseBeforeSync` sends two noise bytes before a valid
  encoded frame; the decoder must still emit that frame.

## 14. Corrupt-stream recovery

When the expected number of bytes has arrived but `Codec::decode` reports a
bad checksum, do not emit the candidate. Discard the bad candidate and resume
looking for a sync marker so a following valid frame can be decoded. A bad
candidate must not permanently block later input.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_framing_tests.cpp`
- **Key check:** `RecoversAfterBadChecksum` sends a corrupted frame followed
  by a valid frame; only the valid frame should be emitted.

## 15. Multiple frames

Support back-to-back encoded frames with no separator. After consuming one
valid frame, continue with any remaining buffered bytes; each following frame
has its own sync, length, and checksum. Emit each valid frame exactly once and
in wire order.

- **Update:** `src/stream_decoder.cpp`
- **Verify:** `StreamDecoder` tests in `tests/stream_framing_tests.cpp`
- **Key check:** `EmitsConsecutiveFrames` concatenates two encoded frames and
  expects exactly two frames to be emitted.
