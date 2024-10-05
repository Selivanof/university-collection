#include "finnhub_data.hpp"

#include "logger.hpp"

using finnhub::TradeData;

void finnhub::SimpleAverage::addTrade(const finnhub::TradeData &trade) {
    // Update sum and count (weighted average)
    int64_t int_price = static_cast<int64_t>(trade.price * finnhub::FLOAT_TO_INT_FACTOR);
    int64_t int_volume = static_cast<int64_t>(trade.volume * finnhub::FLOAT_TO_INT_FACTOR);
    sum += ((int_price * int_volume) / finnhub::FLOAT_TO_INT_FACTOR);
    count += int_volume;
}

void finnhub::Candlestick::addTrade(const finnhub::TradeData &trade) {
    int64_t int_volume = static_cast<int64_t>(trade.volume * finnhub::FLOAT_TO_INT_FACTOR);
    if (trade.timestamp >= close_timestamp) {
        close_price = trade.price;
        close_timestamp = trade.timestamp;
    }
    if (trade.timestamp < open_timestamp) {
        open_price = trade.price;
        open_timestamp = trade.timestamp;
    }
    if (trade.price > max_price) {
        max_price = trade.price;
    }
    if (trade.price < min_price) {
        min_price = trade.price;
    }
    volume += int_volume;
}

finnhub::StaticCandlestick finnhub::Candlestick::getStatic() const {
    return std::move(StaticCandlestick(open_price, close_price, min_price, max_price,
                                       static_cast<double>(volume) / finnhub::FLOAT_TO_INT_FACTOR));
}

finnhub::CandlestickManager::CandlestickManager(int time_window) : m_minutes_to_store(time_window) {}

finnhub::CandlestickManager::~CandlestickManager() {}

void finnhub::CandlestickManager::addTrade(const finnhub::TradeData &trade) {
    std::lock_guard<std::mutex> lock(m_mutex);

    int64_t current_minute_timestamp_ms = timestamps::minute_floor_ms(timestamps::current<std::chrono::milliseconds>());
    int64_t min_acceptable_timestamp_ms = current_minute_timestamp_ms - (MS_IN_MIN * (m_minutes_to_store - 1));
    // Ignore trades that are too old
    if (trade.timestamp < min_acceptable_timestamp_ms) {
        return;
    }
    int64_t trade_minute_timestamp_ms = timestamps::minute_floor_ms(trade.timestamp);
    m_candlesticks[trade_minute_timestamp_ms].addTrade(trade);
    // If there are more candlesticks than it is supposed to hold (guaranteed to be only 1 more) since it is check on
    // every addition
    if (m_candlesticks.size() > (m_minutes_to_store)) {
        for (auto it = m_candlesticks.begin(); it != m_candlesticks.end();) {
            finnhub::Candlestick &candlestick = it->second;
            if (timestamps::minute_floor_ms(candlestick.close_timestamp) < min_acceptable_timestamp_ms) {
                it = m_candlesticks.erase(it);
                break;  // We can break since we know there can only be 1 excess candlestick
            } else {
                ++it;
            }
        }
    }
}

finnhub::StaticCandlestick finnhub::CandlestickManager::getCandlestick(int64_t timestamp_ms) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto requested_candlestick = m_candlesticks.find(timestamp_ms);
    if (requested_candlestick == m_candlesticks.end()) {
        return std::move(StaticCandlestick(0, 0, 0, 0, 0));
    } else {
        return std::move(requested_candlestick->second.getStatic());
    }
}

finnhub::MovingAverage::MovingAverage(int time_window_minutes) : m_minutes_to_store(time_window_minutes + 1) {}

finnhub::MovingAverage::~MovingAverage() {}

void finnhub::MovingAverage::addTrade(const finnhub::TradeData &trade) {
    std::lock_guard<std::mutex> lock(m_mutex);
    int64_t int_price = static_cast<int64_t>(trade.price * finnhub::FLOAT_TO_INT_FACTOR);
    int64_t int_volume = static_cast<int64_t>(trade.volume * finnhub::FLOAT_TO_INT_FACTOR);
    int64_t current_minute_timestamp_ms = timestamps::minute_floor_ms(timestamps::current<std::chrono::milliseconds>());
    int64_t min_acceptable_timestamp_ms = current_minute_timestamp_ms - (MS_IN_MIN * (m_minutes_to_store - 1));
    // Ignore trades that are too old
    if (trade.timestamp < min_acceptable_timestamp_ms) {
        return;
    }
    // Update statistics
    int64_t trade_minute_timestamp_ms = timestamps::minute_floor_ms(trade.timestamp);

    m_sum += ((int_price * int_volume) / finnhub::FLOAT_TO_INT_FACTOR);
    // Since each int is float*factor, the result of int*int is float*float*factor*factor. We need to scale down once.
    m_count += int_volume;
    m_per_min_averages[trade_minute_timestamp_ms].addTrade(trade);
    // Remove old containers
    if (m_per_min_averages.size() > m_minutes_to_store) {
        prune(min_acceptable_timestamp_ms);
    }
}

finnhub::StaticAverage finnhub::MovingAverage::getStatic() {
    std::lock_guard<std::mutex> lock(m_mutex);
    int64_t current_minute_timestamp_ms = timestamps::minute_floor_ms(timestamps::current<std::chrono::milliseconds>());

    // We want to ignore the current minute's contribution towards the moving average.
    auto &current_average = m_per_min_averages[current_minute_timestamp_ms];
    if (m_per_min_averages.size() > m_minutes_to_store) {
        int64_t min_acceptable_timestamp_ms = current_minute_timestamp_ms - (MS_IN_MIN * (m_minutes_to_store - 1));
        prune(min_acceptable_timestamp_ms);
    }
    double effective_sum = static_cast<double>(m_sum - current_average.sum) / finnhub::FLOAT_TO_INT_FACTOR;
    double effective_count = static_cast<double>(m_count - current_average.count) / finnhub::FLOAT_TO_INT_FACTOR;
    double effective_avg = effective_count == 0 ? 0 : effective_sum / effective_count;

    return std::move(finnhub::StaticAverage(effective_sum, effective_count, effective_avg));
}

/**
 * Prunes outdated trade data based on a timestamp threshold. Only 1 item is remove with each called, since there can
 * only be one excess container at any given time. This is ensured by checking the number of stored averages with every
 * addition.
 * @param threshold_timestamp_ms The timestamp, in milliseconds, below which trade data should be pruned.
 */
void finnhub::MovingAverage::prune(int64_t threshold_timestamp_ms) {
    for (auto it = m_per_min_averages.begin(); it != m_per_min_averages.end();) {
        int64_t container_timestamp = it->first;
        if (container_timestamp < threshold_timestamp_ms) {
            m_sum -= it->second.sum;
            m_count -= it->second.count;
            it = m_per_min_averages.erase(it);
        } else {
            ++it;
        }
    }
}