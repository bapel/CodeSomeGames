#include "pstl\Allocator.hpp"
#include <malloc.h>

namespace detail
{
    struct FallbackAllocator final : pstl::IAllocator
    {
        void* Malloc(size_t size, size_t alignment, size_t offset) override final
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

detail::FallbackAllocator FallbackAllocator;

pstl::IAllocator* pstl::GetFallbackAllocator()
{ return &FallbackAllocator; }