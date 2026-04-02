#pragma once
#include <queue>
#include <vector>
#include <mutex>

template<typename T, typename CMP = std::greater<T>>
class ThreadSafeBoundPriorityQueue {
private:
    std::priority_queue<T, std::vector<T>, CMP> pq;
    size_t max_items;
    std::mutex mtx;
    bool consumed = true;
public:
    ThreadSafeBoundPriorityQueue(size_t max_items)
    : max_items(max_items)
    {}

    template <typename TType>
    void push(TType&& item) {
        std::lock_guard<std::mutex> lock(mtx);
        if (pq.size() < max_items) {
            pq.push(std::forward<TType>(item));
            consumed = false;
        } else if (CMP()(item, pq.top())) {
            pq.pop();
            pq.push(std::forward<TType>(item));
            consumed = false;
        }
    }

    void pop() { std::lock_guard<std::mutex> lock(mtx); consumed = false; pq.pop(); }
    const T& top() { std::lock_guard<std::mutex> lock(mtx); return pq.top(); }
    size_t size() { std::lock_guard<std::mutex> lock(mtx); return pq.size(); }
    bool empty() { std::lock_guard<std::mutex> lock(mtx); return pq.empty(); }

    bool canConsume() { 
        std::lock_guard<std::mutex> lock(mtx); 
        return !consumed; 
    } 
    std::priority_queue<T, std::vector<T>, CMP> consume() { 
        std::lock_guard<std::mutex> lock(mtx); 
        consumed = true; 
        return pq; 
    } 

    size_t getMaxItems() const { return max_items; }
};