#pragma once

#include "..\vx\Allocator.hpp"
#include <emmintrin.h>
#include <intrin.h>

namespace vstl {

    const __m128i set1s[] = 
    {
        _mm_set1_epi8(0),
        _mm_set1_epi8(1),
        _mm_set1_epi8(2),
        _mm_set1_epi8(3),
        _mm_set1_epi8(4),
        _mm_set1_epi8(5),
        _mm_set1_epi8(6),
        _mm_set1_epi8(7),
        _mm_set1_epi8(8),
        _mm_set1_epi8(9),
        _mm_set1_epi8(10),
        _mm_set1_epi8(11),
        _mm_set1_epi8(12),
        _mm_set1_epi8(13),
        _mm_set1_epi8(14),
        _mm_set1_epi8(15),
        _mm_set1_epi8(16),
        _mm_set1_epi8(17),
        _mm_set1_epi8(18),
        _mm_set1_epi8(19),
        _mm_set1_epi8(20),
        _mm_set1_epi8(21),
        _mm_set1_epi8(22),
        _mm_set1_epi8(23),
        _mm_set1_epi8(24),
        _mm_set1_epi8(25),
        _mm_set1_epi8(26),
        _mm_set1_epi8(27),
        _mm_set1_epi8(28),
        _mm_set1_epi8(29),
        _mm_set1_epi8(30),
        _mm_set1_epi8(31),
        _mm_set1_epi8(32),
        _mm_set1_epi8(33),
        _mm_set1_epi8(34),
        _mm_set1_epi8(35),
        _mm_set1_epi8(36),
        _mm_set1_epi8(37),
        _mm_set1_epi8(38),
        _mm_set1_epi8(39),
        _mm_set1_epi8(40),
        _mm_set1_epi8(41),
        _mm_set1_epi8(42),
        _mm_set1_epi8(43),
        _mm_set1_epi8(44),
        _mm_set1_epi8(45),
        _mm_set1_epi8(46),
        _mm_set1_epi8(47),
        _mm_set1_epi8(48),
        _mm_set1_epi8(49),
        _mm_set1_epi8(50),
        _mm_set1_epi8(51),
        _mm_set1_epi8(52),
        _mm_set1_epi8(53),
        _mm_set1_epi8(54),
        _mm_set1_epi8(55),
        _mm_set1_epi8(56),
        _mm_set1_epi8(57),
        _mm_set1_epi8(58),
        _mm_set1_epi8(59),
        _mm_set1_epi8(60),
        _mm_set1_epi8(61),
        _mm_set1_epi8(62),
        _mm_set1_epi8(63),
        _mm_set1_epi8(64),
        _mm_set1_epi8(65),
        _mm_set1_epi8(66),
        _mm_set1_epi8(67),
        _mm_set1_epi8(68),
        _mm_set1_epi8(69),
        _mm_set1_epi8(70),
        _mm_set1_epi8(71),
        _mm_set1_epi8(72),
        _mm_set1_epi8(73),
        _mm_set1_epi8(74),
        _mm_set1_epi8(75),
        _mm_set1_epi8(76),
        _mm_set1_epi8(77),
        _mm_set1_epi8(78),
        _mm_set1_epi8(79),
        _mm_set1_epi8(80),
        _mm_set1_epi8(81),
        _mm_set1_epi8(82),
        _mm_set1_epi8(83),
        _mm_set1_epi8(84),
        _mm_set1_epi8(85),
        _mm_set1_epi8(86),
        _mm_set1_epi8(87),
        _mm_set1_epi8(88),
        _mm_set1_epi8(89),
        _mm_set1_epi8(90),
        _mm_set1_epi8(91),
        _mm_set1_epi8(92),
        _mm_set1_epi8(93),
        _mm_set1_epi8(94),
        _mm_set1_epi8(95),
        _mm_set1_epi8(96),
        _mm_set1_epi8(97),
        _mm_set1_epi8(98),
        _mm_set1_epi8(99),
        _mm_set1_epi8(100),
        _mm_set1_epi8(101),
        _mm_set1_epi8(102),
        _mm_set1_epi8(103),
        _mm_set1_epi8(104),
        _mm_set1_epi8(105),
        _mm_set1_epi8(106),
        _mm_set1_epi8(107),
        _mm_set1_epi8(108),
        _mm_set1_epi8(109),
        _mm_set1_epi8(110),
        _mm_set1_epi8(111),
        _mm_set1_epi8(112),
        _mm_set1_epi8(113),
        _mm_set1_epi8(114),
        _mm_set1_epi8(115),
        _mm_set1_epi8(116),
        _mm_set1_epi8(117),
        _mm_set1_epi8(118),
        _mm_set1_epi8(119),
        _mm_set1_epi8(120),
        _mm_set1_epi8(121),
        _mm_set1_epi8(122),
        _mm_set1_epi8(123),
        _mm_set1_epi8(124),
        _mm_set1_epi8(125),
        _mm_set1_epi8(126),
        _mm_set1_epi8(127)
    };

    template <class T, class Hash = std::hash<T>>
    class UnorderedSet
    {
    private:
        __declspec(align(16))
        struct Bucket
        {
            union
            {
                struct {
                    uint8_t count;
                    uint8_t displace;
                    uint8_t control[14];
                } header;
                __m128i m128;
            };
            T values[14];
        };

    public:
        class IteratorBase
        {
        protected:
            T* m_value;
            void* m_end;

        public:
            IteratorBase() = default;

