# Sensor C++ ramp-up exercises

This is a C++17 sensor project for the topic-based learning path in
[`EXERCISES.md`](EXERCISES.md), with detailed assignments in `exercises/`.
The active implementation in `src/` is buildable and tested. The intentionally
incomplete implementations in `starter/src/` can be used to restart the
component exercises.

## Prerequisites

- CMake 3.26 or newer
- A C++17 compiler (Visual Studio 2019+, GCC, or Clang)
- Git and internet access on the first configure, so CMake can fetch the pinned
  GoogleTest release

GoogleTest is fetched during CMake configuration and built as part of this
project into the local `build` directory; no separate or global GoogleTest
installation is required. The project has no Python runtime dependencies. CI
uses Python only to install the pinned clang-format tool.

The compiler is a system toolchain, but generated files and fetched project
dependencies stay under `build`. The provided `.gitignore` excludes these local
environment files from source control.

## Build and run tests on Windows

The Windows CI job uses the MSYS2 CLANG64 toolchain. Install the compiler,
sanitizer runtime, CMake, and Ninja from an MSYS2 CLANG64 shell:

```sh
pacman -S mingw-w64-clang-x86_64-clang mingw-w64-clang-x86_64-compiler-rt \
  mingw-w64-clang-x86_64-cmake mingw-w64-clang-x86_64-ninja
```

Then run these commands from the repository root in PowerShell. The first line
makes the MSYS2 tools available in the current terminal:

```powershell
$env:Path = "C:\msys64\usr\bin;C:\msys64\clang64\bin;$env:Path"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_CXX_COMPILER=clang++ `
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" `
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build
ctest --test-dir build -C Debug --output-on-failure --no-tests=error
```

The project fetches GoogleTest into `build` at configure time.

Run a focused behavior group by filtering on its GoogleTest suite name. For
example:

```powershell
ctest --test-dir build -R Crc --output-on-failure
ctest --test-dir build -R CodecBoundaries --output-on-failure
```

Tests are organized into descriptive groups and files under `tests/`; each
test remains independently discoverable by CTest. Use
`ctest --test-dir build -N` to list the available checks. When restarting an
exercise from `starter/`, run the relevant group as you implement the behavior,
then run the full suite. Do not weaken or remove the provided tests to get a
green result.

## Build and run on Linux/macOS

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure --no-tests=error
```

## Project map

- `include/sensor/sensor.hpp` — shared public contract, component APIs, and
  the buffered byte-transport `SensorHal`, `SensorDevice`, and `SensorDriver`
  contracts, including typed `SensorStatus` results and `SampleResult`
- `src/codec.cpp` — frame CRC, encoding, and decoding implementation
- `src/stream_decoder.cpp` — incremental stream parser for byte and buffer
  input
- `src/sample_window.cpp` — rolling sample window implementation
- `src/sensor_driver.cpp` — sensor startup and sample protocol over the
  buffered byte HAL, with distinct transport, protocol, and device outcomes
- `src/sensor_lifecycle.cpp` — startup orchestration through `SensorDevice`,
  returning the first failing `SensorStatus`
- `starter/src/` — starter implementations matching the active component
  split; copy these over `src/` to restore the exercise starting point
- `tests/` — behavior-grouped GoogleTest files, including a simulated transport
  and end-to-end sensor stack check; see the source/test map in
  `EXERCISES.md` to find the test group for each component
- `EXERCISES.md` — topic index and source/test map
- `exercises/` — topic guides with implementation tasks and focused checks
- `CMakeLists.txt` — library source list, GoogleTest, and CTest setup

## Hardware boundary and error policy

The codec, stream decoder, sample window, sensor driver, and lifecycle
coordinator are hardware independent. A target-specific `SensorHal` adapter
owns UART/SPI/I2C or other device I/O; no board-specific adapter is included in
this host project. The simulated HAL is used by host tests, which do not
require a board or target toolchain.

The HAL transports bytes, not protocol frames. Each driver write submits one
complete encoded request. A successful write reports the full byte count; the
HAL handles any lower-level partial transfers internally. Reads can return
short chunks, which the driver feeds to `StreamDecoder`. The driver rejects
transport errors, invalid transfer counts, and zero-progress reads.

The stream decoder discards noise and malformed frames while recovering. A
valid frame for a different command returns `unexpected_response`; if no valid
response arrives before the transaction deadline, the driver returns
`read_timeout`. Parsing does not directly change lifecycle state. If a
startup operation fails because of a transport or response error,
`SensorLifecycle` receives that status, stops startup, and enters `fault`.

The stream decoder's partial-frame buffer is bounded by the maximum frame size
(`Codec::max_payload_size + 4`). The implementation uses `std::vector` for
frame payloads, decoded frames, and sample storage. A target that must avoid
the general heap could provide a bounded static allocation pool behind the
program's allocation operators without changing the public C++ types. This
requires explicit capacity and allocation-failure policies and affects all
allocations that use those operators. Fixed-capacity public containers are a
simpler alternative but would change the current API.

## Quality checks

Both CI jobs run the clang-format 18.1.8 formatting check, Cppcheck 2.x static
analysis, and the complete CTest suite. Both also run AddressSanitizer and
UndefinedBehaviorSanitizer: GCC on Linux and Clang on Windows. The standard
compiler warnings remain enabled for `sensor_core`.

To run the formatting check locally from a POSIX-compatible shell:

```sh
python -m pip install clang-format==18.1.8
clang-format --dry-run --Werror include/sensor/sensor.hpp src/*.cpp tests/*.cpp
```

The Cppcheck command used by CI can be run from the project root:

```sh
cppcheck --enable=warning,performance,portability \
  --suppress=normalCheckLevelMaxBranches --error-exitcode=1 --std=c++17 \
  -I include src/*.cpp
```

The Linux CI sanitizer build can also be run locally with:

```sh
cmake -S . -B build/quality -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build/quality
ctest --test-dir build/quality -C Debug --output-on-failure --no-tests=error
```

## TDD loop

1. Read the corresponding exercise linked from `EXERCISES.md` and the API
   under test.
2. Run the relevant focused CTest group and inspect the assertion failure.
3. Implement the smallest change that satisfies the stated behavior.
4. Re-run the focused check, then run the full suite when practical.
5. Review the diff, warnings, and any behavior affected by the change.

When using the incomplete starter implementations, behavioral tests are
expected to fail until the corresponding components are implemented. A
**build failure** is not expected; resolve setup or compiler errors separately
from behavioral test failures.
