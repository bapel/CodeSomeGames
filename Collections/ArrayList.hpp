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
    template <class Value_T>
    class ArrayList
    {
    public:
        ArrayList() : 
            m_Allocator(GetFallbackAllocator())
        { AssertIsPod__(Value_T); }
        
        ArrayList(CountType capacity, IAllocator* allocator = GetFallbackAllocator()) :
            m_Allocator(allocator)
        {
            AssertIsPod__(Value_T);
            SetCapacity(capacity);
        }

        ArrayList(std::initializer_list<Value_T> initList) :
            ArrayList((CountType)initList.size())
        {
            for (auto iter = initList.begin(); iter < initList.end(); iter++)
                m_Data[m_Count++] = *iter;
        }
        
        ArrayList(const ArrayList<Value_T>& other) = delete;

        ArrayList(ArrayList<Value_T>&& other) :
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

        ArrayList<Value_T>& operator = (std::initializer_list<Value_T> initList)
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
        { return sizeof(Value_T) * m_Count; }

        inline SizeType AllocatedSize() const 
        { return sizeof(Value_T) * m_Capacity; }

        inline PtrType<Value_T> Data() 
        { return m_Data; }

        inline ConstPtrType<Value_T> Data() const 
        { return m_Data; }

        inline RefType<Value_T> operator[] (IndexType index)
        {
            #ifdef BoundsCheck__
            assert(index < m_Count);
            #endif
            return m_Data[index];
        }

        inline ConstRefType<Value_T> operator[] (IndexType index) const
        {
            #ifdef BoundsCheck__
            assert(index < m_Count);
            #endif
            return m_Data[index];
        }

        void Add(ConstRefType<Value_T> value)
        {
            if (m_Count == m_Capacity)
                Grow();

            m_Data[m_Count++] = value;
        }

        void Insert(IndexType index, ConstRefType<Value_T> value)
        {
            #ifdef BoundsCheck__
            assert(index <= m_Count);
            #endif

            // @Todo: Do insertion along with growth?
            if (m_Count == m_Capacity)
                Grow();

            auto dst = m_Data + index + 1;
            auto src = m_Data + index;
            auto size = sizeof(Value_T) * (m_Count - index);

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
                auto size = sizeof(Value_T) * (m_Count - index - 1);

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

            Value_T* newData = nullptr;

            if (newCapacity > 0) 
            {
                newData = m_Allocator->Malloc_n<Value_T>(newCapacity);
                memcpy(newData, m_Data, sizeof(Value_T) * m_Count);
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

        bool CompareTo(const ArrayList<Value_T>& other)
        {
            if (other.m_Count != m_Count)
                return false;

            return 0 == memcmp(m_Data, other.m_Data, sizeof(Value_T) * m_Count);
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
        PtrType<Value_T> m_Data = nullptr;
        CountType m_Count = 0;
        CountType m_Capacity = 0;
    };
}