#include "sensor/sensor.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace {

using sensor::Codec;
using sensor::Frame;
using sensor::ParseError;

Frame sample_frame()
{
    return Frame{0x10, {0x01, 0x02}};
}

std::vector<std::uint8_t> wire_frame(const Frame& frame)
{
    auto encoded = Codec::encode(frame);
    return encoded;
}

TEST(CodecBoundaries, EncodesMaximumPayload)
{
    Frame frame{0x31, std::vector<std::uint8_t>(Codec::max_payload_size, 0x5A)};
    const auto encoded = Codec::encode(frame);
    ASSERT_EQ(encoded.size(), Codec::max_payload_size + 4);
    EXPECT_EQ(encoded[2], Codec::max_payload_size);
    EXPECT_EQ(encoded[3], 0x5A);
    EXPECT_EQ(encoded[encoded.size() - 2], 0x5A);
}

TEST(CodecBoundaries, RejectsPayloadLargerThanMaximumWhenEncoding)
{
    Frame frame{0x31, std::vector<std::uint8_t>(Codec::max_payload_size + 1, 0)};
    EXPECT_TRUE(Codec::encode(frame).empty());
}

TEST(CodecBoundaries, DecodesMaximumPayload)
{
    Frame frame{0x31, std::vector<std::uint8_t>(Codec::max_payload_size, 0x5A)};
    const auto result = Codec::decode(Codec::encode(frame));
    ASSERT_TRUE(result.ok);
    EXPECT_EQ(result.frame.payload, frame.payload);
}

TEST(CodecBoundaries, RejectsPayloadLargerThanMaximumWhenDecoding)
{
    const auto payload_size = Codec::max_payload_size + 1;
    std::vector<std::uint8_t> bytes{
        Codec::sync_byte, 0x31, static_cast<std::uint8_t>(payload_size)};
    bytes.insert(bytes.end(), payload_size, 0x5A);
    bytes.push_back(0x00);

    const auto result = Codec::decode(bytes);
    EXPECT_FALSE(result.ok);
    EXPECT_EQ(result.error, ParseError::bad_length);
}

TEST(CodecBoundaries, DecodesEmptyPayload)
{
    const Frame frame{0x22, {}};
    const auto result = Codec::decode(Codec::encode(frame));
    ASSERT_TRUE(result.ok);
    EXPECT_EQ(result.frame.command, frame.command);
    EXPECT_TRUE(result.frame.payload.empty());
}

TEST(CodecBoundaries, RejectsTrailingBytes)
{
    auto bytes = wire_frame(sample_frame());
    bytes.push_back(0xEE);
    const auto result = Codec::decode(bytes);
    EXPECT_FALSE(result.ok);
    EXPECT_EQ(result.error, ParseError::bad_length);
}

TEST(CodecBoundaries, PreservesCommandAndAllByteValuesOnRoundTrip)
{
    const Frame frame{0xFF, {0x00, 0x7F, 0x80, 0xFF}};
    const auto result = Codec::decode(Codec::encode(frame));
    ASSERT_TRUE(result.ok);
    EXPECT_EQ(result.frame.command, frame.command);
    EXPECT_EQ(result.frame.payload, frame.payload);
}

}  // namespace
