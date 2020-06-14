#pragma once

#include <assert.h>
#include <string.h>

// @Todo: Think of another name for core libs?
// pstl stands for POD STL.
#define NamespaceName__ pstl
#define Namespace__ namespace NamespaceName__

#if defined(_DEBUG)
#define BoundsCheck__
#define AssertIsPod__(T__) static_assert(__is_pod(T__), #T__" is not POD")
#else
#define AssertIsPod__(T__)
#endif

#include <stdint.h>
#include <array>

Namespace__
{
    using SizeType = size_t;
    using CountType = uint32_t;
    using IndexType = uint32_t;

    template <class T> using PtrType = T *;
    template <class T> using ConstPtrType = const T*;
    template <class T> using RefType = T &;
    template <class T> using ConstRefType = const T &;

    // Static fixed array, like T x[N].
    //template <class T, int N>
    //using Array = std::array<T, N>;

    // @Todo: Do we need "bit-twiddling" versions of Min/Max?
    // Are those actually faster?

    template <class T, class U>
    constexpr T Min(ConstRefType<T> a, ConstRefType<U> b)
    { return (a < b) ? a : b; }

    template <class T, class U>
    constexpr T Max(ConstRefType<T> a, ConstRefType<U> b)
    { return (a > b) ? a : b; }
}

Namespace__
{
    // @Todo: Implement

    // Bit vector.
    template<class T = uint64_t>
    class BitArray;

    // Double ended queue.
    template <class T>
    class Dequeue;

    // HashMap.
    template <class K, class V>
    class HashMap;
}