#pragma once
#include "condition_variable"
#include "queue"

template<typename T>
class ThreadSafeQueue{
    public:
        void push(T item);
        T pop();
    private:
        std::mutex mtx;
        std::queue<T> q;
        std::condition_variable cv;
};

#include "threadsafequeue.tpp"