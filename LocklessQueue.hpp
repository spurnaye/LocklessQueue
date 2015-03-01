#ifndef LOCKLESS_QUEUE_HPP
#define LOCKLESS_QUEUE_HPP

#include <atomic>
#include <vector>
#include <mutex>
#include <utility>
#include <exception>
#include "LocklessSemaphore.hpp"
#include "Queue.hpp"
#include "CopyableAtomic.hpp"

template <class T>
class LocklessQueue : public Queue<T> {
    LocklessSemaphore count_sem;
    LocklessSemaphore empty_sem;
    std::vector<std::atomic<bool>> *full_signal;
    std::vector<T> items;
    std::atomic<size_t> capacity;
    std::atomic<size_t> head; // Items are queued on head. (back of vector)
    std::atomic<size_t> tail; // Items are dequeued off tail. (front of vector)
    size_t max_size;
    std::mutex expand_lock; // This is the lock for expansion.
                            // Only expansion requires the use of a lock.
    std::atomic<bool> is_shutdown;
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
