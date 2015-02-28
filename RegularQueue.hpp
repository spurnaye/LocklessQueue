#ifndef REGULAR_QUEUE_HPP
#define REGULAR_QUEUE_HPP

#include <vector>
#include <mutex>
#include "Queue.hpp"

template <class T>
class RegularQueue : public Queue<T> {
    unsigned long count;
    unsigned long empty;
    std::vector<T> items;
    size_t head;
    size_t tail;
    std::mutex lock;
    std::condition_variable dequeue_event;
    std::condition_variable enqueue_event;
public:
    RegularQueue(size_t n);
    void enqueue(T x);
    T dequeue();
};

#include "RegularQueue.cpp"

#endif
