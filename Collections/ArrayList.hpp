#pragma once

#include "Allocator.hpp"
#include <initializer_list>

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
    class ArrayList
    {
    public:
        using ValueType = T;

        ArrayList() : 
            m_Allocator(GetFallbackAllocator())
        { AssertIsPod__(ValueType); }
        
        ArrayList(CountType capacity, IAllocator* allocator = GetFallbackAllocator()) :
            m_Allocator(allocator)
        {
            AssertIsPod__(ValueType);
            SetCapacity(capacity);
        }

        ArrayList(std::initializer_list<ValueType> initList) :
            ArrayList((CountType)initList.size())
        {
            for (auto iter = initList.begin(); iter < initList.end(); iter++)
                m_Data[m_Count++] = *iter;
        }
        
        ArrayList(const ArrayList<ValueType>& other) = delete;

        ArrayList(ArrayList<ValueType>&& other) :
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

        ArrayList<ValueType>& operator = (std::initializer_list<ValueType> initList)
        {
            m_Count = 0;

            if (initList.size() > m_Capacity)
                Reserve((CountType)initList.size());

            for (auto iter = initList.begin(); iter < initList.end(); iter++)
                m_Data[m_Count++] = *iter;

            return *this;
        }

        ~ArrayList()
        {
            if (m_Data != nullptr)
                m_Allocator->Free(m_Data);
        }

        inline CountType Count() const 
        { return m_Count; }

        inline CountType Capacity() const 
        { return m_Capacity; }

        inline SizeType DataSize() const 
        { return sizeof(ValueType) * m_Count; }

        inline SizeType AllocatedSize() const 
        { return sizeof(ValueType) * m_Capacity; }

        inline PtrType<ValueType> Data() 
        { return m_Data; }

        inline ConstPtrType<ValueType> Data() const 
        { return m_Data; }

        inline RefType<ValueType> operator[] (IndexType index)
        {
            #ifdef BoundsCheck__
            assert(index < m_Count);
            #endif
            return m_Data[index];
        }

        inline ConstRefType<ValueType> operator[] (IndexType index) const
        {
            #ifdef BoundsCheck__
            assert(index < m_Count);
            #endif
            return m_Data[index];
        }

        void Add(ConstRefType<ValueType> value)
        {
            if (m_Count == m_Capacity)
                Grow();

            m_Data[m_Count++] = value;
        }

        void Insert(IndexType index, ConstRefType<ValueType> value)
        {
            #ifdef BoundsCheck__
            assert(index <= m_Count);
            #endif

            // @Todo: Do insertion along with growth?
            if (m_Count == m_Capacity)
                Grow();

            auto dst = m_Data + index + 1;
            auto src = m_Data + index;
            auto size = sizeof(ValueType) * (m_Count - index);

            memmove_s(dst, size, src, size);
            m_Data[index] = value;
            m_Count++;
        }

        void RemoveAt(IndexType index)
        {
            #ifdef BoundsCheck__
            assert(index < m_Count);
            #endif

            if (index < m_Count - 1)
            {
                auto dst = m_Data + index;
                auto src = m_Data + index + 1;
                auto size = sizeof(ValueType) * (m_Count - index - 1);

                memmove_s(dst, size, src, size);
            }

            m_Count--;
        }

        void RemoveAtSwapBack(IndexType index)
        {
            #ifdef BoundsCheck__
            assert(index < m_Count);
            #endif

            if (index < m_Count - 1)
                m_Data[index] = m_Data[m_Count - 1];

            m_Count--;
        }

        void Clear()
        { m_Count = 0; }

        void TrimExcess()
        { SetCapacity(m_Count); }

        void Resize(CountType newCount)
        {
            if (newCount > m_Capacity)
                Grow(newCount);

            m_Count = newCount;
        }

        void SetCapacity(CountType newCapacity)
        {
            if (newCapacity == m_Capacity)
                return;

            if (newCapacity < m_Count)
                m_Count = newCapacity;

            ValueType* newData = nullptr;

            if (newCapacity > 0) 
            {
                newData = m_Allocator->Malloc_n<ValueType>(newCapacity);
                memcpy(newData, m_Data, sizeof(ValueType) * m_Count);
            }

            m_Allocator->Free(m_Data);
            m_Data = newData;
            m_Capacity = newCapacity;
        }

        void Reserve(CountType newCapacity)
        {
            if (newCapacity > m_Capacity)
                SetCapacity(newCapacity);
        }

        bool CompareTo(const ArrayList<ValueType>& other)
        {
            if (other.m_Count != m_Count)
                return false;

            return 0 == memcmp(m_Data, other.m_Data, sizeof(ValueType) * m_Count);
        }

    private:
        void Grow(CountType minCapacity = 0)
        {
            auto newCapacity = m_Capacity * 2 + 8;

            if (newCapacity < minCapacity)
                newCapacity = minCapacity;

            SetCapacity(newCapacity);
        }

    private:
        IAllocator* m_Allocator = nullptr;
        PtrType<ValueType> m_Data = nullptr;
        CountType m_Count = 0;
        CountType m_Capacity = 0;
    };
}