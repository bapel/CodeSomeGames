#pragma once

#include "pstl\detail\ArrayBase.hpp"
#include "Allocator.hpp"

namespace pstl {
namespace detail {

    template <uint32_t n>
    using ArrayCount = std::integral_constant<uint32_t, n>;

}}

namespace pstl {

    template <class T, uint32_t n, bool AssertBounds = true>
    using Array = detail::ArrayBase<T, detail::ArrayCount<n>, AssertBounds>;

    template <class T, bool AssertBounds = true>
    using FixedArray = detail::ArrayBase<T*, T*, AssertBounds>;

    template <class T, bool AssertBounds = true>
    class DynamicArray final : public FixedArray<T, AssertBounds>
    {
        using Base = FixedArray<T, AssertBounds>;

    public:
        __inline DynamicArray(IAllocator* allocator) : 
            Base(), m_allocator(allocator)
        { }

        __inline DynamicArray(uint32_t n, IAllocator* allocator = GetFallbackAllocator()) : 
            DynamicArray(allocator->Malloc_n<T>(n), n, allocator)
        { }

        __inline DynamicArray(DynamicArray&& other) :
            Base(other.m_start, other.m_end),
            m_allocator(other.m_allocator)
        { other.m_start = nullptr; }

        __inline ~DynamicArray()
        { m_allocator->Free(m_start); }

        DynamicArray(const DynamicArray&) = delete;
        DynamicArray& operator= (const DynamicArray&) = delete;

        using Base::Count;

        void Resize(uint32_t n, IAllocator* allocator = GetFallbackAllocator())
        {
            auto start = allocator->Malloc_n<T>(n);
            auto dstSize = sizeof(T) * n;
            auto srcSize = sizeof(T) * Count();

            memcpy(start, m_start, Min(dstSize, srcSize));
            m_allocator->Free(m_start);

            m_start = start;
            m_end = start + n;
            m_allocator = allocator;
        }

    private:
        __inline DynamicArray(T* start, uint32_t n, IAllocator* allocator) :
            Base(start, start + n),
            m_allocator(allocator)
        { }

    private:
        using Base::m_start;
        using Base::m_end;
        IAllocator* m_allocator = GetFallbackAllocator();
    };

}