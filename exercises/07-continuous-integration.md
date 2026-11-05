# Continuous integration

These exercises focus on build and test automation rather than one component.
The main build configuration is [`CMakeLists.txt`](../CMakeLists.txt); add
automation under `.github/workflows/` when required.

## CI workflow

Add a GitHub Actions workflow that configures the CMake project, builds it, and
runs CTest on both pushes and pull requests.

- **Update:** `.github/workflows/ci.yml` and, if needed, `CMakeLists.txt`
- **Verify:** inspect the workflow and run the same configure/build/test
  commands locally.

## Failure visibility

Temporarily make an assertion fail and confirm the test command and CI job
report failure with a nonzero result. Restore the test immediately afterward;
do not commit the temporary defect.

- **Update:** temporary local-only change to a test under `tests/`
- **Verify:** run the focused test and confirm it fails, then restore and
  confirm it passes.

## Runner matrix

Run the workflow on Windows and at least one Linux runner. If a real project
constraint prevents that, record the constraint and its impact.

- **Update:** `.github/workflows/ci.yml`
- **Verify:** check both runner configurations in the workflow or document the
  constraint.

## Repeatability

Using only the commands documented in `README.md`, configure a fresh build
directory, compile the project, and run all tests.

- **Update:** `README.md` only if its commands are incomplete or inaccurate
- **Verify:** configure/build/CTest from a new build directory.

## Regression task

Introduce a temporary local defect and confirm CI catches it. Revert only that
temporary edit afterward; do not discard unrelated work.

- **Update:** a temporary local source or test change
- **Verify:** the affected check fails with the defect and passes after
  restoration.
