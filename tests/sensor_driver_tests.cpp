#include "sensor/sensor.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <thread>
#include <vector>

namespace {

using sensor::Codec;
using sensor::Frame;
using sensor::IoResult;
using sensor::IoStatus;
using sensor::LifecycleState;
using sensor::SampleWindow;
using sensor::SensorStatus;
using sensor::SensorDriver;
using sensor::SensorHal;
using sensor::SensorLifecycle;
using sensor::StreamDecoder;

struct Request {
    std::uint8_t command;
    std::vector<std::uint8_t> payload;
};

class SimulatedSensor final : public SensorHal {
public:
    IoResult write(const std::uint8_t* data,
                   std::size_t size,
                   std::uint32_t) override
    {
        ++write_calls;
        if (fail_write) {
            return {IoStatus::error, 0};
        }

        if (timeout_write) {
            return {IoStatus::timeout, 0};
        }

        if (zero_progress_write) {
            return {IoStatus::ok, 0};
        }

        if (incomplete_write) {
            return {IoStatus::ok, size - 1};
        }

        if (invalid_write_count) {
            return {IoStatus::ok, size + 1};
        }

        write_sizes.push_back(size);
        for (std::size_t index = 0; index < size; ++index) {
            const auto byte = data[index];
            for (const auto& frame : request_decoder.feed(byte)) {
                handle(frame);
            }
        }
        return {IoStatus::ok, size};
    }

    IoResult read(std::uint8_t* data,
                  std::size_t capacity,
                  std::uint32_t timeout_ms) override
    {
        ++read_calls;
        if (fail_read) {
            return {IoStatus::error, 0};
        }
        if (noise_until_deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
            data[0] = 0;
            return {IoStatus::ok, 1};
        }
        if (response_bytes.empty()) {
            return {IoStatus::timeout, 0};
        }
        if (zero_progress_read) {
            return {IoStatus::ok, 0};
        }

        if (invalid_read_count) {
            return {IoStatus::ok, capacity + 1};
        }

        const auto count =
            std::min({capacity, max_read_size, response_bytes.size()});
        for (std::size_t index = 0; index < count; ++index) {
            data[index] = response_bytes.front();
            response_bytes.pop_front();
        }
        return {IoStatus::ok, count};
    }

    void queue_sample(std::int16_t sample)
    {
        samples.push_back(sample);
    }

    void handle(const Frame& request)
    {
        requests.push_back({request.command, request.payload});

        std::vector<std::uint8_t> payload{0};
        switch (request.command) {
        case SensorDriver::reset_command:
            reset_done = true;
            configured = false;
            self_tested = false;
            running = false;
            break;
        case SensorDriver::configure_command:
            if (!reset_done || request.payload.size() != 2 ||
                !accept_configuration ||
                request.payload[0] != SensorDriver::sample_rate_hz ||
                request.payload[1] != SensorDriver::sample_range_g) {
                payload[0] = 1;
            } else {
                configured = true;
            }
            break;
        case SensorDriver::self_test_command:
            if (!configured) {
                payload[0] = 1;
                payload.push_back(0);
            } else {
                self_tested = true;
                payload.push_back(self_test_pass ? 1 : 0);
            }
            break;
        case SensorDriver::start_command:
            if (!self_tested) {
                payload[0] = 1;
            } else {
                running = true;
            }
            break;
        case SensorDriver::read_sample_command:
            if (!running || samples.empty()) {
                payload[0] = 1;
            } else {
                const auto sample = samples.front();
                samples.pop_front();
                const auto bits = static_cast<std::uint16_t>(sample);
                payload.push_back(static_cast<std::uint8_t>(bits >> 8));
                payload.push_back(static_cast<std::uint8_t>(bits & 0xFF));
            }
            break;
        default:
            payload[0] = 1;
            break;
        }

        Frame response{
            static_cast<std::uint8_t>(request.command | SensorDriver::response_mask),
            payload
        };
        if (empty_response_payload) {
            response.payload.clear();
            empty_response_payload = false;
        }
        if (drop_next_response) {
            drop_next_response = false;
            return;
        }
        if (response_command.has_value()) {
            response.command = *response_command;
        }
        auto encoded = Codec::encode(response);
        if (noise_before_response) {
            response_bytes.push_back(0x00);
        }
        if (corrupt_next_response && !encoded.empty()) {
            encoded.back() ^= 0x01;
            corrupt_next_response = false;
        }
        for (const auto byte : encoded) {
            response_bytes.push_back(byte);
        }
    }

