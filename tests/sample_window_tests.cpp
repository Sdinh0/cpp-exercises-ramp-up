#include "sensor/sensor.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <vector>

namespace {

using sensor::SampleWindow;

TEST(SampleWindow, EmptyWindowHasNoAverage)
{
    const SampleWindow window(3);
    EXPECT_FALSE(window.average().has_value());
}

TEST(SampleWindow, ReportsCapacityAndCurrentSize)
{
    SampleWindow window(3);
    window.add(10);
    EXPECT_EQ(window.capacity(), 3U);
    EXPECT_EQ(window.size(), 1U);
    EXPECT_FALSE(window.full());
}

TEST(SampleWindow, AveragesOneSample)
{
    SampleWindow window(3);
    window.add(42);
    ASSERT_TRUE(window.average().has_value());
    EXPECT_EQ(*window.average(), 42);
}

TEST(SampleWindow, TruncatesFractionalAverageTowardZero)
{
    SampleWindow window(3);
    window.add(-1);
    window.add(0);
    ASSERT_TRUE(window.average().has_value());
    EXPECT_EQ(*window.average(), 0);
}

TEST(SampleWindow, BecomesFullAtCapacity)
{
    SampleWindow window(2);
    window.add(1);
    EXPECT_FALSE(window.full());
    window.add(2);
    EXPECT_TRUE(window.full());
}

TEST(SampleWindow, RetainsMostRecentSamplesWhenFull)
{
    SampleWindow window(2);
    window.add(10);
    window.add(20);
    window.add(30);
    ASSERT_TRUE(window.average().has_value());
    EXPECT_EQ(*window.average(), 25);
    EXPECT_EQ(window.size(), 2U);
}

TEST(SampleWindow, AveragesNegativeValues)
{
    SampleWindow window(2);
    window.add(-10);
    window.add(0);
    ASSERT_TRUE(window.average().has_value());
    EXPECT_EQ(*window.average(), -5);
}

TEST(SampleWindow, ClearEmptiesWindowAndResetsFullState)
{
    SampleWindow window(2);
    window.add(10);
    window.add(20);
    ASSERT_TRUE(window.full());
    window.clear();
    EXPECT_EQ(window.size(), 0U);
    EXPECT_FALSE(window.average().has_value());
    EXPECT_FALSE(window.full());
}

TEST(SampleWindow, ZeroCapacityDoesNotStoreSamples)
{
    SampleWindow window(0);
    window.add(10);
    EXPECT_EQ(window.size(), 0U);
    EXPECT_FALSE(window.average().has_value());
}

TEST(SampleWindow, CanBeReusedAfterClear)
{
    SampleWindow window(2);
    window.add(10);
    window.clear();
    window.add(8);
    window.add(12);
    ASSERT_TRUE(window.average().has_value());
    EXPECT_EQ(*window.average(), 10);
}

TEST(SampleWindow, AveragesInt32BoundaryValuesWithoutOverflow)
{
    SampleWindow window(2);
    window.add(std::numeric_limits<std::int32_t>::min());
    window.add(std::numeric_limits<std::int32_t>::max());
    ASSERT_TRUE(window.average().has_value());
    EXPECT_EQ(*window.average(), 0);
}

}  // namespace
