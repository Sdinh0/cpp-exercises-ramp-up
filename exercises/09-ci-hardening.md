# CI hardening and developer workflow

Improve build feedback while keeping the documented host workflow repeatable.
Relevant project configuration is in `CMakeLists.txt` and
`.github/workflows/`.

## Warnings

Enable useful compiler warnings for the library and remove warnings introduced
by your changes.

- **Update:** `CMakeLists.txt` and affected `src/*.cpp` files
- **Verify:** build `sensor_core` with warnings enabled on the available
  compiler.

## Formatting

Add either a formatting check or a documented formatter command, then apply it
to changed C++ files.

- **Update:** formatter configuration or `README.md`; format the changed
  `src/*.cpp` and `tests/*.cpp` files
- **Verify:** run the formatter check or documented command.

## Static analysis

Run `clang-tidy` if installed. Otherwise, identify and document an equivalent
compiler analysis that is available in the environment.

- **Update:** `CMakeLists.txt` or `README.md` only if configuration or
  documentation is needed
- **Verify:** record the command and result; do not silently skip analysis.

## Sanitizers

Run an address- and undefined-behavior-sanitizer build where the compiler and
platform support it. Record platform limitations instead of silently skipping.

- **Update:** optional sanitizer build configuration or `README.md`
- **Verify:** build and run CTest with the supported sanitizers enabled.

## PR simulation

Make a focused branch/change, review its diff, and verify the checks required
by the project before treating the change as complete.

- **Update:** no specific source file; work through the normal branch and PR
  workflow
- **Verify:** inspect the diff and confirm the required local/CI checks pass.
