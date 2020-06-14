#pragma once

#include "pstl\detail\ArrayBase.hpp"

namespace pstl {
namespace detail {

    template <class T, class C = void>
    struct FixedArrayStore
    {
        __inline FixedArrayStore(T* start, T* end) : 
            m_start(start), 
            m_end(end)
        { }

        __inline T* Start() { return m_start; }
        __inline T* End()   { return m_end; }

        __inline uint32_t Count() const 
        { return m_end - m_start; }

    protected:
        T* m_start = nullptr;
        T* m_end = nullptr;
    };
}}

namespace pstl {

    template <class T, bool AssertBounds = true>
    using FixedArray = detail::ArrayBase<
        detail::FixedArrayStore, 
        detail::RandomAccess,
        T, void, AssertBounds>;

}