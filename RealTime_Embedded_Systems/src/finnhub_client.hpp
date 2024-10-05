
#include <libwebsockets.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <fstream>
#include <mutex>
#include <numeric>

#include "finnhub_data.hpp"
#include "logger.hpp"
#include "tasks.hpp"
#include "thread_safe_queue.hpp"
#include "timed_worker.hpp"

namespace finnhub {

/**
 * @class WebsocketClient
 * @brief A wrapper for wlibwebsockets' client for managing a WebSocket connection to Finnhub.
 *
 * This class handles the establishment and management of a WebSocket connection to Finnhub, including connecting to a
 * server, handling messages, and managing connection state. It supports subscribing to symbols, checking the connection
 * health using the ping-pong method and tracking unsuccessful connection attempts.
 *
 * This WebsocketClient is NOT thread safe. The only action that can be safely performed from other threads is stopping
 * the client.
 *
 * Constructor Parameters:
 *  @param tasks_map: A reference to a map of symbols to their TaskQueues.
 */
class WebsocketClient {
   public:
    /**
     * Constructor for WebsocketClient.
     * Initializes the WebSocket client with the provided task map.
     * @param tasks_map A reference to a map of symbols and their TaskQueues.
     */
    WebsocketClient(const std::unordered_map<std::string, TaskQueues>& tasks_map);

    /**
     * Destructor for WebsocketClient.
     * Cleans up the WebSocket connection and context.
     */
    ~WebsocketClient();

    /**
     * Closes the WebSocket connection and stops the client loop.
     */
    void stop();

    /**
     * Connects to finnhub with the given API token, subscribes to the provided symbols and starts the client loop.
     * @param token A valid Finnhub API token.
     * @param symbols A vector of symbols to subscribe to.
     */
    void run(const std::string& token, const std::vector<std::string>& symbols);

    /**
     * Sets the duration (seconds) for which the websocket will wait on an idle
     * connection before sending a ping message.
     * @param interval_sec The idle duration in seconds
     */
    void setPingInterval(int interval_sec);
    /**
     * Sets the timeout (seconds) for pong responses, after which the connection will be closed.
     * @param idle_sec The timeout in seconds
     */
    void setPongTimeout(int timeout_sec);
    /**
     * Sets the parameters for the client's reconnection strategy.
     * @param initial_delay The initial delay to wait before attempting to reconnect
     * @param max_delay The maximum delay to wait before attempting to reconnect
     * @param max_attempts The maximum attempts to reconnect
     */
    void setReconnectionStrategy(int initial_delay, int max_delay, int max_attempts);

   private:
    void connect(const std::string& token);
    static int websocketCallback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);
    static std::vector<finnhub::TradeData> parseMessage(const std::string& message, int64_t arrival_timestamp_us);
    static void logToFile(const std::string& message);
    void subscribeToSymbols();
    void reset();

   private:
    // Determines if the client should be runnint or not
    std::atomic<bool> m_should_stop = {true};
    // Libwebsockets context for managing WebSocket connections
    struct lws_context* m_context = nullptr;
    // The WebSocket instance
    struct lws* m_wsi = nullptr;
    // A map of symbols to task queues for processing trade data
    std::unordered_map<std::string, TaskQueues> m_tasks_map;
    // A list of symbols that the client is subscribed to. Should not be changed while the client is running.
    std::vector<std::string> m_symbols;
    // Whether the client is currently subscribed to symbols or not
    bool m_subscribed = false;
    // Libwebsockets retry and idle policy, used to implement ping-pong mechanism and detect dead connections.
    lws_retry_bo_t m_retry_policy;
    // The number of consecutive unsuccessful connection attempts
    unsigned int m_connection_attempts = 0;
    int m_max_connection_attempts = 0;
    int m_initial_reconnection_delay = 0;
    int m_max_reconnection_delay = 0;
};
}  // namespace finnhub