#include "sensor/sensor.hpp"

namespace sensor {

LifecycleState SensorLifecycle::state() const
{
    return state_;
}

void SensorLifecycle::begin()
{
}

void SensorLifecycle::initialization_succeeded()
{
}

void SensorLifecycle::fail()
{
}

void SensorLifecycle::receive_sample()
{
}

void SensorLifecycle::timeout()
{
}

void SensorLifecycle::reset()
{
    state_ = LifecycleState::uninitialized;
}

}  // namespace sensor
