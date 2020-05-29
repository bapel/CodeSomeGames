#pragma once

#include <assert.h>
#include <string.h>

#define NamespaceName__ pstl
#define Namespace__ namespace NamespaceName__

#if defined(_DEBUG)
#define BoundsCheck__
#define AssertIsPod__(T__) static_assert(__is_pod(T))
#else
#define AssertIsPod__(T__)
#endif

#include <stdint.h>

Namespace__
{
    using SizeType = size_t;
    using CountType = uint32_t;
    using IndexType = uint32_t;

    template <class T> using PtrType = T *;
    template <class T> using ConstPtrType = const T*;
    template <class T> using RefType = T &;
    template <class T> using ConstRefType = const T &;

    template <class T>
    T Min(ConstRefType<T> a, ConstRefType<T> b)
    { return (a < b) ? a : b; }

    template <class T>
    T Max(ConstRefType<T> a, ConstRefType<T> b)
    { return (a > b) ? a : b; }
}

Namespace__
{
    // Static fixed array, like T x[N].
    template <class T, int N = -1>
    struct Array;

    // Double ended queue.
    template <class T>
    struct Dequeue;
}