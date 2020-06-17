#include "pstl\allocator.hpp"
#include <malloc.h>

namespace detail
{
    struct malloc_allocator final : pstl::allocator
    {
        void* malloc(size_t size, size_t alignment, size_t offset) override final
        {
            if (size == 0)
                return nullptr;

            auto block = _aligned_offset_malloc(size, alignment, offset);

            if (block != nullptr)
                m_num_allocations++;

            return block;
        }

        void free(void* block) override final
        {
            if (block == nullptr)
                return;

            _aligned_free(block);

            assert(m_num_allocations > 0);
            m_num_allocations--;
        }
    };
}

detail::malloc_allocator fallback_allocator_;

pstl::allocator* pstl::fallback_allocator()
{ return &fallback_allocator_; }