    StreamDecoder request_decoder;
    std::deque<std::uint8_t> response_bytes;
    std::deque<std::int16_t> samples;
    std::vector<Request> requests;
    std::vector<std::size_t> write_sizes;
    std::optional<std::uint8_t> response_command;
    std::size_t write_calls{};
    std::size_t read_calls{};
    std::size_t max_read_size{static_cast<std::size_t>(-1)};
    bool reset_done{};
    bool configured{};
    bool self_tested{};
    bool running{};
    bool self_test_pass{true};
    bool accept_configuration{true};
    bool noise_before_response{};
    bool fail_write{};
    bool timeout_write{};
    bool incomplete_write{};
    bool invalid_write_count{};
    bool fail_read{};
    bool invalid_read_count{};
    bool zero_progress_write{};
    bool zero_progress_read{};
    bool empty_response_payload{};
    bool drop_next_response{};
    bool corrupt_next_response{};
    bool noise_until_deadline{};
};

SensorStatus initialize(SensorDriver& driver)
{
    auto status = driver.hardware_reset();
    if (status != SensorStatus::ok) {
        return status;
    }
    status = driver.configure();
    if (status != SensorStatus::ok) {
        return status;
    }
    status = driver.self_test();
    if (status != SensorStatus::ok) {
        return status;
    }
    return driver.start();
}

TEST(SensorDriver, EncodesOrderedStartupCommandsAndParsesResponses)
{
    SimulatedSensor simulated_sensor;
    SensorDriver driver(simulated_sensor);

    EXPECT_EQ(initialize(driver), SensorStatus::ok);
    ASSERT_EQ(simulated_sensor.requests.size(), 4U);
    EXPECT_EQ(simulated_sensor.requests[0].command, SensorDriver::reset_command);
    EXPECT_EQ(simulated_sensor.requests[1].command,
              SensorDriver::configure_command);
    EXPECT_EQ(simulated_sensor.requests[1].payload,
              (std::vector<std::uint8_t>{SensorDriver::sample_rate_hz,
                                         SensorDriver::sample_range_g}));
    EXPECT_EQ(simulated_sensor.requests[2].command,
              SensorDriver::self_test_command);
    EXPECT_EQ(simulated_sensor.requests[3].command, SensorDriver::start_command);
    EXPECT_TRUE(simulated_sensor.running);
}

TEST(SensorDriver, RejectsOutOfOrderCommandsWithoutSendingFrames)
{
    SimulatedSensor simulated_sensor;
    SensorDriver driver(simulated_sensor);

    EXPECT_EQ(driver.configure(), SensorStatus::invalid_state);
    EXPECT_EQ(driver.self_test(), SensorStatus::invalid_state);
    EXPECT_EQ(driver.start(), SensorStatus::invalid_state);
    EXPECT_EQ(driver.read_sample().status, SensorStatus::invalid_state);
    EXPECT_TRUE(simulated_sensor.requests.empty());
}

TEST(SensorDriver, StopsInitializationWhenTheSensorRejectsConfiguration)
{
    SimulatedSensor simulated_sensor;
    simulated_sensor.accept_configuration = false;
    SensorDriver driver(simulated_sensor);
    ASSERT_EQ(driver.hardware_reset(), SensorStatus::ok);
    simulated_sensor.requests.clear();

    EXPECT_EQ(driver.configure(), SensorStatus::sensor_rejected);
    ASSERT_EQ(simulated_sensor.requests.size(), 1U);
    EXPECT_EQ(simulated_sensor.requests.front().command,
              SensorDriver::configure_command);
    EXPECT_EQ(driver.self_test(), SensorStatus::invalid_state);
    EXPECT_FALSE(simulated_sensor.running);
}

TEST(SensorDriver, SelfTestFailurePreventsStartingTheSensor)
{
    SimulatedSensor simulated_sensor;
    simulated_sensor.self_test_pass = false;
    SensorDriver driver(simulated_sensor);

    EXPECT_EQ(driver.hardware_reset(), SensorStatus::ok);
    EXPECT_EQ(driver.configure(), SensorStatus::ok);
    EXPECT_EQ(driver.self_test(), SensorStatus::self_test_failed);
    EXPECT_EQ(driver.start(), SensorStatus::invalid_state);
    EXPECT_FALSE(simulated_sensor.running);
}

TEST(SensorDriver, BadChecksumResponseIsRejected)
{
    SimulatedSensor simulated_sensor;
    simulated_sensor.corrupt_next_response = true;
    SensorDriver driver(simulated_sensor);

    EXPECT_NE(driver.hardware_reset(), SensorStatus::ok);
    ASSERT_EQ(simulated_sensor.requests.size(), 1U);
    EXPECT_EQ(simulated_sensor.requests.front().command,
              SensorDriver::reset_command);
    EXPECT_GT(simulated_sensor.read_calls, 0U);
}

TEST(SensorDriver, StreamDecoderRecoversFromLeadingTransportNoise)
{
    SimulatedSensor simulated_sensor;
    simulated_sensor.noise_before_response = true;
    SensorDriver driver(simulated_sensor);

    EXPECT_EQ(driver.hardware_reset(), SensorStatus::ok);
    ASSERT_EQ(simulated_sensor.requests.size(), 1U);
    EXPECT_EQ(simulated_sensor.requests.front().command,
              SensorDriver::reset_command);
}

TEST(SensorDriver, TransportErrorsAreReported)
{
    SimulatedSensor simulated_sensor;
    SensorDriver driver(simulated_sensor);

    simulated_sensor.fail_write = true;
    EXPECT_EQ(driver.hardware_reset(), SensorStatus::write_error);
    EXPECT_EQ(simulated_sensor.write_calls, 1U);
    EXPECT_EQ(simulated_sensor.read_calls, 0U);

    simulated_sensor.fail_write = false;
    simulated_sensor.fail_read = true;
    EXPECT_EQ(driver.hardware_reset(), SensorStatus::read_error);
    EXPECT_EQ(simulated_sensor.write_calls, 2U);
    EXPECT_EQ(simulated_sensor.read_calls, 1U);
}

TEST(SensorDriver, DistinguishesWriteTimeoutFromWriteError)
{
    SimulatedSensor simulated_sensor;
    SensorDriver driver(simulated_sensor);

    simulated_sensor.timeout_write = true;
    EXPECT_EQ(driver.hardware_reset(), SensorStatus::write_timeout);
}

TEST(SensorDriver, RejectsIncompleteWritesAndInvalidTransferCounts)
{
    {
        SimulatedSensor simulated_sensor;
        simulated_sensor.incomplete_write = true;
        SensorDriver driver(simulated_sensor);

        EXPECT_EQ(driver.hardware_reset(), SensorStatus::incomplete_write);
    }

    {
        SimulatedSensor simulated_sensor;
        simulated_sensor.invalid_write_count = true;
        SensorDriver driver(simulated_sensor);

        EXPECT_EQ(driver.hardware_reset(), SensorStatus::invalid_transfer_count);
    }

    {
        SimulatedSensor simulated_sensor;
        simulated_sensor.invalid_read_count = true;
        SensorDriver driver(simulated_sensor);

        EXPECT_EQ(driver.hardware_reset(), SensorStatus::invalid_transfer_count);
    }
}

TEST(SensorDriver, RejectsResponseWithoutStatusByte)
{
    SimulatedSensor simulated_sensor;
    simulated_sensor.empty_response_payload = true;
    SensorDriver driver(simulated_sensor);

    EXPECT_EQ(driver.hardware_reset(), SensorStatus::invalid_response);
}

TEST(SensorDriver, WritesEachCompleteRequestOnceAndParsesChunkedReads)
{
    SimulatedSensor simulated_sensor;
    simulated_sensor.max_read_size = 2;
    SensorDriver driver(simulated_sensor);

    EXPECT_EQ(initialize(driver), SensorStatus::ok);
    EXPECT_EQ(simulated_sensor.write_calls, 4U);
    EXPECT_EQ(simulated_sensor.write_sizes.size(), 4U);
    EXPECT_GT(simulated_sensor.read_calls, 4U);
    EXPECT_TRUE(simulated_sensor.running);
}

TEST(SensorDriver, ZeroProgressTransportResultsFailInsteadOfLooping)
{
    SimulatedSensor simulated_sensor;
    simulated_sensor.zero_progress_write = true;
    SensorDriver driver(simulated_sensor);

    EXPECT_EQ(driver.hardware_reset(), SensorStatus::no_progress);
    EXPECT_TRUE(simulated_sensor.requests.empty());

    simulated_sensor.zero_progress_write = false;
    simulated_sensor.zero_progress_read = true;
    EXPECT_EQ(driver.hardware_reset(), SensorStatus::no_progress);
}

TEST(SensorDriver, ReadTimeoutFailsTheTransaction)
{
    SimulatedSensor simulated_sensor;
    simulated_sensor.drop_next_response = true;
    SensorDriver driver(simulated_sensor);

    EXPECT_EQ(driver.hardware_reset(), SensorStatus::read_timeout);
    ASSERT_EQ(simulated_sensor.requests.size(), 1U);
    EXPECT_EQ(simulated_sensor.requests.front().command,
              SensorDriver::reset_command);
    EXPECT_GT(simulated_sensor.read_calls, 0U);
}

TEST(SensorDriver, ReadDeadlineExpiresAfterSuccessfulNoiseReads)
{
    SimulatedSensor simulated_sensor;
    simulated_sensor.noise_until_deadline = true;
    SensorDriver driver(simulated_sensor);

    EXPECT_EQ(driver.hardware_reset(), SensorStatus::read_timeout);
    EXPECT_GT(simulated_sensor.read_calls, 1U);
}

TEST(SensorDriver, IgnoresResponseForAnotherCommand)
{
    SimulatedSensor simulated_sensor;
    simulated_sensor.response_command = static_cast<std::uint8_t>(
        SensorDriver::configure_command | SensorDriver::response_mask);
    SensorDriver driver(simulated_sensor);

    EXPECT_EQ(driver.hardware_reset(), SensorStatus::unexpected_response);
    ASSERT_EQ(simulated_sensor.requests.size(), 1U);
    EXPECT_EQ(simulated_sensor.requests.front().command,
              SensorDriver::reset_command);
    EXPECT_GT(simulated_sensor.read_calls, 0U);
}

TEST(SensorDriver, ReadsSignedSamplesAndFeedsTheExistingSampleWindow)
{
    SimulatedSensor simulated_sensor;
    simulated_sensor.queue_sample(-12);
    simulated_sensor.queue_sample(20);
    SensorDriver driver(simulated_sensor);
    ASSERT_EQ(initialize(driver), SensorStatus::ok);

    SampleWindow window(2);
    const auto first = driver.read_sample();
    ASSERT_EQ(first.status, SensorStatus::ok);
    window.add(first.value);
    const auto second = driver.read_sample();
    ASSERT_EQ(second.status, SensorStatus::ok);
    window.add(second.value);

    ASSERT_TRUE(window.average().has_value());
    EXPECT_EQ(*window.average(), 4);
    EXPECT_EQ(simulated_sensor.requests.back().command,
              SensorDriver::read_sample_command);
}

TEST(SensorStackIntegration, StartsDriverAndFeedsDecodedSamplesToWindow)
{
    SimulatedSensor simulated_sensor;
    simulated_sensor.queue_sample(100);
    simulated_sensor.queue_sample(200);
    SensorDriver driver(simulated_sensor);
    SensorLifecycle lifecycle(driver);

    EXPECT_EQ(lifecycle.begin(), SensorStatus::ok);
    ASSERT_EQ(lifecycle.state(), LifecycleState::ready);

    SampleWindow window(2);
    const auto first = driver.read_sample();
    ASSERT_EQ(first.status, SensorStatus::ok);
    window.add(first.value);
    const auto second = driver.read_sample();
    ASSERT_EQ(second.status, SensorStatus::ok);
    window.add(second.value);

    ASSERT_TRUE(window.average().has_value());
    EXPECT_EQ(*window.average(), 150);
}

}  // namespace
