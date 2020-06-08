#pragma once

#include "Allocator.hpp"
#include <functional> // std::hash
#include <intrin.h> // __lzcnt

namespace NamespaceName__
{
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
            assert(m_Capacity > 0);

            const auto hash = Hash(key);
            auto [found, index] = TryGetSlot(key, hash);

            if (found)
            {
                if (m_Control[index] == H2(hash))
                    return false; // already present.

                // Divide by capacity.
                auto sh = __lzcnt(m_Capacity);
                if (((m_Count * 1000) >> sh) > k_LoadFactor)
                    Rehash(m_Capacity << 1);

                m_Control[index] = H2(hash);
                m_Slots[index] = key;
                m_Count++;
                return true;
            }

        present__:
            return false;
        }

        bool Contains(ConstRefType<KeyType> key) const
        {
            assert(m_Capacity > 0);

            const auto hash = Hash(key);
            auto [found, index] = TryGetSlot(key, hash);

            return found;
        }

        bool Remove(ConstRefType<KeyType> key)
        {
            assert(m_Capacity > 0);

            const auto hash = Hash(key);
            auto [found, index] = TryGetSlot(key, hash);

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
        // Capacity is always a power of two.
        inline static CountType CalcCapacity(CountType n)
        {
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

        inline static CountType Index(uint64_t hash, CountType capacity)
        { return H1(hash) & (capacity - 1); }

        void Rehash(CountType newCapacity)
        {
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

        inline std::pair<bool, IndexType> TryGetSlot(const KeyType& key, uint64_t hash) const
        {
            const auto index = Index(hash, m_Capacity);
            const auto h2 = H2(hash);

            auto pos = index;
            do
            {
                if (k_Empty == m_Control[pos] || 
                    k_Deleted == m_Control[pos] ||
                    h2 == m_Control[pos] ||
                    key == m_Slots[pos])
                {
                    return { true, pos };
                }

                NumCollisions++;
                pos = (pos + 1) & (m_Capacity - 1);
            }
            // @Todo: Have a max probe length? instead of wrapping around the whole table.
            while (pos != index);

            return { false, -1 };
        }

    private:
        constexpr static HashFunction k_HashFunction = HashFunction();
        constexpr static auto k_LoadFactor = 875;

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