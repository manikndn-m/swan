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
void buildstr_(std::string &accum, T t)
{
    accum += t;
}

template <typename T, typename U, typename... Args>
void buildstr_(std::string &accum, T t, U u, Args... args)
{
    accum += t;
    buildstr_(accum, u, args...);
}

template <typename... Args>
std::string buildstr(Args... args)
{
    std::string accum;
    buildstr_(accum, args...);
    return accum;
}

template <typename... Args>
void panic(Args... args)
{
    print("\npanic: ", args...);
}

[[noreturn]]
inline void fixme_()
{
    std::abort();
}

#define fixme()                                                                \
    {                                                                          \
        panic("internal compiler error at ", __LINE__, " : ", __FILE__,        \
              " : ");                                                          \
        fixme_();                                                              \
    }

using StringRef = std::string_view;

template <typename T>
class Span {
    T *ptr;
    size_t size_;

  public:
    Span()
    {
        this->ptr = nullptr;
    }

    Span(T *ptr, size_t size) : ptr(ptr), size_(size)
    {
    }

    T *begin()
    {
        return ptr;
    }

    T *end()
    {
        return ptr + size_;
    }

    T operator[](size_t idx)
    {
        return ptr[idx];
    }

    size_t size() const
    {
        return size_;
    }
};

template <typename F>
auto benchmark(F fn, int iterations)
{
    auto start = std::chrono::system_clock::now();
    while (iterations-- > 0) {
        fn();
    }

    auto stop = std::chrono::system_clock::now();
    auto secs = std::chrono::duration<double>(stop - start);

    return secs.count();
}

} // namespace swan

#define DP(X) std::cout << X << std::endl;

#define DPE(X)                                                                 \
    std::cout << #X << " = " << X << std::endl;                                \
    std::exit(-1);
