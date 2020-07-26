#pragma once

#include "vx\Allocator.hpp"

template <class T, size_t smallest_ = 64>
struct BuddyAllocator
{
    constexpr k_num_T_in_chunk = smallest_ / sizeof(T);

    struct Chunk { T values[k_num_T_in_chunk]; };

    struct ChunkInfo
    {
        // in-use.
        // split.
    };
    
    Chunk* m_chunks;
    size_t m_chunks_size;
};