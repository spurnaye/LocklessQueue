#include "Semaphore.hpp"
#include <iostream>
#include "ShutdownException.hpp"

Semaphore::Semaphore(unsigned long initial_capacity)
    : count(initial_capacity), is_shutdown(false) {}

Semaphore::~Semaphore() {
    shutdown();
}

void Semaphore::shutdown() {
    is_shutdown.store(true);
    cv.notify_all();
}

void Semaphore::post() {
    std::lock_guard<std::mutex> lock(sem_lock);
    count++;
    cv.notify_one();
}

void Semaphore::wait() {

    std::unique_lock<std::mutex> lock(sem_lock);
    
    while(!count) {
        cv.wait(lock);
        if(is_shutdown.load()) {
            throw ShutdownException();
        }
    }
    count--;
    lock.unlock();
}
