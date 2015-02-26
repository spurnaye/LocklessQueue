#include <iostream>
#include <future>
#include <exception>
#include <ctime>
#include <sys/time.h>
#include <string>
#include <cassert>
#include <unistd.h>
#include "LocklessQueue.hpp"
#include "RegularQueue.hpp"
#include "ShutdownException.hpp"


#define COUNT 10000000
int x[COUNT];

void enqueue_count(LocklessQueue<int>* q) {
    try {
        for(int i = 0; i < COUNT; i++) {
            q->enqueue(i);
        }
    } catch (ShutdownException e) {
        std::cout << "enqueue Got shutdown exception." << std::endl;
    }
}

void dequeue_count(LocklessQueue<int>* q) {
    try {
        for(int i = 0; i < COUNT * 2; i++) {
            int qx = q->dequeue();
            x[qx]++;
        }
    } catch (ShutdownException e) {
        std::cout << "dequeue Got shutdown exception." << std::endl;
    }
}

void reg_enqueue_count(RegularQueue<int>* q) {
    try {
        for(int i = 0; i < COUNT; i++) {
            q->enqueue(i);
        }
    } catch (ShutdownException e) {}
}

void reg_dequeue_count(RegularQueue<int>* q) {
    try {
        for(int i = 0; i < COUNT * 2; i++) {
            int qx = q->dequeue();
            x[qx]++;
        }
    } catch (ShutdownException e) {}
}

uint64_t ms_since_epoch() {
    try {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (tv.tv_usec / 1000) + (tv.tv_sec * 1000);
    } catch (ShutdownException e) {}
}

int main(void) {

    std::cout << "Lockless Semaphore Size: " << sizeof(LocklessSemaphore) << std::endl;
        std::cout << "Semphore Size: " << sizeof(Semaphore) << std::endl;
    
    {
        LocklessQueue<int> q(COUNT/2, 2);
        
        uint64_t start = ms_since_epoch();
//        auto put = std::async(std::launch::async,
//                              enqueue_count, &q);
//        auto put2 = std::async(std::launch::async,
//                               enqueue_count, &q);
//        auto get = std::async(std::launch::async,
//                              dequeue_count, &q);
//        auto get2 = std::async(std::launch::async,
//                               dequeue_count, &q);

        auto put = std::thread(enqueue_count, &q);
        auto put2 = std::thread(enqueue_count, &q);
        auto get = std::thread(dequeue_count, &q);
//        auto get2 = std::thread(dequeue_count, &q);

//        get.detach();
//        get2.detach();
        
        put.join();
        put2.join();
        uint64_t end = ms_since_epoch();
        std::cout << "Done. Lockless took: " << end - start << "ms." << std::endl;

        sleep(1);
//        q.shutdown();
        get.detach();
//        get2.detach();
//        get.join();
//        get2.join();
        
        // Verify correct counts
//        for(int i = 0; i < COUNT; i++) {
//            std::cout << x[i] << ",";
//        }
        std::cout << std::endl;
        for(int i = 0; i < COUNT; i++) {
            if(x[i] != 2) {
                std::cout << "x[" << i << "] == " << x[i] << std::endl;
//                exit(0);
            }
        }
    }

//    {
//        RegularQueue<int> q(200);
//        
//        std::cout << "Starting regular queue." << std::endl;
//        uint64_t start = ms_since_epoch();
//        auto put = std::async(std::launch::async,
//                              reg_enqueue_count, &q);
//        auto put2 = std::async(std::launch::async,
//                               reg_enqueue_count, &q);
//        auto get = std::async(std::launch::async,
//                              reg_dequeue_count, &q);
//        auto get2 = std::async(std::launch::async,
//                               reg_dequeue_count, &q);
//        
//        put.get();
//        put2.get();
//        uint64_t end = ms_since_epoch();
//        std::cout << "Done. Regular took: " << end - start << "ms." << std::endl;
//        // Verify correct counts
//        for(int i = 0; i < COUNT; i++) {
//            if(x[i] != 2) {
//                std::cout << "x[" << i << "] != 2" << std::endl;
//                exit(0);
//            }
//        }
//    }
}
