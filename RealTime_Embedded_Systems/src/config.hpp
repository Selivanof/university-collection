#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstdint>
#include <set>
#include <string>
#include <vector>

// Forward declarations
namespace simdjson {
namespace dom {
class array;
}  // namespace dom
}  // namespace simdjson

/**
 * @class Config
 * @brief Singleton class for managing configuration settings.
 *
 * The Config class provides a centralized location for managing and accessing
 * configuration settings used throughout the application. It follows the singleton
 * design pattern to ensure that only one instance of the class exists. Configuration
 * parameters are loaded from a JSON file and validated to ensure they fall within
 * acceptable ranges.
 *
 * This class does not use any thread-safety mechanisms. Should be only used by multiple threads
 * if it is certain that loadFromFile() will NOT be called.
 */
class Config {
   public:
    /**
     * Retrieves the singleton instance of the Config class.
     * @return Reference to the single instance of Config.
     */
    static Config &getInstance();

    // Duration in seconds to wait on an idle websocket before sending a ping.
    int pingIntervalSec;
    // Timeout duration in seconds for pong responses.
    int pongTimeoutSec;
    // Initial delay in milliseconds before attempting a reconnect.
    int initialReconnectionDelayMs;
    // Maximum delay in milliseconds between reconnection attempts.
    int maxReconnectionDelayMs;
    // Maximum number of consecutive connection attempts allowed.
    int maxReconnectionAttempts;

    // Time window in minutes for calculating the moving average.
    int movingAverageTimeWindowMinutes;

    // Number of recording worker threads.
    size_t recordingWorkersCount;
    //  Number of statistics worker threads.
    size_t statisticsWorkersCount;
    // Number of exporting worker threads.
    size_t exportingWorkersCount;

    // Vector of CPU affinities for recording worker threads.
    std::vector<unsigned int> recordingAffinities;
    // Vector of CPU affinities for statistics worker threads.
    std::vector<unsigned int> statisticsAffinities;
    // Vector of CPU affinities for exporting worker threads.
    std::vector<unsigned int> exportingAffinities;

    //  Vector of symbols to monitor.
    std::vector<std::string> symbols;

    /**
     * Loads configuration values from a JSON file.
     * @param filename The name of the JSON file.
     * @return True if loading was successful, false otherwise.
     */
    bool loadFromFile(const std::string &filename);
    /**
     * Prints the current configuration settings.
     */
    void printConfig() const;

   private:
    Config();
    ~Config();
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;
    bool createConfigFile(const std::string &filename) const;
    void sanitize();
    std::vector<std::string> getStringVectorFromJson(const simdjson::dom::array &json_array);
    std::vector<unsigned int> getAffinityVectorFromJson(const simdjson::dom::array &json_array);
    void printAffinities(const std::vector<unsigned int> &affinities) const;

   private:
    static Config *instance;
};

#endif  // CONFIG_HPP
