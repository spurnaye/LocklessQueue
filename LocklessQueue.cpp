//#include "LocklessQueue.hpp"
#include <iostream>
#include <algorithm>
#include "ShutdownException.hpp"

template <class T>
LocklessQueue<T>::LocklessQueue(size_t n) :
    LocklessQueue(n, 0) {}

template <class T>
LocklessQueue<T>::LocklessQueue(size_t n, size_t max)
    :items(new std::vector<std::atomic<T>>(n)), capacity(n), head(0), tail(0),
     count_sem(0), empty_sem(n), is_shutdown(false),
     dequeue_atom(true), enqueue_atom(true), max_size(max) {}

template <class T>
LocklessQueue<T>::~LocklessQueue() {
    shutdown();
}

template <class T>
void LocklessQueue<T>::shutdown() {
    count_sem.shutdown();
    empty_sem.shutdown();
    is_shutdown.store(true);
    delete items;
}

template <class T>
void LocklessQueue<T>::enqueue(T x){
    if(is_shutdown.load()) {
        throw ShutdownException();
    }

    bool expected;
    do {
        expected = true;
    } while(!std::atomic_compare_exchange_strong(&enqueue_atom, &expected, false));

    if((capacity >= max_size && (empty_sem.wait(),1))
       || empty_sem.try_wait()) {
        
        (*items)[head].store(x);
        head = ((head + 1) % capacity);
        enqueue_atom = true;
        count_sem.post();
    }
    else {
        expand();
        enqueue_atom = true;
        enqueue(x);
        
    }
}
    
template <class T>
T LocklessQueue<T>::dequeue() {
    if(is_shutdown.load()) {
        throw ShutdownException();
    }

    bool expected;
    do {
        expected = true;
    } while(!std::atomic_compare_exchange_strong(&dequeue_atom, &expected, false));
    count_sem.wait();
    
    T ret = (*items)[tail].load();
    (*items)[tail].store(0);
    tail = ((tail + 1) % capacity);

    empty_sem.post();
    dequeue_atom = true;
    return ret;
}

template <class T>
std::pair<long, long> LocklessQueue<T>::drain_semaphores() {
    long count = 0;
    long empty = 0;
    while(count + empty < capacity) {
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

    std::unique_lock<std::mutex> lock(expand_lock);
    size_t quarter_cap = capacity/4;
    size_t e = empty_sem.current();

    if(e < std::max(quarter_cap, (size_t)1)) {
        long count = 0;
        long empty = 0;


        auto pair = drain_semaphores();
        count = pair.first;
        empty = pair.second;
                
        size_t newcapacity = capacity * 2;
        

        if(empty > std::max(capacity/4, (size_t)1)) {
            fill_semaphores(pair);
            return;
        }

        auto new_items = new std::vector<std::atomic<T>>(newcapacity);
        for(size_t i = 0; i < capacity; i++) {
            (*new_items)[i].store((*items)[i].load());
        }
        
        empty += capacity;
        
        if(head <= tail) {
            for(size_t i = tail; i < capacity; i++) {
                size_t dist_from_end = (capacity - i);
                (*new_items)[newcapacity - dist_from_end].store((*new_items)[i]);
                (*new_items)[i].store(T());
            }
            tail = newcapacity - (capacity - tail);
        }
        capacity = newcapacity;

        delete items;
        
        items = new_items;
        
        pair.second = empty;
        fill_semaphores(pair);
    }
}
