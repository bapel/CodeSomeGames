#pragma once

#include "..\vx\Allocator.hpp"
#include <emmintrin.h>

namespace vstl {

    template <class T, class Hash = std::hash<T>>
    class UnorderedSet
    {
    private:
        __declspec(align(16))
        struct Bucket
        {
            struct {
                uint8_t count;
                uint8_t displace;
                uint8_t control[14];
            } header;
            T values[14];
        };

    public:
        class IteratorBase
        {
        protected:
            uintptr_t m_pointer;
            const Bucket* m_buckets_end;

        public:
            IteratorBase(T* bucket, size_t offset, const UnorderedSet& set) :
                m_pointer(static_cast<uintptr_t>(bucket) | offset),
                m_buckets_end(set.m_buckets + set.m_num_buckets)
            { }

            vx_inline__ IteratorBase& operator++ ()
            {
                const auto b = m_pointer | 0xffff'ffff'ffff'fff0;
                auto bucket = *static_cast<Bucket*>(b);
                auto index = m_pointer & 0xf;

                if (++index > bucket->header.count)
                {
                    bucket += 1;
                    index = 0;

                    assert(bucket < m_buckets_end);
                }

                m_pointer = static_cast<uintptr_t>(bucket);
                m_pointer |= index;

                return *this;
            }

            vx_inline__ bool operator <  (const IteratorBase& other) const 
            {
                assert(m_buckets == other.m_buckets);
                return m_pointer <  other.m_pointer; 
            }

            vx_inline__ bool operator >  (const IteratorBase& other) const 
            { 
                assert(m_buckets == other.m_buckets);
                return m_pointer >  other.m_pointer; 
            }

            vx_inline__ bool operator <= (const IteratorBase& other) const 
            { 
                assert(m_buckets == other.m_buckets);
                return m_pointer <= other.m_pointer; 
            }

            vx_inline__ bool operator >= (const IteratorBase& other) const 
            { 
                assert(m_buckets == other.m_buckets);
                return m_pointer >= other.m_pointer; 
            }
        };

        class Iterator : public IteratorBase
        {
        private:
            using IteratorBase::m_pointer;
            
        public:
            using IteratorBase::IteratorBase;

            vx_inline__ T& operator->() 
            {
                const auto b = m_pointer | 0xffff'ffff'ffff'fff0;
                auto bucket = *static_cast<Bucket*>(b);
                auto index = m_pointer & 0xf;
                return bucket.values[index];
            }
        };

        class ConstIterator : public IteratorBase
        {
        private:
            using IteratorBase::m_pointer;

        public:
            using IteratorBase::IteratorBase;

            vx_inline__ const T& operator->() const
            {
                const auto b = m_pointer | 0xffff'ffff'ffff'fff0;
                auto bucket = *static_cast<Bucket*>(b);
                auto index = m_pointer & 0xf;
                return bucket.values[index];
            }
        };
        
    private:
        Bucket* m_buckets;
        size_t m_num_buckets;
        size_t m_count;
        vx::IAllocator* m_allocator;

    public:
        UnorderedSet() :
            m_buckets(nullptr),
            m_num_buckets(0),
            m_count(0),
            m_allocator(nullptr)
        { }

        ~UnorderedSet()
        { m_allocator->free(m_buckets); }

        vx_inline__ explicit UnorderedSet(size_t count) :
            UnorderedSet(count, vx::fallback_allocator())
        { }

        vx_inline__ explicit UnorderedSet(size_t count, vx::IAllocator* allocator)
        {
            m_allocator = allocator;
            m_num_buckets = calc_bucket_count(count);
            m_buckets = m_allocator->malloc_n<Bucket>(m_num_buckets);
            memset(m_buckets, 0, sizeof(Bucket) * m_num_buckets);
            m_count = 0;
        }

        vx_inline__ Iterator  begin() { return Iterator(m_buckets,                 0, *this); }
        vx_inline__ Iterator  end()   { return Iterator(m_buckets + m_num_buckets, 0, *this); }

        vx_inline__ ConstIterator cbegin() const 
        { return ConstIterator(m_buckets,                 0, *this); }

        vx_inline__ ConstIterator cend() const 
        { return ConstIterator(m_buckets + m_num_buckets, 0, *this); }

        vx_inline__ std::pair<Iterator, bool> insert(const T& value)
        {
            const auto hash = Hash()(value);
            const auto h1 = static_cast<uint8_t>(hash & 0b1111'1111);
            auto b = hash & (m_num_buckets - 1);

            // @Todo: implement rehash/growth.
            //if (m_buckets[b].header.count == 14)
            //{
            //    rehash(m_num_buckets << 1);
            //    b = hash & (m_num_buckets - 1);
            //}

            const auto h1epi8 = _mm_set1_epi8(h1);

            while (b < m_num_buckets)
            {
                auto& bucket = m_buckets[b];
                auto& header = bucket.header;

                auto a = *reinterpret_cast<__m128i*>(&header);
                auto result = _mm_movemask_epi8(_mm_cmpeq_epi8(a, h1epi8)) >> 2;

                auto i = 0;
                while (result != 0)
                {
                    if ((result & 1) && (value == bucket.values[i]))
                        return std::make_pair(Iterator(&bucket, i, *this), false);

                    result >>= 1;
                    i++;
                }

                if (header.count < 14)
                {
                    const auto i = header.count;
                    auto& slot = (bucket.values[i] = value);
                    header.control[i] = h1;
                    header.count++;

                    return std::make_pair(Iterator(&bucket, i, *this), true);
                }

                b++;
            }

            assert(false); // Should never get here!
            return std::make_pair(end(), false);
        }

    private:
        vx_inline__ size_t calc_bucket_count(size_t count)
        {
            count = ((count - 1ULL) | 15ULL) + 1ULL;
            return count >> 4;
        }
    };

}