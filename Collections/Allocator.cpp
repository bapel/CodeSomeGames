#include "Allocator.hpp"
using namespace NamespaceName__;

#include <malloc.h>

namespace internal_
{
    struct FallbackAllocator final : IAllocator
    {
        void* Malloc(SizeType size, SizeType alignment, SizeType offset) override final
        {
            if (size == 0)
                return nullptr;

            auto block = _aligned_offset_malloc(size, alignment, offset);

            if (block != nullptr)
                m_NumAllocations++;

            return block;
        }

        void Free(void* block) override final
        {
            if (block == nullptr)
                return;

            _aligned_free(block);

            assert(m_NumAllocations > 0);
            m_NumAllocations--;
        }
    };
}

internal_::FallbackAllocator FallbackAllocator;

IAllocator* NamespaceName__::GetFallbackAllocator()
{ return &FallbackAllocator; }