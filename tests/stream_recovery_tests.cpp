#include "sensor/sensor.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace {

using sensor::Codec;
using sensor::Frame;
using sensor::StreamDecoder;

Frame sample_frame()
{
    return Frame{0x10, {0x01, 0x02}};
}

std::vector<std::uint8_t> wire_frame(const Frame& frame)
{
    auto encoded = Codec::encode(frame);
    return encoded;
}

TEST(StreamDecoder, ResetDiscardsPartialInput)
{
    const auto encoded = wire_frame(sample_frame());
    StreamDecoder decoder;
    decoder.feed(encoded.front());
    EXPECT_EQ(decoder.buffered_size(), 1U);
    decoder.reset();
    EXPECT_EQ(decoder.buffered_size(), 0U);
}

TEST(StreamDecoder, AcceptsEmptyPayload)
{
    const auto encoded = wire_frame(Frame{0x24, {}});
    StreamDecoder decoder;
    std::vector<Frame> frames;
    for (const auto byte : encoded) {
        frames = decoder.feed(byte);
    }
    ASSERT_EQ(frames.size(), 1U);
    EXPECT_EQ(frames.front().command, 0x24);
    EXPECT_TRUE(frames.front().payload.empty());
    EXPECT_EQ(decoder.buffered_size(), 0U);
}

TEST(StreamDecoder, RecoversAfterInvalidLength)
{
    const auto good = wire_frame(Frame{0x20, {0x77}});
    StreamDecoder decoder;
    decoder.feed(Codec::sync_byte);
    decoder.feed(0x10);
    decoder.feed(0xFF);

    std::vector<Frame> frames;
    for (const auto byte : good) {
        const auto emitted = decoder.feed(byte);
        frames.insert(frames.end(), emitted.begin(), emitted.end());
    }

    ASSERT_EQ(frames.size(), 1U);
    EXPECT_EQ(frames.front().command, 0x20);
    EXPECT_EQ(frames.front().payload, (std::vector<std::uint8_t>{0x77}));
}

TEST(StreamDecoder, RecoversValidFrameInsideCorruptCandidate)
{
    const auto embedded = wire_frame(Frame{0x24, {}});
    Frame corrupt_candidate{0x31, embedded};
    corrupt_candidate.payload.push_back(0xCC);
    auto encoded_corrupt_candidate = wire_frame(corrupt_candidate);
    encoded_corrupt_candidate.back() ^= 0x01;

    StreamDecoder decoder;
    std::vector<Frame> frames;
    for (const auto byte : encoded_corrupt_candidate) {
        const auto emitted = decoder.feed(byte);
        frames.insert(frames.end(), emitted.begin(), emitted.end());
    }

    ASSERT_EQ(frames.size(), 1U);
    EXPECT_EQ(frames.front().command, 0x24);
    EXPECT_TRUE(frames.front().payload.empty());
    EXPECT_EQ(decoder.buffered_size(), 0U);
}

TEST(StreamDecoder, BoundsPartialFrameBuffer)
{
    StreamDecoder decoder;
    for (int i = 0; i < 100; ++i) {
        decoder.feed(0xA5);
        decoder.feed(0x10);
        decoder.feed(0xFF);
    }
    EXPECT_LE(decoder.buffered_size(), Codec::max_payload_size + 4);
}

}  // namespace
