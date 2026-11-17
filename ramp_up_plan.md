# C++ and Tooling Ramp-Up Plan

## Starting point and goal

This plan uses the portable C++17 sensor project in this repository to develop
modern C++, CMake, automated testing, TDD, and reliable CI practices. The
project includes a frame codec, stream decoder, sample window, sensor driver,
lifecycle coordinator, and host-side tests.

The goal is to design, test, build, and integrate embedded-oriented C++
components while keeping hardware-specific I/O separate from the
host-testable core.

## Suggested approach

Work through the topic guides at a pace that fits your goals. For each
behavior, read the contract and focused test, understand the expected failure,
implement the behavior, and review the result. The companion
[`README.md`](./README.md) explains the project and test commands;
[`EXERCISES.md`](./EXERCISES.md) gives the topic progression and source/test
map.

## Capstone project

Use the existing portable sensor library as the capstone. Its codec, stream
decoder, sample window, driver, and lifecycle operate independently of board
hardware. Hardware access belongs behind the byte-transport `SensorHal`
interface, allowing the core and simulated sensor to be built and tested on a
desktop. A real board or target toolchain is optional.

## Learning themes

1. **Modern C++ and value semantics** — convert and validate sensor data with
   explicit types, lifetimes, and error handling.
2. **API design and ownership** — make the codec, stream decoder, sample
   window, and sensor lifecycle small, testable components.
3. **CMake foundations** — understand the provided targets, build types,
   dependency wiring, and CTest discovery by changing and validating the build.
4. **GoogleTest, CTest, and TDD** — implement behavior against focused failing
   tests, covering edge cases and deterministic host tests.
5. **Testability and integration** — exercise the byte-transport HAL and
   simulated sensor through fakes; then layer lifecycle coordination over the
   driver and test the complete codec-to-sample path.
6. **Build quality** — add warnings, formatting, static analysis, and
   sanitizers where supported.
7. **CI/CD** — make local build and test commands run in GitHub Actions.
8. **Embedded constraints** — review allocation, exceptions, and target
   boundaries; keep host validation independent of hardware.
9. **CI hardening** — check failure behavior, diagnostics, and pull-request
   workflow.
10. **Consolidation** — rebuild from a clean checkout, review the capstone, and
    reflect on the concepts practiced.

## Definition of done

By the end of the plan, the project should:

- Build from a clean checkout with CMake and documented commands.
- Use modern C++ with clear ownership, lifetimes, and error behavior.
- Run deterministic unit and integration tests through CTest.
- Include at least one feature developed using TDD.
- Run build and test checks automatically in CI.
- Keep hardware-specific code isolated from the host-testable core.

## Working rhythm

For each topic, orient yourself with the public contract and focused tests,
implement in small steps, then run the relevant checks and review compiler
warnings and behavior. Preserve the failing-test → implementation →
passing-test loop; adjust the depth and pace to suit the topic.
