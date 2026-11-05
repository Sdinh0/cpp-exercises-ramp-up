#include "sensor/sensor.hpp"
#include <algorithm>

namespace sensor {

std::vector<Frame> StreamDecoder::feed(std::uint8_t byte)
{
    std::vector<Frame> frames;
    buffer_.push_back(byte);

    while (buffer_.size() >= 3) {
        const auto payload_length = static_cast<std::size_t>(buffer_[2]);
        const auto expected_frame_size = 3 + payload_length + 1; // Header, payload, and checksum.

        if (buffer_[0] != Codec::sync_byte) {
            // Discard noise before the next possible frame boundary.
            const auto sync_byte_pos = std::find(buffer_.begin(), buffer_.end(), Codec::sync_byte);
            buffer_.erase(buffer_.begin(), sync_byte_pos);
        }
        else if (buffer_.size() == static_cast<std::size_t>(3) && payload_length > Codec::max_payload_size) {
            // An impossible length makes this candidate unusable.
            reset();
        }
        else if (buffer_.size() >= static_cast<std::size_t>(expected_frame_size)) { 
            // Decode only the candidate frame; trailing bytes remain stream input.
            const std::vector<std::uint8_t> frame_bytes(buffer_.begin(), buffer_.begin() + expected_frame_size);
            const auto result = Codec::decode(frame_bytes);
            if (result.ok) {
                // Keep a later sync byte so a following frame can be parsed.
                const auto sync_byte_pos = std::find(buffer_.begin() + expected_frame_size, buffer_.end(), Codec::sync_byte); 
                buffer_.erase(buffer_.begin(), sync_byte_pos);
                frames.push_back(result.frame);
            } else {
                // Retry from the next byte in case another sync is embedded in the candidate.
                buffer_.erase(buffer_.begin(), buffer_.begin() + 1); 
            }
        }
        else {
            // Not enough data for a complete frame yet
            break;
        }
    }

    // Return every complete frame recovered from this byte.
    return frames;
}

std::vector<Frame> StreamDecoder::feed(const std::uint8_t* bytes, std::size_t size)
{
    std::vector<Frame> frames;
    // Reuse byte-wise parsing so chunk and single-byte input behave identically.
    for (std::size_t i = 0; i < size; ++i) {
        const auto emitted = feed(bytes[i]);
        frames.insert(frames.end(), emitted.begin(), emitted.end());
    }
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
