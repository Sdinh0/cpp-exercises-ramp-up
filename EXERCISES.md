# Daily workday exercises

There are 50 exercises: five days per week for ten weeks. Each is scoped as a
four-hour defect-fix or improvement task. The starter implementation is
deliberately incomplete. Tests describe observable behavior; don't change a test
just to make it pass.

Work in order through the topic guides below. Each exercise lists the files to
read or update and the focused test group to run. The implementation is split
by component under `src/`; the tests are split by behavior under `tests/`.
Exercises that do not have a dedicated test group describe a review, workflow,
or documentation check instead.

## Topic guides

| Exercises | Topic | Guide |
|---|---|---|
| 1-5 | CRC and wire format | [Codec fundamentals](exercises/01-codec-fundamentals.md) |
| 6-10 | Codec boundaries and C++ APIs | [Codec boundaries and APIs](exercises/02-codec-boundaries-and-apis.md) |
| 11-15 | Stream framing and parsing | [Stream framing](exercises/03-stream-framing.md) |
| 16-20 | Stream recovery and TDD | [Stream recovery and TDD](exercises/04-stream-recovery-and-tdd.md) |
| 21-25 | Sample processing | [Sample window](exercises/05-sample-window.md) |
| 26-30 | Lifecycle state and transitions | [Sensor lifecycle](exercises/06-sensor-lifecycle.md) |
| 31-35 | Continuous integration | [Continuous integration](exercises/07-continuous-integration.md) |
| 36-40 | Integration and embedded constraints | [Integration and embedded constraints](exercises/08-integration-and-embedded.md) |
| 41-45 | CI hardening and developer workflow | [CI hardening](exercises/09-ci-hardening.md) |
| 46-50 | Final integration and assessment | [Final integration](exercises/10-final-integration.md) |

## Source and test map

| Component | Implementation | Tests |
|---|---|---|
| Frame codec and CRC | `src/codec.cpp` | `tests/wire_format_tests.cpp`, `tests/codec_boundaries_tests.cpp` |
| Incremental stream parser | `src/stream_decoder.cpp` | `tests/stream_framing_tests.cpp`, `tests/stream_recovery_tests.cpp` |
| Rolling sample average | `src/sample_window.cpp` | `tests/sample_window_tests.cpp` |
| Sensor state machine | `src/sensor_lifecycle.cpp` | `tests/sensor_lifecycle_tests.cpp` |
| Public types and interfaces | `include/sensor/sensor.hpp` | All component tests |

The public API stays in `include/sensor/sensor.hpp`; `CMakeLists.txt` lists the
implementation files that are compiled into `sensor_core`.

## Daily completion record

After each task, record the date, focused test result, full-suite result, and a
one-sentence root-cause or learning note. Do not mark a task complete just
because the code compiles; its focused check must pass.
