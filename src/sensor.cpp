#include "sensor/sensor.hpp"

namespace sensor {

std::uint8_t Codec::crc8(const std::vector<std::uint8_t>& bytes)
{
    const std::uint8_t polynomial = 0x07;
    std::uint8_t crc = 0x00;
    for (const auto value : bytes) {
        crc ^= value;
        for (std::size_t i = 0; i < 8; ++i) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

std::vector<std::uint8_t> Codec::encode(const Frame& frame)
{
    std::vector<std::uint8_t> encoded;

    if (frame.payload.size() > Codec::max_payload_size) {
        return {};
    }
    
    encoded.push_back(Codec::sync_byte);
    encoded.push_back(frame.command);
    encoded.push_back(static_cast<std::uint8_t>(frame.payload.size()));
    encoded.insert(encoded.end(), frame.payload.begin(), frame.payload.end());
    encoded.push_back(crc8(std::vector<std::uint8_t>(encoded.begin() + 1, encoded.end())));
    return encoded;
}

DecodeResult Codec::decode(const std::vector<std::uint8_t>& bytes)
{
    DecodeResult result;

    if (bytes.size() < 4) {
        result.ok = false;
        result.error = ParseError::frame_too_short;
        return result;
    }
    else if (bytes[0] != Codec::sync_byte) {
        result.ok = false;
        result.error = ParseError::bad_sync;
        return result;
    }

    result.frame.command = bytes[1];
    const std::uint8_t payload_length = bytes[2];

    if (bytes.size() != static_cast<std::size_t>(4 + payload_length) || payload_length > Codec::max_payload_size) {
        result.ok = false;
        result.error = ParseError::bad_length;
        return result;
    }

    result.frame.payload.assign(bytes.begin() + 3, bytes.begin() + 3 + payload_length);

    if (bytes.back() != crc8(std::vector<std::uint8_t>(bytes.begin() + 1, bytes.end() - 1))) {
        result.ok = false;
        result.error = ParseError::bad_checksum;
        return result;
    }
    else {
        result.ok = true;
        result.error = ParseError::none;
    }

    return result;
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
