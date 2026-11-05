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
    return SensorStatus::invalid_state;
}

void SensorLifecycle::reset_state()
{
    state_ = LifecycleState::uninitialized;
}

}  // namespace sensor
