#include "finnhub_client.hpp"

#include <fmt/format.h>
#include <libwebsockets.h>
#include <simdjson.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <fstream>
#include <numeric>

#include "finnhub_data.hpp"
#include "logger.hpp"
#include "tasks.hpp"
#include "thread_safe_queue.hpp"
#include "timed_worker.hpp"

finnhub::WebsocketClient::WebsocketClient(const std::unordered_map<std::string, TaskQueues>& tasks_map)
    : m_tasks_map(tasks_map) {
    struct lws_protocols protocols[] = {
        {"finnhub-protocol", websocketCallback, 0, 1024}, {nullptr, nullptr, 0, 0}  // Terminator
    };
    lws_set_log_level(0, NULL);
    struct lws_context_creation_info info = {0};
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.ssl_ca_filepath = "/etc/ssl/certs/ca-certificates.crt";

    m_context = lws_create_context(&info);
    if (!m_context) {
        LOG_ERROR("Failed to create websocket context\n");
        return;
    }
    logToFile("Starting Client\n");
}

finnhub::WebsocketClient::~WebsocketClient() {
    if (m_context) {
        lws_context_destroy(m_context);
    }
    logToFile("Exiting Client\n");
}

void finnhub::WebsocketClient::run(const std::string& token, const std::vector<std::string>& symbols) {
    m_symbols = symbols;
    m_should_stop = false;
    connect(token);
    m_connection_attempts = 0;
    // Reconnection Mechanism
    while (!m_should_stop && m_connection_attempts < m_max_connection_attempts) {
        // Calculate exponential backoff delay and sleep before attempting a reconnect
        int reconnect_timeout_ms =
            std::min(m_initial_reconnection_delay * (1 << m_connection_attempts), m_max_reconnection_delay);
        LOG_INFO("Attempting to reconnect in %ds\n", reconnect_timeout_ms / 1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(reconnect_timeout_ms));
        LOG_INFO("Reconnecting...\n");
        connect(token);
    }
}

void finnhub::WebsocketClient::stop() {
    m_should_stop = true;
    reset();
}

// Closes any open connections and resets the client.
void finnhub::WebsocketClient::reset() {
    if (m_wsi) {
        // Checking if socket is valid (WebSocket still open)
        if (lws_get_socket_fd(m_wsi) >= 0) {
            lws_close_reason(m_wsi, LWS_CLOSE_STATUS_NORMAL, nullptr, 0);
        }
        m_wsi = nullptr;
    }
    m_subscribed = false;
    lws_cancel_service(m_context);
}

void finnhub::WebsocketClient::connect(const std::string& token) {
    ++m_connection_attempts;
    struct lws_client_connect_info ccinfo = {0};
    std::string url = "wss://ws.finnhub.io/?token=" + token;
    std::string path = "/?token=" + token;
    ccinfo.context = m_context;
    ccinfo.address = "ws.finnhub.io";
    ccinfo.port = 443;
    ccinfo.path = path.c_str();
    ccinfo.host = "ws.finnhub.io";
    ccinfo.origin = "origin";
    ccinfo.protocol = "finnhub-protocol";
    ccinfo.ssl_connection = LCCSCF_USE_SSL;
    ccinfo.userdata = static_cast<void*>(this);
    ccinfo.retry_and_idle_policy = &m_retry_policy;

    m_wsi = lws_client_connect_via_info(&ccinfo);
    if (!m_wsi) {
        LOG_ERROR("Failed to create client connection\n");
        return;
    }
    lws_set_timeout(m_wsi, PENDING_TIMEOUT_ESTABLISH_WITH_SERVER, 2);

    while (m_wsi && lws_service(m_context, 0) >= 0) {
        // Event loop
    }
}

int finnhub::WebsocketClient::websocketCallback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in,
                                                size_t len) {
    // Get pointer to calling WebsocketClient
    finnhub::WebsocketClient* client = static_cast<finnhub::WebsocketClient*>(user);
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            LOG_INFO("Connection Established\n");
            client->m_connection_attempts = 0;
            client->logToFile("Connected to finnhub\n");
            lws_callback_on_writable(wsi);
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE: {
            auto arrival_timestamp = timestamps::current<std::chrono::microseconds>();
            std::string message((char*)in, len);
            std::vector<finnhub::TradeData> received_data = std::move(parseMessage(message, arrival_timestamp));
            for (const auto& trade : received_data) {
                const TaskQueues& task_queues = client->m_tasks_map.at(trade.symbol);
                task_queues.recordingQueue->enqueue(trade);
                task_queues.statisticsQueue->enqueue(trade);
            }
            break;
        }
        case LWS_CALLBACK_CLIENT_WRITEABLE:
            if (!client->m_subscribed) {
                client->subscribeToSymbols();
                client->m_subscribed = true;

                LOG_INFO("Subscribed to symbols\n");
            }
            break;
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            LOG_ERROR("Connection Error\n");
            client->m_wsi = nullptr;
            client->reset();
            logToFile("Connection Error\n");
            break;
        case LWS_CALLBACK_CLIENT_CLOSED:
            LOG_INFO("Connection Closed\n");
            logToFile("Connection Closed\n");
            client->m_wsi = nullptr;
            client->reset();
            break;
        case LWS_CALLBACK_WSI_DESTROY:
            client->m_wsi = nullptr;
            break;
        default:
            break;
    }
    return 0;
}

