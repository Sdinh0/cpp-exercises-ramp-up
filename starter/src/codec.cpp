#include "sensor/sensor.hpp"

namespace sensor {

std::uint8_t Codec::crc8(const std::vector<std::uint8_t>&)
{
    return 0;
}

std::vector<std::uint8_t> Codec::encode(const Frame&)
{
    return {};
}

DecodeResult Codec::decode(const std::vector<std::uint8_t>&)
{
    return DecodeResult{};
}

}  // namespace sensor
