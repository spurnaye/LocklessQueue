# LocklessQueue
A lockless, multi-producer, multi-consumer, expandable queue written in C++

Mutexes are slow. Lockless data structures are fast.
Here's a fairly quick 'lockless' queue that can have as many threads enqueueing and dequeueing as you want, and can grow as needed. 

It only locks when necessary (trying to dequeue on an empty queue, trying to enqueue a full queue)

Check out the [benchmarks](LocklessQueue.md) to see the comparison with a fairly standard multithreaded queue implementation. In the cases presented it's about twice as fast, (although the advantage seems to diminish as contention increases)
The code used to test is also presented (the version for 2 producers, 2 consumers is what's checked in. Only add or remove threads and parameters to verify_x to change it. Change COUNT to alter the number of integers enqueued per producer.)

And check out [main_test.cpp](main_test.cpp) to see the queue in action.