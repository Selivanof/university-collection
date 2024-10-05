#ifndef FINNHUB_DATA_HPP
#define FINNHUB_DATA_HPP

#include <chrono>
#include <cmath>
#include <mutex>
#include <string>
#include <unordered_map>

#include "timestamps.hpp"

namespace finnhub {

const int8_t PRECISION_DIGITS = 6;
const int32_t FLOAT_TO_INT_FACTOR = static_cast<int32_t>(std::pow(10, PRECISION_DIGITS));

/**
 * @struct TradeData
 * @brief A struct that represents a trade, containing fields for all the information that is received from
 *Finnhub's websocket.
 *
 **/
struct TradeData {
    double price;
    std::string symbol;
    int64_t timestamp;
    double volume;
    int64_t arrival_timestamp;
};

/**
 * @struct StaticCandlestick
 * @brief A struct that represents the candlestick data for a financial symbol, capturing its open, close, minimum, and
 * maximum prices.
 *
 * This structure is designed to be immutable once created. All member variables (`open_price`, `close_price`,
 *`min_price`, and `max_price`) are marked as `const` to ensure that the candlestick data remains consistent and cannot
 * be modified after initialization. This should be treaded as a snapshot of a candlestick.
 *
 * Constructor Parameters:
 *  @param open: The open price.
 *  @param close: The close price.
 *  @param min: The minimum price.
 *  @param max: The maximum price.
 **/
struct StaticCandlestick {
    const double open_price;
    const double close_price;
    const double min_price;
    const double max_price;
    const double volume;

    StaticCandlestick(double open, double close, double min, double max, double total_volume)
        : open_price(open), close_price(close), min_price(min), max_price(max), volume(total_volume) {}
};

/**
 * @struct Candlestick
 * @brief A struct that dynamically accumulates trade data to form a candlestick, which can then be converted into a
 * static, immutable form.
 *
 * This structure represents a candlestick that is built over time by adding trade data. It dynamically tracks the
 * opening and closing prices, along with the minimum and maximum prices observed during the candlestick's timeframe.
 * Unlike `StaticCandlestick`, this struct allows incremental updates through the `addTrade()` .
 *
 */
class Candlestick {
   public:
    /**
     * Adds a trade to the candlestick, updating its data accordingly.
     * @param trade The trade data to be added to the candlestick.
     */
    void addTrade(const finnhub::TradeData &trade);
    /**
     * etrieves an immutable version of the current candlestick data.
     * @returns A `StaticCandlestick` object representing the immutable state of the candlestick.
     */
    finnhub::StaticCandlestick getStatic() const;

   public:
    int64_t open_timestamp = std::numeric_limits<int64_t>::max();
    int64_t close_timestamp = std::numeric_limits<int64_t>::min();

   private:
    double open_price = 0;
    double close_price = 0;
    double min_price = std::numeric_limits<double>::max();
    double max_price = std::numeric_limits<double>::min();
    int64_t volume = 0;
};
/**
 * @class CandlestickManager
 * @brief Manages multiple candlesticks over a time window, allowing for dynamic trade data accumulation and retrieval
 * of finalized candlesticks.
 *
 * This class is responsible for maintaining and managing a collection of `Candlestick` objects, each representing a
 * specific time period. It handles the addition of trade data and provides access to finalized, immutable candlesticks
 * for specified time intervals. The manager also ensures thread safety through the use of a mutex, allowing safe
 * multi-threaded access to candlestick data.
 *
 * Constructor Parameters:
 *  @param time_window: The number of whole, exact minutes for which candlesticks should be stored.
 *                      This defines how many candlesticks are kept in memory at any given time before older ones are
 *                      pruned. For example, a time_window of 60 means that candlesticks for the last 60 minutes are
 *                      maintained, and candlesticks older than this are discarded.
 */
class CandlestickManager {
   public:
    CandlestickManager(int time_window);
    ~CandlestickManager();
    /**
     *  Adds a trade to the manager, updating the appropriate candlesticks.
     * @param trade The trade data to be added.
     */
    void addTrade(const finnhub::TradeData &trade);
    /**
     * Retrieves an immutable version of the candlestick information for a specific timestamp.
     * @param timestamp_ms The timestamp, in milliseconds, for which the candlestick is to be retrieved. It should
     * represent an exact minute. A timestamp that represents XX:XX:00 will retrieve the candlestick for all the trades
     * from XX:XX:00 to XX:XX:59.
     * @returns A `StaticCandlestick` object representing the candlestick data at the specified timestamp.
     */
    finnhub::StaticCandlestick getCandlestick(int64_t timestamp_ms) const;

   private:
    std::unordered_map<int64_t, finnhub::Candlestick> m_candlesticks;
    const int m_minutes_to_store;
    mutable std::mutex m_mutex;
};

/**
 * @struct SimpleAverage
 * @brief A simple struct that represents a generic weighted average of a symbol. Adding trades by calling `addTrade`
 * ensures the the average is updated correctly waking up every nth exact minute to execute a user-specified function.
 *
 **/
struct SimpleAverage {
    int64_t sum = 0;
    int64_t count = 0;
    /**
     * Adds a trade to the average, updating the average value accordingly.
     * @param trade The trade data to be added.
     */
    void addTrade(const finnhub::TradeData &trade);
};

/**
 * @struct StaticAverage
 * @brief A struct that represents the average for the price financial symbol at a specific point in time.
 *
 **/
struct StaticAverage {
    const double sum;
    const double volume;
    const double value;

    StaticAverage(double item_sum, double item_count, double item_average)
        : sum(item_sum), volume(item_count), value(item_average) {}
};

/**
 * @class MovingAverage
 * @brief Computes and maintains a time-based moving average of trade data over a specified time window.
 *
 * This class calculates the moving average of trade values, updating the average as new trades are added.
 * It uses a sliding window approach, storing average values per whole, exact minutes and pruning outdated data based on
 * the specified time window. Thread safety is ensured through the use of a mutex, allowing safe concurrent access and
 * modification of the average data.
 *
 * Constructor Parameters:
 *  @param time_window: The duration of the time window (in minutes) over which the moving average is computed.
 *                      This defines how long the average is maintained before outdated data is pruned.
 *
 * Note:
 * - For correct operation, either `addTrade()` or `getValue()` must be called at least once every minute.
 *   This ensures that the pruning of old trade data is performed regularly and that the moving average is accurately
 *   updated.
 */
class MovingAverage {
   public:
    MovingAverage(int time_window);
    ~MovingAverage();
    /**
     * Adds a trade to the moving average, updating the average value accordingly.
     * @param trade The trade data to be added.
     */
    void addTrade(const finnhub::TradeData &trade);
    /**
     * Retrieves the moving average of the past completed minutes that were specified on construction.
     * @returns The calculated moving average value.
     */
    finnhub::StaticAverage getStatic();

   private:
    void prune(int64_t threshold_timestamp_ms);

   private:
    std::unordered_map<int64_t, finnhub::SimpleAverage> m_per_min_averages;
    const int m_minutes_to_store;
    int64_t m_last_updated;
    int64_t m_sum = 0;
    int64_t m_count = 0;
    mutable std::mutex m_mutex;
};
}  // namespace finnhub

#endif  // FINNHUB_DATA_HPP
