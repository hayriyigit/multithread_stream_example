#pragma once
#include "condition_variable"
#include "queue"

/**
 * @class ThreadSafeQueue
 *
 * @brief  A thread-safe queue that blocks on pop until items are available.
 *
 * This queue allows multiple producer and consumer threads to safely push
 * and pop items of type T. The pop() method will block until the queue is
 * not empty. An additional callback (wait_callback) can be registered to
 * be notified when waiting begins and ends.
 *
 * @tparam T The type of elements stored in the queue.
 */
template <typename T>
class ThreadSafeQueue {
   public:
    /**
     * @brief Enqueue an item into the queue.
     *
     * Adds the given item to the back of the queue and notifies one waiting
     * consumer.
     *
     * @param item  The item to add to the queue.
     */
    void push(T item);

    /**
     * @brief Dequeue an item, blocking until one is available.
     *
     * If the queue is empty, this call will block until an item pushed to the
     * queue. During the wait, if a wait-callback is registered, it will be
     * invoked with `true` which indicates the pipeline is in idle state. Once
     * an item becomes available, the callback is invoked again with `false`,
     * indicates the pipeline is running.
     *
     * @return The next item from the queue.
     */
    T pop();

    /**
     * @brief Register a callback to be notified of pop() wait events.
     *
     * The callback, if set, will be called with:
     *   - `true`  when pop() finds the queue empty and begins waiting
     *   - `false` when pop() unblocks because an item was pushed
     *
     * @param callback  Function to call on wait start/end.
     */
    void set_wait_callback(std::function<void(bool)> callback);

   private:
    std::mutex mtx;
    std::queue<T> q;
    std::condition_variable cv;
    std::function<void(bool)> wait_callback;
};

#include "threadsafequeue.tpp"
