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

TEST(StreamDecoder, WaitsUntilFrameIsComplete)
{
    const auto encoded = wire_frame(sample_frame());
    StreamDecoder decoder;
    std::vector<Frame> frames;
    for (std::size_t i = 0; i + 1 < encoded.size(); ++i) {
        frames = decoder.feed(encoded[i]);
        EXPECT_TRUE(frames.empty());
    }
}

TEST(StreamDecoder, EmitsCompleteFrame)
{
    const auto encoded = wire_frame(sample_frame());
    StreamDecoder decoder;
    std::vector<Frame> frames;
    for (const auto byte : encoded) {
        frames = decoder.feed(byte);
    }
    ASSERT_EQ(frames.size(), 1U);
    EXPECT_EQ(frames.front().command, sample_frame().command);
    EXPECT_EQ(frames.front().payload, sample_frame().payload);
}

TEST(StreamDecoder, AcceptsAFrameSplitAcrossInputChunks)
{
    const auto encoded = wire_frame(sample_frame());
    StreamDecoder decoder;

    const auto first = decoder.feed(encoded.data(), 2);
    EXPECT_TRUE(first.empty());
    EXPECT_EQ(decoder.buffered_size(), 2U);

    const auto second =
        decoder.feed(encoded.data() + 2, encoded.size() - 2);
    ASSERT_EQ(second.size(), 1U);
    EXPECT_EQ(second.front().command, sample_frame().command);
    EXPECT_EQ(second.front().payload, sample_frame().payload);
    EXPECT_EQ(decoder.buffered_size(), 0U);
}

TEST(StreamDecoder, EmitsMultipleFramesFromOneInputChunk)
{
    auto bytes = wire_frame(sample_frame());
    const auto second_frame = wire_frame(Frame{0x20, {0x03}});
    bytes.insert(bytes.end(), second_frame.begin(), second_frame.end());
    StreamDecoder decoder;

    const auto frames = decoder.feed(bytes.data(), bytes.size());

    ASSERT_EQ(frames.size(), 2U);
    EXPECT_EQ(frames[0].command, sample_frame().command);
    EXPECT_EQ(frames[1].command, 0x20);
}

TEST(StreamDecoder, IgnoresNoiseAndRecoversWithinAnInputChunk)
{
    auto bytes = std::vector<std::uint8_t>{0x11, 0x22};
    const auto encoded = wire_frame(sample_frame());
    bytes.insert(bytes.end(), encoded.begin(), encoded.end());
    StreamDecoder decoder;

    const auto frames = decoder.feed(bytes.data(), bytes.size());

    ASSERT_EQ(frames.size(), 1U);
    EXPECT_EQ(frames.front().command, sample_frame().command);
    EXPECT_EQ(frames.front().payload, sample_frame().payload);
}

TEST(StreamDecoder, EmptyChunkDoesNotChangeBufferedInput)
{
    StreamDecoder decoder;
    const auto frame = wire_frame(sample_frame());
    decoder.feed(frame.front());
    const auto buffered_before = decoder.buffered_size();

    const auto frames = decoder.feed(nullptr, 0);

    EXPECT_TRUE(frames.empty());
    EXPECT_EQ(decoder.buffered_size(), buffered_before);
}

TEST(StreamDecoder, PreservesSyncByteInsidePayload)
{
    const Frame frame{0x31, {Codec::sync_byte}};
    const auto encoded = wire_frame(frame);
    StreamDecoder decoder;
    std::vector<Frame> frames;

    for (const auto byte : encoded) {
        const auto emitted = decoder.feed(byte);
        frames.insert(frames.end(), emitted.begin(), emitted.end());
    }

    ASSERT_EQ(frames.size(), 1U);
    EXPECT_EQ(frames.front().command, frame.command);
    EXPECT_EQ(frames.front().payload, frame.payload);
    EXPECT_EQ(decoder.buffered_size(), 0U);
}

TEST(StreamDecoder, EmitsMaximumPayloadFrame)
{
    const Frame frame{0x31, std::vector<std::uint8_t>(Codec::max_payload_size, 0x5A)};
    const auto encoded = wire_frame(frame);
    StreamDecoder decoder;
    std::vector<Frame> frames;

    for (const auto byte : encoded) {
        frames = decoder.feed(byte);
    }

    ASSERT_EQ(frames.size(), 1U);
    EXPECT_EQ(frames.front().command, frame.command);
    EXPECT_EQ(frames.front().payload, frame.payload);
    EXPECT_EQ(decoder.buffered_size(), 0U);
}

TEST(StreamDecoder, ClearsBufferAfterEmittingFrame)
{
    const auto encoded = wire_frame(sample_frame());
    StreamDecoder decoder;
    for (const auto byte : encoded) {
        decoder.feed(byte);
    }
    EXPECT_EQ(decoder.buffered_size(), 0U);
}

TEST(StreamDecoder, IgnoresNoiseBeforeSync)
{
    const auto encoded = wire_frame(sample_frame());
    StreamDecoder decoder;
    decoder.feed(0x11);
    decoder.feed(0x22);
    std::vector<Frame> frames;
    for (const auto byte : encoded) {
        frames = decoder.feed(byte);
    }
    ASSERT_EQ(frames.size(), 1U);
    EXPECT_EQ(frames.front().payload, sample_frame().payload);
}

TEST(StreamDecoder, RecoversAfterBadChecksum)
{
    auto bad = wire_frame(sample_frame());
    bad.back() ^= 1;
    const auto good = wire_frame(Frame{0x20, {0x77}});
    StreamDecoder decoder;
    std::vector<Frame> frames;

    for (const auto byte : bad) {
        const auto emitted = decoder.feed(byte);
        EXPECT_TRUE(emitted.empty());
    }

    for (const auto byte : good) {
        const auto emitted = decoder.feed(byte);
        frames.insert(frames.end(), emitted.begin(), emitted.end());
    }

    ASSERT_EQ(frames.size(), 1U);
    EXPECT_EQ(frames.front().command, 0x20);
    EXPECT_EQ(frames.front().payload, (std::vector<std::uint8_t>{0x77}));
}

TEST(StreamDecoder, EmitsConsecutiveFrames)
{
    auto bytes = wire_frame(sample_frame());
    const auto second = wire_frame(Frame{0x20, {0x03}});
    bytes.insert(bytes.end(), second.begin(), second.end());
    StreamDecoder decoder;
    std::size_t emitted = 0;
    for (const auto byte : bytes) {
        emitted += decoder.feed(byte).size();
    }
    EXPECT_EQ(emitted, 2U);
}

}  // namespace
