#include "logger.hpp"

template <typename T, typename Work>
QueuedWorker<T, Work>::QueuedWorker(std::shared_ptr<ThreadSafeQueue<T>> queue, Work work)
    : m_queue(queue), m_work(std::move(work)) {
    start();
}

template <typename T, typename Work>
QueuedWorker<T, Work>::~QueuedWorker() {
    stop();
}

template <typename T, typename Work>
void QueuedWorker<T, Work>::start() {
    // No need for a mutex, assuming stop and start will NOT be called at the same time from different threads
    if (!m_thread.joinable()) {
        m_stop_flag = false;
        m_thread = std::thread(&QueuedWorker::loop, this);
    }
}

template <typename T, typename Work>
void QueuedWorker<T, Work>::stop() {
    // No need for a mutex, assuming stop and start will NOT be called at the same time from different threads
    m_stop_flag = true;
    m_queue->wakeConsumers();
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

template <typename T, typename Work>
void QueuedWorker<T, Work>::loop() {
    while (1) {
        T item = m_queue->dequeueConditional(m_stop_flag);
        if (m_stop_flag) break;
        m_work(item);
    }
}

template <typename T, typename Work>
void QueuedWorker<T, Work>::setAffinity(unsigned int core) {
    pthread_t nativeHandle = m_thread.native_handle();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);

    int result = pthread_setaffinity_np(nativeHandle, sizeof(cpu_set_t), &cpuset);
    if (result != 0) {
        LOG_ERROR("Failed to set QueuedWorker's affinity.\n");
    } else {
        LOG_DEBUG("QueuedWorker bound to core %d.\n", core);
    }
}