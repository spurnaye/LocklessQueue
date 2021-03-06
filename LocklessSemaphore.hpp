#ifndef LOCKLESS_SEMAPHORE_HPP
#define LOCKLESS_SEMAPHORE_HPP

#include <atomic>
#include "Semaphore.hpp"

class LocklessSemaphore {
    std::atomic<long> capacity;
    Semaphore sem;
    
public:
    LocklessSemaphore() : LocklessSemaphore(0) {}
    LocklessSemaphore(int initial_capacity);
    virtual ~LocklessSemaphore();
    long current();
    void post();
    long wait();
    bool try_wait();
    void shutdown();

};

#endif
