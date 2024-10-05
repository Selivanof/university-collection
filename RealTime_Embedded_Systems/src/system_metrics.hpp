#ifndef SYSTEM_METRICS_HPP
#define SYSTEM_METRICS_HPP

#include <fmt/format.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#include <thread>

#include "logger.hpp"
#include "timestamps.hpp"

/**
 * @brief Calculates the normalized cpu usage percentage
 * @param elapsed_time The total runtime of the program (system time)
 * @param usage An `rusage` struct with the disired cpu time
 */
inline double get_cpu_usage(double elapsed_time, rusage &usage) {
    double userTime = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
    double systemTime = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;

    double totalCPUTime = userTime + systemTime;
    int numCores = std::thread::hardware_concurrency();

    return (totalCPUTime / elapsed_time) * 100.0 / numCores;
}

/**
 * @brief Calculates the elapsed time based on 2 `timeval` structs
 * @param start The `timeval` obtained at the start of the duration
 * @param usage he `timeval` obtained at the end of the duration
 */
inline double get_elapsed_time(timeval start, timeval end) {
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    return seconds + microseconds / 1e6;
}

/**
 * @brief Writes metrics to a `system_metrics.csv` file
 * @param runtime The total runtime of the program (in minutes)
 * @param cpu_usage The cpu usage percentage
 * @param max_rss The maximim resident set size
 */
inline void write_metrics(double runtime, double cpu_usage, long max_rss) {
    std::ofstream metrics_file;
    metrics_file.rdbuf()->pubsetbuf(0, 0);
    metrics_file.open("system_metrics.csv", std::ios::app);
    if (metrics_file.is_open()) {
        std::string metrics_string =
            fmt::format("{},{},{},{}\n", timestamps::current<std::chrono::milliseconds>(), runtime, cpu_usage, max_rss);
        metrics_file.write(metrics_string.c_str(), metrics_string.size());
        metrics_file.close();
    } else {
        LOG_ERROR("Failed to open system_metrics.csv\n");
    }
}

/**
 * @brief Calculates and write the cpu_usage and max_rss statistics to a `system_metrics.csv` file
 * @param start The `timeval` obtained at the start of the program
 * @param usage he `timeval` obtained at the end of the program
 */
inline void record_system_metrics(timeval start, timeval end) {
    gettimeofday(&end, nullptr);
    rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    long max_rss = usage.ru_maxrss;                      // Max RSS in kilobytes
    double elapsed_time = get_elapsed_time(start, end);  // Total time in seconds
    long runtime_minutes = static_cast<long>(elapsed_time / (60));
    double cpu_usage = get_cpu_usage(elapsed_time, usage);
    write_metrics(runtime_minutes, cpu_usage, max_rss);
}

#endif  // SYSTEM_METRICS_HPP