# Exercises 26-30: Lifecycle state and transitions

Implement `SensorLifecycle` in
[`src/sensor_lifecycle.cpp`](../src/sensor_lifecycle.cpp). The states and
public transition methods are declared in
[`include/sensor/sensor.hpp`](../include/sensor/sensor.hpp).

## 26. Lifecycle start

Preserve the initial `uninitialized` state and make `begin()` start an
initialization attempt by entering `initializing`.

- **Update:** `src/sensor_lifecycle.cpp`
- **Verify:** `SensorLifecycle` tests in `tests/sensor_lifecycle_tests.cpp`

## 27. Initialization success

Enter `ready` only when initialization succeeds during an active
`initializing` attempt.

- **Update:** `src/sensor_lifecycle.cpp`
- **Verify:** `SensorLifecycle` tests in `tests/sensor_lifecycle_tests.cpp`

## 28. Failure paths

Make explicit failure and timeout enter the `fault` state, including timeout
after the sensor has reached `ready`.

- **Update:** `src/sensor_lifecycle.cpp`
- **Verify:** `SensorLifecycle` tests in `tests/sensor_lifecycle_tests.cpp`

## 29. Recovery

Make `reset()` return the state to `uninitialized`. A subsequent `begin()`
must start a fresh initialization attempt.

- **Update:** `src/sensor_lifecycle.cpp`
- **Verify:** `SensorLifecycle` tests in `tests/sensor_lifecycle_tests.cpp`

## 30. State invariants

A received sample must not promote an uninitialized, initializing, or faulted
sensor to `ready`. A sample received while ready must leave it ready.

- **Update:** `src/sensor_lifecycle.cpp`
- **Verify:** `SensorLifecycle` tests in `tests/sensor_lifecycle_tests.cpp`
