#pragma once

#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory_resource>
#include <span>

namespace swan {

namespace fs = std::filesystem;

template <typename T>
void print(T t)
{
    std::cout << t;
}

template <typename T, typename U, typename... Args>
void print(T t, U u, Args... args)
{
    std::cout << t;
    print(u, args...);
}

template <typename T>
using Span = std::span<T>;
using StringRef = Span<char>;
using Arena = std::pmr::monotonic_buffer_resource;

template <typename F>
auto benchmark(F fn, int iterations)
{
    const auto start = std::chrono::system_clock::now();
    while (iterations-- > 0) {
        fn();
    }
    const auto stop = std::chrono::system_clock::now();
    const auto secs = std::chrono::duration<double>(stop - start);
    return secs.count();
}

} // namespace swan