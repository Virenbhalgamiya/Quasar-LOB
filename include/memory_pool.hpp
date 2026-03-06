#pragma once

#include "types.hpp"
#include <vector>
#include <memory>
#include <stdexcept>

namespace ullt {

// A pre-allocated pool of Objects to eliminate dynamic allocation overhead
template <typename T>
class MemoryPool {
private:
    std::vector<T> data;
    std::vector<T*> free_list;
    size_t capacity;

public:
    explicit MemoryPool(size_t capacity) : capacity(capacity) {
        data.resize(capacity);
        free_list.reserve(capacity);
        for (size_t i = 0; i < capacity; ++i) {
            free_list.push_back(&data[capacity - 1 - i]); // Allocate backwards so pop() is ascending
        }
    }

    T* allocate() {
        if (free_list.empty()) {
            // Alternatively, could dynamically resize but for latency we throw or expand in large chunks
            throw std::runtime_error("Memory pool exhausted");
        }
        T* obj = free_list.back();
        free_list.pop_back();
        return obj;
    }

    void deallocate(T* obj) {
        free_list.push_back(obj);
    }

    size_t available() const {
        return free_list.size();
    }
};

using OrderPool = MemoryPool<Order>;

} // namespace ullt
