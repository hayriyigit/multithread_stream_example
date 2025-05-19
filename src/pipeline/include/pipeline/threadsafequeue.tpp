#include "threadsafequeue.h"

template <typename T>
void ThreadSafeQueue<T>::push(T item) {
    std::unique_lock<std::mutex> lock(mtx);
    q.push(std::move(item));
    cv.notify_one();
};

template <typename T>
T ThreadSafeQueue<T>::pop() {
    std::unique_lock<std::mutex> lock(mtx);
    if (q.empty() && wait_callback) {
        wait_callback(true);
    };

    cv.wait(lock, [this] { return !q.empty(); });

    if (wait_callback) {
        wait_callback(false);
    };

    T item = std::move(q.front());
    q.pop();
    return item;
};

template <typename T>
void ThreadSafeQueue<T>::set_wait_callback(std::function<void(bool)> callback) {
    wait_callback = callback;
};
