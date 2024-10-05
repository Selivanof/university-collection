#ifndef TASKS_HPP
#define TASKS_HPP

#include <fstream>
#include <memory>
#include <unordered_map>
#include <vector>

// Forward declarations
namespace finnhub {
struct TradeData;
struct Candlestick;
struct MovingAverage;
class CandlestickManager;
}  // namespace finnhub
template <typename T>
struct ThreadedContainer;
template <typename T>
class ThreadSafeQueue;
struct SymbolFiles;

/**
 * @struct TaskQueues
 * @brief A structure that holds the queues on which workers are waiting for a specific symbol
 *
 * recordingQueue: The queue from which dequeued trades are recorded to files
 * statisticsQueue: The queue from which dequeued trades are used for updating the statistics
 *  */
struct TaskQueues {
    std::shared_ptr<ThreadSafeQueue<finnhub::TradeData>> recordingQueue;
    std::shared_ptr<ThreadSafeQueue<finnhub::TradeData>> statisticsQueue;
};

/**
 * @struct SymbolFiles
 * @brief A structure that holds `ofstream`s to a symbol's files
 *
 * recordingFile: File for realtime trade recording
 * statisticsFile: File for per-minute statistics calculation
 *  */
struct SymbolFiles {
    std::ofstream recordingFile;
    std::ofstream statisticsFile;
};

// Aliases
using SymbolToCandlesticksMap = std::unordered_map<std::string, std::unique_ptr<finnhub::CandlestickManager>>;
using SymbolToMovingAverageMap = std::unordered_map<std::string, std::unique_ptr<finnhub::MovingAverage>>;
using TradeQueueVector = std::vector<std::shared_ptr<ThreadSafeQueue<finnhub::TradeData>>>;
using SymbolToTaskQueuesMap = std::unordered_map<std::string, TaskQueues>;

/**
 * @brief Exports statistical data for a list of assigned symbols to their respective files.
 *
 * This function generates and writes statistical data for each symbol to a file. It retrieves the current candlestick
 * and moving average values, formats them into a CSV-like string, and writes this data to the corresponding statistics
 * file.
 *
 * @param m_assigned_symbols: A vector of symbols for which statistics will be exported.
 * @param file_map: A map of symbol names to `SymbolFiles` objects containing the file streams for writing statistics.
 * @param m_candlestick_map: A map of symbols to their associated `CandlestickManager`.
 * @param m_moving_average_map: A map of symbols to their associated `MovingAverage`.
 */
void exportStatistics(const std::vector<std::string> &m_assigned_symbols,
                      std::unordered_map<std::string, SymbolFiles> &file_map,
                      const SymbolToCandlesticksMap &m_candlestick_map,
                      const SymbolToMovingAverageMap &m_moving_average_map);

/**
 * @brief Records trade data to a file.
 *
 * This function formats the trade data into a CSV-like string and writes it to the appropriate recording file for the
 * symbol.
 *
 * @param file_map: A map of symbol names to `SymbolFiles` objects containing the file streams for recording trades.
 * @param trade: The trade data to be recorded.
 */
void recordTrade(std::unordered_map<std::string, SymbolFiles> &file_map, const finnhub::TradeData &trade);

/**
 * @brief Updates statistics with the latest trade data.
 *
 * This function processes incoming trade data by adding it to the relevant `CandlestickManager` and `MovingAverage`
 * objects. It updates the internal state of these objects with the new trade information.
 *
 * @param m_candlestick_map: A map of symbols to their associated `CandlestickManager`.
 * @param m_moving_average_map: A map of symbols to their associated `MovingAverage`.
 * @param trade: The trade data to be processed and added to the candlestick and moving average objects.
 */
void updateStatistics(const SymbolToCandlesticksMap &candlestick_map,
                      const SymbolToMovingAverageMap &moving_average_map, const finnhub::TradeData &trade);

void print_delay_memory();
#endif  // TASKS_HPP