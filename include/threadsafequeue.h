#pragma once
#include "condition_variable"
#include "queue"

template<typename T>
class ThreadSafeQueue{
    public:
        void push(T item);
        T pop();
        void set_wait_callback(std::function<void(bool)> callback);
    private:
        std::mutex mtx;
        std::queue<T> q;
        std::condition_variable cv;
        std::function<void(bool)> wait_callback;
};

#include "threadsafequeue.tpp"