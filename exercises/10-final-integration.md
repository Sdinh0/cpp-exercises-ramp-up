# Exercises 46-50: Final integration and assessment

Finish by validating the components together, reviewing boundaries, and
checking that another developer can build the project from a clean checkout.

## 46. Lifecycle integration

Test a successful initialization and data path, then exercise a timeout and
recovery path.

- **Update:** integration coverage in `tests/integration_tests.cpp` and
  `CMakeLists.txt`
- **Verify:** run the integration test and `SensorLifecycle` tests.

## 47. Boundary audit

Check that malformed frames have defined parse errors and cannot make the
stream parser buffer grow beyond the protocol bound.

- **Review/update:** `src/codec.cpp`, `src/stream_decoder.cpp`, and focused
  tests in `tests/wire_format_tests.cpp`,
  `tests/codec_boundaries_tests.cpp`, and `tests/stream_recovery_tests.cpp`
- **Verify:** run the related `FrameDecoding`, `CodecBoundaries`, and
  `StreamDecoder` groups.

## 48. Refactor

Improve names or remove duplication without changing observable behavior.
Keep each component in its matching `src/*.cpp` file.

- **Update:** only the implementation or tests directly affected by the
  refactor
- **Verify:** run the full CTest suite before and after; compare behavior.

## 49. Clean-checkout test

Remove only your generated build directory, then follow `README.md` to
configure, build, and test from scratch. Preserve all source and unrelated
local files.

- **Update:** `README.md` if a clean build exposes missing instructions
- **Verify:** complete the documented clean configure/build/CTest sequence.

## 50. Assessment

Run all tests and review the project against the definition of done in
[`ramp_up_plan.md`](../ramp_up_plan.md). Reassess your skill estimates in
your own skill matrix.

- **Review:** `ramp_up_plan.md`, `README.md`, and all project tests
- **Update:** your own skill matrix and completion notes
- **Verify:** full CTest suite passes and the project goals are accounted for.
