#include "sensor/sensor.hpp"

#include <gtest/gtest.h>

namespace {

using sensor::LifecycleState;
using sensor::SensorLifecycle;

TEST(SensorLifecycle, StartsUninitialized)
{
    const SensorLifecycle lifecycle;
    EXPECT_EQ(lifecycle.state(), LifecycleState::uninitialized);
}

TEST(SensorLifecycle, BeginStartsInitialization)
{
    SensorLifecycle lifecycle;
    lifecycle.begin();
    EXPECT_EQ(lifecycle.state(), LifecycleState::initializing);
}

TEST(SensorLifecycle, SuccessfulInitializationMakesSensorReady)
{
    SensorLifecycle lifecycle;
    lifecycle.begin();
    lifecycle.initialization_succeeded();
    EXPECT_EQ(lifecycle.state(), LifecycleState::ready);
}

TEST(SensorLifecycle, FailureEntersFaultState)
{
    SensorLifecycle lifecycle;
    lifecycle.begin();
    lifecycle.fail();
    EXPECT_EQ(lifecycle.state(), LifecycleState::fault);
}

TEST(SensorLifecycle, TimeoutDuringInitializationEntersFaultState)
{
    SensorLifecycle lifecycle;
    lifecycle.begin();
    lifecycle.timeout();
    EXPECT_EQ(lifecycle.state(), LifecycleState::fault);
}

TEST(SensorLifecycle, ResetReturnsToUninitialized)
{
    SensorLifecycle lifecycle;
    lifecycle.begin();
    lifecycle.initialization_succeeded();
    lifecycle.reset();
    EXPECT_EQ(lifecycle.state(), LifecycleState::uninitialized);
}

TEST(SensorLifecycle, SampleKeepsReadySensorReady)
{
    SensorLifecycle lifecycle;
    lifecycle.begin();
    lifecycle.initialization_succeeded();
    lifecycle.receive_sample();
    EXPECT_EQ(lifecycle.state(), LifecycleState::ready);
}

TEST(SensorLifecycle, SampleBeforeInitializationDoesNotBecomeReady)
{
    SensorLifecycle lifecycle;
    lifecycle.receive_sample();
    EXPECT_EQ(lifecycle.state(), LifecycleState::uninitialized);
}

TEST(SensorLifecycle, BeginFromFaultStartsNewAttempt)
{
    SensorLifecycle lifecycle;
    lifecycle.fail();
    lifecycle.begin();
    EXPECT_EQ(lifecycle.state(), LifecycleState::initializing);
}

TEST(SensorLifecycle, InitializationFailureCanBeReset)
{
    SensorLifecycle lifecycle;
    lifecycle.begin();
    lifecycle.fail();
    lifecycle.reset();
    EXPECT_EQ(lifecycle.state(), LifecycleState::uninitialized);
}

TEST(SensorLifecycle, SampleDoesNotRecoverFaultedSensor)
{
    SensorLifecycle lifecycle;
    lifecycle.fail();
    lifecycle.receive_sample();
    EXPECT_EQ(lifecycle.state(), LifecycleState::fault);
}

TEST(SensorLifecycle, ReadySensorCanBeResetAndReinitialized)
{
    SensorLifecycle lifecycle;
    lifecycle.begin();
    lifecycle.initialization_succeeded();
    lifecycle.reset();
    lifecycle.begin();
    EXPECT_EQ(lifecycle.state(), LifecycleState::initializing);
}

TEST(SensorLifecycle, TimeoutFromReadyEntersFaultState)
{
    SensorLifecycle lifecycle;
    lifecycle.begin();
    lifecycle.initialization_succeeded();
    lifecycle.timeout();
    EXPECT_EQ(lifecycle.state(), LifecycleState::fault);
}

TEST(SensorLifecycle, FaultRemainsFaultAfterSample)
{
    SensorLifecycle lifecycle;
    lifecycle.fail();
    lifecycle.receive_sample();
    EXPECT_EQ(lifecycle.state(), LifecycleState::fault);
}

TEST(SensorLifecycle, InitializationSuccessRequiresActiveAttempt)
{
    SensorLifecycle lifecycle;
    lifecycle.initialization_succeeded();
    EXPECT_EQ(lifecycle.state(), LifecycleState::uninitialized);
}

TEST(SensorLifecycle, InitializationSuccessDoesNotRecoverFaultedSensor)
{
    SensorLifecycle lifecycle;
    lifecycle.fail();
    lifecycle.initialization_succeeded();
    EXPECT_EQ(lifecycle.state(), LifecycleState::fault);
}

TEST(SensorLifecycle, SampleDuringInitializationDoesNotChangeState)
{
    SensorLifecycle lifecycle;
    lifecycle.begin();
    lifecycle.receive_sample();
    EXPECT_EQ(lifecycle.state(), LifecycleState::initializing);
}

TEST(SensorLifecycle, RepeatedResetIsSafe)
{
    SensorLifecycle lifecycle;
    lifecycle.reset();
    lifecycle.reset();
    EXPECT_EQ(lifecycle.state(), LifecycleState::uninitialized);
}

}  // namespace
