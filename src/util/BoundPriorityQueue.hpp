#pragma once
#include <queue>
#include <vector>

template<typename T, typename CMP = std::greater<T>>
class BoundPriorityQueue {
private:
    std::priority_queue<T, std::vector<T>, CMP> pq;
    size_t max_items;
public:
    BoundPriorityQueue(size_t max_items)
    : max_items(max_items)
    {}

    template <typename TType>
    void push(TType&& item) {
        if (pq.size() < max_items) {
            pq.push(std::forward<TType>(item));
        } else if (CMP()(item, pq.top())) {
            pq.pop();
            pq.push(std::forward<TType>(item));
        }
    }

    void pop() { pq.pop(); }
    const T& top() { return pq.top(); }
    size_t size() { return pq.size(); }
    bool empty() { return pq.empty(); }

    size_t getMaxItems() const { return max_items; }
};