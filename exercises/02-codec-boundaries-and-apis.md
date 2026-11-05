# Codec boundaries and C++ APIs

Continue the codec work in [`src/codec.cpp`](../src/codec.cpp). The protocol
types and limits are declared in [`include/sensor/sensor.hpp`](../include/sensor/sensor.hpp).

## Payload limit

Enforce `Codec::max_payload_size` (16 bytes) when encoding. A payload exactly
at the limit is valid; a larger payload must be rejected using the API's
existing failure representation.

- **Update:** `src/codec.cpp`
- **Verify:** `CodecBoundaries` tests in `tests/codec_boundaries_tests.cpp`

## Decode boundary

Decode a maximum-sized payload and reject bytes beyond the frame declared by
its length field. Do not accept trailing data as part of a valid frame.

- **Update:** `src/codec.cpp`
- **Verify:** `CodecBoundaries` tests in `tests/codec_boundaries_tests.cpp`

## Round-trip improvement

Check that encoding and then decoding preserves the command and every payload
byte, including `0x00`, `0x7F`, `0x80`, and `0xFF`.

- **Update:** `tests/codec_boundaries_tests.cpp` if an additional case is
  useful; adjust `src/codec.cpp` only if the behavior is incorrect.
- **Verify:** `CodecBoundaries` tests in `tests/codec_boundaries_tests.cpp`

## API review

Review `Frame`, `DecodeResult`, and `ParseError`. In your own learning notes,
describe payload ownership, the meaning of the `ok` and `error` fields, and
what a caller should expect for invalid input.

- **Review:** `include/sensor/sensor.hpp` and `src/codec.cpp`
- **Update:** your own notes; no implementation change is required.
- **Verify:** no dedicated test; check your notes against the declarations and
  observed decode behavior.

## C++ review

Build with warnings enabled. In your own notes, explain the interface roles of
`const Frame&`, `std::vector`, and `std::optional`.

- **Review:** `include/sensor/sensor.hpp`, `CMakeLists.txt`, and `README.md`
- **Update:** your own notes; only change project files if you find a concrete
  issue.
- **Verify:** build `sensor_core` with `SENSOR_ENABLE_WARNINGS` enabled.
