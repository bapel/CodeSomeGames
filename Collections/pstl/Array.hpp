#pragma once

#include "pstl\detail\ArrayBase.hpp"

namespace pstl {
namespace detail {

    template <uint32_t n>
    struct StaticCount
    {
        static const uint32_t m_count = n;
    };

    template <class T, class C>
    struct StaticArrayStore
    {
        __inline T* Start() { return m_start; }
        __inline T* End()   { return m_start + m_count; }

        __inline uint32_t Count() const { return m_count; }

    protected:
        using C::m_count;
        T m_start[m_count];
    };

}}

namespace pstl {

    template <class T, uint32_t n, bool AssertBounds = true>
    using Array = detail::ArrayBase<
        detail::StaticArrayStore, 
        detail::RandomAccess,
        T, detail::StaticCount<n>, AssertBounds>;

}