#include "sensor/sensor.hpp"

namespace sensor {

SensorDriver::SensorDriver(SensorHal& hal)
    : hal_(hal)
{
}

SensorStatus SensorDriver::hardware_reset()
{
    return SensorStatus::invalid_state;
}

SensorStatus SensorDriver::configure()
{
    return SensorStatus::invalid_state;
}

SensorStatus SensorDriver::self_test()
{
    return SensorStatus::invalid_state;
}

SensorStatus SensorDriver::start()
{
    return SensorStatus::invalid_state;
}

SampleResult SensorDriver::read_sample()
{
    return {SensorStatus::invalid_state, 0};
}

}  // namespace sensor