// Configuration

void finnhub::WebsocketClient::setPingInterval(int interval_sec) {
    uint16_t previous_timeout = m_retry_policy.secs_since_valid_hangup - m_retry_policy.secs_since_valid_ping;
    m_retry_policy.secs_since_valid_ping = interval_sec;
    m_retry_policy.secs_since_valid_hangup = interval_sec + previous_timeout;
}

void finnhub::WebsocketClient::setPongTimeout(int timeout_sec) {
    m_retry_policy.secs_since_valid_hangup = m_retry_policy.secs_since_valid_ping + timeout_sec;
}

void finnhub::WebsocketClient::setReconnectionStrategy(int initial_delay, int max_delay, int max_attempts) {
    m_initial_reconnection_delay = initial_delay;
    m_max_reconnection_delay = max_delay;
    m_max_connection_attempts = max_attempts;
}

// Utility Functions

void finnhub::WebsocketClient::logToFile(const std::string& message) {
    std::ofstream log_file("connection.log", std::ios_base::app);
    if (!log_file.is_open()) {
        LOG_ERROR("Could not open client log file\n");
    } else {
        const char* timestamp = get_current_time();
        log_file << "[" << timestamp << "] " << message << std::flush;
    }
}

std::vector<finnhub::TradeData> finnhub::WebsocketClient::parseMessage(const std::string& message,
                                                                       int64_t arrival_timestamp_us) {
    std::vector<finnhub::TradeData> trades;
    simdjson::ondemand::parser parser;
    simdjson::padded_string padded_message(message);

    auto json = parser.iterate(padded_message);
    if (json.error()) {
        LOG_ERROR("Error parsing JSON message\n");
        std::cout << json.error();
        return trades;
    }

    auto type = json["type"].get_string();
    if (type.error() || type.value() != "trade") {
        if (type.value() == "error") {
            LOG_ERROR("%s\n", message.c_str());
            logToFile("Failed to subscribe to symbol\n");
        } else {
            LOG_DEBUG("Ignoring non-trade message\n");
        }
        return trades;
    }

    auto data_array = json["data"];
    if (data_array.error()) {
        LOG_ERROR("Error accessing 'data' array\n");
        return trades;
    }

    for (auto item : data_array) {
        try {
            finnhub::TradeData td;
            td.price = item["p"].get_double();
            td.symbol = item["s"].get_string().value();
            td.timestamp = item["t"].get_int64();
            td.volume = item["v"].get_double();
            td.arrival_timestamp = arrival_timestamp_us;

            if (td.volume == 0) td.volume = 1;

            trades.push_back(td);
        } catch (const simdjson::simdjson_error& e) {
            LOG_ERROR("Error processing item: %s\n", e.what());
            continue;
        }
    }
    LOG_DEBUG("Received message with %d TradeData objects!\n", trades.size());
    return trades;
}

void finnhub::WebsocketClient::subscribeToSymbols() {
    for (const std::string& symbol : m_symbols) {
        // Create the subscription message
        std::string subscribe_message = fmt::format("{{\"type\": \"subscribe\", \"symbol\": \"{}\"}}", symbol);
        // Allocate a buffer with LWS_PRE space and copy the subscription message
        size_t message_length = subscribe_message.size();
        char buf[LWS_PRE + message_length];
        memset(buf, 0, sizeof(buf));
        memcpy(&buf[LWS_PRE], subscribe_message.c_str(), message_length);
        // Send the message
        int write_result = lws_write(m_wsi, (unsigned char*)&buf[LWS_PRE], message_length, LWS_WRITE_TEXT);
        if (write_result < 0) {
            LOG_ERROR("Failed to write message to WebSocket: %s\n", subscribe_message.c_str());
            continue;
        }
    }
}