# Final integration and assessment

Finish by validating the components together, reviewing boundaries, and
checking that another developer can build the project from a clean checkout.

## Lifecycle integration

Review the successful initialization and data path, then check the existing
transport-timeout and lifecycle-failure coverage.

- **Review:** integration and timeout coverage in
  `tests/sensor_driver_tests.cpp`, plus lifecycle coverage in
  `tests/sensor_lifecycle_tests.cpp`
- **Verify:** run the `SensorDriver`, `SensorStackIntegration`, and
  `SensorLifecycle` CTest groups.

## Boundary audit

Check that malformed frames have defined parse errors and cannot make the
stream parser buffer grow beyond the protocol bound.

- **Review/update:** `src/codec.cpp`, `src/stream_decoder.cpp`, and focused
  tests in `tests/wire_format_tests.cpp`,
  `tests/codec_boundaries_tests.cpp`, and `tests/stream_recovery_tests.cpp`
- **Verify:** run the related `FrameDecoding`, `CodecBoundaries`, and
  `StreamDecoder` groups.

## Refactor

Improve names or remove duplication without changing observable behavior.
Keep each component in its matching `src/*.cpp` file.

- **Update:** only the implementation or tests directly affected by the
  refactor
- **Verify:** run the full CTest suite before and after; compare behavior.

## Clean-checkout test

Remove only your generated build directory, then follow `README.md` to
configure, build, and test from scratch. Preserve all source and unrelated
local files.

- **Update:** `README.md` if a clean build exposes missing instructions
- **Verify:** complete the documented clean configure/build/CTest sequence.

## Assessment

Run all tests and review the project against the definition of done in
[`ramp_up_plan.md`](../ramp_up_plan.md). Reassess your skill estimates in
your own skill matrix.

- **Review:** `ramp_up_plan.md`, `README.md`, and all project tests
- **Update:** your own skill matrix and completion notes
- **Verify:** full CTest suite passes and the project goals are accounted for.
