/**
 * @file        HistoryBuffer.h
 * @project     AquaLeaf — Intelligent Plant Irrigation Platform
 * @brief       Ring buffer riwayat kelembaban berukuran tetap (tanpa heap).
 *
 * Setiap sampel dikemas dalam 16 bit:
 *   bit 0–14  kelembaban × 10 (0–1000), atau NO_READING jika sensor fault
 *   bit 15    pompa sempat menyala selama interval sampel ini
 */

#pragma once

#include <stdint.h>

struct HistorySample {
    float moisturePercent;  ///< Diabaikan jika hasReading = false
    bool hasReading;
    bool watered;
};

template <uint16_t Capacity>
class HistoryBuffer {
public:
    HistoryBuffer() : head(0), count(0) {}

    void push(const HistorySample &sample) {
        samples[head] = pack(sample);
        head = static_cast<uint16_t>((head + 1) % Capacity);
        if (count < Capacity) {
            ++count;
        }
    }

    uint16_t size() const {
        return count;
    }

    uint16_t capacity() const {
        return Capacity;
    }

    /** index 0 = sampel tertua, size() - 1 = terbaru. */
    HistorySample at(uint16_t index) const {
        uint16_t start = static_cast<uint16_t>((head + Capacity - count) % Capacity);
        return unpack(samples[(start + index) % Capacity]);
    }

private:
    static constexpr uint16_t WATERED_BIT = 0x8000;
    static constexpr uint16_t VALUE_MASK = 0x7FFF;
    static constexpr uint16_t NO_READING = 0x7FFF;

    static uint16_t pack(const HistorySample &sample) {
        uint16_t value = NO_READING;
        if (sample.hasReading) {
            float clamped = sample.moisturePercent < 0.0f ? 0.0f
                          : (sample.moisturePercent > 100.0f ? 100.0f : sample.moisturePercent);
            value = static_cast<uint16_t>(clamped * 10.0f + 0.5f);
        }
        return static_cast<uint16_t>(value | (sample.watered ? WATERED_BIT : 0));
    }

    static HistorySample unpack(uint16_t packed) {
        HistorySample sample;
        uint16_t value = packed & VALUE_MASK;
        sample.hasReading = value != NO_READING;
        sample.moisturePercent = sample.hasReading ? value / 10.0f : 0.0f;
        sample.watered = (packed & WATERED_BIT) != 0;
        return sample;
    }

    uint16_t samples[Capacity];
    uint16_t head;
    uint16_t count;
};
