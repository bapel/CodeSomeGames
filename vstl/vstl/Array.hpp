#pragma once

#include "..\vx\common.hpp"

namespace vstl {

    template <class T, size_t n_>
    struct Array final
    {
        vx_assert_trivial__(T);

        T m_values[n_];
        static const auto m_count = n_;

        Array() = default;
        Array(std::initializer_list<T> ilist)
        {
            const auto size = sizeof(T) * vx::min(n_, ilist.size());
            memcpy(m_values, ilist.begin(), size);
        }

        // Element access.

        constexpr T& at(size_t pos)
        {
            assert(pos < m_count);
            return m_values[pos];
        };

        constexpr const T& at(size_t pos) const 
        {
            assert(pos < m_count);
            return m_values[pos];
        };

        constexpr       T& operator[](size_t pos)       { return m_values[pos]; }
        constexpr const T& operator[](size_t pos) const { return m_values[pos]; }

        constexpr       T* data()        { return m_values; }
        constexpr const T* data() const  { return m_values; }

        constexpr       T& front()       { return m_values[0]; }
        constexpr const T& front() const { return m_values[0]; }

        constexpr       T& back()        { return m_values[m_count - 1]; }
        constexpr const T& back() const  { return m_values[m_count - 1]; }

        // Iterators.

        constexpr       T* begin()        { return m_values; }
        constexpr const T* begin() const  { return m_values; }
        constexpr const T* cbegin() const { return m_values; }

        constexpr       T* end()          { return m_values + m_count; }
        constexpr const T* end() const    { return m_values + m_count; }
        constexpr const T* cend() const   { return m_values + m_count; }

        // Capacity.

        constexpr bool empty() const      { return m_count == 0; }
        constexpr size_t size() const     { return m_count; }
        constexpr size_t max_size() const { return m_count; }

        // Operations.

        // @Todo: Specialize for int types? Use SIMD?
        constexpr void fill(const T& value)
        {
            for (auto i = 0U; i < m_count; i++)
                m_values = value;
        }

        constexpr void swap(Array& other)
        {
            T temp[m_count];
            const auto size = sizeof(temp);

            memcpy(temp, other.m_values, size);
            memcpy(other.m_values, m_values, size);
            memcpy(m_values, temp, size);
        }
    };

}