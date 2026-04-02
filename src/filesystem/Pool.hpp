#pragma once
#include <vector>
#include <unordered_map>

template<typename T>
class Pool {
private:
    std::unordered_map<T*, size_t>  item_to_index;
    std::vector<T*>                 items;
    std::vector<bool>               is_free;
    size_t                          yeildPos = 0;
public:
    Pool() {}

    T* yield() {
        while (yeildPos < is_free.size() && !is_free[yeildPos])
            yeildPos++;

        if (yeildPos == is_free.size())
            return nullptr;

        is_free[yeildPos] = false;
        return items[yeildPos];
    }

    void add(T* item) {
        item_to_index[item] = items.size();
        items.push_back(item);
        is_free.push_back(true);
    }

    void free(T* item) {
        auto it = item_to_index.find(item);
        if (it == item_to_index.end())
            return;
        size_t idx = it->second;
        is_free[idx] = true;
        yeildPos = std::min(yeildPos, idx);
    }

    void freeAll() {
        for (size_t i = 0; i < is_free.size(); i++)
            is_free[i] = true;
        yeildPos = 0;
    }

    std::vector<T*>& getItems() {
        return items;
    }

    std::vector<bool>& getIsFree() {
        return is_free;
    }

    size_t getSize() const {
        return is_free.size();
    }
};