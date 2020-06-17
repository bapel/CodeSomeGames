#pragma once

#include "pstl\detail\common.hpp"

namespace pstl {
namespace detail {

    template <class T, size_t n_, bool assert_bounds_>
    class array_base;

    template <class T, size_t n_>
    class array_base<T, n_, true>
    {
    public:
        constexpr T& operator[](size_t index)
        {
            assert(index < n_);
            return m_items[index]; 
        }

        constexpr const T& operator[](size_t index) const 
        {
            assert(index < n_);
            return m_items[index]; 
        }

    protected:
        T m_items[n_];
    };

    template <class T, size_t n_>
    class array_base<T, n_, false>
    {
    public:
        constexpr       T& operator[](size_t index)       { return m_items[index]; }
        constexpr const T& operator[](size_t index) const { return m_items[index]; }

    protected:
        T m_items[n_];
    };

}}
