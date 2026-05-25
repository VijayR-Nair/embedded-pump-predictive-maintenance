#pragma once
#include <array>
#include <cstddef>
#include <optional>

namespace SmartSensor {

template <typename T, size_t Capacity>
class CircularBuffer {
public:
    CircularBuffer() : head_(0), tail_(0), size_(0) {}

    // Push a new item into the buffer. Overwrites oldest if full.
    void push(T item) {
        buffer_[head_] = item;
        head_ = (head_ + 1) % Capacity;

        if (size_ < Capacity) {
            size_++;
        } else {
            // Buffer was full, advance tail to drop the oldest item
            tail_ = (tail_ + 1) % Capacity;
        }
    }

    // Get current number of items in the buffer
    size_t size() const noexcept { return size_; }
    
    // Check if the buffer is empty
    bool empty() const noexcept { return size_ == 0; }

    // Calculate the simple average of the values currently in the buffer
    float get_average() const {
        if (size_ == 0) return 0.0f;
        float sum = 0.0f;
        for (size_t i = 0; i < size_; ++i) {
            size_t idx = (tail_ + i) % Capacity;
            sum += buffer_[idx];
        }
        return sum / static_cast<float>(size_);;
    }

private:
    std::array<T, Capacity> buffer_;
    size_t head_;
    size_t tail_;
    size_t size_;
};

} // namespace SmartSensor