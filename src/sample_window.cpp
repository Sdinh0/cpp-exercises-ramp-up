#include "sensor/sensor.hpp"

namespace sensor {

SampleWindow::SampleWindow(std::size_t capacity)
    : capacity_(capacity)
{
}

void SampleWindow::add(std::int32_t sample)
{
    // A zero-capacity window intentionally stores no samples.
    if (capacity_ == 0) {
        return;
    }

    if (full()) {
        // Evict the oldest value so the window always contains the newest samples.
        samples_.erase(samples_.begin());
    }
    samples_.push_back(sample);
}

std::optional<std::int32_t> SampleWindow::average() const
{
    if (samples_.empty()) {
        // No average exists until at least one sample has been added.
        return std::nullopt;
    }
    else{
        long long sum = 0;
        for (const auto& sample : samples_) {
            sum += sample;
        }
        return sum / static_cast<long long>(samples_.size());
    }
}

std::size_t SampleWindow::size() const
{
    return samples_.size();
}

std::size_t SampleWindow::capacity() const
{
    return capacity_;
}

bool SampleWindow::full() const
{
    return samples_.size() >= capacity_;
}

void SampleWindow::clear()
{
    samples_.clear();
}

}  // namespace sensor
