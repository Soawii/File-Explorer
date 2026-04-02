#pragma once
#include "ThreadBlockingQueue.hpp"
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <functional>

class ThreadPool {
private:
    ThreadBlockingQueue<std::function<void()>>  tasks;
    std::vector<std::thread>                workers;

    std::mutex  mtx;
    bool        stopped = false;
public:
    ThreadPool(size_t num_threads) {
        start(num_threads);
    }

    ~ThreadPool() {
        stop();
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    void enqueue(const std::function<void()>& task) {
        std::lock_guard<std::mutex> lock(mtx);
        if (stopped)
            return;
        tasks.push(task);
    }

    void enqueue(std::function<void()>&& task) {
        std::lock_guard<std::mutex> lock(mtx);
        if (stopped)
            return;
        tasks.push(std::move(task));
    }

    void start(size_t count) {
        for (auto& t : workers) {
            if (t.joinable())
                t.join();
        }
        workers.clear();

        stopped = false;
        tasks.setDone(false);
        
        for (size_t i = 0; i < count; i++) {
            workers.emplace_back([this]() {
                std::function<void()> task;
                while (tasks.pop(task)) {
                    try {
                        task();
                    }
                    catch (...) {}
                }
            });
        }
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            stopped = true;
            tasks.setDone(true);
        }
        for (auto& t : workers) {
            if (t.joinable())
                t.join();
        }
    }

    void stopNow() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            stopped = true;
            tasks.clear();
            tasks.setDone(true);
        }
        for (auto& t : workers) {
            if (t.joinable())
                t.join();
        }
    }
};