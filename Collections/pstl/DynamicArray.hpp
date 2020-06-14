#pragma once

#include "pstl\detail\ArrayBase.hpp"
#include "Allocator.hpp"

namespace pstl {
namespace detail {

    template <class T, class C = void>
    class DynamicArrayStore
    {
        using IAllocator = pstl::IAllocator;

    public:
        DynamicArrayStore() = default;

        __inline DynamicArrayStore(IAllocator* allocator) : 
            m_allocator(allocator) 
        { }

        __inline DynamicArrayStore(uint32_t n, IAllocator* allocator = pstl::GetFallbackAllocator()) : 
            m_start(m_allocator->Malloc_n<T>(n)),
            m_allocator(allocator)
        { }

        __inline DynamicArrayStore(DynamicArrayStore&& other) :
            m_start(other.m_start),
            m_end(other.m_end),
            m_allocator(other.m_allocator)
        { other.m_start = nullptr; }

        __inline ~DynamicArrayStore()
        { m_allocator->Free(m_start); }

        DynamicArrayStore(const DynamicArrayStore&) = delete;
        DynamicArrayStore& operator= (const DynamicArrayStore&) = delete;

        __inline T* Start() { return m_start; }
        __inline T* End()   { return m_end; }

        __inline uint32_t Count() const 
        { return m_end - m_start; }

        void Resize(uint32_t n, IAllocator* allocator = pstl::GetFallbackAllocator())
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

    protected:
        T* m_start = nullptr;
        T* m_end = nullptr;
        IAllocator* m_allocator = pstl::GetFallbackAllocator();
    };

}}

namespace pstl {

    template <class T, bool AssertBounds = true>
    using DynamicArray = detail::ArrayBase<
        detail::DynamicArrayStore, 
        detail::RandomAccess,
        T, void, AssertBounds>;

}