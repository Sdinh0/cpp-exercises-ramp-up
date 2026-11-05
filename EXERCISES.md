# Exercise guides

Use these topic guides as a learning progression, not as a fixed day count or
schedule. The starter implementation is deliberately incomplete. Tests describe
observable behavior; don't change a test just to make it pass.

Work through the guides in order where one component builds on another. Each
guide lists the relevant files and focused checks. Implementation is split by
component under `src/`, with tests grouped by behavior under `tests/`.

## Learning progression

| Topic | Guide |
|---|---|
| CRC and wire format | [Codec fundamentals](exercises/01-codec-fundamentals.md) |
| Codec boundaries and C++ APIs | [Codec boundaries and APIs](exercises/02-codec-boundaries-and-apis.md) |
| Stream framing and parsing | [Stream framing](exercises/03-stream-framing.md) |
| Stream recovery and TDD | [Stream recovery and TDD](exercises/04-stream-recovery-and-tdd.md) |
| Sample processing | [Sample window](exercises/05-sample-window.md) |
| Sensor driver, simulated HAL, and lifecycle integration | [Sensor stack](exercises/06-sensor-stack.md) |
| Continuous integration | [Continuous integration](exercises/07-continuous-integration.md) |
| Integration and embedded constraints | [Integration and embedded constraints](exercises/08-integration-and-embedded.md) |
| CI hardening and developer workflow | [CI hardening](exercises/09-ci-hardening.md) |
| Final integration and assessment | [Final integration](exercises/10-final-integration.md) |

## Source and test map

| Component | Implementation | Tests |
|---|---|---|
| Frame codec and CRC | `src/codec.cpp` | `tests/wire_format_tests.cpp`, `tests/codec_boundaries_tests.cpp` |
| Incremental stream parser (byte and buffer input) | `src/stream_decoder.cpp` | `tests/stream_framing_tests.cpp`, `tests/stream_recovery_tests.cpp` |
| Rolling sample average | `src/sample_window.cpp` | `tests/sample_window_tests.cpp` |
| Buffered byte-transport HAL and sensor driver | `SensorHal` and `SensorDriver` in `include/sensor/sensor.hpp`; `src/sensor_driver.cpp` | `tests/sensor_driver_tests.cpp` |
| Lifecycle coordinator | `src/sensor_lifecycle.cpp` | `tests/sensor_lifecycle_tests.cpp`; end-to-end coverage in `tests/sensor_driver_tests.cpp` |
| Public types and interfaces | `include/sensor/sensor.hpp` | Component tests |

The public API stays in `include/sensor/sensor.hpp`; `CMakeLists.txt` lists the
implementation files that are compiled into `sensor_core`.

## Progress notes

Track completed topics and useful takeaways in whatever format suits your
workflow. Do not mark a behavior complete just because it compiles; run its
focused check and review failures.
