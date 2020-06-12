#pragma once

#include "../Allocator.hpp"
#include <functional> // std::hash
#include <intrin.h> // __lzcnt

namespace NamespaceName__
{
    template <class K, class H = std::hash<K>>
    class ChunkHashSet
    {
    public:
        using KeyType = K;
        using HashFunction = H;

        ChunkHashSet() = default;
        ChunkHashSet(IAllocator* allocator) : m_Allocator(allocator)  {}

        ChunkHashSet(CountType capacity, IAllocator* allocator = GetFallbackAllocator()) :
            SimdHashSet(allocator)
        { EnsureCapacity(capacity); }

        ~ChunkHashSet()
        { m_Allocator->Free(m_Control); }

        CountType Count() const { return m_Count; }
        CountType Capacity() const { return m_Capacity; }

    private:
        template <class T>
        struct Chunk
        {
            uint8_t control[64]; //
            T keys[14];          // 
        };
    };
}