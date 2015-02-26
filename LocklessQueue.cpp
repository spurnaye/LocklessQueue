//#include "LocklessQueue.hpp"
#include <iostream>
#include <algorithm>
#include "ShutdownException.hpp"

template <class T>
LocklessQueue<T>::LocklessQueue(size_t n)
    : LocklessQueue(n, std::numeric_limits<size_t>::max()) {}

template <class T>
LocklessQueue<T>::LocklessQueue(size_t n, size_t max)
    :items(n), capacity(n), head(0), tail(0),
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
        }while(!std::atomic_compare_exchange_weak(&head, &h, (h + 1) % c));
        items[h] = x;
        //sems[h].post();
        count_sem.post(); 
    }
    else {
        expand();
        enqueue(x);
    }
}

template <class T>
T LocklessQueue<T>::dequeue() {
    if(is_shutdown.load()) {
        throw ShutdownException();
    }
    long w = count_sem.wait();
//    executing_threads++;
    size_t h,t,c;
    volatile T ret;
    int i = 0;
    do {
        h = std::atomic_load(&head);
        t = std::atomic_load(&tail);
        c = std::atomic_load(&capacity);
        ret = items[t];
        i++;
    }while(!std::atomic_compare_exchange_weak(&tail, &t, (t + 1) % c));
    //sems[t].wait();
    //ret = items[t];
    items[t] = 0;
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
    std::cout << "Expanding!" << std::endl;
    std::unique_lock<std::mutex> lock(expand_lock);

    size_t quarter_cap = capacity.load()/4;
    size_t e = empty_sem.current();

    if(e < std::max(quarter_cap, (size_t)1)) {
        long count = 0;
        long empty = 0;

        auto pair = drain_semaphores();
        count = pair.first;
        empty = pair.second;
                
        size_t cap = capacity.load();
        size_t newcapacity = cap * 2;

        if(empty > std::max(cap/4, (size_t)1)) {
            fill_semaphores(pair);
            return;
        }

        items.resize(newcapacity);
        empty += cap;
        
        if(head <= tail) {
            for(size_t i = tail; i < cap; i++) {
                size_t dist_from_end = (cap - i);
                items[newcapacity - dist_from_end] = items[i];
                items[i] = T();
            }
            tail = newcapacity - (cap - tail);
        }
        capacity.store(newcapacity);
        
        pair.second = empty;
        fill_semaphores(pair);
    }
}
