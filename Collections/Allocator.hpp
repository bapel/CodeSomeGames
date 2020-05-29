#pragma once

#include "Collections.hpp"

Namespace__
{
    class IAllocator
    {
    public:
        virtual ~IAllocator()
        { assert(m_NumAllocations == 0); }

        // @Todo: Debug variants?
        virtual void* Malloc(SizeType size, SizeType alignment, SizeType offset = 0) = 0;
        virtual void Free(void* block) = 0;

        template <class T>
        T* Malloc_n(CountType count, SizeType alignment = alignof(T), SizeType offset = 0)
        { return dynamic_cast<T*>(Malloc(count * sizeof(T), alignment, offset)); }

    protected:
        CountType m_NumAllocations = 0;
    };

    IAllocator* GetFallbackAllocator();
}