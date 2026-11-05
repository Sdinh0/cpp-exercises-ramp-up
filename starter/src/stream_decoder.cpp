#include "sensor/sensor.hpp"

namespace sensor {

std::vector<Frame> StreamDecoder::feed(std::uint8_t)
{
    return {};
}

std::vector<Frame> StreamDecoder::feed(const std::uint8_t*, std::size_t)
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

}  // namespace sensor
