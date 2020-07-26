#pragma once

#include "..\vx\Allocator.hpp"
#include <emmintrin.h>
#include <intrin.h>

namespace vstl {

    template <class T, class Hash = std::hash<T>>
    class UnorderedSet1
    {
    private:
        union Bucket
        {
            T value;
            T* values;
        };

    public:
        template <bool is_const_>
        struct Iterator
        {
        private:
            using set_type = std::conditional_t<is_const_, const UnorderedSet1, UnorderedSet1>;
            using value_type = std::conditional_t<is_const_, const T, T>;

            set_type* m_set;
            size_t m_pos;
            size_t m_pos_in_bucket;

        public:
            Iterator() = default;

            Iterator(set_type* set) : m_set(set), m_pos(0), m_pos_in_bucket(0) { }

            Iterator(set_type* set, size_t bucket) : 
                m_set(set), m_pos(bucket), m_pos_in_bucket(0)
            { }

            Iterator(set_type* set, size_t bucket, size_t pos_in_bucket) : 
                m_set(set), m_pos(bucket), m_pos_in_bucket(pos_in_bucket)
            { }

            vx_inline__ T& operator * () 
            {
                assert(m_pos < m_set->bucket_count());

                const auto bucket = m_set->m_bucket[m_pos];
                
                if (bucket_count() == 1)
                    return bucket.values[m_pos_in_bucket];

                return bucket.value;
            }

            vx_inline__ T& operator -> () { return *this; }

            Iterator& operator ++ ()
            {
                assert(m_pos < m_set->bucket_count());

                m_pos_in_bucket++;

                if (m_pos_in_bucket >= m_set->m_bucket_size[m_pos])
                {
                    next_bucket();
                    return *this;
                }

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

        private:
            vx_inline__ size_t bucket_count() const
            { return m_set->m_bucket_end - m_pos; }

            vx_inline__ next_bucket()
            {
                while (m_set->m_bucket_size[++m_pos] == 0 
                    || m_pos < m_set->bucket_count());

                m_pos_in_bucket = 0;
            }
        };

    public:
        using iterator = Iterator<false>;
        using const_iterator = Iterator<true>;

    private:
        size_t m_count;
        size_t m_bucket_count;
        uint8_t* m_bucket_size;
        Bucket* m_buckets;
        Bucket* m_buckets_end;
        vx::IAllocator* m_allocator;

    public:
        UnorderedSet1() :
            m_count(0),
            m_bucket_count(0),
            m_bucket_size(0),
            m_buckets(nullptr),
            m_buckets_end(nullptr),
            m_allocator(nullptr)
        { }

        ~UnorderedSet1()
        {
            for (auto i = 0ULL; i < m_bucket_size; i++)
            {
                if (m_bucket_size[i] > 1)
                    m_allocator->free(m_buckets[i].values);
            }
            
            m_allocator->free(m_bucket_size);
            m_allocator->free(m_buckets); 
        }

        vx_inline__ explicit UnorderedSet1(size_t count) :
            UnorderedSet1(count, vx::fallback_allocator())
        { }

        vx_inline__ explicit UnorderedSet1(size_t count, vx::IAllocator* allocator)
        {
            const auto nb = calc_bucket_count(count);

            m_count = 0;
            m_bucket_count = nb;
            m_bucket_size = allocator->malloc_n<uint8_t>(nb);
            m_buckets = allocator->malloc_n<Bucket>(nb);
            m_buckets_end = m_buckets + nb;
            m_allocator = allocator;

            memset(m_bucket_size, 0, nb);
        }

        vx_inline__ size_t size() const         { return m_count; }
        vx_inline__ size_t bucket_count() const { return m_buckets_end - m_buckets; }
        vx_inline__ float load_factor() const   { return (float)size() / capacity(); }

        vx_inline__ iterator begin() { return { this }; }
        vx_inline__ iterator end()   { return { this, bucket_count() }; }

        vx_inline__ ConstIterator cbegin() const 
        { return ConstIterator(m_buckets,     0, *this); }

        vx_inline__ ConstIterator cend() const 
        { return ConstIterator(m_buckets_end, 0, *this); }

        std::pair<Iterator, bool> insert(const T& value)
        {
            const auto hash = Hash()(value);
            const auto buckets_size = bucket_count();
            const auto b = hash & (buckets_size - 1);

            // @Todo: implement rehash/growth.
            //if (m_buckets[b].header.count == 14)
            //{
            //    rehash(m_num_buckets << 1);
            //    b = hash & (m_num_buckets - 1);
            //}

            
        }

        Iterator find(const T& value)
        {
            const auto hash = Hash()(value);
            const auto h1 = H1(hash);
            const auto nb = buckets();
            auto b = hash & (nb - 1);

            while (true)
            {
                const __m128i ctrl = _mm_load_si128(&m_buckets[b].m128);
                int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(ctrl, set1s[h1])) >> 2;

                auto i = 0ULL;
                while (mask != 0)
                {
                    if (mask & 1)
                    {
                        if (value == m_buckets[b].values[i])
                            return { &m_buckets[b].values[i], m_buckets_end };

                        mask &= ~1;
                    }

                    auto shift = _tzcnt_u32(mask);
                    mask >>= shift;
                    i += shift;
                }

                if (m_buckets[b].header.count != 14)
                    return { m_buckets_end, m_buckets_end };

                b = (b + 1) & (nb - 1);
            }

            return { m_buckets_end, m_buckets_end };
        }

    private:
        static vx_inline__ size_t calc_bucket_count(size_t count)
        {
            auto d = std::div(count, 14LL);
            if (d.rem == 0) 
                return d.quot;

            auto n = d.quot + 1;

            n--;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;

            return n + 1;
        }

        static vx_inline__ uint8_t H1(uint64_t hash) 
        { return static_cast<uint8_t>(hash & 0b0111'1111); }

        //static vx_inline__ uint8_t H1(uint64_t hash) 
        //{ return static_cast<uint8_t>(hash >> 57); }
    };

}