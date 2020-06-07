#pragma once

#include "Allocator.hpp"
#include <functional> // std::hash
#include <immintrin.h>
#include <emmintrin.h>

namespace NamespaceName__
{
    uint64_t murmur_hash_64(const void* key, uint32_t len, uint64_t seed)
    {
        const uint64_t m = 0xc6a4a7935bd1e995ULL;
        const uint32_t r = 47;

        uint64_t h = seed ^ (len * m);

        const uint64_t * data = (const uint64_t *)key;
        const uint64_t * end = data + (len/8);

        while(data != end)
        {
            #ifdef PLATFORM_BIG_ENDIAN
            uint64 k = *data++;
            char *p = (char *)&k;
            char c;
            c = p[0]; p[0] = p[7]; p[7] = c;
            c = p[1]; p[1] = p[6]; p[6] = c;
            c = p[2]; p[2] = p[5]; p[5] = c;
            c = p[3]; p[3] = p[4]; p[4] = c;
            #else
            uint64_t k = *data++;
            #endif

            k *= m;
            k ^= k >> r;
            k *= m;

            h ^= k;
            h *= m;
        }

        const unsigned char * data2 = (const unsigned char*)data;

        switch(len & 7)
        {
            case 7: h ^= uint64_t(data2[6]) << 48;
            case 6: h ^= uint64_t(data2[5]) << 40;
            case 5: h ^= uint64_t(data2[4]) << 32;
            case 4: h ^= uint64_t(data2[3]) << 24;
            case 3: h ^= uint64_t(data2[2]) << 16;
            case 2: h ^= uint64_t(data2[1]) << 8;
            case 1: h ^= uint64_t(data2[0]);
                h *= m;
        };

        h ^= h >> r;
        h *= m;
        h ^= h >> r;

        return h;
    }

    template <class K, typename H = std::hash<K>>
    class FlatHashSet
    {
    public:
        using KeyType = K;
        using HashFunction = H;

        FlatHashSet() = default;
        FlatHashSet(IAllocator* allocator) : m_Allocator(allocator)  {}
        
        FlatHashSet(CountType capacity, IAllocator* allocator = GetFallbackAllocator()) :
            FlatHashSet(allocator)
        { EnsureCapacity(capacity); }

        ~FlatHashSet()
        { m_Allocator->Free(m_Control); }

        CountType Count() const { return m_Count; }
        CountType Capacity() const { return m_Capacity; }

        bool Add(ConstRefType<KeyType> key)
        {
            if (nullptr == m_Control)
                Rehash(16);

            auto loadFactor = (float)(m_Count + 1) / m_Capacity;
            if (loadFactor >= k_LoadFactor)
                Rehash(m_Capacity << 1);

            auto inserted = Insert(key, m_Control, m_Slots, m_Capacity);
            m_Count += inserted;

            return inserted;
        }

        bool Contains(ConstRefType<KeyType> key) const
        {
            if (m_Count == 0)
                return false;

            const auto hash = Hash(key);
            const auto index = Index(hash, m_Capacity);
            const auto h2 = H2(hash);
            auto pos = index;// - (index & 15);

            while (true)
            {
                auto a = _mm_set1_epi8(h2);
                auto b = _mm_loadu_si128((__m128i*)(m_Control + pos));
                auto mask = _mm_movemask_epi8(_mm_cmpeq_epi8(a, b));

                auto i = 0U;
                while (mask > 0)
                {
                    if (mask & 1)
                    {
                        auto p = (pos + i) & (m_Capacity - 1);
                        if (key == m_Slots[p])
                            return true;

                        if (m_Control[p] == k_Empty)
                            return false;   
                    }

                    mask >>= 1;
                    i++;
                }

                pos = (pos + 16) & (m_Capacity - 1);
            }

            //while (true)
            //{
            //    if (h2 == m_Control[pos] && key == m_Slots[pos])
            //        return true;

            //    if (m_Control[pos] == k_Empty)
            //        return false;

            //    pos = (pos + 1) & (m_Capacity - 1);
            //}
        }

        bool Remove(ConstRefType<KeyType> key)
        {
            if (m_Count == 0)
                return false;

            const auto hash = Hash(key);
            auto pos = Index(hash);
            const auto h2 = H2(hash);

            while (true)
            {
                if (h2 == m_Control[pos] && key == m_Slots[pos])
                {
                    m_Control[pos] = k_Deleted;
                    m_Count--;
                    return true;
                }

                if (m_Control[pos] == k_Empty)
                    return false;

                pos = (pos + 1) & (m_Capacity - 1);
            }
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
        // Next power of two.
        inline static CountType CalcCapacity(CountType n)
        {
            n--;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
            ++n;

            if (n % 16)
                return n + (16 - n % 16);
            return n;
        }

        inline static uint64_t Hash(const KeyType& key)
        { return HashFunction::_Do_hash(key); }

        //inline static uint64_t Hash(const KeyType& key)
        //{ return murmur_hash_64(&key, sizeof(key), 13); }

        inline static uint64_t H1(uint64_t hash) { return hash >> 7; }
        inline static uint8_t  H2(uint64_t hash) { return hash & 0b0111'1111U; }

        inline static CountType Index(uint64_t hash, CountType capacity)
        { return H1(hash) & (capacity - 1); }

        //inline static CountType Index(uint64_t hash, CountType capacity) 
        //{ return H1(hash) % capacity; }

        inline static void SetControl(
            uint8_t* control, 
            CountType capacity, 
            IndexType index, 
            uint8_t value)
        {
            assert(control[index] == k_Empty);
            control[index] = value;
            if (index < 16)
                control[index + capacity] = value;
        }

        void Rehash(CountType capacity)
        {
            // @Todo: Support for capacity < 16.
            // Enough for all keys and a byte per key.
            auto size = capacity * (1 + sizeof(KeyType)) + 16;
            auto control = (uint8_t*)m_Allocator->Malloc(size, 16);
            auto slots = (KeyType*)(control + (capacity + 16));

            memset(control, k_Empty32, capacity + 16);

            for (auto i = 0U; i < m_Capacity; i++)
                Insert(m_Slots[i], control, slots, capacity);

            m_Allocator->Free(m_Control);
            m_Control = control;
            m_Slots = slots;
            m_Capacity = capacity;
        }

        static bool Insert(
            ConstRefType<KeyType> key, 
            uint8_t* control, 
            KeyType* slots, 
            CountType capacity)
        {
            const auto hash = Hash(key);
            const auto index = Index(hash, capacity);

            auto pos = index;
            do
            {
                if (k_Empty == control[pos] || 
                    k_Deleted == control[pos])
                {
                    SetControl(control, capacity, pos, H2(hash));
                    slots[pos] = key;
                    return true;
                }

                if (slots[pos] == key)
                    return false;

                NumCollisions++;
                pos = (pos + 1) & (capacity - 1);
            }
            while (pos != index);

            return false;
        }

    private:
        constexpr static HashFunction k_HashFunction = HashFunction();
        constexpr static auto k_LoadFactor = 0.875f;

        const static auto k_Empty    = 0b1000'0000U; // 0x80
        const static auto k_Deleted  = 0b1111'1110U;
        const static auto k_Sentinel = 0b1111'1111U;
        const static auto k_Empty32  = 0x80808080; // k_Empty x4

        IAllocator* m_Allocator = GetFallbackAllocator();
        uint8_t* m_Control = nullptr;
        KeyType* m_Slots = nullptr;
        CountType m_Capacity = 0;
        CountType m_Count = 0;

    public:
        static inline uint32_t NumCollisions = 0;
    };
}