#pragma once

#include <cstring>

#include "util.h"

namespace swan {

class Arena {
    std::pmr::monotonic_buffer_resource mbr;

   public:
    template <typename T>
    T* alloc() {
        auto ptr = reinterpret_cast<T*>(mbr.allocate(sizeof(T)));
        new (ptr) T();
        return ptr;
    }

    template <typename T>
    Span<T> vec_to_span(const std::vector<T>& v) {
        T* ptr = reinterpret_cast<T*>(mbr.allocate(sizeof(T) * v.size()));
        memcpy(ptr, v.data(), sizeof(T) * v.size());
        return Span<T>(ptr, v.size());
    }

    StringRef alloc_str(const std::string& str) {
        char* ptr =
            reinterpret_cast<char*>(mbr.allocate(sizeof(char) * str.size()));
        memcpy(ptr, str.data(), sizeof(char) * str.size());
        return StringRef(ptr, str.size());
    }
};

}  // namespace swan