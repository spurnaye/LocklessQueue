# LocklessQueue
A lockless, multi-producer, multi-consumer, expandable queue written in C++

Mutexes are slow. Lockless data structures are fast.
Here's a fairly quick 'lockless' queue that can have as many threads enqueueing and dequeueing as you want, and can grow as needed. 

It only locks when necessary (trying to dequeue on an empty queue, trying to enqueue a full queue)

Check out the [benchmarks](LocklessQueue.md) to see the comparison with a locking queue

And check out [main_test.cpp](main_test.cpp) to see the queue in action.