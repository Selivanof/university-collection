#ifndef TIMESTAMPS_HPP
#define TIMESTAMPS_HPP

#include <cstdint>

#define MS_IN_SEC 1000
#define US_IN_MS 1000
#define MS_IN_MIN 60000

namespace timestamps {
/**
 * Rounds down a timestamp to the start of the nearest minute.
 *
 * @param timestamp_ms: The timestamp in milliseconds to be floored.
 * @returns: The timestamp of the start of the minute containing the input timestamp.
 */
inline int64_t minute_floor_ms(int64_t timestamp_ms) {
    int64_t remainder = timestamp_ms % MS_IN_MIN;
    return timestamp_ms - remainder;
}

/**
 * Rounds up a timestamp to the start of the next minute.
 *
 * @param timestamp_ms: The timestamp in milliseconds to be ceiled.
 * @returns: The timestamp of the start of the next minute, or the original timestamp if it's already at the start of a
 * minute.
 */
inline int64_t minute_ceil_ms(int64_t timestamp_ms) {
    int64_t remainder = timestamp_ms % MS_IN_MIN;
    if (remainder == 0) {
        return timestamp_ms;
    }
    return timestamp_ms + (MS_IN_MIN - remainder);
}

/**
 * Converts a timestamp from milliseconds to microseconds.
 *
 * @param timestamp_ms: The timestamp in milliseconds to be converted.
 * @returns: The timestamp in microseconds.
 */
inline int64_t ms_to_us(int64_t timestamp_ms) { return timestamp_ms * US_IN_MS; }

/**
 * @brief Converts a timestamp from microseconds to milliseconds.
 *
 *
 * @param timestamp_us: The timestamp in microseconds to be converted.
 * @returns: The timestamp in milliseconds.
 */
inline int64_t us_to_ms(int64_t timestamp_us) { return timestamp_us / US_IN_MS; }

/**
 * @brief Retrieves the current time since epoch in a specified duration type.
 *
 * This template function returns the current time since the epoch in a duration type specified by the template
 * parameter T. It can be used to obtain timestamps in various units (e.g., milliseconds, microseconds) depending on the
 * template argument.
 *
 * @tparam T: The duration type to represent the time since epoch (e.g., std::chrono::milliseconds).
 * @returns: The current time since epoch in the specified duration type.
 */
template <typename T>
inline int64_t current() {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<T>(now).count();
    return milliseconds;
}
}  // namespace timestamps

#endif  // TIMESTAMPS_HPP