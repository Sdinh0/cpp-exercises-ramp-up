#include "sensor/sensor.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>

namespace sensor {

namespace {

constexpr std::uint32_t io_timeout_ms = 100;
constexpr std::uint32_t transaction_timeout_ms = 1000;

}  // namespace

SensorDriver::SensorDriver(SensorHal& hal)
    : hal_(hal)
{
}

enum class IoOperation { read, write };

SensorStatus check_io_result(
    const IoResult& result,
    std::size_t requested,
    IoOperation operation)
{
    // Validate the reported count before interpreting status or using the data.
    if (result.transferred > requested) {
        return SensorStatus::invalid_transfer_count;
    }

    if (result.status == IoStatus::timeout) {
        return operation == IoOperation::write
            ? SensorStatus::write_timeout
            : SensorStatus::read_timeout;
    }

    if (result.status == IoStatus::error) {
        return operation == IoOperation::write
            ? SensorStatus::write_error
            : SensorStatus::read_error;
    }

    if (result.transferred == 0) {
        return SensorStatus::no_progress;
    }

    // Writes are all-or-failure; short reads are expected and decoded incrementally.
    if (operation == IoOperation::write &&
        result.transferred < requested) {
        return SensorStatus::incomplete_write;
    }

    return SensorStatus::ok;
}

SensorStatus SensorDriver::hardware_reset()
{
    // A reset invalidates all previously established protocol state.
    state_ = State::unknown;

    SensorResponse sensor_response;
    SensorStatus status = transact(reset_command, {}, sensor_response);
    if (status != SensorStatus::ok) {
        return status;
    }

    state_ = State::reset;
    return SensorStatus::ok;
}

SensorStatus SensorDriver::configure()
{
    if (state_ != State::reset) {
        return SensorStatus::invalid_state;
    }

    const std::vector<std::uint8_t> request_payload = {sample_rate_hz, sample_range_g};
    SensorResponse sensor_response;

    SensorStatus status = transact(configure_command, request_payload, sensor_response);
    if (status != SensorStatus::ok) {
        state_ = State::unknown;
        return status;
    }

    state_ = State::configured;
    return SensorStatus::ok;
}

SensorStatus SensorDriver::self_test()
{
    if (state_ != State::configured) {
        return SensorStatus::invalid_state;
    }

    SensorResponse sensor_response;
    SensorStatus status = transact(self_test_command, {}, sensor_response);
    if (status != SensorStatus::ok) {
        state_ = State::unknown;
        return status;
    }
    if (sensor_response.payload.empty() || sensor_response.payload[0] == 0) {
        state_ = State::unknown;
        return SensorStatus::self_test_failed;
    }

    state_ = State::self_tested;
    return SensorStatus::ok;
}

SensorStatus SensorDriver::start()
{
    if (state_ != State::self_tested) {
        return SensorStatus::invalid_state;
    }

    SensorResponse sensor_response;
    SensorStatus status = transact(start_command, {}, sensor_response);
    if (status != SensorStatus::ok) {
        state_ = State::unknown;
        return status;
    }
    state_ = State::running;
    return SensorStatus::ok;
}

SampleResult SensorDriver::read_sample()
{
    if (state_ != State::running) {
        return SampleResult{SensorStatus::invalid_state, 0};
    }

    SensorResponse sensor_response;

    SensorStatus status = transact(read_sample_command, {}, sensor_response);
    if (status != SensorStatus::ok) {
        return SampleResult{status, 0};
    }
    if (sensor_response.payload.size() != 2) {
        return SampleResult{SensorStatus::invalid_response, 0};
    }

    const auto bits = static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(sensor_response.payload[0]) << 8) |
        static_cast<std::uint16_t>(sensor_response.payload[1]));

    // Convert the wire's 16-bit two's-complement value without relying on
    // implementation-defined conversion from an out-of-range unsigned value.
    std::int32_t sample = (bits & 0x8000U) != 0
        ? static_cast<std::int32_t>(bits) - 0x10000
        : static_cast<std::int32_t>(bits);

    return SampleResult{SensorStatus::ok, sample};
}

SensorStatus SensorDriver::transact(std::uint8_t command,
                  const std::vector<std::uint8_t>& request_payload,
                  SensorResponse& response)
{
    response.payload.clear();
    // Each transaction starts at a frame boundary; only its response is consumed.
    decoder_.reset();
    SensorStatus status;
    const auto request_bytes = Codec::encode({command, request_payload});

    if (request_bytes.empty()) {
        return SensorStatus::invalid_request;
    }

    // The HAL handles lower-level partial transmission; the driver submits once.
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(transaction_timeout_ms);

    IoResult write_result = hal_.write(request_bytes.data(), request_bytes.size(), io_timeout_ms);

    status = check_io_result(write_result, request_bytes.size(), IoOperation::write);
    if (status != SensorStatus::ok) {
        return status;
    }

    // A read may contain only part of a frame, so preserve parser state between reads.
    std::vector<std::uint8_t> buffer(Codec::max_payload_size + 4);
    while (std::chrono::steady_clock::now() < deadline) {
        const auto remaining =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                deadline - std::chrono::steady_clock::now());

        if (remaining.count() <= 0) {
            return SensorStatus::read_timeout;
        }

        const auto read_timeout = std::min(static_cast<std::uint32_t>(remaining.count()), io_timeout_ms);
        IoResult read_result = hal_.read(buffer.data(), buffer.size(), read_timeout);

        status = check_io_result(read_result, buffer.size(), IoOperation::read);
        if (status != SensorStatus::ok) {
            return status;
        }

        const auto frames = decoder_.feed(buffer.data(), read_result.transferred);
        for (const auto& frame : frames) {
            if (frame.command !=
                    static_cast<std::uint8_t>(command | response_mask)) {
                return SensorStatus::unexpected_response;
            }
            if (frame.payload.empty()) {
                return SensorStatus::invalid_response;
            }

            // The first response payload byte is status; remaining bytes belong
            // to the command-specific response (for example self-test or sample data).
            response.payload.assign(frame.payload.begin() + 1, frame.payload.end());
            response.status = frame.payload[0];
            return response.status == 0 ? SensorStatus::ok : SensorStatus::sensor_rejected; 
        }
    }

    return SensorStatus::read_timeout;
}

}  // namespace sensor
