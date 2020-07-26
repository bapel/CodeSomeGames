#pragma once

#include "..\vx\Allocator.hpp"
#include <intrin.h>

namespace vstl {
namespace detail {

    struct PowerOfTwoPolicy
    {
        struct Data
        { size_t count; };

        static constexpr size_t bucket_count(Data data)    { return data.count; }

        static constexpr   Data zero_buckets()             { return { 0 }; }
        static constexpr   Data init_buckets()             { return { 8 }; }
        static constexpr   Data next_buckets(Data data)    { return { data.count << 1 }; }
        static constexpr   Data calc_buckets(size_t count)
        {
            auto cap = count;

            // round-up power of two.
            cap--;
            cap |= cap >> 1;
            cap |= cap >> 2;
            cap |= cap >> 4;
            cap |= cap >> 8;
            cap |= cap >> 16;
            cap |= cap >> 32;
            cap = cap + 1;

            // adequate for 0.5 load-factor?
            if (cap >= count << 1)
                return { cap };

            return { cap << 1 };
        }

        // load-factor = 0.5
        static constexpr size_t max_size(Data data) 
        { return data.count >> 1; }

        static constexpr size_t bucket(uint64_t hash, Data data)
        { return (hash) & (data.count - 1); }
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
        template <bool is_const_>
        struct Iterator
        {
        private:
            using set_type = std::conditional_t<is_const_, const RobinHoodSet, RobinHoodSet>;
            using value_type = std::conditional_t<is_const_, const T, T>;

            set_type* m_set;
            size_t m_pos;

        public:
            Iterator() = default;

            vx_inline__ Iterator(set_type* set, size_t pos) : m_set(set), m_pos(pos) { }

            vx_inline__ Iterator(const Iterator&) = default;
            vx_inline__ Iterator(Iterator&&) = default;
            vx_inline__ Iterator& operator = (const Iterator&) = default;
            vx_inline__ Iterator& operator = (Iterator&&) = default;

            vx_inline__ T& operator *  () { return m_set->m_buckets[m_pos]; }
            vx_inline__ T& operator -> () { return m_set->m_buckets[m_pos]; }

