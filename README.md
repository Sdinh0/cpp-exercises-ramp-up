# Sensor C++ ramp-up exercises

This is a deliberately incomplete C++17 project for the 50 workday tasks in
[`EXERCISES.md`](EXERCISES.md). The tests are written first. The initial
implementation should compile, but the behavioral tests should fail until you
implement the corresponding exercises.

## Prerequisites

- CMake 3.20 or newer
- A C++17 compiler (Visual Studio 2019+, GCC, or Clang)
- Git and internet access on the first configure, so CMake can fetch the pinned
  GoogleTest release

GoogleTest is fetched during CMake configuration and built as part of this
project into the local `build` directory; no separate or global GoogleTest
installation is required. The exercise project contains no Python code or
Python package dependencies, so it does not need a Python virtual environment.
If you add Python-based tooling later, keep its virtual environment in this
project directory and do not install its packages globally.

The compiler is a system toolchain, but generated files and fetched project
dependencies stay under `build`. The provided `.gitignore` excludes these local
environment files from source control.

## Build and run tests on Windows

For the MSYS2 UCRT64 toolchain, run these commands from the root 
directory in PowerShell. The first line makes the installed CMake and compiler
available in the current terminal:

```powershell
$env:Path = "C:\msys64\usr\bin;C:\msys64\ucrt64\bin;$env:Path"
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

If CMake is not installed yet, install it in the MSYS2 UCRT64 environment with
`pacman -S mingw-w64-ucrt-x86_64-cmake`, then open a new PowerShell terminal.
The project fetches GoogleTest into `build` at configure time.

Run a focused behavior group by filtering on its GoogleTest suite name. For
example:

```powershell
ctest --test-dir build -R Crc --output-on-failure
ctest --test-dir build -R CodecBoundaries --output-on-failure
```

Tests are organized into descriptive groups and files under `tests/`; each
test remains independently discoverable by CTest. Use
`ctest --test-dir build -N` to list the available checks. At the start, running
the full suite is expected to report failures. As you complete a task, run the
relevant group, then the full suite. Do not weaken or remove the provided tests
to get a green result.

## Build and run on Linux/macOS

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

## Project map

- `include/sensor/sensor.hpp` — public contract to implement against
- `src/sensor.cpp` — active implementation file compiled by CMake
- `src/sensor_empty.cpp` — preserved snapshot of the original empty starter
  implementation; kept for reference and not compiled by CMake
- `tests/` — behavior-grouped GoogleTest files
- `EXERCISES.md` — defect/improvement task sequence and acceptance goals
- `CMakeLists.txt` — library, GoogleTest, and CTest setup

## TDD loop

1. Read the corresponding exercise in `EXERCISES.md` and the API under test.
2. Run the relevant focused CTest group and inspect the assertion failure.
3. Implement the smallest change that satisfies the stated behavior.
4. Re-run the focused check, then run the full suite when practical.
5. Review the diff, warnings, and any behavior affected by the change.

The suite is intentionally failing at the beginning. That is the red phase, not
a broken setup. A **build failure** is not expected; resolve setup or compiler
errors separately from behavioral test failures.
