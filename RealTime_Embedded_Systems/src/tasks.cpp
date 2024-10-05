#include "tasks.hpp"

#include <fmt/format.h>

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <unordered_map>
#include <vector>

#include "config.hpp"
#include "finnhub_data.hpp"
#include "logger.hpp"
#include "thread_safe_queue.hpp"

void exportStatistics(const std::vector<std::string> &m_assigned_symbols,
                      std::unordered_map<std::string, SymbolFiles> &file_map,
                      const SymbolToCandlesticksMap &m_candlestick_map,
                      const SymbolToMovingAverageMap &m_moving_average_map) {
    // Referance timestamps
    int64_t timer_wake_timestamp_ms = timestamps::minute_floor_ms(timestamps::current<std::chrono::milliseconds>());
    int64_t previous_minute_timestamp_ms = timer_wake_timestamp_ms - MS_IN_MIN;

    for (const auto &symbol : m_assigned_symbols) {
        // Get output file
        std::ofstream &statistics_file = file_map[symbol].statisticsFile;
        // Get average and candlestick containers for current symbol
        auto &moving_average = m_moving_average_map.at(symbol);
        auto &recent_candlesticks = m_candlestick_map.at(symbol);
        // Get the static candlestick for the previous minute
        finnhub::StaticCandlestick candlestick = recent_candlesticks->getCandlestick(previous_minute_timestamp_ms);
        int64_t timer_wakeup_us = timer_wake_timestamp_ms * US_IN_MS;
        finnhub::StaticAverage static_moving_average = moving_average->getStatic();
        // Format and write to file
        // Finnhub does not provide maximum number of precision digits. It seems it has a limit of 6 decimal places
        // (from tests), so we limit double to that
        std::string output =
            fmt::format("{},{},{:.6f},{:.6f},{:.6f},{:.6f},{:.6f},{:.8f},{:.6f}\n", timer_wakeup_us,
                        timestamps::current<std::chrono::microseconds>() - timer_wakeup_us, candlestick.open_price,
                        candlestick.close_price, candlestick.min_price, candlestick.max_price, candlestick.volume,
                        static_moving_average.value, static_moving_average.volume);
        statistics_file.write(output.c_str(), output.size());
    }
}

void updateStatistics(const SymbolToCandlesticksMap &candlestick_map,
                      const SymbolToMovingAverageMap &moving_average_map, const finnhub::TradeData &trade) {
    // Get average and candlestick containers for current trade's symbol
    auto &moving_average = moving_average_map.at(trade.symbol);
    auto &recent_candlesticks = candlestick_map.at(trade.symbol);
    // Add the the containers
    recent_candlesticks->addTrade(trade);
    moving_average->addTrade(trade);
}

void recordTrade(std::unordered_map<std::string, SymbolFiles> &file_map, const finnhub::TradeData &trade) {
    // Get output file
    std::ofstream &records_file = file_map[trade.symbol].recordingFile;
    int64_t trade_timestamp_us = trade.timestamp * US_IN_MS;
    // Format and write to file
    // Finnhub does not provide maximum number of precision digits. It seems it has a limit of 6 decimal places (from
    // tests), so we limit double to that
    std::string record = fmt::format(
        "{},{},{},{:.6f},{:.6f}\n", trade_timestamp_us, trade.arrival_timestamp - trade_timestamp_us,
        timestamps::current<std::chrono::microseconds>() - trade.arrival_timestamp, trade.volume, trade.price);
    records_file.write(record.c_str(), record.size());
}
