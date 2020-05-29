#pragma once

#include "Allocator.hpp"

/*  
    @Todo/Consider:

    - Interface:
      - Iterator/Enumerator?

    - Methods:
      - AddRange(data)
      - InsertRange(index, data)
      - RemoveRange(index, count)

    - Algorithms (Maybe have these be generic for all containers?):
      - Fill
      - Sort(comparer?) -> sorted[] (inplace?)
      - Find(comparer?) -> index
      - FindAll(comparer?) -> indices[]
      - Map<Other>() -> Other[] (Convert)
      - Reduce() -> reduced[] (Filter)
      - Reverse -> reversed[]
      - Test -> true/fale[] (True for all)
*/

Namespace__
{
    // Resizable growing array.
    template <class T>
    class List
    {
    public:
        List() : m_Allocator(GetFallbackAllocator())
        { AssertIsPod__(T); }
        
        List(CountType capacity, IAllocator* allocator = GetFallbackAllocator()) :
            m_Allocator(allocator), 
            m_Count(0), 
            m_Capacity(capacity)
        {
            AssertIsPod__(T);
            m_Data = m_Allocator->Malloc_n(capacity);
        }
        
        List(const List<T>& other) = delete;

        List(List<T>&& other) :
            m_Allocator(other.m_Allocator), 
            m_Data(other.m_Data),
            m_Count(other.m_Data),
            m_Capacity(other.m_Capacity)
        {
            other.m_Allocator = nullptr;
            other.m_Data = nullptr;
            other.m_Count = 0;
            other.m_Capacity = 0;
        }

        ~List()
        {
            if (m_Data != nullptr)
                m_Allocator->Free(m_Data);
        }

        inline CountType Count() const 
        { return m_Count; }

        inline CountType Capacity() const 
        { return m_Capacity; }

        inline SizeType DataSize() const 
        { return sizeof(T) * m_Count; }

        inline SizeType AllocatedSize() const 
        { return sizeof(T) * m_Capacity; }

        inline PtrType<T> Data() 
        { return m_Data; }

        inline ConstPtrType<T> Data() const 
        { return m_Data; }

        inline RefType<T> operator[] (IndexType index)
        {
            #ifdef BoundsCheck__
            assert(index < m_Count);
            #endif
            return m_Data[index];
        }

        inline ConstRefType<T> operator[] (IndexType index) const
        {
            #ifdef BoundsCheck__
            assert(index < m_Count);
            #endif
            return m_Data[index];
        }

        void Add(ConstRefType<T> value)
        {
            if (m_Count == m_Capacity)
                Reserve(m_Capacity << 1);

            m_Data[m_Count++] = value;
        }

        void Insert(IndexType index, ConstRefType<T> value)
        {
            #ifdef BoundsCheck__
            assert(index < m_Count);
            #endif

            // @Todo: Do insertion along with growth?
            if (m_Count == m_Capacity)
                Reserve(m_Capacity << 1);

            auto dst = m_Data + index + 1;
            auto dstSize = sizeof(T) * (m_Count - index);
            auto src = m_Data + index;
            auto srcSize = dstSize;

            memmove_s(dst, dstSize, src, srcSize);
            m_Data[index] = value;
            m_Count++;
        }

        void Remove(IndexType index)
        {
            #ifdef BoundsCheck__
            assert(index < m_Count);
            #endif

            if (index == m_Count - 1)
            {
                m_Count--;
                return;
            }

            auto dst = m_Data + index;
            auto dstSize = sizeof(T) * (m_Count + index);
            auto src = m_Data + index + 1;
            auto srcSize = sizeof(T) * (m_Count + index - 1);

            memmove_s(dst, dstSize, src, srcSize);
            m_Count--;
        }

        void Clear()
        { m_Count = 0; }

        void TrimExcess()
        { Reserve(m_Count); }

        void Resize(CountType count)
        {
            if (count > m_Capacity)
                Reserve(count);

            m_Count = count;
        }

        void Resize(CountType count, ConstRefType<T> value)
        {
            if (count <= m_Count)
            {
                m_Count = count;
                return;
            }

            if (count > m_Capacity)
                Reserve(count);

            while (m_Count < count)
                m_Data[m_Count++] = value;
        }

        void Reserve(CountType capacity)
        {
            assert(m_Capacity != capacity);

            auto count = Min(m_Count, capacity);

            auto dst = m_Allocator->Malloc_n(capacity);
            auto dstSize = count * sizeof(T);
            auto src = m_Data;
            auto srcSize = dstSize;

            memcpy_s(dst, dstSize, src, srcSize);
            m_Allocator->Free(m_Data);

            m_Data = dst;
            m_Count = count;
            m_Capacity = capacity;
        }

    private:
        IAllocator* m_Allocator = nullptr;
        PtrType<T> m_Data = nullptr;
        CountType m_Count = 0;
        CountType m_Capacity = 0;
    };
}

#ifdef CatchAvailable__

#endif