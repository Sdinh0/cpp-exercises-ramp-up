#include "sensor/sensor.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace {

using sensor::LifecycleState;
using sensor::SampleResult;
using sensor::SensorDevice;
using sensor::SensorLifecycle;
using sensor::SensorStatus;

enum class StartupCall {
    hardware_reset,
    configure,
    self_test,
    start
};

class FakeSensorDevice final : public SensorDevice {
public:
    SensorStatus hardware_reset() override
    {
        return invoke(StartupCall::hardware_reset);
    }

    SensorStatus configure() override
    {
        return invoke(StartupCall::configure);
    }

    SensorStatus self_test() override
    {
        return invoke(StartupCall::self_test);
    }

    SensorStatus start() override
    {
        return invoke(StartupCall::start);
    }

    SampleResult read_sample() override
    {
        return {SensorStatus::invalid_state, 0};
    }

    SensorLifecycle* lifecycle{};
    std::optional<StartupCall> fail_at;
    SensorStatus failure_status{SensorStatus::sensor_rejected};
    std::vector<StartupCall> calls;
    std::vector<LifecycleState> states_seen;

private:
    SensorStatus invoke(StartupCall call)
    {
        calls.push_back(call);
        if (lifecycle != nullptr) {
            states_seen.push_back(lifecycle->state());
        }
        return fail_at.has_value() && *fail_at == call
            ? failure_status
            : SensorStatus::ok;
    }
};

const std::vector<StartupCall> startup_sequence{
    StartupCall::hardware_reset,
    StartupCall::configure,
    StartupCall::self_test,
    StartupCall::start
};

TEST(SensorLifecycle, StartsUninitialized)
{
    FakeSensorDevice device;
    const SensorLifecycle lifecycle(device);
    EXPECT_EQ(lifecycle.state(), LifecycleState::uninitialized);
}

TEST(SensorLifecycle, StartsDeviceInOrder)
{
    FakeSensorDevice device;
    SensorLifecycle lifecycle(device);
    device.lifecycle = &lifecycle;

    EXPECT_EQ(lifecycle.begin(), SensorStatus::ok);

    EXPECT_EQ(device.calls, startup_sequence);
    EXPECT_EQ(device.states_seen,
              std::vector<LifecycleState>(startup_sequence.size(),
                                          LifecycleState::initializing));
    EXPECT_EQ(lifecycle.state(), LifecycleState::ready);
}

TEST(SensorLifecycle, StopsAtTheFirstFailedStartupStep)
{
    for (std::size_t failure_index = 0;
         failure_index < startup_sequence.size();
         ++failure_index) {
        SCOPED_TRACE(failure_index);

        FakeSensorDevice device;
        device.fail_at = startup_sequence[failure_index];
        SensorLifecycle lifecycle(device);
        device.lifecycle = &lifecycle;

        EXPECT_EQ(lifecycle.begin(), SensorStatus::sensor_rejected);

        const auto expected_calls =
            std::vector<StartupCall>(startup_sequence.begin(),
                                     startup_sequence.begin() + failure_index + 1);
        EXPECT_EQ(device.calls, expected_calls);
        EXPECT_EQ(lifecycle.state(), LifecycleState::fault);
    }
}

TEST(SensorLifecycle, ResetStateAllowsACompleteNewInitializationAttempt)
{
    FakeSensorDevice device;
    device.fail_at = StartupCall::self_test;
    SensorLifecycle lifecycle(device);

    EXPECT_EQ(lifecycle.begin(), SensorStatus::sensor_rejected);
    ASSERT_EQ(lifecycle.state(), LifecycleState::fault);

    EXPECT_EQ(lifecycle.begin(), SensorStatus::invalid_state);
    EXPECT_EQ(device.calls.size(), 3U);
    EXPECT_EQ(lifecycle.state(), LifecycleState::fault);

    lifecycle.reset_state();
    EXPECT_EQ(lifecycle.state(), LifecycleState::uninitialized);

    device.fail_at.reset();
    EXPECT_EQ(lifecycle.begin(), SensorStatus::ok);
    EXPECT_EQ(lifecycle.state(), LifecycleState::ready);
    EXPECT_EQ(device.calls.size(), 3U + startup_sequence.size());
}

TEST(SensorLifecycle, BeginDoesNotRepeatInitializationWhileReady)
{
    FakeSensorDevice device;
    SensorLifecycle lifecycle(device);

    EXPECT_EQ(lifecycle.begin(), SensorStatus::ok);
    EXPECT_EQ(lifecycle.begin(), SensorStatus::invalid_state);

    EXPECT_EQ(device.calls, startup_sequence);
    EXPECT_EQ(lifecycle.state(), LifecycleState::ready);
}

TEST(SensorLifecycle, ResetStateDoesNotInvokeDevice)
{
    FakeSensorDevice device;
    SensorLifecycle lifecycle(device);
    lifecycle.reset_state();

    EXPECT_TRUE(device.calls.empty());
    EXPECT_EQ(lifecycle.state(), LifecycleState::uninitialized);
}

}  // namespace
