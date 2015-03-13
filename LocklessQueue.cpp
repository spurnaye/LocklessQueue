//#include "LocklessQueue.hpp"
#include <iostream>
#include <algorithm>
#include "ShutdownException.hpp"

template <class T>
LocklessQueue<T>::LocklessQueue(size_t n)
    :items(n), capacity(n), head(0), tail(0),
     count_sem(0), empty_sem(n), is_shutdown(false),
     dequeue_atom(true), enqueue_atom(true) {}

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
void LocklessQueue<T>::enqueue(T x){
    if(is_shutdown.load()) {
        throw ShutdownException();
    }

    bool expected;
    do {
        expected = true;
    } while(!std::atomic_compare_exchange_strong(&enqueue_atom, &expected, false));
    empty_sem.wait();
    
    items[head].store(x);
    head = ((head + 1) % capacity);
    
    count_sem.post();
    enqueue_atom = true;
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
    
    T ret = items[tail].load();
    items[tail].store(0);
    tail = ((tail + 1) % capacity);

    empty_sem.post();
    dequeue_atom = true;
    return ret;
}

//template <class T>
 //std::pair<long, long> LocklessQueue<T>::drain_semaphores() {
//    long count = 0;
//    long empty = 0;
//    while(count + empty < capacity.load()) {
//        while(count_sem.try_wait()) count++;
//        while(empty_sem.try_wait()) empty++;
//    }
//    return std::pair<long, long>(count, empty);
//}
//
 //template <class T>
 //void LocklessQueue<T>::fill_semaphores(std::pair<long, long> &p) {
//    long count = p.first;
//    long empty = p.second;
//
//    for(int i = 0; i < count; i++) {
//        count_sem.post();
//    }
//    for(int i = 0; i < empty; i++) {
//        empty_sem.post();
//    }
//}
//
 //template <class T>
 //void LocklessQueue<T>::expand() {
//
////    std::cout << "Expanding."<<std::endl;
//    std::unique_lock<std::mutex> lock(expand_lock);
//    size_t quarter_cap = capacity.load()/4;
//    size_t e = empty_sem.current();
//
//    if(e < std::max(quarter_cap, (size_t)1)) {
//        long count = 0;
//        long empty = 0;
//
//        auto pair = drain_semaphores();
//        count = pair.first;
//        empty = pair.second;
//                
//        size_t cap = capacity.load();
//        size_t newcapacity = cap * 2;
//
//        if(empty > std::max(cap/4, (size_t)1)) {
//            fill_semaphores(pair);
//            return;
//        }
//        
//        std::cout << "Expanding to: " << newcapacity
//                  << " Executing: " << executing.load() << std::endl;
////        (*items).resize(newcapacity);
//        auto new_items = new std::vector<std::atomic<T>>(newcapacity);
//        auto new_signal = new std::vector<std::atomic<bool>>(newcapacity);
//        for(size_t i = 0; i < cap; i++) {
//            (*new_items)[i].store((*items)[i].load());
//            (*new_signal)[i].store((*full_signal)[i].load());
//        }
//        
//        empty += cap;
//        
//        if(head <= tail) {
//            for(size_t i = tail; i < cap; i++) {
//                size_t dist_from_end = (cap - i);
//                (*new_items)[newcapacity - dist_from_end].store((*new_items)[i]);
//                (*new_items)[i].store(T());
//                (*new_signal)[newcapacity - dist_from_end].store((*new_signal)[i]);
//                (*new_signal)[i].store(T());
//            }
//            tail = newcapacity - (cap - tail);
//        }
//        capacity.store(newcapacity);
//
//        delete items;
//        delete full_signal;
//        
//        items = new_items;
//        full_signal = new_signal;
//        
//        pair.second = empty;
//        fill_semaphores(pair);
//    }
//}
