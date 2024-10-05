#include "timed_worker.hpp"

#include "logger.hpp"

TimedWorker::TimedWorker(unsigned int timer_minutes, std::function<void()> work)
    : m_work(std::move(work)), m_timer_minutes(timer_minutes) {
    start();
}

TimedWorker::~TimedWorker() { stop(); }

void TimedWorker::start() {
    // No need for a mutex, assuming stop and start will NOT be called at the same time from different threads
    if (!m_thread.joinable()) {
        m_stop_flag = false;
        m_thread = std::thread(&TimedWorker::timedLoop, this);
    }
}

void TimedWorker::stop() {
    // No need for a mutex, assuming stop and start will NOT be called at the same time from different threads
    m_stop_flag = true;
    m_cv.notify_all();
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void TimedWorker::timedLoop() {
    while (1) {
        std::unique_lock<std::mutex> lock(m_mutex);

        std::chrono::system_clock::time_point current_minute =
            std::chrono::floor<std::chrono::minutes>(std::chrono::system_clock::now());
        std::chrono::system_clock::time_point timer_target = current_minute + std::chrono::minutes(m_timer_minutes);

        while (!m_stop_flag && (std::chrono::system_clock::now() < timer_target)) {
            m_cv.wait_until(lock, timer_target);
        }

        if (m_stop_flag) break;
        m_work();
    }
}

void TimedWorker::setAffinity(unsigned int core) {
    pthread_t nativeHandle = m_thread.native_handle();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);

    int result = pthread_setaffinity_np(nativeHandle, sizeof(cpu_set_t), &cpuset);
    if (result != 0) {
        LOG_ERROR("Failed to set TimedWorker's affinity.\n");
    } else {
        LOG_DEBUG("TimedWorker bound to core %d.\n", core);
    }
}