            vx_inline__ Iterator& operator ++ ()
            {
                const auto dst = m_set->m_probes;
                const auto cap = m_set->bucket_count();

                assert(m_pos < cap);

                do { m_pos++; } while (dst[m_pos] == k_empty && m_pos < cap);

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

        using value_type = T;
        using iterator = Iterator<false>;
        using const_iterator = Iterator<true>;

        using bucket_policy = B;
        using buckets_data = typename B::Data;

    private:
        size_t m_count;
        uint8_t* m_probes;
        value_type* m_buckets;
        buckets_data m_buckets_data;
        vx::IAllocator* m_allocator;

        static const uint8_t  k_empty    = 0b1000'0000; // 0x80
        static const uint32_t k_empty_x4 = 0x80'80'80'80;

    public:
        RobinHoodSet() : 
            m_count(0),
            m_probes(nullptr), 
            m_buckets(nullptr),
            m_buckets_data(bucket_policy::zero_buckets()),
            m_allocator(vx::fallback_allocator())
        {}
        
        RobinHoodSet(size_t cap) : 
            m_count(0),
            m_buckets_data(bucket_policy::calc_buckets(cap)),
            m_allocator(vx::fallback_allocator())
        {
            allocate(bucket_count());
        }

        ~RobinHoodSet()
        { 
            m_allocator->free(m_probes);
            m_allocator->free(m_buckets);
        }

        vx_inline__ size_t size() const           { return m_count; }
        vx_inline__ size_t bucket_count() const
        { return bucket_policy::bucket_count(m_buckets_data); }

        vx_inline__ float  load_factor() const    { return (float)m_count / bucket_count(); }
        static constexpr uint8_t max_probe()      { return k_empty; }

        vx_inline__       iterator begin()        { return { this, begin_pos() }; }
        vx_inline__ const_iterator begin()  const { return { this, begin_pos() }; }
        vx_inline__ const_iterator cbegin() const { return const_iterator(this, begin_pos()); }

        vx_inline__       iterator end()          { return { this, bucket_count() }; }
        vx_inline__ const_iterator end()    const { return { this, bucket_count() }; }
        vx_inline__ const_iterator cend()   const { return { this, bucket_count() }; }

        std::pair<iterator, bool> insert(const value_type& value_to_insert)
        {
            if (m_buckets == nullptr)
            {
                m_buckets_data = bucket_policy::init_buckets();
                allocate(bucket_count());
            }
            else if ((m_count + 1) > bucket_policy::max_size(m_buckets_data))
                rehash(bucket_policy::next_buckets(m_buckets_data));

            return insert_impl(value_to_insert);
        }

        iterator find(const value_type& value)
        {
            const auto hash = H()(value);
            const auto start_pos = bucket_policy::bucket(hash, m_buckets_data);

            auto pos = start_pos;

            do
            {
                // not found.
                if (k_empty == m_probes[pos])
                    return { this, bucket_count() };

                // found.
                if (value == m_buckets[pos])
                    return { this, pos };

                if (++pos >= bucket_count())
                    pos = 0;
            }
            while (pos != start_pos);

            return { this, bucket_count() };
        }

        // @Todo: erase method.

        void probe_distribution(uint8_t* buf, size_t size) const
        {
            auto max = 0U;
            memset(buf, 0, size);

            for (auto i = 0ULL; i < bucket_count(); i++)
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

            m_probes = m_allocator->malloc_n<uint8_t>(cap);
            m_buckets = m_allocator->malloc_n<T>(cap);

            memset(m_probes, k_empty_x4, cap);
        }

        size_t begin_pos() const
        {
            if (m_count == 0)
                return 0;

            // Skip to first non-empty element.
            auto pos = 0ULL;

            while (pos < bucket_count())
            {
                if (m_probes[pos] != k_empty)
                    return pos;

                pos++;
            }

            return pos;
        }

        void rehash(buckets_data new_buckets_data)
        {
            auto probes = m_probes;
            auto buckets = m_buckets;
            auto buckets_data = m_buckets_data;

            m_count = 0;
            m_buckets_data = new_buckets_data;

            allocate(bucket_policy::bucket_count(new_buckets_data));

            for (auto i = 0ULL; i < bucket_policy::bucket_count(buckets_data); i++)
            {
                auto p = probes[i];
                if (p == k_empty)
                    continue;

                insert_impl(buckets[i]);
            }

            m_allocator->free(probes);
            m_allocator->free(buckets);
        }

        std::pair<iterator, bool> insert_impl(const value_type& value_to_insert)
        {
            const auto hash = H()(value_to_insert);
            const auto ideal_pos = bucket_policy::bucket(hash, m_buckets_data);
            const auto capacity = bucket_policy::bucket_count(m_buckets_data);

            auto pos = ideal_pos;
            uint8_t probe = 0U;
            auto value = value_to_insert;
            auto pos_to_return = pos;

            do
            {
                if (k_empty == m_probes[pos])
                {
                    m_probes[pos] = probe;
                    m_buckets[pos] = value;
                    m_count++;

                    // Return the requested insertion's position.
                    return { { this, pos_to_return }, true };
                }

                if (value_to_insert == m_buckets[pos])
                    return { { this, pos_to_return }, false };

                if (m_probes[pos] < probe)
                {
                    std::swap(probe, m_probes[pos]);
                    std::swap(value, m_buckets[pos]);
                }

                probe++;
                // pos = bucket_policy::mod(pos + 1, m_capacity);
                if (++pos >= capacity)
                    pos = 0;

                if (value == value_to_insert)
                    pos_to_return = pos;

                // Should never exceed this probe length.
                // Could be because the map has a very-high load-factor.
                assert(probe < k_empty);
            }
            while (pos != ideal_pos);

            // We should ideally never reach this.
            return { { this, bucket_count() }, false };
        }
    };

}