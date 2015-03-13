#ifndef LOCKLESS_QUEUE_HPP
#define LOCKLESS_QUEUE_HPP

#include <atomic>
#include <vector>
#include <mutex>
#include <utility>
#include <exception>
#include "LocklessSemaphore.hpp"
#include "Queue.hpp"

template <class T>
class LocklessQueue : public Queue<T> {
    LocklessSemaphore count_sem;
    LocklessSemaphore empty_sem;
    std::atomic<bool> dequeue_atom;
    std::atomic<bool> enqueue_atom;
    std::vector<std::atomic<T>> *items;
    size_t capacity;
    size_t max_size;
    size_t head;     // Items queued on head. (back of vector)
    size_t tail;     // Items dequeued off tail. (front of vector)
    std::atomic<bool> is_shutdown;
    std::mutex expand_lock;

    std::pair<long, long> drain_semaphores();
    void fill_semaphores(std::pair<long, long> &p);
    
public:
    LocklessQueue(size_t n);
    LocklessQueue(size_t n, size_t max);
    virtual ~LocklessQueue();
    void enqueue(T x);
    T dequeue();
    void expand();
    void shutdown();
};

#include "LocklessQueue.cpp"

#endif
