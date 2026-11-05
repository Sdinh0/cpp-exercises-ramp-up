# Sample processing

Implement the rolling sample buffer in
[`src/sample_window.cpp`](../src/sample_window.cpp). Its API is
`SampleWindow` in [`include/sensor/sensor.hpp`](../include/sensor/sensor.hpp).

## Empty-window behavior

When no samples are stored, `average()` must return an empty `std::optional`
rather than a numeric value.

- **Update:** `src/sample_window.cpp`
- **Verify:** `SampleWindow` tests in `tests/sample_window_tests.cpp`

## Window metadata

Report the configured capacity, number of currently stored samples, and
whether the window has reached capacity.

- **Update:** `src/sample_window.cpp`
- **Verify:** `SampleWindow` tests in `tests/sample_window_tests.cpp`

## Sample average

Calculate the integer average of one or more stored samples. Integer division
must truncate toward zero.

- **Update:** `src/sample_window.cpp`
- **Verify:** `SampleWindow` tests in `tests/sample_window_tests.cpp`

## Rolling behavior

When the window is full, adding a new sample must discard the oldest sample and
retain the newest `capacity` samples.

- **Update:** `src/sample_window.cpp`
- **Verify:** `SampleWindow` tests in `tests/sample_window_tests.cpp`

## Signed values and reuse

Correctly average negative samples without overflow, clear stored samples,
handle zero capacity without storing data, and allow adding samples after
clearing.

- **Update:** `src/sample_window.cpp`
- **Verify:** `SampleWindow` tests in `tests/sample_window_tests.cpp`
