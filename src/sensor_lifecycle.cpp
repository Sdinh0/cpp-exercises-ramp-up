#include "sensor/sensor.hpp"

namespace sensor {

SensorLifecycle::SensorLifecycle(SensorDevice& device)
    : device_(device)
{
}

LifecycleState SensorLifecycle::state() const
{
    return state_;
}

SensorStatus SensorLifecycle::begin()
{
    SensorStatus status = SensorStatus::no_progress;
    if ( state_ != LifecycleState::uninitialized ) {
        return SensorStatus::invalid_state;
    }
    
    // Expose initialization before invoking device operations.
    state_ = LifecycleState::initializing;

    // Preserve the first device error and stop; later steps depend on earlier ones.
    status = device_.hardware_reset();
    if (status != SensorStatus::ok) {
        state_ = LifecycleState::fault;
        return status;
    }

    status = device_.configure();
    if (status != SensorStatus::ok) {
        state_ = LifecycleState::fault;
        return status;
    }

    status = device_.self_test();
    if (status != SensorStatus::ok) {
        state_ = LifecycleState::fault;
        return status;
    }

    status = device_.start();
    if (status != SensorStatus::ok) {
        state_ = LifecycleState::fault;
        return status;
    }

    state_ = LifecycleState::ready;
    return SensorStatus::ok;
}

void SensorLifecycle::reset_state()
{
    // This resets coordinator state only; begin() performs hardware reset.
    state_ = LifecycleState::uninitialized;
}

}  // namespace sensor
