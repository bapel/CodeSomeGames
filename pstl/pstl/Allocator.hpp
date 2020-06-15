#pragma once

#include "pstl\detail\Common.hpp"

namespace pstl
{
    class IAllocator
    {
    public:
        virtual ~IAllocator()
        { assert(m_NumAllocations == 0); }

        // @Todo: Debug variants?
        virtual void* Malloc(size_t size, size_t alignment, size_t offset = 0) = 0;
        virtual void Free(void* block) = 0;

        // Alignment is inferred from the type.
        template <class T>
        T* Malloc_n(uint32_t count, size_t offset = 0)
        { return static_cast<T*>(Malloc(count * sizeof(T), alignof(T), offset)); }

    protected:
        uint32_t m_NumAllocations = 0;
    };

    IAllocator* GetFallbackAllocator();
}