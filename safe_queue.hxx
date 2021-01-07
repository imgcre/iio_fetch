//
// Created by imgcr on 2020/12/7.
//

#ifndef IIO_FETCH_SAFE_QUEUE_HXX
#define IIO_FETCH_SAFE_QUEUE_HXX

#include <queue>
#include <mutex>
#include <condition_variable>

template<class T>
class safe_queue {
public:
    void enqueue(T t) {
        std::lock_guard lock(m);
        q.push(t);
        c.notify_one();
    }

    T dequeue() {
        std::unique_lock lock(m);
        while (q.empty()) {
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


#endif //IIO_FETCH_SAFE_QUEUE_HXX
