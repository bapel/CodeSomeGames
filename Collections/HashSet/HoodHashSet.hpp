#pragma once

#include "../Allocator.hpp"
#include <functional> // std::hash
#include <intrin.h> // __lzcnt

Namespace__
{
    template <class K, class H = std::hash<K>>
    class HoodHashSet
    {
    public:
        using KeyType = K;
        using HashFunction = H;

        HoodHashSet() = default;
        HoodHashSet(IAllocator* allocator) : m_Allocator(allocator)  {}

        HoodHashSet(CountType capacity, IAllocator* allocator = GetFallbackAllocator()) :
            HoodHashSet(allocator)
        { EnsureCapacity(capacity); }

        ~HoodHashSet()
        { m_Allocator->Free(m_Probes); }

        CountType Count() const { return m_Count; }
        CountType Capacity() const { return m_Capacity; }

        bool Add(const KeyType& key)
        { return Add(key, Hash(key)); }

        // Close to 100% load can cause probe/displacement to grow very large.
        // So we keep the limit at 15/16 (=0.9375) of Capacity.
        bool ShouldRehash() const
        { return (m_Count + 1) >= (m_Capacity - (m_Capacity >> 1)); }

        bool Add(const KeyType& key, uint64_t hash)
        {
            if (m_Capacity == 0)
                Rehash(16);

            if (ShouldRehash())
                Rehash(m_Capacity << 1);

            //const auto hash = Hash(key);
            const auto index = Index(hash);
            
            auto k = key;
            uint8_t h = H2(hash);
            uint8_t p = 0;

            auto pos = index;
            do
            {
                if (k_Empty == m_Hashes[pos])
                {
                    SetProbeAndHash(pos, p, h);
                    m_Slots[pos] = k;
                    m_Count++;
                    return true;
                }

                if (key == m_Slots[pos])
                    return false;
                
                if (m_Probes[pos] < p)
                    Swap(pos, &p, &h, &k);

                p++;
                pos = (pos + 1) & (m_Capacity - 1);

            #ifdef HashMetrics__
                MaxProbeLength = Max(p, MaxProbeLength);
            #endif

                // Probe/displacement could hit 255. So we'd want to rehash the table.
                // This however just reports a failure and leaves it up to the caller.
                // Note that this only (almost-always?) happens with high load-factors.
                if (p == 255)
                    return false;
            }
            // @Todo: Have a max probe length? instead of wrapping around the whole table.
            while (pos != index);
           
            // @Todo: Maybe we can rehash and try again? Will we ever reach here?
            return false;
        }

        #undef Write_Hash_and_Probe__

        bool Contains(ConstRefType<KeyType> key) const
        {
            assert(m_Capacity > 0);
            auto [found, index] = Find(key, Hash(key));
            return found;
        }

        bool Contains(ConstRefType<KeyType> key, uint64_t hash) const
        {
            assert(m_Capacity > 0);
            auto [found, index] = Find(key, hash);
            return found;
        }

        // @Todo: This is not implemented.
        // This should move following slots back on deletion.
        bool Remove(ConstRefType<KeyType> key)
        {
            assert(m_Capacity > 0);

            const auto hash = Hash(key);
            auto [found, index] = Find(key, hash);

            if (found)
            {
                m_Hashes[index] = k_Empty;
                m_Count--;
                return true;
            }

            return false;
        }

        void Clear()
        { memset(m_Hashes, k_Empty_x4, m_Capacity); }

        void EnsureCapacity(CountType minCapacity)
        { Rehash(CalcCapacity(minCapacity)); }

        void GetProbes(uint8_t* outProbes, CountType count) const
        {
            auto j = 0U;
            for (auto i = 0U; i < m_Capacity; i++)
            {
                if (k_Empty == m_Hashes[i])
                    continue;

                assert(j < count);
                outProbes[j++] = m_Probes[i];
            }
        }

        void GetSlots(KeyType* outSlots, CountType count) const
        {
            auto j = 0U;
            for (auto i = 0U; i < m_Capacity; i++)
            {
                if (k_Empty == m_Hashes[i])
                    continue;

                assert(j < count);
                outSlots[j++] = m_Slots[i];
            }
        }

    private:
        // Capacity is always a power of two.
        static CountType CalcCapacity(CountType n)
        {
            n--;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
            return n + 1;
        }

        static uint64_t Hash(KeyType x) { return HashFunction()(x); }
        static uint64_t H1(uint64_t hash) { return hash >> 7; }
        static uint8_t  H2(uint64_t hash) { return hash & 0b0111'1111U; }

        IndexType Index(uint64_t hash) const
        { return hash & (m_Capacity - 1); }

        void SetProbeAndHash(IndexType pos, uint8_t probe, uint8_t hash)
        {
            m_Probes[pos] = probe;
            m_Hashes[pos] = hash;

            if (pos < 16) 
            {
                m_Probes[pos + m_Capacity] = probe;
                m_Hashes[pos + m_Capacity] = hash;
            }
        }

        void Swap(IndexType pos, uint8_t* p, uint8_t* h, KeyType* k)
        {
            auto p_ = m_Probes[pos];
            auto h_ = m_Hashes[pos];
            auto k_ = m_Slots[pos];

            SetProbeAndHash(pos, *p, *h);
            m_Slots[pos] = *k;

            *p = p_;
            *h = h_;
            *k = k_;
        }

        //std::pair<bool, IndexType> Find(const KeyType& key, uint64_t hash) const
        //{
        //    const auto index = Index(hash);
        //    const auto h2 = H2(hash);

        //    auto pos = index;
        //    do
        //    {
        //        if (k_Empty == m_Hashes[pos])
        //            return { false, pos };

        //        if (h2 == m_Hashes[pos] && key == m_Slots[pos])
        //            return { true, pos };

        //        pos = (pos + 1) & (m_Capacity - 1);
        //    }
        //    // @Todo: Have a max probe length? instead of wrapping around the whole table.
        //    while (pos != index);

        //    return { false, -1 };
        //}

        __inline std::pair<bool, IndexType> Find(const KeyType& key, uint64_t hash) const
        {
            const auto index = Index(hash);
            const auto h2 = H2(hash);
            const auto mask = m_Capacity - 1;
            const auto value = set1s[h2];//_mm_set1_epi8(h2);

            auto pos = index;
            do
            {
                const auto hashes = *reinterpret_cast<__m128i*>(m_Hashes + pos);
                auto result = _mm_movemask_epi8(_mm_cmpeq_epi8(value, hashes));

                auto i = 0;
                while (result != 0)
                {
                    if (result & 1)
                    {
                        const auto offset = (pos + i) & mask;
                        if (key == m_Slots[offset])
                            return { true, offset };

                        result >>= 1;
                        i++;
                        continue;
                    }

                    unsigned long shift;
                    _BitScanForward64(&shift, result);
                    result >>= shift;
                    i += shift;
                }

                result = _mm_movemask_epi8(_mm_cmpeq_epi8(k_Empty_x16, hashes));
                if (result != 0)
                    return { false, pos };

                pos = (pos + 16) & mask;
            }
            // @Todo: Have a max probe length? instead of wrapping around the whole table.
            while (pos != index);

            return { false, -1 };
        }

        void Rehash(CountType newCapacity)
        {
            assert(newCapacity > m_Capacity);

            // src for copying.
            auto probes = m_Probes;
            auto hashes = m_Hashes;
            auto slots = m_Slots;
            auto capacity = m_Capacity;
            auto count = m_Count;

            auto retries = 0U;
        retry:
            // Too many reattempts? Got too much clustering.
            // Require a better hash function.
            assert(retries < 4);

            {
                auto sizeOfProbes = newCapacity + 16;
                auto sizeOfHashes = newCapacity + 16;
                auto sizeOfSlots = newCapacity * sizeof(KeyType);
                auto totalSize = sizeOfSlots + (sizeOfProbes + sizeOfHashes);

                m_Probes = (uint8_t*)m_Allocator->Malloc(totalSize, 16);
                m_Hashes = (uint8_t*)(m_Probes + sizeOfProbes);
                m_Slots = (KeyType*)(m_Probes + sizeOfProbes + sizeOfHashes);
                m_Capacity = newCapacity;
                m_Count = 0;

                memset(m_Probes, 0, sizeOfProbes);
                memset(m_Hashes, k_Empty_x4, sizeOfHashes);
            }

            //std::cout 
            //    << count << " / " << capacity 
            //    << " Rehash: " << capacity << " to: " << newCapacity << std::endl;

            for (auto i = 0U; i < capacity; i++)
            {
                if (k_Empty != hashes[i])
                {
                    bool success = false;

                    if (i == 1867)
                        success = Add(slots[i], Hash(slots[i]));
                    else
                        success = Add(slots[i], Hash(slots[i]));

                    if (!success)
                    {
                        // Failure because probe/displacement got large? Rehash.
                        newCapacity << 1;
                        retries++;
                        goto retry;
                    }
                }
            }

            m_Allocator->Free(probes);
        }
    
    private:
        const static uint8_t k_Empty = 0b1000'0000; // 0x80
        const static uint32_t k_Empty_x4 = 0x80808080; // k_Empty x4
        const static inline auto k_Empty_x16 = _mm_set1_epi8(k_Empty);

        IAllocator* m_Allocator = GetFallbackAllocator();
        uint8_t* m_Probes = nullptr;
        uint8_t* m_Hashes = nullptr;
        KeyType* m_Slots = nullptr;
        CountType m_Count = 0;
        CountType m_Capacity = 0;

    public:
    #ifdef HashMetrics__
        static inline uint32_t MaxProbeLength = 0;
    #endif
    };
}