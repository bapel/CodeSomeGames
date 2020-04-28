#pragma once

#include "Platform.hpp"
#include "FreeList.hpp"

namespace pfk {

    class AllocTracker;

    class IAllocator
    {
    private:
        AllocTracker* m_Tracker;

    public:
        virtual void* Alloc(size_t size, size_t align, const char* file, uint32_t line) = 0;
        virtual void Free(void* block) = 0;

        inline void SetTracker(AllocTracker* tracker) { m_Tracker = tracker; }
        inline AllocTracker* GetTracker() { return m_Tracker; }
    };

    namespace memory {

        void Init();
        void Shutdown();

        IAllocator* GetDefaultAllocator();

        void* Alloc(IAllocator* allocator, size_t size, size_t align, const char* file, uint32_t line);
        void Free(IAllocator* allocator, void* block);

    }

}

#define pfk_malloc(allocator__, size__, align__)\
    pfk::memory::Alloc(allocator__, size__, align__, __FILE__, __LINE__);

#define pfk_free(allocator__, block__)\
    pfk::memory::Free(allocator__, block__);
    