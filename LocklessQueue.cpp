//#include "LocklessQueue.hpp"
#include <iostream>
#include <algorithm>
#include "ShutdownException.hpp"

template <class T>
LocklessQueue<T>::LocklessQueue(size_t n)
    : LocklessQueue(n, std::numeric_limits<size_t>::max()) {}

template <class T>
LocklessQueue<T>::LocklessQueue(size_t n, size_t max)
    :items(n), full_signal(new std::vector<std::atomic<bool>>(n)),
     capacity(n), head(0), tail(0),
     count_sem(0), empty_sem(n), max_size(max),
     is_shutdown(false) {}

template <class T>
LocklessQueue<T>::~LocklessQueue() {
    shutdown();
}

template <class T>
void LocklessQueue<T>::shutdown() {
    count_sem.shutdown();
    empty_sem.shutdown();
    is_shutdown.store(true);
}

template <class T>
void LocklessQueue<T>::enqueue(T x) {
    //std::cout << "Empty: " << empty_sem.current() << std::endl;
    if(is_shutdown.load()) {
        throw ShutdownException();
    }
    if((capacity.load() >= max_size && (empty_sem.wait(),1))
       || empty_sem.try_wait()) {
        size_t h,t,c;
        do {
            h = std::atomic_load(&head);
            t = std::atomic_load(&tail);
            c = std::atomic_load(&capacity);
//            std::cout << "Spinning enqueue." << std::endl;
        }while(!std::atomic_compare_exchange_weak(&head, &h, (h + 1) % c));

//        std::cout << "Waiting for our spot to be empty." << std::endl;
        // Wait for our spot to be empty.
        while((*full_signal)[h].load());

        items[h] = x;
        
        (*full_signal)[h].store(true);
        count_sem.post(); 
//        std::cout << "Done Enqueueing." << std::endl;
    }
    else {
        expand();
        enqueue(x);
    }
}

template <class T>
T LocklessQueue<T>::dequeue() {
//    std::cout << "Full: " << count_sem.current() << std::endl;
    if(is_shutdown.load()) {
        throw ShutdownException();
    }
    long w = count_sem.wait();
    size_t h,t,c;
    do {
        h = std::atomic_load(&head);
        t = std::atomic_load(&tail);
        c = std::atomic_load(&capacity);
//        std::cout << "Dequeue Spinning." << std::endl;
    }while(!std::atomic_compare_exchange_weak(&tail, &t, (t + 1) % c));

    // Wait for full flag, then load ret.
    while(!(*full_signal)[t].load());
    T ret = items[t];

    // Flip full flag back to empty.
    (*full_signal)[t].store(false);
    
    items[t] = T();
    empty_sem.post();
    return ret;
}

template <class T>
std::pair<long, long> LocklessQueue<T>::drain_semaphores() {
    long count = 0;
    long empty = 0;
    while(count + empty < capacity.load()) {
        while(count_sem.try_wait()) count++;
        while(empty_sem.try_wait()) empty++;
    }
    return std::pair<long, long>(count, empty);
}

template <class T>
void LocklessQueue<T>::fill_semaphores(std::pair<long, long> &p) {
    long count = p.first;
    long empty = p.second;

    for(int i = 0; i < count; i++) {
        count_sem.post();
    }
    for(int i = 0; i < empty; i++) {
        empty_sem.post();
    }
}

template <class T>
void LocklessQueue<T>::expand() {
//    std::cout << "Expanding!" << std::endl;
    std::unique_lock<std::mutex> lock(expand_lock);

    size_t quarter_cap = capacity.load()/4;
    size_t e = empty_sem.current();
//    std::cout << "Expanding." << std::endl;
    if(e < std::max(quarter_cap, (size_t)1)) {
        long count = 0;
        long empty = 0;

        auto pair = drain_semaphores();
        count = pair.first;
        empty = pair.second;
//        sleep(5);
                
        size_t cap = capacity.load();
        size_t newcapacity = cap * 2;

        if(empty > std::max(cap/4, (size_t)1)) {
            fill_semaphores(pair);
            return;
        }

        std::cout << "Expanding to: " << newcapacity << std::endl;
        for(size_t i = 0; i < items.size(); i++) {
            std::cout << (*full_signal)[i].load() << ",";
        }
        std::cout << " -> ";
        
        items.resize(newcapacity);
        auto new_signal = new std::vector<std::atomic<bool>>(newcapacity);
        for(size_t i = 0; i < cap; i++) {
            (*new_signal)[i].store((*full_signal)[i].load());
        }
        delete full_signal;
        full_signal = new_signal;
        empty += cap;
        
        if(head <= tail) {
            for(size_t i = tail; i < cap; i++) {
                size_t dist_from_end = (cap - i);
                items[newcapacity - dist_from_end] = items[i];
                (*full_signal)[newcapacity - dist_from_end].store((*full_signal)[i].load());
                items[i] = T();
                (*full_signal)[i].store(false);
            }
            tail = newcapacity - (cap - tail);
        }
        capacity.store(newcapacity);

        for(size_t i = 0; i < items.size(); i++) {
            std::cout << (*full_signal)[i].load() << ",";
        }
        std::cout << std::endl;
        
        pair.second = empty;
        fill_semaphores(pair);
//        std::cout << "Done." << std::endl;
    }
}
