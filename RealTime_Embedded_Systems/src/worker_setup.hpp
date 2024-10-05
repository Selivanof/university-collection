#ifndef WORKER_SETUP_HPP
#define WORKER_SETUP_HPP

#include <cstdio>
#include <set>
#include <vector>

#include "queued_worker.hpp"
#include "tasks.hpp"
#include "timed_worker.hpp"

/**
 * Creates a map of symbols to a managing container `T` from`finnhub_datd.h`
 * @param symbols: The symbols that will be mapped to containers
 * @returns A map of symbols to unique managing containers `T`
 */
template <typename T>
inline std::unordered_map<std::string, std::unique_ptr<T>> create_data_container_map(
    const std::vector<std::string> &symbols, int size) {
    std::unordered_map<std::string, std::unique_ptr<T>> map;
    for (const auto &symbol : symbols) {
        map.emplace(symbol, std::make_unique<T>(size));
    }
    return map;
}

/**
 * Creates a vector of ThreadSafeQueues for the specified worker count.
 * @param worker_count: The number of workers,
 * @returns A vector with one ThreadSafeQueue for each worker.
 */
template <typename T>
inline std::vector<std::shared_ptr<ThreadSafeQueue<T>>> create_worker_queues(size_t worker_count) {
    std::vector<std::shared_ptr<ThreadSafeQueue<T>>> queues;
    for (size_t i = 0; i < worker_count; ++i) {
        queues.push_back(std::make_shared<ThreadSafeQueue<T>>());
    }
    return queues;
}

/**
 * Distributes the given symbols equally for the specified amount of workers.
 * @param symbols: The symbols that will be distributed
 * @param worker_count: The number of workers
 * @returns A vector that contains a unique vector of symbols for each worker
 */
inline std::vector<std::vector<std::string>> assign_symbols(const std::vector<std::string> &symbols,
                                                            size_t worker_count) {
    std::vector<std::vector<std::string>> assigned_symbols(worker_count);
    for (size_t i = 0; i < symbols.size(); ++i) {
        assigned_symbols[i % worker_count].push_back(symbols[i]);
    }
    return assigned_symbols;
}

/**
 * Creates a map of symbols to TaskQueues.
 * The symbols will be equally distributed to the available recording and
 * statistics queues based on round-robin distribution.
 * @param symbols: The symbols that will be mapped
 * @param queues: The available queues to which symbols will be mapped
 * @returns A map of symbols to ThreadSafeQueues
 */
inline SymbolToTaskQueuesMap create_tasks_map(
    const std::vector<std::string> &symbols,
    const std::vector<std::shared_ptr<ThreadSafeQueue<finnhub::TradeData>>> &recording_queues,
    const std::vector<std::shared_ptr<ThreadSafeQueue<finnhub::TradeData>>> &statistics_queues) {
    SymbolToTaskQueuesMap map;
    int recording_queue_count = recording_queues.size();
    int statistic_queue_count = statistics_queues.size();
    for (size_t i = 0; i < symbols.size(); ++i) {
        const std::string &symbol = symbols[i];
        int recording_index = i % recording_queue_count;
        int statistics_index = i % statistic_queue_count;
        map[symbol] = TaskQueues{
            recordingQueue : recording_queues[recording_index],
            statisticsQueue : statistics_queues[statistics_index]
        };
    }
    return map;
}

/**
 * This function assigns CPU core affinities to a set of workers (either `QueuedWorker` or `TimedWorker`), distributing
 * them across the specified core affinities. If the number of affinities is less than the number of workers, the
 * affinities are applied in a round-robin fashion.
 *
 * @param workers: A vector of unique pointers to worker objects.
 * @param affinities: A vector of integers representing the CPU core IDs to which the workers will be assigned.
 */
template <typename WorkerType>
inline void set_worker_affinities(const std::vector<std::unique_ptr<WorkerType>> &workers,
                                  const std::vector<unsigned int> &affinities) {
    if (!affinities.empty()) {
        int affinities_count = affinities.size();
        for (int i = 0; i < workers.size(); ++i) {
            int affinity_index = i % affinities_count;
            int core_id = affinities[affinity_index];
            workers[i]->setAffinity(core_id);
        }
    }
}

/**
 *
 * This function generates a map of symbosl to `SymbolFiles` object containing file streams for trade records and
 * statistics associated with that symbol. These files are named using the symbol with appropriate suffixes
 * (`"_recording.csv"` and
 * `"_statistics.csv"`), and are opened in append mode. Buffered I/O is disabled for the file streams.
 *
 * @param symbols: A vector of strings where each string represents a symbol for which files will be created.
 * @returns: An map of symbols to a `SymbolFiles` object containing the file streams for that symbol.
 */
inline std::unordered_map<std::string, SymbolFiles> create_symbol_files_map(const std::vector<std::string> &symbols) {
    std::unordered_map<std::string, SymbolFiles> files_map;

    for (const auto &symbol : symbols) {
        // Open unbuffered recording file
        SymbolFiles symbol_files;
        std::string recording_filename = symbol + "_records.csv";
        symbol_files.recordingFile.rdbuf()->pubsetbuf(0, 0);
        symbol_files.recordingFile.open(recording_filename, std::ios::app);
        if (!symbol_files.recordingFile.is_open()) {
            LOG_ERROR("Failed to open %s", recording_filename.c_str());
        }
        // Open unbuffered statistics file
        std::string statistics_filename = symbol + "_statistics.csv";
        symbol_files.statisticsFile.rdbuf()->pubsetbuf(0, 0);
        symbol_files.statisticsFile.open(statistics_filename, std::ios::app);
        if (!symbol_files.statisticsFile.is_open()) {
            LOG_ERROR("Failed to open %s", statistics_filename.c_str());
        }
        files_map[symbol] = std::move(symbol_files);
    }
    return files_map;
}
#endif  // WORKER_SETUP_HPP