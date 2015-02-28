#ifndef QUEUE_HPP
#define QUEUE_HPP

template <class T>
class Queue {
public:
    virtual void enqueue(T x) = 0;
    virtual T dequeue() = 0;
};

#endif
