#pragma once

#include "..\vx\Allocator.hpp"

namespace vstl {

    template <class T, class H = std::hash<T>>
    class RobinHoodSet
    {
    public:
        using value_type = T;
        
    private:
        vx::IAllocator* m_alloc;

        size_t m_capacity;
        size_t m_count;
        uint8_t* m_probes;
        value_type* m_values;

        static const uint8_t  k_empty    = 0b1000'0000; // 0x80
        static const uint32_t k_empty_x4 = 0x80'80'80'80;

    public:
        template <bool is_const_>
        struct Iterator
        {
        private:
            using Set_t = std::conditional_t<is_const_, const RobinHoodSet, RobinHoodSet>;
            using Value_t = std::conditional_t<is_const_, const T, T>;

            Set_t* m_set;
            size_t m_pos;

        public:
            Iterator() = default;

            Iterator(Set_t* set)             : m_set(set), m_pos(0)   { }
            Iterator(Set_t* set, size_t pos) : m_set(set), m_pos(pos) { }

            vx_inline__ T& operator *  () { return m_set->m_values[m_pos]; }
            vx_inline__ T& operator -> () { return m_set->m_values[m_pos]; }

            Iterator& operator ++ ()
            {
                const auto dst = m_set->m_probes;
                const auto cap = m_set->m_capacity;

                assert(m_pos < cap);

                do { m_pos++; } 
                while (dst[m_pos] == k_empty && m_pos < cap);

                return *this;
            }

            #define compare_op__(Op__)\
            vx_inline__ bool operator Op__ (const Iterator& other) const\
            { assert(m_set == other.m_set); return m_pos Op__ other.m_pos; }

            compare_op__(==)
            compare_op__(!=)
            compare_op__(< )
            compare_op__(> )
            compare_op__(<=)
            compare_op__(>=)

            #undef compare_op__
        };

    public:
        using iterator = Iterator<false>;
        using const_iterator = Iterator<true>;

    public:
        RobinHoodSet() : 
            m_alloc(vx::fallback_allocator()),
            m_probes(nullptr), 
            m_values(nullptr), 
            m_count(0), 
            m_capacity(0) {}
        
        RobinHoodSet(size_t cap) : 
            m_alloc(vx::fallback_allocator()),
            m_count(0),
            m_capacity(calc_capacity(cap))
        {
            alloc(m_capacity);
        }

        ~RobinHoodSet()
        { 
            m_alloc->free(m_probes);
            m_alloc->free(m_values);
        }

        vx_inline__ size_t size() const            { return m_count; }
        vx_inline__ size_t capacity() const        { return m_capacity; }
        vx_inline__ float  load_factor() const     { return (float)m_count / m_capacity; }

        static constexpr uint8_t max_probe()       { return k_empty; }
        static constexpr float   max_load_factor() { return (float)max_current_size(8) / 8.0f; }
        
        static constexpr size_t max_current_size(size_t cap) 
        { return (cap - (cap >> 3)); }

        static constexpr bool requires_rehash(size_t size, size_t cap) 
        { return size >= max_current_size(cap); }

        vx_inline__       iterator begin()        { return { this }; }
        vx_inline__ const_iterator begin()  const { return { this }; }
        vx_inline__ const_iterator cbegin() const { return { this }; }

        vx_inline__       iterator end()          { return       iterator(this, m_capacity); }
        vx_inline__ const_iterator end()    const { return const_iterator(this, m_capacity); }
        vx_inline__ const_iterator cend()   const { return const_iterator(this, m_capacity); }

        std::pair<iterator, bool> insert(const value_type& value_to_insert)
        {
            if (m_values == nullptr)
            {
                m_capacity = 8;
                alloc(m_capacity);
            }
            else if (requires_rehash(m_count + 1, m_capacity))
                rehash(m_capacity << 1);

            return insert_impl(value_to_insert);
        }

        iterator find(const value_type& value)
        {
            const auto hash = H()(value);
            const auto cap_mod = (m_capacity - 1);
            const auto pos = hash & cap_mod;

            auto p = pos;
            auto d = 0;

            do
            {
                if (value == m_values[p])
                    return { this, p };

                if (k_empty == m_probes[p])
                    break;

                d++;
                p = (p + 1) & cap_mod; // wrap-around.
            }
            while (d < k_empty && p != pos);

            return { this, m_capacity };
        }

        void probe_distribution(uint8_t* buf, size_t size) const
        {
            auto max = 0U;
            memset(buf, 0, size);

            for (auto i = 0ULL; i < m_capacity; i++)
            {
                auto p = m_probes[i];
                if (p == k_empty)
                    continue;

                assert(p < size);

                buf[p]++;
                max = vx::max(max, p);
            }
        }

    private:
        // Power of two capacity.
        static constexpr vx_inline__ size_t calc_capacity(size_t n)
        {
            n--;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;

            return n + 1;
        }

        vx_inline__ void alloc(size_t cap)
        {
            m_probes = m_alloc->malloc_n<uint8_t>(cap);
            m_values = m_alloc->malloc_n<T>(cap);
            memset(m_probes, k_empty_x4, cap);
        }

        void rehash(size_t new_cap)
        {
            auto probes = m_probes;
            auto values = m_values;
            auto cap = m_capacity;

            alloc(new_cap);
            m_count = 0;
            m_capacity = new_cap;

            for (auto i = 0ULL; i < cap; i++)
            {
                auto p = probes[i];
                if (p == k_empty)
                    continue;

                insert(values[i]);
            }

            m_alloc->free(probes);
            m_alloc->free(values);
        }

        std::pair<iterator, bool> insert_impl(const value_type& value_to_insert)
        {
            const auto hash = H()(value_to_insert);
            const auto cap_mod = (m_capacity - 1);
            const auto ideal_pos = hash & cap_mod;

            auto pos = ideal_pos;
            uint8_t probe = 0U;
            auto value = value_to_insert;
            auto pos_to_return = pos;

            do
            {
                if (k_empty == m_probes[pos])
                {
                    m_probes[pos] = probe;
                    m_values[pos] = value;
                    m_count++;

                    // Return the requested insertion's position.
                    return { { this, pos_to_return }, true };
                }

                if (value_to_insert == m_values[pos])
                    return { { this, pos_to_return }, false };

                if (m_probes[pos] < probe)
                {
                    std::swap(probe, m_probes[pos]);
                    std::swap(value, m_values[pos]);
                }

                probe++;
                pos = (pos + 1) & cap_mod; // wrap-around.

                if (value == value_to_insert)
                    pos_to_return = pos;

                // Should never exceed this probe length.
                // Could be because the map has a very-high load-factor.
                assert(probe < k_empty);
            }
            while (pos != ideal_pos);

            // We should ideally never reach this.
            return { { this, m_capacity }, false };
        }
    };

}