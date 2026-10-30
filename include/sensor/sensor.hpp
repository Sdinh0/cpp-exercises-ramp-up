#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace sensor {

struct Frame {
    std::uint8_t command{};
    std::vector<std::uint8_t> payload;
};

enum class ParseError {
    none,
    frame_too_short,
    bad_sync,
    bad_length,
    bad_checksum
};

struct DecodeResult {
    bool ok{};
    Frame frame;
    ParseError error{ParseError::frame_too_short};
};

class Codec {
public:
    static constexpr std::uint8_t sync_byte = 0xA5;
    static constexpr std::size_t max_payload_size = 16;

    static std::uint8_t crc8(const std::vector<std::uint8_t>& bytes);
    static std::vector<std::uint8_t> encode(const Frame& frame);
    static DecodeResult decode(const std::vector<std::uint8_t>& bytes);
};

class StreamDecoder {
public:
    std::vector<Frame> feed(std::uint8_t byte);
    void reset();
    std::size_t buffered_size() const;

private:
    std::vector<std::uint8_t> buffer_;
};

class SampleWindow {
public:
    explicit SampleWindow(std::size_t capacity);

    void add(std::int32_t sample);
    std::optional<std::int32_t> average() const;
    std::size_t size() const;
    std::size_t capacity() const;
    bool full() const;
    void clear();

private:
    std::size_t capacity_{};
    std::vector<std::int32_t> samples_;
};

enum class LifecycleState {
    uninitialized,
    initializing,
    ready,
    fault
};

class SensorLifecycle {
public:
    LifecycleState state() const;
    void begin();
    void initialization_succeeded();
    void fail();
    void receive_sample();
    void timeout();
    void reset();

private:
    LifecycleState state_{LifecycleState::uninitialized};
};

}  // namespace sensor
