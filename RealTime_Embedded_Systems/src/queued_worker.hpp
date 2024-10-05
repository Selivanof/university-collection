#ifndef QUEUED_WORKER_HPP
#define QUEUED_WORKER_HPP

#include <memory>
#include <queue>
#include <thread>
#include <vector>

#include "finnhub_data.hpp"
#include "thread_safe_queue.hpp"

/**
 * @class QueuedWorker
 * @brief A class template that represents a worker which continuously processes
 *        tasks from a thread-safe queue in a separate thread.
 *
 * QueuedWorker is responsible for consuming elements from a provided ThreadSafeQueue
 * and applying a user-specified function (work) on each element. It operates in a
 * separate thread, running in a loop that waits for an element from the queue, processes
 * it, and repeats. The worker starts and stops automatically upon construction and destruction
 * of the object. If desired, it can be started and stopped explicitly.
 *
 * Template Parameters:
 *  - T: The type of elements to be processed, which are dequeued from the ThreadSafeQueue.
 * Constructor Parameters:
 *  @param queue: The ThreadSafeQueue from which to dequeue elements.
 *  @param work: The function to be performed on each element.
 */
template <typename T, typename Work>
class QueuedWorker {
   public:
    QueuedWorker(std::shared_ptr<ThreadSafeQueue<T>> queue, Work work);
    ~QueuedWorker();
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
    void loop();

   private:
    // A flag that indicates the worker's running state.
    std::atomic<bool> m_stop_flag{true};
    // The queue from which the worker will remove elements to work on.
    std::shared_ptr<ThreadSafeQueue<T>> m_queue;
    // The worker's thread.
    std::thread m_thread;
    // The function that will be applied on each element that is removed from `m_queue`.
    Work m_work;
};

#include "queued_worker.ipp"

#endif  // QUEUED_WORKER_HPP