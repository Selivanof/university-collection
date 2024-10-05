#include "thread_safe_queue.hpp"

#include "finnhub_data.hpp"

template <typename T>
void ThreadSafeQueue<T>::enqueue(T item) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(std::move(item));
    m_cv.notify_one();
}

template <typename T>
T ThreadSafeQueue<T>::dequeue() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [this]() { return !m_queue.empty(); });
    T item = std::move(m_queue.front());
    m_queue.pop();
    return item;
}

template <typename T>
T ThreadSafeQueue<T>::dequeueConditional(std::atomic<bool> &condition) {
    std::unique_lock<std::mutex> lock(m_mutex);
    // Should wake a consumer if the queue is not empty, or the exit condition has become true
    m_cv.wait(lock, [this, &condition]() { return !m_queue.empty() || condition.load(); });
    if (condition.load()) {
        return T();
    }
    T item = std::move(m_queue.front());
    m_queue.pop();
    return item;
}

template <typename T>
const T &ThreadSafeQueue<T>::front() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.front();
}

template <typename T>
bool ThreadSafeQueue<T>::isEmpty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

template <typename T>
void ThreadSafeQueue<T>::wakeConsumers() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cv.notify_all();
}

// Explicit instantiation of types used
template class ThreadSafeQueue<finnhub::TradeData>;
template class ThreadSafeQueue<std::vector<finnhub::TradeData>>;
