#include <pthread.h>

#include <csignal>
#include <cstdio>
#include <fstream>
#include <vector>

#include "config.hpp"
#include "finnhub_client.hpp"
#include "finnhub_data.hpp"
#include "logger.hpp"
#include "queued_worker.hpp"
#include "system_metrics.hpp"
#include "tasks.hpp"
#include "timed_worker.hpp"
#include "worker_setup.hpp"

using WorkerSymbols = std::vector<std::string>;
using TimedWorkerVector = std::vector<std::unique_ptr<TimedWorker> >;
template <typename T, typename Work>
using QueuedWorkerVector = std::vector<std::unique_ptr<QueuedWorker<T, Work> > >;

finnhub::WebsocketClient *clientPtr = nullptr;  // Global pointer to the client

void signalHandler(int signal) {
    if (signal == SIGINT) {
        LOG_INFO("RECEIVED SIGINT\n");
        if (clientPtr) {
            clientPtr->stop();
        }
    }
}

// Function to set the affinity of a thread to a specific core
// Linux only
int set_current_thread_affinity(int core) {
    pthread_t nativeHandle = pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);

    return pthread_setaffinity_np(nativeHandle, sizeof(cpu_set_t), &cpuset);
}

int main() {
    // For system usage stiatistics
    timeval start, end;
    gettimeofday(&start, nullptr);
    // Signal Handler for exiting using SIGINT
    std::signal(SIGINT, signalHandler);
    // Load configuration from file
    Config &config = Config::getInstance();
    if (!config.loadFromFile("websocket_config.json")) {
        return 1;
    }
    config.printConfig();

    const std::vector<std::string> symbols = config.symbols;

    // Obtain API Token
    const char *api_token = std::getenv("FINNHUB_API_TOKEN");
    if (!api_token) {
        LOG_ERROR("FINNHUB_API_TOKEN environment variable is not set.");
        return 1;
    }

    // Opening the necessary files once to significantly improve writing
    // performance. Files must NOT be deleted while the app is running. Could
    // add file rotation in the future if necessary.
    std::unordered_map<std::string, SymbolFiles> symbol_files_map = create_symbol_files_map(symbols);

    const size_t recording_worker_count = config.recordingWorkersCount;
    const size_t statistics_worker_count = config.statisticsWorkersCount;
    const size_t exporting_worker_count = config.exportingWorkersCount;

    // Create Maps for moving average and candlestick management
    SymbolToMovingAverageMap average_map =
        create_data_container_map<finnhub::MovingAverage>(symbols, config.movingAverageTimeWindowMinutes);
    SymbolToCandlesticksMap candlesticks_map = create_data_container_map<finnhub::CandlestickManager>(symbols, 2);

    // Create the queues from which the RealTime Workers will consume
    TradeQueueVector recording_queues = create_worker_queues<finnhub::TradeData>(recording_worker_count);
    TradeQueueVector statistics_queues = create_worker_queues<finnhub::TradeData>(statistics_worker_count);

    // Create a map of each symbol to its respective recording and statistics queues
    SymbolToTaskQueuesMap task_queue_map = create_tasks_map(symbols, recording_queues, statistics_queues);

    // Distribute symbols to workers
    std::vector<WorkerSymbols> recording_worker_symbols = assign_symbols(symbols, recording_worker_count);
    std::vector<WorkerSymbols> statistics_worker_symbols = assign_symbols(symbols, statistics_worker_count);
    std::vector<WorkerSymbols> exporting_worker_symbols = assign_symbols(symbols, exporting_worker_count);

    // Create Recording Workers
    auto recording_work = [&symbol_files_map](const finnhub::TradeData &trade) {
        recordTrade(symbol_files_map, trade);
    };
    QueuedWorkerVector<finnhub::TradeData, decltype(recording_work)> recording_workers;
    for (size_t i = 0; i < recording_worker_count; ++i) {
        recording_workers.push_back(std::make_unique<QueuedWorker<finnhub::TradeData, decltype(recording_work)> >(
            recording_queues[i], recording_work));
    }
    set_worker_affinities(recording_workers, config.recordingAffinities);

    // Create Statistics Workers
    auto statistics_work = [&candlesticks_map, &average_map](const finnhub::TradeData &trade) {
        updateStatistics(candlesticks_map, average_map, trade);
    };
    QueuedWorkerVector<finnhub::TradeData, decltype(statistics_work)> statistics_workers;
    for (size_t i = 0; i < statistics_worker_count; ++i) {
        statistics_workers.push_back(std::make_unique<QueuedWorker<finnhub::TradeData, decltype(statistics_work)> >(
            statistics_queues[i], statistics_work));
    }
    set_worker_affinities(statistics_workers, config.statisticsAffinities);

    // Create Exporting Workers
    TimedWorkerVector exporting_workers;
    for (size_t i = 0; i < exporting_worker_count; ++i) {
        exporting_workers.push_back(std::make_unique<TimedWorker>(
            1, [symbols = exporting_worker_symbols[i], &symbol_files_map, &candlesticks_map, &average_map] {
                exportStatistics(symbols, symbol_files_map, candlesticks_map, average_map);
            }));
    }
    set_worker_affinities(exporting_workers, config.exportingAffinities);

    // Set the main thread's affinity to CPU 0
    if (set_current_thread_affinity(0) != 0) {
        LOG_ERROR("Failed to bound main thread to core to core 0.\n");
    } else {
        LOG_DEBUG("Main thread bound to core 0.\n");
    }

    // Create the client and connect to finnhub
    finnhub::WebsocketClient client(task_queue_map);
    // Set the client pointer for the signal handler
    clientPtr = &client;
    // Configure the client
    client.setPingInterval(config.pingIntervalSec);
    client.setPongTimeout(config.pongTimeoutSec);
    client.setReconnectionStrategy(config.initialReconnectionDelayMs, config.maxReconnectionDelayMs,
                                   config.maxReconnectionAttempts);
    // Start the client
    client.run(api_token, config.symbols);
    // System usage statistics
    gettimeofday(&end, nullptr);
    record_system_metrics(start, end);
    LOG_INFO("Exiting...\n");
    return 0;
}
