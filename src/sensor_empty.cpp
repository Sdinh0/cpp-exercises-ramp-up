#include "sensor/sensor.hpp"

namespace sensor {

std::uint8_t Codec::crc8(const std::vector<std::uint8_t>&)
{
    return 0;
}

std::vector<std::uint8_t> Codec::encode(const Frame& )
{
    return {};
}

DecodeResult Codec::decode(const std::vector<std::uint8_t>& )
{
    return DecodeResult{};
}

std::vector<Frame> StreamDecoder::feed(std::uint8_t)
{
    return {};
}

void StreamDecoder::reset()
{
}

std::size_t StreamDecoder::buffered_size() const
{
    return buffer_.size();
}

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
