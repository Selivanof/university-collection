#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
struct ThreadedContainer {
    T container;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
};

/**
 * A basic thread safe queue that can be used in a simple producer-consumer enviroment
 */
template <typename T>
class ThreadSafeQueue {
   public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() = default;

    void enqueue(T item);
    /**
     * Removes and returns the element at the front of the queue.
     * If the queue is empty, the current thread will block until an element is added to the queue.
     * @returns T: The element that was removed from the front of the queue.
     */
    T dequeue();
    /**
     * Removes and returns the element at the front of the queue.
     *
     * If the queue is empty, the current thread will block until an element is added to the queue or
     * the condition is set to true. Note that `wakeConsumers` must be called after changing the condition
     * in order to wake up the affected consumers. In that case, a default constructed `T` object will be
     returned.

     * @param condition A reference to an `std::atomic<bool>` that determines whether to force wake the
     consumer.
     * @returns T: The element that was removed from the front of the queue.
     */
    T dequeueConditional(std::atomic<bool> &condition);
    /**
     * Retrieves the element at the front of the queue without removing it.
     *
     * @returns: A const reference to the element at the front of the queue.
     */
    const T &front() const;
    /**
     * Checks if the queue is empty.
     *
     * @returns: True if the queue is empty, false otherwise.
     */
    bool isEmpty() const;
    /**
     * Wakes up any blocked consumers waiting for the queue to be populated. Note that `dequeueConditional` must have
     * been used by the consumer for this function to have an effect on consumers waiting on an empty queue. Consumers
     * that are waiting on a non-empty queue will be automatically woken up as soon as new elements are added.
     */
    void wakeConsumers();

   private:
    // The underlying queue
    std::queue<T> m_queue;
    // The mutex used to ensure thread-safety
    mutable std::mutex m_mutex;
    // The condition variable used to ensure thread-safety
    std::condition_variable m_cv;
};

#endif  // THREAD_SAFE_QUEUE_HPP