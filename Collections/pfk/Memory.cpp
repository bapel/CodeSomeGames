#include "Memory.hpp"
#include "AllocTracker.hpp"
using namespace pfk;

#include <malloc.h>

struct AllocHeader
{
    size_t TrackingIdx;
    void* BlockStart;
};

class MallocAllocator final : public IAllocator
{
public:
    void* Alloc(size_t size, size_t align, const char* file, uint32_t line)
    {
        assert(size > 0);
        assert(align > 0);

        size += sizeof(AllocHeader);
        auto offset = sizeof(AllocHeader);
        uint8_t* block = (uint8_t*)_aligned_offset_malloc(size, align, offset);

        if (block != nullptr)
        {
            auto header = (AllocHeader*)((block + offset) - sizeof(AllocHeader));
            // header->TrackingIdx = Track(size, align, file, line);
            header->BlockStart = block;
            return block + offset;
        }

        return nullptr;
    }

    void Free(void* block)
    {
        assert(block != nullptr);

        auto header = (AllocHeader*)((uint8_t*)block - sizeof(AllocHeader));
        // Untrack(header->TrackingIdx);
        _aligned_free(header->BlockStart);
    }
};

inline void SetTrackingIndex(void* block, size_t idx)
{
    ((AllocHeader*)((uint8_t*)block - sizeof(AllocHeader)))->TrackingIdx = idx;
}

inline size_t GetTrackingIndex(void* block)
{
    return ((AllocHeader*)((uint8_t*)block - sizeof(AllocHeader)))->TrackingIdx;
}

MallocAllocator g_MallocAllocator;
AllocTracker g_MallocTracker;

void pfk::memory::Init()
{
    g_MallocTracker.Init("Malloc");
    g_MallocAllocator.SetTracker(&g_MallocTracker);
}

void pfk::memory::Shutdown()
{
    g_MallocTracker.Shutdown();
}

IAllocator* pfk::memory::GetDefaultAllocator()
{
    return &g_MallocAllocator;
}

void* pfk::memory::Alloc(IAllocator* allocator, size_t size, size_t align, const char* file, uint32_t line)
{
    void* block = allocator->Alloc(size, align, file, line);

    if (block != nullptr)
    {
        auto tracker = g_MallocAllocator.GetTracker();
        auto idx = tracker->Track(size, align, file, line);
        SetTrackingIndex(block, idx);
    }

    return block;
}

void pfk::memory::Free(IAllocator* allocator, void* block)
{
    auto idx = GetTrackingIndex(block);
    g_MallocAllocator.GetTracker()->Untrack(idx);
    return allocator->Free(block);
}