#pragma once

#include "Allocator.hpp"
#include <functional> // std::hash
#include <intrin.h> // __lzcnt

namespace NamespaceName__
{
    template <class K, typename H = std::hash<K>>
    class SimdHashSet
    {
    public:
        using KeyType = K;
        using HashFunction = H;

        SimdHashSet() = default;
        SimdHashSet(IAllocator* allocator) : m_Allocator(allocator)  {}

        SimdHashSet(CountType capacity, IAllocator* allocator = GetFallbackAllocator()) :
            SimdHashSet(allocator)
        { EnsureCapacity(capacity); }

        ~SimdHashSet()
        { m_Allocator->Free(m_Control); }

        CountType Count() const { return m_Count; }
        CountType Capacity() const { return m_Capacity; }

        bool Add(ConstRefType<KeyType> key)
        {
            assert(m_Capacity > 0);

            const auto hash = Hash(key);
            auto [exists, index] = FindForAdd(key, hash);

            if (exists)
                return false;

            auto po2 = __lzcnt(m_Capacity);
            if ((m_Count + 1) >= k_RehashLUT[po2])
            {
                Rehash(m_Capacity << 1);
                std::tie(exists, index) = FindForAdd(key, hash);
            }

            m_Control[index] = H2(hash);
            m_Slots[index] = key;
            m_Count++;

            return true;
        }

        bool Contains(ConstRefType<KeyType> key) const
        {
            assert(m_Capacity > 0);

            const auto hash = Hash(key);
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
                m_Control[index] = k_Deleted;
                m_Count--;
                return true;
            }

            return false;
        }

        void Clear()
        { memset(m_Control, k_Empty32, m_Capacity); }

        void EnsureCapacity(CountType minCapacity)
        { Rehash(CalcCapacity(minCapacity)); }

        void GetKeys(KeyType* outKeys) const
        {
            for (auto i = 0U; i < m_Capacity; i++)
            {
                if (k_Empty == m_Control[i] || k_Deleted == m_Control[i])
                    continue;

                outKeys.Add(m_Slots[i]);
            }
        }

    private:
        // Capacity is always a power of two and a multiple of 16.
        // Note that all powers of two > 16 are multiple of 16.
        inline static CountType CalcCapacity(CountType n)
        {
            if (n < 16)
                return 16;

            n--;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
            return n + 1;
        }

        inline static uint64_t Hash(const KeyType& key)
        { 
            const static auto hasher = HashFunction();
            return hasher(key); 
        }

        inline static uint64_t H1(uint64_t hash) { return hash >> 7; }
        inline static uint8_t  H2(uint64_t hash) { return hash & 0b0111'1111U; }

        void Rehash(CountType newCapacity)
        {
            assert(newCapacity <= k_MaxCapacity);

            auto control = m_Control;
            auto slots = m_Slots;
            auto capacity = m_Capacity;
            auto count = m_Count;

            // @Todo: Support for capacity < 16.
            // Enough for all keys and a byte per key.
            auto size = newCapacity * (1 + sizeof(KeyType));
            m_Control = (uint8_t*)m_Allocator->Malloc(size, 16);
            m_Slots = (KeyType*)(m_Control + newCapacity);
            m_Capacity = newCapacity;
            m_Count = 0;

            memset(m_Control, k_Empty32, m_Capacity);

            for (auto i = 0U; i < capacity; i++)
            {
                if (k_Empty != control[i] && 
                    k_Deleted != control[i])
                    Add(slots[i]);
            }

            m_Allocator->Free(control);
        }

        inline std::pair<bool, IndexType> FindForAdd(const KeyType& key, uint64_t hash) const
        {
            const auto index = H1(hash) & (m_Capacity - 1);
            const auto h2 = H2(hash);

            auto pos = index;
            do
            {
                if (k_Empty == m_Control[pos] || k_Deleted == m_Control[pos])
                    return { false, pos };

                if (h2 == m_Control[pos] && key == m_Slots[pos])
                    return { true, pos };

                pos = (pos + 1) & (m_Capacity - 1);
            }
            // @Todo: Have a max probe length? instead of wrapping around the whole table.
            while (pos != index);

            return { false, -1 };
        }

        inline std::pair<bool, IndexType> Find(const KeyType& key, uint64_t hash) const
        {
            const auto index = H1(hash) & (m_Capacity - 1);
            const auto h2 = H2(hash);

            auto pos = index;
            do
            {
                //auto a = _mm_set1_epi8(h2);
                //auto b = _mm_load_si128((__m128i*)(m_Control + pos));
                //auto r = _mm_cmpeq_epi8(a, b);

                if (k_Empty == m_Control[pos])
                    return { false, pos };

                if (h2 == m_Control[pos] && key == m_Slots[pos])
                    return { true, pos };

                pos = (pos + 1) & (m_Capacity - 1);
            }
            // @Todo: Have a max probe length? instead of wrapping around the whole table.
            while (pos != index);

            return { false, -1 };
        }

    private:
        constexpr static HashFunction k_HashFunction = HashFunction();
        constexpr static auto k_MaxCapacity = 4'294'967'296; // 2^32

        const static auto k_Empty    = 0b1000'0000U; // 0x80
        const static auto k_Deleted  = 0b1111'1110U;
        const static auto k_Sentinel = 0b1111'1111U;
        const static auto k_Empty32  = 0x80808080; // k_Empty x4

        // Max possible counts considering a load factor of 0.875.
        constexpr static CountType k_RehashLUT[] = 
        { 0, 0, 0,
                        7,
                       14,
                       28,
                       56,
                      112,
                      224,
                      448,
                      896,
                    1'792,
                    3'584,
                    7'168,
                   14'336,
                   28'672,
                   57'344,
                  114'688,
                  229'376,
                  458'752,
                  917'504,
                1'835'008,
                3'670'016,
                7'340'032,
               14'680'064,
               29'360'128,
               58'720'256,
              117'440'512,
              234'881'024,
              469'762'048,
              939'524'096,
            1'879'048'192,
            3'758'096'384
        };

        IAllocator* m_Allocator = GetFallbackAllocator();
        uint8_t* m_Control = nullptr;
        KeyType* m_Slots = nullptr;
        CountType m_Capacity = 0;
        CountType m_Count = 0;

    public:
        static inline uint32_t NumCollisions = 0;
    };
}