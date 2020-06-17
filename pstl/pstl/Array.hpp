#pragma once

#include "pstl\detail\array_base.hpp"

namespace pstl {

    template <class T, size_t n_, 
    #ifdef pstl_is_debug__
        bool assert_bounds_ = true
    #else
        bool assert_bounds_ = false
    #endif
    >
    class array final : public detail::array_base<T, n_, assert_bounds_>
    {
        pstl_assert_trivial__(T);
        using base = detail::array_base<T, n_, assert_bounds_>;

    private:
        using base::m_items;
        static const auto m_count = n_;

    public:
        array() = default;
        array(std::initializer_list<T> ilist)
        {
            const auto size = sizeof(T) * ilist.size();
            memcpy(m_items, ilist.begin(), min(size, sizeof(m_items)));
        }

        // Element access.

        constexpr T& at(size_t index)
        {
            assert(index < m_count);
            return m_items[index];
        };

        constexpr const T& at(size_t index) const 
        {
            assert(index < m_count);
            return m_items[index];
        };

        constexpr       T* data()        { return m_items; }
        constexpr const T* data() const  { return m_items; }

        constexpr       T& front()       { return m_items[0]; }
        constexpr const T& front() const { return m_items[0]; }

        constexpr       T& back()        { return m_items[m_count - 1]; }
        constexpr const T& back() const  { return m_items[m_count - 1]; }

        // Iterators.

        constexpr       T* begin()        { return m_items; }
        constexpr const T* begin() const  { return m_items; }
        constexpr const T* cbegin() const { return m_items; }

        constexpr       T* end()          { return m_items + m_count; }
        constexpr const T* end() const    { return m_items + m_count; }
        constexpr const T* cend() const   { return m_items + m_count; }

        // Capacity.

        constexpr bool empty() const      { return m_count == 0; }
        constexpr size_t size() const     { return m_count; }
        constexpr size_t max_size() const { return m_count; }

        // Operations.

        constexpr void fill(const T& value)
        {
            for (auto i = 0U; i < m_count; i++)
                m_items = value;
        }

        constexpr void swap(array& other)
        {
            T temp[m_count];
            const auto size = sizeof(temp);

            memcpy(temp, other.m_items, size);
            memcpy(other.m_items, m_items, size);
            memcpy(m_items, temp, size);
        }
    };

}