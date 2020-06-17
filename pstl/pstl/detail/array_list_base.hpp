#pragma once

#include "pstl\detail\common.hpp"

namespace pstl {
namespace detail {

    template <class T, bool assert_bounds_>
    class array_list_base;

    template <class T>
    class array_list_base<T, true>
    {
    public:
        pstl_inline__ array_list_base() = default;
        pstl_inline__ array_list_base(T* begin, T* end) : m_begin(begin), m_end(end) { }
        pstl_inline__ array_list_base(T* begin, size_t n) : m_begin(begin), m_end(begin + n) { }

        constexpr T& operator[](size_t index)
        {
            assert(index < (size_t)(m_end - m_begin));
            return m_begin[index]; 
        }

        constexpr const T& operator[](size_t index) const 
        {
            assert(index < (size_t)(m_end - m_begin));
            return m_begin[index]; 
        }

    protected:
        T* m_begin = nullptr;
        T* m_end = nullptr;
    };

    template <class T>
    class array_list_base<T, false>
    {
    public:
        pstl_inline__ array_list_base() = default;
        pstl_inline__ array_list_base(T* begin, T* end) : m_begin(begin), m_end(end) { }
        pstl_inline__ array_list_base(T* begin, size_t n) : m_begin(begin), m_end(begin + n) { }

        constexpr       T& operator[](size_t index)       { return m_begin[index]; }
        constexpr const T& operator[](size_t index) const { return m_begin[index]; }

    protected:
        T* m_begin = nullptr;
        T* m_end = nullptr;
    };

}}
