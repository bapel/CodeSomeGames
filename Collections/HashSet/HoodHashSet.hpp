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

        #define Write_Hash_and_Probe__(I__, P__, H__)\
        {\
            m_Probes[I__] = P__;\
            m_Hashes[I__] = H__;\
            if (I__ < 16) {\
                m_Probes[I__ + m_Capacity] = P__;\
                m_Hashes[I__ + m_Capacity] = H__;\
            }\
        }

        bool Add(const KeyType& key, uint64_t hash)
        {
            if (m_Capacity == 0)
                Rehash(16);

            // Keep load factors close but not close to 100%.
            // That can cause probe/displacement to grow very large.
            // Here we go with a limit of 15/16th (=0.9375) of Capacity.
            if ((m_Count + 1) >= (m_Capacity - (m_Capacity >> 4)))
                Rehash(m_Capacity << 1);

            //const auto hash = Hash(key);
            const auto index = Index(hash);
            
            auto k = key;
            auto h = hash & 0b1111'1111;
            auto p = 0U;

            auto pos = index;
            do
            {
                if (k_Empty == m_Probes[pos])
                {
                    Write_Hash_and_Probe__(pos, p, h);
                    m_Slots[pos] = k;
                    m_Count++;

                    //if (k == 15704)
                    //    std::cout << k << " at: " << pos << " +" << p << std::endl;

                    return true;
                }

                if (key == m_Slots[pos])
                    return false;
                else if (m_Probes[pos] < p)
                {
                    auto p_ = m_Probes[pos];
                    auto h_ = m_Hashes[pos];
                    auto k_ = m_Slots[pos];

                    Write_Hash_and_Probe__(pos, p, h);
                    m_Slots[pos] = k;

                    //if (k == 15704)
                    //    std::cout << k << " at: " << pos << " +" << p << std::endl;

                    p = p_;
                    h = h_;
                    k = k_;
                }

                p++;
                pos = (pos + 1) & (m_Capacity - 1);

                // Probe/displacement could hit 255. So we'd want to rehash the table.
                // This however just reports a failure and leaves it up to the caller.
                // Note that this only (almost-always?) happens with high load-factors.
                if (p == k_Empty)
                    return false;

            #ifdef HashMetrics__
                MaxProbeLength = Max(p, MaxProbeLength);
            #endif
            }
            // @Todo: Have a max probe length? instead of wrapping around the whole table.
            while (pos != index);
           
            // @Todo: Maybe we can rehash and try again? Will we ever reach here?
            return false;
        }

        #undef Write_Hash_and_Probe__

        //__forceinline bool Contains(ConstRefType<KeyType> key) const
        //{
        //    assert(m_Capacity > 0);
        //    auto [found, index] = Find(key, Hash(key));
        //    return found;
        //}

        __forceinline bool Contains(ConstRefType<KeyType> key, uint64_t hash) const
        {
            assert(m_Capacity > 0);
            auto [found, index] = Find(key, hash);
            return found;
        }

        bool Remove(ConstRefType<KeyType> key)
        {
            assert(m_Capacity > 0);

            const auto hash = Hash(key);
            auto [found, index] = Find(key, hash);

            if (found)
            {
                m_Probes[index] = k_Empty;
                m_Count--;
                return true;
            }

            return false;
        }

        __forceinline void Clear()
        { memset(m_Probes, k_Empty32, m_Capacity); }

        void EnsureCapacity(CountType minCapacity)
        { Rehash(CalcCapacity(minCapacity)); }

        void GetKeys(KeyType* outKeys) const
        {
            for (auto i = 0U; i < m_Capacity; i++)
            {
                if (k_Empty == m_Probes[i])
                    continue;

                outKeys.Add(m_Slots[i]);
            }
        }

    private:
        // Capacity is always a power of two.
        __forceinline static CountType CalcCapacity(CountType n)
        {
            n--;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
            return n + 1;
        }

        __forceinline static uint64_t Hash(KeyType x) { return HashFunction()(x); }
        __forceinline IndexType Index(uint64_t hash) const { return hash & (m_Capacity - 1); }

        /*
        __forceinline std::pair<bool, IndexType> Find(const KeyType& key, uint64_t hash) const
        {
            const auto index = Index(hash);
            const auto h = hash & 0b1111'1111;

            auto pos = index;
            do
            {
                if (k_Empty == m_Probes[pos])
                    return { false, pos };

                if (h == m_Hashes[pos] && key == m_Slots[pos])
                    return { true, pos };

                pos = (pos + 1) & (m_Capacity - 1);
            }
            // @Todo: Have a max probe length? instead of wrapping around the whole table.
            while (pos != index);

            return { false, -1 };
        }
        */

        __forceinline std::pair<bool, IndexType> Find(const KeyType& key, uint64_t hash) const
        {
            const auto index = Index(hash);
            const auto h = hash & 0b1111'1111;

            auto pos = index;
            do
            {
                const auto value = _mm_set1_epi8(h);
                const auto hashes = _mm_loadu_si128((__m128i*)(m_Hashes + pos));
                auto result = _mm_movemask_epi8(_mm_cmpeq_epi8(value, hashes));

                auto i = 0;
                while (result != 0)
                {
                    const auto offset = (pos + i) & (m_Capacity - 1);
                    if ((result & 1) && (key == m_Slots[offset]))
                        return { true, offset };

                    result >>= 1;
                    i++;
                }

                const auto empty = _mm_set1_epi8(k_Empty);
                const auto probes = _mm_loadu_si128((__m128i*)(m_Probes + pos));
                result = _mm_movemask_epi8(_mm_cmpeq_epi8(empty, probes));
                if (result != 0)
                    return { false, pos };

                pos = (pos + 16) & (m_Capacity - 1);
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
                size_t sizeOfProbes = newCapacity + 16;
                size_t sizeOfHashes = newCapacity + 16;
                size_t sizeOfSlots = newCapacity * sizeof(KeyType);
                auto totalSize = sizeOfProbes + sizeOfHashes + sizeOfSlots;

                m_Probes = (uint8_t*)m_Allocator->Malloc(totalSize, 16);
                m_Hashes = (uint8_t*)(m_Probes + sizeOfProbes);
                m_Slots = (KeyType*)(m_Probes + sizeOfProbes + sizeOfHashes);
                m_Capacity = newCapacity;
                m_Count = 0;

                memset(m_Probes, k_Empty32, sizeOfProbes);
                memset(m_Hashes, 0, sizeOfHashes);
            }

            //std::cout 
            //    << count << " / " << capacity 
            //    << " Rehash: " << capacity << " to: " << newCapacity << std::endl;

            for (auto i = 0U; i < capacity; i++)
            {
                if (k_Empty != probes[i])
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
        const uint8_t k_Empty = 0b1111'1111; // 0xFF
        const uint32_t k_Empty32 = 0xFFFFFFFF; // k_Empty x4

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