#include "sensor/sensor.hpp"
#include <algorithm>

namespace sensor {

std::vector<Frame> StreamDecoder::feed(std::uint8_t byte)
{
    std::vector<Frame> frames;
    buffer_.push_back(byte);

    while (buffer_.size() >= 3) {
        const auto payload_length = static_cast<std::size_t>(buffer_[2]);
        const auto expected_frame_size = 3 + payload_length + 1; /* sync, command, payload length + payload + checksum */

        if (buffer_[0] != Codec::sync_byte) {
            // Invalid sync byte, discard bytes until the next sync byte
            const auto sync_byte_pos = std::find(buffer_.begin(), buffer_.end(), Codec::sync_byte);
            if (sync_byte_pos == buffer_.end()) {
                reset();
                break; // No sync byte found, clear the buffer and wait for more data
            }
            else {
                buffer_.erase(buffer_.begin(), sync_byte_pos);
            }
            continue; // Try to process the buffer again after discarding bytes
        }

        if (buffer_.size() == static_cast<std::size_t>(3) && payload_length > Codec::max_payload_size) {
            // Invalid payload length, reset buffer
            reset();
        }
        else if (buffer_.size() >= static_cast<std::size_t>(expected_frame_size)) { /* sync, command, payload length + payload + checksum */
            // We have a complete frame
            const std::vector<std::uint8_t> frame_bytes(buffer_.begin(), buffer_.begin() + expected_frame_size);
            const auto result = Codec::decode(frame_bytes);
            if (result.ok) {
                const auto sync_byte_pos = std::find(buffer_.begin() + expected_frame_size, buffer_.end(), Codec::sync_byte); 
                buffer_.erase(buffer_.begin(), sync_byte_pos); // Remove the processed frame and trailing bytes from the buffer
                frames.push_back(result.frame);
            } else {
                buffer_.erase(buffer_.begin(), buffer_.begin() + 1); // Discard the first byte and try again
            }
        }
        else {
            // Not enough data for a complete frame yet
            break;
        }
    }

    /* Process the buffer and return any complete frames */
    return frames;
}

void StreamDecoder::reset()
{
    buffer_.clear();
}

std::size_t StreamDecoder::buffered_size() const
{
    return buffer_.size();
}

}  // namespace sensor
