#ifndef TIMED_WORKER_HPP
#define TIMED_WORKER_HPP

#include "atomic"
#include "condition_variable"
#include "functional"
#include "mutex"
#include "thread"

/**
 * @class TimedWorker
 * @brief A class that represents a worker that performs a task at regular intervals,
 *        waking up every nth exact minute to execute a user-specified function.
 *
 * TimedWorker is designed to execute a user-defined function (work) at precise intervals,
 * specifically at every nth minute when the time is in the format HH:MM:00. The worker
 * remains idle and wakes up only when the system time reaches the start of an exact minute.
 * This ensures that tasks are executed at fixed times, such as 12:00, 12:05, 12:10, etc.,
 * depending on the interval provided.
 *
 * Constructor Parameters:
 *  @param timer_minutes: The interval, in minutes, at which the TimedWorker will perform the work.
 *  @param work: The function to be executed every nth minute.
 **/
class TimedWorker {
   public:
    TimedWorker(unsigned int timer_minutes, std::function<void()> work);
    ~TimedWorker();
    /**
     * Starts the worker and its thread. Is automatically called on object construction.
     */
    void start();
    /**
     * Stops the worker and its thread. Is automatically called on object destruction.
     */
    void stop();
    /**
     * Bounds the worker to a specific physical core.
     * @param core The core to which to bind the worker to. Uses 0-based indexing.
     */
    void setAffinity(unsigned int core);

   private:
    void timedLoop();

   private:
    // A flag that indicates the worker's running state.
    std::atomic<bool> m_stop_flag{true};
    // Every how many minutes the worker will wake up and perform work
    unsigned int m_timer_minutes;
    // The worker's thread
    std::thread m_thread;
    // The function to be performed on every wakeup
    std::function<void()> m_work;
    // A mutex to be used with `m_cv` in order wake up the worker while sleeping.
    mutable std::mutex m_mutex;
    // A condition variable in order to be able to wake the worker before its set timer.
    std::condition_variable m_cv;
};

#endif  // TIMED_WORKER_HPP