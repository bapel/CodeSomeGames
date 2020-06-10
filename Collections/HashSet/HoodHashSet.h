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
            SimdHashSet(allocator)
        { EnsureCapacity(capacity); }

        ~HoodHashSet()
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
        IAllocator* m_Allocator;

    };
}