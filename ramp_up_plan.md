# C++ and Tooling Ramp-Up Plan

## Starting point and goal

This plan is tailored to the skill levels in `Skill_matrix.pdf` and the goals in
`start.md`:

- **Build on:** C (level 3), embedded development and microcontrollers (level 3),
  Make (level 3), Jenkins and GitHub Actions (level 2), and Git (level 2).
- **Develop:** modern C++ (currently level 2), CMake (level 1), automated
  testing and test frameworks, TDD, and reliable CI practices.

The aim is to become productive designing, testing, building, and integrating
C++ components in an embedded-oriented project—not to relearn programming or
general CI concepts from scratch.

## Suggested cadence

Allow **10 weeks, 5 days per week, 4 hours per day** (about 200 focused hours).
Each workday is a small defect-fix or improvement task on the same capstone.
Start by reading that day's exercise, run its failing test, implement the
behavior, then run the test again and review the change. The companion
[`exercises/README.md`](./exercises/README.md) explains the starter project and
test commands; [`exercises/EXERCISES.md`](./exercises/EXERCISES.md) contains the
50 daily assignments.

### Capstone project

Create a small portable C++ library for an embedded subsystem, such as a sensor
protocol parser and device state machine. Keep hardware access behind a narrow
interface so the core can be built and tested on a desktop without a board.
Use a real board or target toolchain only where available; host tests remain the
fast, repeatable feedback loop.

## Weekly themes

1. **Modern C++ and value semantics** — convert and validate sensor data with
   explicit types, lifetimes, and error handling.
2. **API design and ownership** — make the codec, stream decoder, sample
   window, and sensor lifecycle small, testable components.
3. **CMake foundations** — understand the provided targets, build types,
   dependency wiring, and CTest discovery by changing and validating the build.
4. **GoogleTest, CTest, and TDD** — implement the daily behavior against
   focused failing tests, covering edge cases and deterministic host tests.
5. **Testability and integration** — exercise streaming and lifecycle behavior
   through fakes, then test cross-component scenarios.
6. **Build quality** — add warnings, formatting, static analysis, and
   sanitizers where supported.
7. **CI/CD** — make local build and test commands run in GitHub Actions.
8. **Embedded constraints** — review allocation, exceptions, and target
   boundaries; keep host validation independent of hardware.
9. **CI hardening** — check failure behavior, diagnostics, and pull-request
   workflow.
10. **Consolidation** — rebuild from a clean checkout, review the capstone, and
    reassess the skill matrix.

## Definition of done

By the end of the plan, the project should:

- Build from a clean checkout with CMake and documented commands.
- Use modern C++ with clear ownership, lifetimes, and error behavior.
- Run deterministic unit and integration tests through CTest.
- Include at least one feature developed using TDD.
- Run build and test checks automatically in CI.
- Keep hardware-specific code isolated from the host-testable core.

## Suggested weekly routine

## Four-hour workday

Use the same workday rhythm for each of the 50 exercises:

1. **30 min — Orient:** read the task, relevant API, and failing test; write
   down the expected behavior and one edge case.
2. **45 min — Learn:** study only the C++/CMake/testing concept needed for the
   task, using a reliable reference.
3. **2 hours — Implement:** make the smallest change that satisfies the
   requirement; keep hardware-independent code on the host.
4. **30 min — Verify:** run the focused exercise test, then the full suite when
   it is practical; inspect compiler warnings and test output.
5. **15 min — Review:** summarize the defect/improvement, the root cause, and
   one follow-up question in your own notes or commit message.

The time boxes are guidelines; preserve the failing-test → implementation →
passing-test loop rather than rushing to finish a task.
