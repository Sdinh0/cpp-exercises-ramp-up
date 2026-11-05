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
    std::vector<Frame> feed(const std::uint8_t* bytes, std::size_t size);
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

enum class IoStatus {
    ok,
    timeout,
    error
};

enum class SensorStatus : std::uint8_t {
    ok,
    invalid_state,
    invalid_request,
    write_timeout,
    write_error,
    incomplete_write,
    read_timeout,
    read_error,
    no_progress,
    invalid_transfer_count,
    unexpected_response,
    sensor_rejected,
    self_test_failed,
    invalid_response
};

struct SampleResult {
    SensorStatus status{SensorStatus::invalid_response};
    std::int32_t value{};
};

struct IoResult {
    IoStatus status{IoStatus::error};
    std::size_t transferred{};
};

class SensorHal {
public:
    virtual ~SensorHal() = default;

    // On success, transferred equals size. The HAL handles lower-level partial
    // transfers internally; the caller submits the buffer only once.
    virtual IoResult write(const std::uint8_t* data,
                           std::size_t size,
                           std::uint32_t timeout_ms) = 0;

    // Reads may return any positive number of bytes up to capacity.
    virtual IoResult read(std::uint8_t* data,
                          std::size_t capacity,
                          std::uint32_t timeout_ms) = 0;
};

class SensorDevice {
public:
    virtual ~SensorDevice() = default;

    virtual SensorStatus hardware_reset() = 0;
    virtual SensorStatus configure() = 0;
    virtual SensorStatus self_test() = 0;
    virtual SensorStatus start() = 0;
    virtual SampleResult read_sample() = 0;
};

class SensorDriver final : public SensorDevice {
public:
    static constexpr std::uint8_t reset_command = 0x10;
    static constexpr std::uint8_t configure_command = 0x11;
    static constexpr std::uint8_t self_test_command = 0x12;
    static constexpr std::uint8_t start_command = 0x13;
    static constexpr std::uint8_t read_sample_command = 0x14;
    static constexpr std::uint8_t response_mask = 0x80;
    static constexpr std::uint8_t sample_rate_hz = 10;
    static constexpr std::uint8_t sample_range_g = 2;

    explicit SensorDriver(SensorHal& hal);

    SensorStatus hardware_reset() override;
    SensorStatus configure() override;
    SensorStatus self_test() override;
    SensorStatus start() override;
    SampleResult read_sample() override;

private:
    enum class State {
        unknown,
        reset,
        configured,
        self_tested,
        running
    };

    struct SensorResponse {
        std::uint8_t status{};
        std::vector<std::uint8_t> payload;
    };

    SensorStatus transact(std::uint8_t command,
                          const std::vector<std::uint8_t>& request_payload,
                          SensorResponse& response);

    SensorHal& hal_;
    StreamDecoder decoder_;
    State state_{State::unknown};
};

class SensorLifecycle {
public:
    explicit SensorLifecycle(SensorDevice& device);

    LifecycleState state() const;
    SensorStatus begin();
    void reset_state();

private:
    SensorDevice& device_;
    LifecycleState state_{LifecycleState::uninitialized};
};

}  // namespace sensor
