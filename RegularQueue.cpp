
template <class T>
RegularQueue<T>::RegularQueue(size_t n)
    : items(n), head(0), tail(0), count(0), empty(n) {}

template <class T>
void RegularQueue<T>::enqueue(T x) {
    std::unique_lock<std::mutex> l(lock);
    while(empty <= 0) {
        dequeue_event.wait(l);
    }
    items[head] = x;
    head = (head + 1) % items.size();
    count++;
    empty--;
    enqueue_event.notify_one();
}

template <class T>
T RegularQueue<T>::dequeue() {
    std::unique_lock<std::mutex> l(lock);
    while(count <= 0) {
        enqueue_event.wait(l);
    }
    T ret = items[tail];
    tail = (tail + 1) % items.size();
    count--;
    empty++;
    dequeue_event.notify_one();
    return ret;
}
