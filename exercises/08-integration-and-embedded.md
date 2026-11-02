# Exercises 36-40: Integration and embedded constraints

These tasks connect the existing components and document the boundary between
the host-testable library and target-specific code.

## 36. End-to-end path

Add an integration test that encodes a frame, feeds its bytes through
`StreamDecoder`, and uses the decoded payload to add a value to `SampleWindow`.

- **Update:** a new `tests/integration_tests.cpp` and `CMakeLists.txt`
- **Verify:** register and run the integration test through CTest.

## 37. Host/target boundary

Document which codec, stream, sample, and lifecycle code is hardware
independent. Describe what a target-specific adapter would own, such as
hardware I/O and translating device events into library calls.

- **Update:** `README.md`
- **Verify:** review the project map and ensure host tests require no board or
  target toolchain.

## 38. Memory review

Inspect vector allocations and the stream buffer bound. In your own notes,
describe one viable no-heap alternative for a constrained target without
changing the current public API.

- **Review:** `src/codec.cpp`, `src/stream_decoder.cpp`, and
  `include/sensor/sensor.hpp`
- **Update:** your own notes; do not change the API for this review.

## 39. Error policy

Document whether malformed transport frames should alter lifecycle state, and
why parsing errors and lifecycle transitions remain separate responsibilities.

- **Update:** `README.md`
- **Verify:** compare the documented policy with `src/codec.cpp`,
  `src/stream_decoder.cpp`, and `src/sensor_lifecycle.cpp`.

## 40. Target smoke check

If a board or target toolchain is available, add an explicitly invoked target
compile or smoke test. Keep normal host tests runnable without it. If no target
is available, document that limitation rather than making host validation
depend on unavailable tooling.

- **Update:** target-specific build configuration and `README.md`, only if a
  target toolchain is available
- **Verify:** host configure/build/CTest still works independently.
