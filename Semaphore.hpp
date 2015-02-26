#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP

#include <mutex>
#include <condition_variable>
#include <atomic>

class Semaphore {
    std::mutex sem_lock;
    std::condition_variable cv;
    unsigned long count;
    std::atomic<bool> is_shutdown;
    
public:
    Semaphore(unsigned long initial_capacity);
    virtual ~Semaphore();
    void post();
    void wait();
    void shutdown();
};

#endif
