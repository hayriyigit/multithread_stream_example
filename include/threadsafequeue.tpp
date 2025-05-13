#include "threadsafequeue.h"

template <typename T>
void ThreadSafeQueue<T>::push(T item)
{
    std::unique_lock<std::mutex> lock(mtx);
    q.push(std::move(item));
    cv.notify_one();
};

template <typename T>
T ThreadSafeQueue<T>::pop()
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]
            { return !q.empty(); });
    T item = std::move(q.front());
    q.pop();
    return item;
};