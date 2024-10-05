// Config.cpp
#include "config.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <thread>
// Linux Headers
#include <limits.h>
#include <simdjson.h>
#include <unistd.h>

#include "logger.hpp"

// Linux Only
std::string nextToExecutable(const std::string &filename) {
    std::filesystem::path exePath;
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        exePath = std::filesystem::path(path).parent_path();
    }
    return (exePath / filename).string();
}

Config *Config::instance = nullptr;

Config &Config::getInstance() {
    if (instance == nullptr) {
        instance = new Config();
    }
    return *instance;
}

Config::Config()
    : pingIntervalSec(30),
      pongTimeoutSec(10),
      maxReconnectionDelayMs(60000),
      initialReconnectionDelayMs(10000),
      maxReconnectionAttempts(1000),
      movingAverageTimeWindowMinutes(15),
      recordingWorkersCount(1),
      statisticsWorkersCount(1),
      exportingWorkersCount(1),
      symbols{"BINANCE:BTCUSDT", "NVDA", "AAPL", "GOOGL", "MSFT"} {}

Config::~Config() {}

bool Config::loadFromFile(const std::string &filename) {
    std::ifstream file(nextToExecutable(filename));
    if (!file.is_open()) {
        LOG_WARNING("Config file not found. Creating default configuration file. \n");
        if (createConfigFile(filename)) {
            LOG_INFO("Default configuration file created. Please modify it as needed.\n");
        }
        return true;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    simdjson::dom::parser parser;
    simdjson::dom::element json = parser.parse(content);

    try {
        pingIntervalSec = json["pingIntervalSec"].get_int64();
        pongTimeoutSec = json["pongTimeoutSec"].get_int64();
        initialReconnectionDelayMs = json["initialReconnectionDelayMs"].get_int64();
        maxReconnectionDelayMs = json["maxReconnectionDelayMs"].get_int64();
        maxReconnectionAttempts = json["maxReconnectionAttempts"].get_int64();

        movingAverageTimeWindowMinutes = json["movingAverageTimeWindowMinutes"].get_int64();

        recordingWorkersCount = json["recordingWorkersCount"].get_int64();
        statisticsWorkersCount = json["statisticsWorkersCount"].get_int64();
        exportingWorkersCount = json["exportingWorkersCount"].get_int64();

        recordingAffinities = getAffinityVectorFromJson(json["recordingAffinities"]);
        statisticsAffinities = getAffinityVectorFromJson(json["statisticsAffinities"]);
        exportingAffinities = getAffinityVectorFromJson(json["exportingAffinities"]);

        symbols = getStringVectorFromJson(json["symbols"].get_array());
    } catch (const simdjson::simdjson_error &e) {
        LOG_ERROR("Invalid configuration file. Make sure all the necessary fields are present in the configuration.\n");
        LOG_ERROR("You can delete %s to automatically create a default configuration file on startup.\n",
                  nextToExecutable(filename).c_str());
        return false;
    }
    sanitize();
    return true;
}

std::vector<std::string> Config::getStringVectorFromJson(const simdjson::dom::array &json_array) {
    std::vector<std::string> vector;
    for (auto symbol : json_array) {
        vector.push_back(std::string{symbol.get_string().value()});
    }
    return vector;
}

// Reads an affinity vector from a json. The returned set is guaranteed to be valid for the current system.
std::vector<unsigned int> Config::getAffinityVectorFromJson(const simdjson::dom::array &json_array) {
    unsigned int cpu_count = std::thread::hardware_concurrency();
    std::set<unsigned int> set;
    for (auto item : json_array) {
        unsigned int affinity = item.get_uint64();
        if (affinity < cpu_count) set.insert(affinity);
    }
    return std::vector<unsigned int>(set.begin(), set.end());
}

void Config::sanitize() {
    pingIntervalSec = std::max(pingIntervalSec, 1);
    pongTimeoutSec = std::max(pongTimeoutSec, 1);
    initialReconnectionDelayMs = std::max(initialReconnectionDelayMs, 0);
    maxReconnectionDelayMs = std::max(maxReconnectionDelayMs, 0);
    maxReconnectionAttempts = std::max(maxReconnectionAttempts, 0);
    movingAverageTimeWindowMinutes = std::max(movingAverageTimeWindowMinutes, 1);
    recordingWorkersCount = std::clamp(recordingWorkersCount, size_t{1}, symbols.size());
    statisticsWorkersCount = std::clamp(statisticsWorkersCount, size_t{1}, symbols.size());
    exportingWorkersCount = std::clamp(exportingWorkersCount, size_t{1}, symbols.size());
}

void Config::printConfig() const {
    std::cout << "-------------------------------------------\n";
    std::cout << "           CURRENT CONFIGURATION\n";
    std::cout << "-------------------------------------------\n";
    std::cout << "Ping Interval                : " << pingIntervalSec << " sec" << "\n";
    std::cout << "Pong Timeout                 : " << pongTimeoutSec << " sec" << "\n";
    std::cout << "Initial Reconnection Delay   : " << initialReconnectionDelayMs << " ms" << "\n";
    std::cout << "Max Reconnection Delay       : " << maxReconnectionDelayMs << " ms" << "\n";
    std::cout << "Max Reconnection Attempts    : " << maxReconnectionAttempts << "\n";
    std::cout << "Moving Average Time Window   : " << movingAverageTimeWindowMinutes << " sec" << "\n";
    std::cout << "Recording Workers Count      : " << recordingWorkersCount << "\n";
    std::cout << "Statistics Workers Count     : " << statisticsWorkersCount << "\n";
    std::cout << "Exporting Workers Count      : " << exportingWorkersCount << "\n";
    std::cout << "Recording Affinities         : ";
    printAffinities(recordingAffinities);
    std::cout << "Statistics Affinities        : ";
    printAffinities(statisticsAffinities);
    std::cout << "Exporting Affinities         : ";
    printAffinities(exportingAffinities);
    std::cout << "Symbol Count                : " << symbols.size() << "\n";
    std::cout << "-------------------------------------------\n";
    std::cout << "\n";
}

void Config::printAffinities(const std::vector<unsigned int> &affinities) const {
    if (!affinities.empty()) {
        for (auto it = affinities.begin(); it != affinities.end(); ++it) {
            std::cout << *it;
            if (std::next(it) != affinities.end()) {
                std::cout << ",";
            }
        }
    } else {
        std::cout << "None";
    }
    std::cout << "\n";
}

bool Config::createConfigFile(const std::string &filename) const {
    auto filepath = nextToExecutable(filename);
    std::ofstream file(filepath);
    if (!file.is_open()) {
        LOG_ERROR("Could not create default config file %s.\n", filepath.c_str());
        return false;
    }
    file << "{\n"
         << "    \"pingIntervalSec\": " << pingIntervalSec << ",\n"
         << "    \"pongTimeoutSec\": " << pongTimeoutSec << ",\n"
         << "    \"initialReconnectionDelayMs\": " << initialReconnectionDelayMs << ",\n"
         << "    \"maxReconnectionDelayMs\": " << maxReconnectionDelayMs << ",\n"
         << "    \"maxReconnectionAttempts\": " << maxReconnectionAttempts << ",\n"
         << "    \"movingAverageTimeWindowMinutes\": " << movingAverageTimeWindowMinutes << ",\n"
         << "    \"recordingWorkersCount\": " << recordingWorkersCount << ",\n"
         << "    \"statisticsWorkersCount\": " << statisticsWorkersCount << ",\n"
         << "    \"exportingWorkersCount\": " << exportingWorkersCount << ",\n"
         << "    \"recordingAffinities\": [],\n"
         << "    \"statisticsAffinities\": [],\n"
         << "    \"exportingAffinities\": [],\n"
         << "    \"symbols\": [\n";
    for (size_t i = 0; i < symbols.size(); ++i) {
        file << "        \"" << symbols[i] << "\"";
        if (i < symbols.size() - 1) {
            file << ",";
        }
        file << "\n";
    }
    file << "    ]\n"
         << "}\n";
    file.close();

    return true;
}