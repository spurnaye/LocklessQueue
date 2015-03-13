#include <iostream>
#include <future>
#include <exception>
#include <ctime>
#include <sys/time.h>
#include <string>
#include <cassert>
#include <unistd.h>
#include <string.h>
#include "LocklessQueue.hpp"
#include "RegularQueue.hpp"
#include "ShutdownException.hpp"


#define COUNT 100000000
std::atomic<int> x[COUNT];

void clear_x();
void test_queue(Queue<int> &q, std::string q_name);

int main(void) {
    std::cout << "Testing queueing and dequeueing of " << COUNT
              << " integers using 4 threads. 2 queueing and 2 dequeueing."
              << std::endl << std::endl;
    LocklessQueue<int> lq(COUNT/2);
    RegularQueue<int> rq(COUNT/2);

    test_queue(lq, "lockless queue");
    std::cout << std::endl;
    clear_x();
    test_queue(rq, "locking queue");
}


uint64_t ms_since_epoch();
void enqueue_count(Queue<int>* q);
void dequeue_count(Queue<int>* q);
void verify_x();

void test_queue(Queue<int> &q, std::string q_name) {
    std::cout << "Starting " << q_name << "." << std::endl;
    uint64_t start = ms_since_epoch();
    auto put = std::thread(enqueue_count, &q);
    auto put2 = std::thread(enqueue_count, &q);
//    sleep(5);
    auto get = std::thread(dequeue_count, &q);
    auto get2 = std::thread(dequeue_count, &q);
    
    put.join();
    put2.join();
    get.join();
    get2.join();
    
    uint64_t end = ms_since_epoch();
    std::cout << "Done. " << q_name << " took: "
              << end - start << "ms." << std::endl;
    verify_x();
}

void clear_x() {
    memset(&x, 0, COUNT * sizeof(x[0]));
}

void verify_x() {
    std::cout << "Verifying correctness." << std::endl;
    bool ok = true;
    for(int i = 0; i < COUNT; i++) {
        if(x[i] != 2) {
            std::cout << "x[" << i << "] == " << x[i] << std::endl;
            ok = false;
        }
    }
    if(ok) {
        std::cout << "Queue functioned correctly." << std::endl;
    }
}

uint64_t ms_since_epoch() {
    try {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (tv.tv_usec / 1000) + (tv.tv_sec * 1000);
    } catch (ShutdownException e) {}
}

void enqueue_count(Queue<int>* q) {
    try {
        for(int i = 0; i < COUNT; i++) {
            q->enqueue(i);
        }
    } catch (ShutdownException e) {
        std::cout << "enqueue Got shutdown exception." << std::endl;
    }
}

void dequeue_count(Queue<int>* q) {
    try {
        for(int i = 0; i < COUNT; i++) {
            int qx = q->dequeue();
            x[qx]++;
        }
    } catch (ShutdownException e) {
        std::cout << "dequeue Got shutdown exception." << std::endl;
    }
}
