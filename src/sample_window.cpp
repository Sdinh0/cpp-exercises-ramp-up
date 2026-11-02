#include "sensor/sensor.hpp"

namespace sensor {

SampleWindow::SampleWindow(std::size_t capacity)
    : capacity_(capacity)
{
}

void SampleWindow::add(std::int32_t)
{
}

std::optional<std::int32_t> SampleWindow::average() const
{
    return std::nullopt;
}

std::size_t SampleWindow::size() const
{
    return samples_.size();
}

std::size_t SampleWindow::capacity() const
{
    return capacity_;
}

bool SampleWindow::full() const
{
    return false;
}

void SampleWindow::clear()
{
    samples_.clear();
}

}  // namespace sensor
