#pragma once

#include "Allocator.hpp"
#include <functional> // std::hash
#include <intrin.h> // __lzcnt

namespace NamespaceName__
{
    template <class K, class H = std::hash<K>>
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
            if (m_Capacity == 0)
                Rehash(16);

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

            SetControl(index, H2(hash));
            m_Slots[index] = key;
            m_Count++;

            return true;
        }

        __forceinline bool Contains(ConstRefType<KeyType> key) const
        {
            assert(m_Capacity > 0);
            auto [found, index] = Find(key, Hash(key));
            return found;
        }

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
                SetControl(index, k_Deleted);
                m_Count--;
                return true;
            }

            return false;
        }

        __forceinline void Clear()
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
        __forceinline static CountType CalcCapacity(CountType n)
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

        __forceinline static uint64_t Hash(KeyType x) { return HashFunction()(x); }
        __forceinline static uint64_t H1(uint64_t hash) { return hash >> 7; }
        __forceinline static uint8_t  H2(uint64_t hash) { return hash & 0b0111'1111U; }

        __forceinline IndexType Index(uint64_t hash) const
        { return H1(hash) & (m_Capacity - 1); }

        __forceinline void SetControl(IndexType index, uint8_t control) const
        {
            m_Control[index] = control;
            if (index < 16)
                m_Control[index + m_Capacity] = control;
        }

        __forceinline std::pair<bool, IndexType> FindForAdd(const KeyType& key, uint64_t hash) const
        {
        #ifdef HashMetrics__
            auto probeLength = 0U;
        #endif
            const auto index = Index(hash);
            const auto h2 = H2(hash);

            auto pos = index;
            do
            {
                if (k_Empty == m_Control[pos] || k_Deleted == m_Control[pos])
                    return { false, pos };

                if (h2 == m_Control[pos] && key == m_Slots[pos])
                    return { true, pos };

            #ifdef HashMetrics__
                probeLength++;
                MaxProbeLength = Max(probeLength, MaxProbeLength);
            #endif
                pos = (pos + 1) & (m_Capacity - 1);
            }
            // @Todo: Have a max probe length? instead of wrapping around the whole table.
            while (pos != index);

            return { false, -1 };
        }

        __forceinline std::pair<bool, IndexType> Find(const KeyType& key, uint64_t hash) const
        {
            const auto index = Index(hash);
            const auto h2 = H2(hash);

            auto pos = index;
            do
            {
                const auto value = _mm_set1_epi8(h2);
                const auto control = _mm_loadu_si128((__m128i*)(m_Control + pos));
                auto result = _mm_movemask_epi8(_mm_cmpeq_epi8(value, control));

                auto i = 0;
                while (result != 0)
                {
                    const auto offset = (pos + i) & (m_Capacity - 1);
                    if ((result & 1) && (key == m_Slots[offset]))
                        return { true, offset };

                    auto s = __lzcnt(result);
                    result >>= 1;
                    i++;
                }

                const auto empty = _mm_set1_epi8(k_Empty);
                result = _mm_movemask_epi8(_mm_cmpeq_epi8(empty, control));
                if (result != 0)
                    return { false, pos };

                pos = (pos + 16) & (m_Capacity - 1);
            }
            // @Todo: Have a max probe length? instead of wrapping around the whole table.
            while (pos != index);

            return { false, -1 };
        }

        /*
        inline std::pair<bool, IndexType> Find(const KeyType& key, uint64_t hash) const
        {
            const auto index = H1(hash) & (m_Capacity - 1);
            const auto h2 = H2(hash);

            auto pos = index;
            do
            {
                // auto a = _mm_set1_epi8(h2);
                // auto b = _mm_load_si128((__m128i*)(m_Control + pos));
                // auto r = _mm_cmpeq_epi8(a, b);

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
        */

        void Rehash(CountType newCapacity)
        {
            assert(newCapacity <= k_MaxCapacity);

            // src for copying.
            auto control = m_Control;
            auto slots = m_Slots;
            auto capacity = m_Capacity;
            auto count = m_Count;

            const auto sizeOfControl = newCapacity + 16;
            const auto sizeOfSlots = newCapacity * sizeof(KeyType);
            const auto size = sizeOfControl + sizeOfSlots;

            m_Control = (uint8_t*)m_Allocator->Malloc(size, 16);
            m_Slots = (KeyType*)(m_Control + sizeOfControl);
            m_Capacity = newCapacity;
            m_Count = 0;

            memset(m_Control, k_Empty32, sizeOfControl);

            for (auto i = 0U; i < capacity; i++)
            {
                if (k_Empty != control[i] && 
                    k_Deleted != control[i])
                    Add(slots[i]);
            }

            m_Allocator->Free(control);
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

        uint8_t* m_Control = nullptr;
        KeyType* m_Slots = nullptr;
        IAllocator* m_Allocator = GetFallbackAllocator();
        CountType m_Capacity = 0;
        CountType m_Count = 0;

    public:
    #ifdef HashMetrics__
        static inline uint32_t MaxProbeLength = 0;
    #endif
    };
}