            vx_inline__ IteratorBase(T* value, void* end) :
                m_value(value),
                m_end(end)
            { }

            vx_inline__ IteratorBase(void* value, void* end) :
                m_value(reinterpret_cast<T*>(value)),
                m_end(end)
            { }

            vx_inline__ IteratorBase& operator++ ()
            {
                assert(m_value < m_end);

                m_value++;

                if (reinterpret_cast<uintptr_t>(m_value) & 0xE)
                {
                    auto b = reinterpret_cast<uintptr_t>(m_value);
                    b = (b & 0xffff'ffff'ffff'fff0ULL) + sizeof(Bucket);
                    m_value = reinterpret_cast<T*>(b);
                }
                
                return *this;
            }

            vx_inline__ bool operator <  (const IteratorBase& other) const 
            { return m_value <  other.m_value; }

            vx_inline__ bool operator >  (const IteratorBase& other) const 
            { return m_value >  other.m_value; }

            vx_inline__ bool operator <= (const IteratorBase& other) const 
            { return m_value <= other.m_value; }

            vx_inline__ bool operator >= (const IteratorBase& other) const 
            { return m_value >= other.m_value; }

            vx_inline__ bool operator == (const IteratorBase& other) const 
            { return m_value == other.m_value; }

            vx_inline__ bool operator != (const IteratorBase& other) const 
            { return m_value != other.m_value; }
        };

        class Iterator : public IteratorBase
        {
        private:
            using IteratorBase::m_value;
            
        public:
            using IteratorBase::IteratorBase;

            vx_inline__ T& operator*()  { return *m_value; }
            vx_inline__ T& operator->() { return *m_value; }
        };

        class ConstIterator : public IteratorBase
        {
        private:
            using IteratorBase::m_value;

        public:
            using IteratorBase::IteratorBase;

            vx_inline__ const T& operator*()  const { return *m_value; }
            vx_inline__ const T& operator->() const { return *m_value; }
        };
        
    private:
        size_t m_count;
        Bucket* m_buckets;
        Bucket* m_buckets_end;
        vx::IAllocator* m_allocator;

    public:
        UnorderedSet() :
            m_count(0),
            m_buckets(nullptr),
            m_buckets_end(nullptr),
            m_allocator(nullptr)
        { }

        ~UnorderedSet()
        { m_allocator->free(m_buckets); }

        vx_inline__ explicit UnorderedSet(size_t count) :
            UnorderedSet(count, vx::fallback_allocator())
        { }

        vx_inline__ explicit UnorderedSet(size_t count, vx::IAllocator* allocator)
        {
            const auto nb = calc_bucket_count(count);

            m_count = 0;
            m_buckets = allocator->malloc_n<Bucket>(nb);
            m_buckets_end = m_buckets + nb;
            m_allocator = allocator;

            for (auto bucket = m_buckets; bucket < m_buckets_end; bucket++)
            {
                // set each control to 0b1000'0000 (0x80)
                auto h = reinterpret_cast<uint64_t*>(&bucket->header);
                h[0] = 0x80'80'80'80'80'80'00'00;
                h[1] = 0x80'80'80'80'80'80'80'80;
            }
        }

        vx_inline__ size_t size() const       { return m_count; }
        vx_inline__ size_t buckets() const    { return m_buckets_end - m_buckets; }
        vx_inline__ size_t capacity() const   { return buckets() * 14; }
        vx_inline__ float load_factor() const { return (float)size() / capacity(); }

        vx_inline__ Iterator begin() { return { m_buckets[0].values, m_buckets_end }; }
        vx_inline__ Iterator end()   { return { m_buckets_end, m_buckets_end }; }

        vx_inline__ ConstIterator cbegin() const 
        { return ConstIterator(m_buckets,     0, *this); }

        vx_inline__ ConstIterator cend() const 
        { return ConstIterator(m_buckets_end, 0, *this); }

        std::pair<Iterator, bool> insert(const T& value)
        {
            const auto hash = Hash()(value);
            const auto h1 = H1(hash);
            const auto nb = buckets();
            const auto b = hash & (nb - 1);

            // @Todo: implement rehash/growth.
            //if (m_buckets[b].header.count == 14)
            //{
            //    rehash(m_num_buckets << 1);
            //    b = hash & (m_num_buckets - 1);
            //}

            const auto h1epi8 = _mm_set1_epi8(h1);

            auto b_ = b;
            do
            {
                auto& bucket = m_buckets[b_];
                auto& header = bucket.header;

                const auto a_ = *reinterpret_cast<__m128i*>(&header);
                const auto c_ = _mm_movemask_epi8(_mm_cmpeq_epi8(a_, h1epi8));
                auto result = c_ >> 2;

                auto i = 0ULL;
                while (result != 0)
                {
                    if ((result & 1) && (value == bucket.values[i]))
                    {
                        Iterator iter = { &bucket.values[i], m_buckets_end };
                        return std::make_pair(iter, false);
                    }

                    result >>= 1;
                    i++;
                }

                if (header.count < 14)
                {
                    const auto i = header.count;
                    bucket.values[i] = value;
                    header.control[i] = h1;
                    header.count++;
                    m_count++;

                    Iterator iter = { &bucket.values[i], m_buckets_end };
                    return std::make_pair(iter, true);
                }

                b_ = (b_ + 1) & (nb - 1);
                header.displace++;
            }
            while (b_ != b);

            assert(false); // Should never get here!
            return std::make_pair(end(), false);
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