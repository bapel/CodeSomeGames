#pragma once

#include "Allocator.hpp"
#include <functional> // std::hash

namespace NamespaceName__
{
    template <
        class KeyType, 
        class HashFunction = std::hash<KeyType>>
        class FlatHashSet
    {
    public:
        FlatHashSet() = default;

        ~FlatHashSet()
        { m_Allocator->Free(m_Control); }

        CountType Count() const { return m_Count; }
        CountType Capacity() const { return m_Capacity; }

        bool Insert(ConstRefType<KeyType> key)
        {
            if (nullptr == m_Control)
                Rehash(16);

            const auto hash = k_HashFunction(key);
            const auto index = H1(hash) % m_Capacity;

            auto pos = index;
            do
            {
                if (k_Empty == m_Control[pos] 
                    || k_Deleted == m_Control[pos] )
                {
                    m_Control[pos] = H2(hash);
                    Slot(pos) = key;
                    m_Count++;
                    return true;
                }

                pos = (pos + 1) % m_Capacity;
            }
            while (pos != index);

            return false;
        }

        bool Contains(ConstRefType<KeyType> key) const
        {
            if (m_Count == 0)
                return false;

            const auto hash = k_HashFunction(key);
            auto pos = H1(hash) % m_Capacity;
            const auto h2 = H2(hash);

            while (true)
            {
                if (h2 == m_Control[pos] && key == Slot(pos))
                    return true;

                if (m_Control[pos] == k_Empty)
                    return false;

                pos = (pos + 1) % m_Capacity;
            }
        }

        bool Remove(ConstRefType<KeyType> key);

        void Clear()
        { memset(m_Control, k_Empty32, m_Capacity); }

        void EnsureCapacity(CountType minCapacity);

        // bool ContainsValue(ConstRefType<V> value);
        // void TrimExcess(CountType minEntries = 0);
        // bool TryAdd(ConstRefType<K> key, ConstRefType<T> value);
        // bool TryGetValue(ConstRefType<K> key, PtrType<T> out_value);

    private:
        inline uint64_t H1(uint64_t hash) { return hash >> 7; }
        inline uint8_t  H2(uint64_t hash) { return hash & 0b0111'1111U; }

        inline       KeyType& Slot(IndexType i)       { return ((KeyType*)(m_Control + m_Capacity))[i]; }
        inline const KeyType& Slot(IndexType i) const { return ((KeyType*)(m_Control + m_Capacity))[i]; }

        void Rehash(CountType newCapacity)
        {
            // Enough for all keys and a byte per key.
            auto allocSize = newCapacity * (1 + sizeof(KeyType));
            auto newControl = (uint8_t*)m_Allocator->Malloc(allocSize, 16);

            memset(newControl, k_Empty32, newCapacity);

            if (m_Control != nullptr)
            {
                // @Todo: implement.
            }

            m_Control = newControl;
            m_Capacity = newCapacity;
        }

    private:
        const static HashFunction k_HashFunction;

        const static auto k_Empty    = 0b1000'0000U; // 0x80
        const static auto k_Deleted  = 0b1111'1110U;
        const static auto k_Sentinel = 0b1111'1111U;
        const static auto k_Empty32  = 0x80808080; // k_Empty x4

        IAllocator* m_Allocator = GetFallbackAllocator();
        uint8_t* m_Control = nullptr;
        CountType m_Capacity = 0;
        CountType m_Count = 0;
    };
}