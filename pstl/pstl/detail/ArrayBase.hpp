#pragma once

#include "pstl\detail\Common.hpp"
#include "pstl\detail\Enumerator.hpp"

#include <utility>
#include <initializer_list>

namespace pstl {
namespace detail {

    template <class S, class E>
    struct ArrayRange;

    template <class T, bool AssertBounds = true>
    struct ArrayAccess;
    
    template <class S, class E, bool AssertBounds = true>
    class ArrayBase : 
        public ArrayRange<S, E>,
        public ArrayAccess<typename ArrayRange<S, E>::Value, AssertBounds>
    {
    public:
        using RangeBase = ArrayRange<S, E>;
        using T = typename ArrayRange<S, E>::Value;
        using AccessBase = ArrayAccess<T, AssertBounds>;

        // Pull in constructors.
        using RangeBase::RangeBase;

        __inline T* Start() { return m_start; }
        using RangeBase::End;
        using RangeBase::Count;

        __inline PointerEnumerator<T> GetEnumerator()
        { return PointerEnumerator<T>(m_start, End()); }

        __inline PointerEnumerator<const T> GetEnumerator() const
        { return PointerEnumerator<const T>(m_start, End()); }

        __inline T& operator[](uint32_t index) 
        { return At(index, m_start, Count()); }

        __inline const T& operator[](uint32_t index) const
        { return At(index, m_start, Count()); }

    protected:
        using AccessBase::At;
        using RangeBase::m_start;
    };

}}

template <class T, uint32_t n>
struct pstl::detail::ArrayRange<T, std::integral_constant<uint32_t, n>>
{
    typedef T Value;

    ArrayRange() = default;

    template <class U>
    __inline ArrayRange(std::initializer_list<U> list)
    {
        const auto count = list.size();
        memcpy(m_start, list.begin(), sizeof(T) * count);
    }

    __inline T* End() { return m_start + n; }
    __inline const T* End() const { return m_start + n; }
    __inline uint32_t Count() const { return n; }

protected:
    T m_start[n];
};

template <class T>
struct pstl::detail::ArrayRange<T*, T*>
{
    typedef T Value;

    __inline ArrayRange(T* start, T* end) : 
        m_start(start), 
        m_end(end)
    { }

    __inline T* End() { return m_end; }
    __inline const T* End() const { return m_end; }
    __inline uint32_t Count() const { return m_end - m_start; }

protected:
    T* m_start = nullptr;
    T* m_end = nullptr;
};

template <class T>
struct pstl::detail::ArrayAccess<T, true>
{
    __inline static T& At(uint32_t index, T* start, uint32_t count)
    {
        assert(index < count);
        return start[index];
    }
};

template <class T>
struct pstl::detail::ArrayAccess<T, false>
{
    __inline static T& At(uint32_t index, T* start, uint32_t count)
    { return start[index]; }
};