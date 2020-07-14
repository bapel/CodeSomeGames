#pragma once

#include "..\vx\Allocator.hpp"
#include <intrin.h>

namespace vstl {
namespace detail {

    struct PowerOfTwoPolicy
    {
        static constexpr size_t translate_capacity(size_t cap) { return cap; }
        static constexpr size_t init_capacity()                { return 8; }
        static constexpr size_t next_capacity(size_t cap)      { return cap << 1; }
        static constexpr size_t calc_capacity(size_t cap)
        {
            cap--;
            cap |= cap >> 1;
            cap |= cap >> 2;
            cap |= cap >> 4;
            cap |= cap >> 8;
            cap |= cap >> 16;
            cap |= cap >> 32;
            return cap + 1;
        }

        static constexpr size_t max_size(size_t cap) 
        { return (cap - (cap >> 3)); }

        static constexpr size_t mod(uint64_t hash, size_t cap)
        { return hash & (cap - 1); }
    };

    struct MersennePolicy
    {
        static constexpr size_t translate_capacity(size_t s) { return (1ULL << s) - 1; }
        static constexpr size_t init_capacity()              { return 3; }
        static constexpr size_t next_capacity(size_t s)      { return s + 1; }
        static constexpr size_t calc_capacity(size_t cap)
        {
            uint64_t v = cap;
            uint64_t r = 0;
            uint64_t shift = 0;

            r =     static_cast<uint64_t>(v > 0xFFFFFFFF) << 8ULL; v >>= r;
            shift = static_cast<uint64_t>(v > 0xFFFF    ) << 4ULL; v >>= shift; r |= shift;
            shift = static_cast<uint64_t>(v > 0xFF      ) << 3ULL; v >>= shift; r |= shift;
            shift = static_cast<uint64_t>(v > 0xF       ) << 2ULL; v >>= shift; r |= shift;
            shift = static_cast<uint64_t>(v > 0x3       ) << 1ULL; v >>= shift; r |= shift;
            r |= (v >> 1);

            return r + 1; // round up.
        }

        static constexpr size_t max_size(size_t s)
        {
            auto cap = translate_capacity(s);
            return (cap - (cap >> 3)) - 1;
        }

        static constexpr size_t mod(uint64_t n, size_t s)
        {
            uint64_t m = 0;                        // n % d goes here.
            const uint64_t d = (1ULL << s) - 1ULL; // so d is either 1, 3, 7, 15, 31, ...).

            for (m = n; n > d; n = m)
                for (m = 0ULL; n; n >>= s)
                    m += n & d;

            // Now m is a value from 0 to d, but since with modulus division
            // we want m to be 0 when it is d.
            m = m == d ? 0ULL : m;
            return m;
        }
    };

}}

namespace vstl {

    template 
    <
        class T, 
        class H = std::hash<T>,
        class B = detail::PowerOfTwoPolicy
    >
    class RobinHoodSet
    {
    public:
        using value_type = T;
        using bucket_policy = B;
        
    private:
        vx::IAllocator* m_alloc;

        size_t m_count;
        size_t m_capacity;
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
                const auto cap = m_set->capacity();

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
            m_capacity(0),
            m_count(0),
            m_probes(nullptr), 
            m_values(nullptr)
        {}
        
        RobinHoodSet(size_t cap) : 
            m_alloc(vx::fallback_allocator()),
            m_capacity(bucket_policy::calc_capacity(cap)),
            m_count(0)
        {
            allocate(bucket_policy::translate_capacity(m_capacity));
        }

        ~RobinHoodSet()
        { 
            m_alloc->free(m_probes);
            m_alloc->free(m_values);
        }

        vx_inline__ size_t size() const           { return m_count; }
        vx_inline__ size_t capacity() const
        { return bucket_policy::translate_capacity(m_capacity); }

        vx_inline__ float  load_factor() const    { return (float)m_count / capacity(); }
        static constexpr uint8_t max_probe()      { return k_empty; }

        vx_inline__       iterator begin()        { return { this }; }
        vx_inline__ const_iterator begin()  const { return { this }; }
        vx_inline__ const_iterator cbegin() const { return { this }; }

        vx_inline__       iterator end()          { return       iterator(this, capacity()); }
        vx_inline__ const_iterator end()    const { return const_iterator(this, capacity()); }
        vx_inline__ const_iterator cend()   const { return const_iterator(this, capacity()); }

        std::pair<iterator, bool> insert(const value_type& value_to_insert)
        {
            if (m_values == nullptr)
            {
                m_capacity = bucket_policy::init_capacity();
                allocate(bucket_policy::translate_capacity(m_capacity));
            }
            else if ((m_count + 1) >= bucket_policy::max_size(m_capacity))
                rehash(bucket_policy::next_capacity(m_capacity));

            return insert_impl(value_to_insert);
        }

        iterator find(const value_type& value)
        {
            const auto hash = H()(value);
            const auto pos = bucket_policy::mod(hash, m_capacity);

            auto p = pos;
            auto d = 0;

            do
            {
                if (value == m_values[p])
                    return { this, p };

                if (k_empty == m_probes[p])
                    break;

                d++;
                p = bucket_policy::mod(p + 1, m_capacity);
            }
            while (d < k_empty && p != pos);

            return { this, capacity() };
        }

        void probe_distribution(uint8_t* buf, size_t size) const
        {
            auto max = 0U;
            memset(buf, 0, size);

            for (auto i = 0ULL; i < capacity(); i++)
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
        vx_inline__ void allocate(size_t cap)
        {
            assert(cap > 0);

            m_probes = m_alloc->malloc_n<uint8_t>(cap);
            m_values = m_alloc->malloc_n<T>(cap);

            memset(m_probes, k_empty_x4, cap);
        }

        void rehash(size_t new_cap)
        {
            // @Todo: Support shrinkage?
            assert(new_cap > m_capacity);

            auto probes = m_probes;
            auto values = m_values;
            auto cap = m_capacity;

            m_count = 0;
            m_capacity = new_cap;

            allocate(bucket_policy::translate_capacity(new_cap));

            for (auto i = 0ULL; i < bucket_policy::translate_capacity(cap); i++)
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
            const auto ideal_pos = bucket_policy::mod(hash, m_capacity);

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
                pos = bucket_policy::mod(pos + 1, m_capacity);

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