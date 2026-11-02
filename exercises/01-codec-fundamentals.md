# Exercises 1-5: CRC and wire format

These tasks introduce the frame codec. Keep its implementation in
[`src/codec.cpp`](../src/codec.cpp); its public API and protocol constants are
in [`include/sensor/sensor.hpp`](../include/sensor/sensor.hpp).

## 1. CRC baseline

Implement `Codec::crc8` with polynomial `0x07`, initial value `0x00`, no bit
reflection, and no final XOR. Process every input byte in order.

- **Update:** `src/codec.cpp`
- **Verify:** `Crc` tests in `tests/wire_format_tests.cpp`
- **Done when:** empty input, known check vectors, and high-bit input match
  their expected CRC values.

## 2. Frame encoding

Implement `Codec::encode` to produce `sync, command, payload length, payload,
CRC`, in that order. The CRC covers the command, length, and payload, but not
the sync byte.

- **Update:** `src/codec.cpp`
- **Verify:** `FrameEncoding` tests in `tests/wire_format_tests.cpp`
- **Done when:** populated and empty payloads produce the expected wire bytes
  and checksum.

## 3. Decode valid input

Implement `Codec::decode` for a complete valid frame. Return the decoded
command and payload, set `ok` to true, and use `ParseError::none`.

- **Update:** `src/codec.cpp`
- **Verify:** `FrameDecoding` tests in `tests/wire_format_tests.cpp`
- **Done when:** a frame produced by the codec decodes to the original values.

## 4. Short input defect

Inputs shorter than the four-byte minimum frame (sync, command, length, and
CRC) must fail with `ParseError::frame_too_short`.

- **Update:** `src/codec.cpp`
- **Verify:** `FrameDecoding` tests in `tests/wire_format_tests.cpp`
- **Done when:** every input length from zero through three is rejected with
  the same short-frame error.

## 5. Input validation

Reject a wrong sync byte, a declared payload length that does not match the
received frame, and a checksum mismatch. Report `bad_sync`, `bad_length`, or
`bad_checksum` respectively.

- **Update:** `src/codec.cpp`
- **Verify:** `FrameDecoding` tests in `tests/wire_format_tests.cpp`
- **Done when:** each invalid input is rejected with its corresponding error.
