#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadBlockingQueue {
private:
    std::queue<T>           q;
    std::mutex              mtx;
    std::condition_variable cv;
    bool                    done = false;
public:
    void push(const T& p) {
        std::lock_guard<std::mutex> lock(mtx);
        q.push(p);
        cv.notify_one();
    }

    void push(T&& p) {
        std::lock_guard<std::mutex> lock(mtx);
        q.push(std::move(p));
        cv.notify_one();
    }

    bool pop(T& out) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&] { return !q.empty() || done; });

        if (q.empty()) return false;

        out = q.front();
        q.pop();
        return true;
    }

    void setDone(bool is_done) {
        std::lock_guard<std::mutex> lock(mtx);
        done = is_done;
        if (done)
            cv.notify_all();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mtx);
        std::queue<T> empty;
        std::swap(q, empty);
    }
};