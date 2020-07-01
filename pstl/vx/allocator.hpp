#pragma once

#include "common.hpp"

namespace vx
{
    class IAllocator
    {
    public:
        virtual ~IAllocator()
        { assert(m_num_allocations == 0); }

        // @Todo: Debug variants?
        virtual void* malloc(size_t size, size_t alignment, size_t offset = 0) = 0;
        virtual void free(void* block) = 0;

        // Alignment is inferred from the type.
        template <class T>
        T* malloc_n(size_t count, size_t offset = 0)
        { return static_cast<T*>(this->malloc(count * sizeof(T), alignof(T), offset)); }

    protected:
        size_t m_num_allocations = 0;
    };

    IAllocator* fallback_allocator();
}