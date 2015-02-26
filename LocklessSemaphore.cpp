#include "LocklessSemaphore.hpp"
#include <iostream>

LocklessSemaphore::LocklessSemaphore(int initial_capacity)
    : capacity(initial_capacity), sem(0) {}

LocklessSemaphore::~LocklessSemaphore() {

}

void LocklessSemaphore::shutdown() {
    sem.shutdown();
}

long LocklessSemaphore::current() {
    return capacity.load();
}

void LocklessSemaphore::post() {
    long cap = capacity++;    
    if(cap < 0) {
//        std::cout << "Posting." << std::endl;
        sem.post();
    }
}

long LocklessSemaphore::wait() {
    long cap = capacity--;
    if(cap <= 0) {
//        std::cout << "Waiting." << std::endl;
        sem.wait();
    }
    return cap - 1;
}

bool LocklessSemaphore::try_wait() {
    long cap;
    do {
        cap = capacity.load();
        if(cap <= 0) {
            return false;
        }
    }while(!atomic_compare_exchange_weak(&capacity, &cap, cap - 1));
    //std::cout << "Doing try_wait." << std::endl;
    return true;
}
