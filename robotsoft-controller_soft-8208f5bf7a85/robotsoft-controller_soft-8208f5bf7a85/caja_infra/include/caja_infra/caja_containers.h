#ifndef PROJECT_CAJA_CONTAINERS_H
#define PROJECT_CAJA_CONTAINERS_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <boost/optional.hpp>

namespace caja_infra {

/// Implements thread safe queue.
/// Return optional<void> in case of empty queue

template<class T>
class SafeQueue {
 public:
    SafeQueue(void)
        : q(), m() {}

    ~SafeQueue(void) {}

    bool empty() { return q.empty(); }
    // Push an element to the queue.
    void push(T t) {
        std::lock_guard<std::mutex> lock(m);
        q.push(t);
    }

    // Get the "front"-element if exist.
    // If the queue is empty, wait till a element is avaiable.
    boost::optional<T> popAndRemove(void) {
        std::unique_lock<std::mutex> lock(m);
        if (!q.empty()) {
            boost::optional<T> val = boost::optional<T>(q.front());
            q.pop();
            return val;
        } else {
            return boost::none;
        }
    }

 private:
    std::queue<T> q;
    mutable std::mutex m;
};

/// Implement thread safe queue blocking until queue not empty.
///
template<class T>
class BlockingQueue {
 public:
    BlockingQueue(void)
        : q(), m(), c() {}

    ~BlockingQueue(void) {}

    // Add an element to the queue.
    void enqueue(T t) {
        std::lock_guard<std::mutex> lock(m);
        q.push(t);
        c.notify_one();
    }

    // Get the "front"-element.
    // If the queue is empty, wait till a element is avaiable.
    T dequeue(void) {
        std::unique_lock<std::mutex> lock(m);
        while (q.empty()) {
            // release lock as long as the wait and reaquire it afterwards.
            c.wait(lock);
        }
        T val = q.front();
        q.pop();
        return val;
    }

 private:
    std::queue<T> q;
    mutable std::mutex m;
    std::condition_variable c;
};
}
#endif //PROJECT_CAJA_CONTAINERS_H
