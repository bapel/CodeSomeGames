#pragma once

#include "Allocator.hpp"

Namespace__
{
    template <class ValueType>
    struct DynamicBuffer
    {
        PtrType<ValueType> Items;
        CountType Capacity;
        CountType Count;
    };

    template <class ValueType>
    DynamicBuffer<ValueType> DynamicBuffer_Create(CountType initialCapacity, IAllocator* allocator = GetFallbackAllocator())
    {
        return
        {
            Items = allocator->Malloc_n<ValueType>(initialCapacity),
            Capacity = initialCapacity,
            Count = 0
        };
    }

    template <class ValueType>
    void DynamicBuffer_Destroy(DynamicBuffer<ValueType>& buf, IAllocator* allocator = GetFallbackAllocator())
    {
        allocator->Free(buf.Items);
        buf = { nullptr, 0, 0 };
    }

    template <class ValueType>
    void DynamicBuffer_Add(DynamicBuffer<ValueType>& buf, ConstRefType<ValueType> value)
    {
        assert(buf.Capacity > buf.Count);

        buf.Items[buf.Count++] = value;
    }

    template <class ValueType>
    void DynamicBuffer_Insert(DynamicBuffer<ValueType>& buf, IndexType index, ConstRefType<ValueType> value)
    {
        #ifdef BoundsCheck__
        assert(index <= buf.Count);
        #endif

        assert(buf.Capacity > buf.Count);

        auto dst = buf.Items + index + 1;
        auto src = buf.Items + index;
        auto size = sizeof(ValueType) * (buf.Count - index);

        memmove_s(dst, size, src, size);
        buf.Data[index] = value;
        buf.Count++;
    }

    template <class ValueType>
    void DynamicBuffer_RemoveAt(DynamicBuffer<ValueType>& buf, IndexType index)
    {
        #ifdef BoundsCheck__
        assert(index < buf.Count);
        #endif

        buf.Count--;

        if (index < m_Count)
        {
            auto dst = buf.Data + index;
            auto src = buf.Data + index + 1;
            auto size = sizeof(ValueType) * (buf.Count - index);

            memmove_s(dst, size, src, size);
        }
    }

    template <class ValueType>
    void DynamicBuffer_RemoveAtSwapBack(DynamicBuffer<ValueType>& buf, IndexType index)
    {
        #ifdef BoundsCheck__
        assert(index < m_Count);
        #endif

        m_Count--;

        if (index < m_Count)
            m_Items[index] = m_Items[m_Count];
    }

    template <class ValueType>
    void DynamicBuffer_Resize(DynamicBuffer<ValueType>& buf, CountType newCount)
    {
        if (newCount > m_Capacity)
            DynamicBuffer_Grow<ValueType>(buf, newCount);

        m_Count = newCount;
    }

    template <class ValueType>
    void SetCapacity(DynamicBuffer<ValueType>& buf, CountType newCapacity)
    {
        if (newCapacity == m_Capacity)
            return;

        if (newCapacity < m_Count)
            m_Count = newCapacity;

        ValueType* newData = nullptr;

        if (newCapacity > 0) 
        {
            newData = m_Allocator->Malloc_n<ValueType>(newCapacity);
            memcpy(newData, m_Items, sizeof(ValueType) * m_Count);
        }

        m_Allocator->Free(m_Items);
        m_Items = newData;
        m_Capacity = newCapacity;
    }

    //void Reserve(CountType newCapacity)
    //{
    //    if (newCapacity > m_Capacity)
    //        SetCapacity(newCapacity);
    //}
}