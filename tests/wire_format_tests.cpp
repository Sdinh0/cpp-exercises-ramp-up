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

TEST(Crc, SingleByteMatchesExpectedValue)
{
    EXPECT_EQ(Codec::crc8({0x01}), 0x07);
}

TEST(Crc, EmptyAndZeroInputProduceZero)
{
    EXPECT_EQ(Codec::crc8({}), 0x00);
    EXPECT_EQ(Codec::crc8({0x00}), 0x00);
}

TEST(Crc, HighBitInputMatchesExpectedValue)
{
    EXPECT_EQ(Codec::crc8({0xFF}), 0xF3);
}

TEST(Crc, StandardCheckVectorMatchesExpectedValue)
{
    const std::vector<std::uint8_t> bytes{'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    EXPECT_EQ(Codec::crc8(bytes), 0xF4);
}

TEST(FrameEncoding, EncodesHeaderPayloadAndChecksum)
{
    const auto encoded = Codec::encode(sample_frame());
    EXPECT_EQ(encoded, (std::vector<std::uint8_t>{0xA5, 0x10, 0x02, 0x01, 0x02, 0xAA}));
}

TEST(FrameEncoding, EncodesEmptyPayload)
{
    const auto encoded = Codec::encode(Frame{0x22, {}});
    ASSERT_EQ(encoded.size(), 4U);
    EXPECT_EQ(encoded[0], Codec::sync_byte);
    EXPECT_EQ(encoded[1], 0x22);
    EXPECT_EQ(encoded[2], 0x00);
}

TEST(FrameEncoding, ChecksumCoversCommandLengthAndPayload)
{
    const auto encoded = Codec::encode(sample_frame());
    ASSERT_EQ(encoded.size(), 6U);
    const std::vector<std::uint8_t> covered(encoded.begin() + 1, encoded.end() - 1);
    EXPECT_EQ(encoded.back(), Codec::crc8(covered));
}

TEST(FrameDecoding, DecodesValidFrame)
{
    const auto result = Codec::decode(wire_frame(sample_frame()));
    ASSERT_TRUE(result.ok);
    EXPECT_EQ(result.error, ParseError::none);
    EXPECT_EQ(result.frame.command, 0x10);
    EXPECT_EQ(result.frame.payload, (std::vector<std::uint8_t>{0x01, 0x02}));
}

TEST(FrameDecoding, RejectsEveryInputShorterThanMinimum)
{
    for (std::size_t size = 0; size < 4; ++size) {
        const auto result = Codec::decode(std::vector<std::uint8_t>(size, Codec::sync_byte));
        EXPECT_FALSE(result.ok) << "input size: " << size;
        EXPECT_EQ(result.error, ParseError::frame_too_short) << "input size: " << size;
    }
}

TEST(FrameDecoding, RejectsWrongSyncByte)
{
    auto bytes = wire_frame(sample_frame());
    ASSERT_FALSE(bytes.empty());
    bytes[0] = 0x00;
    const auto result = Codec::decode(bytes);
    EXPECT_FALSE(result.ok);
    EXPECT_EQ(result.error, ParseError::bad_sync);
}

TEST(FrameDecoding, RejectsPayloadLengthMismatch)
{
    auto bytes = wire_frame(sample_frame());
    ASSERT_EQ(bytes.size(), 6U);
    bytes[2] = 3;
    const auto result = Codec::decode(bytes);
    EXPECT_FALSE(result.ok);
    EXPECT_EQ(result.error, ParseError::bad_length);
}

TEST(FrameDecoding, RejectsTruncatedPayloadAndChecksum)
{
    auto missing_payload = wire_frame(sample_frame());
    missing_payload.erase(missing_payload.end() - 2);
    const auto missing_payload_result = Codec::decode(missing_payload);
    EXPECT_FALSE(missing_payload_result.ok);
    EXPECT_EQ(missing_payload_result.error, ParseError::bad_length);

    auto missing_checksum = wire_frame(sample_frame());
    missing_checksum.pop_back();
    const auto missing_checksum_result = Codec::decode(missing_checksum);
    EXPECT_FALSE(missing_checksum_result.ok);
    EXPECT_EQ(missing_checksum_result.error, ParseError::bad_length);
}

TEST(FrameDecoding, RejectsCorruptedChecksum)
{
    auto bytes = wire_frame(sample_frame());
    ASSERT_FALSE(bytes.empty());
    bytes.back() ^= 0x80;
    const auto result = Codec::decode(bytes);
    EXPECT_FALSE(result.ok);
    EXPECT_EQ(result.error, ParseError::bad_checksum);
}

TEST(FrameDecoding, RejectsPayloadCorruption)
{
    auto bytes = wire_frame(sample_frame());
    bytes[3] ^= 0x01;
    const auto result = Codec::decode(bytes);
    EXPECT_FALSE(result.ok);
    EXPECT_EQ(result.error, ParseError::bad_checksum);
}

}  // namespace
