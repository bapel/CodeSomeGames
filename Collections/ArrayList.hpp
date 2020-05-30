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
        ArrayList() : m_Allocator(GetFallbackAllocator())
        { AssertIsPod__(T); }
        
        ArrayList(CountType capacity, IAllocator* allocator = GetFallbackAllocator()) :
            m_Allocator(allocator), 
            m_Count(0), 
            m_Capacity(capacity)
        {
            AssertIsPod__(T);
            m_Data = m_Allocator->Malloc_n<T>(capacity);
        }
        
        ArrayList(const ArrayList<T>& other) = delete;

        ArrayList(ArrayList<T>&& other) :
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

        ArrayList(std::initializer_list<T> initList)
        {
            auto c = (CountType)initList.size();

            m_Allocator = GetFallbackAllocator();
            m_Data = m_Allocator->Malloc_n<T>(c);
            m_Count = 0;
            m_Capacity = c;

            for (auto iter = initList.begin(); iter < initList.end(); iter++)
                m_Data[m_Count++] = *iter;
        }

        ArrayList<T>& operator = (std::initializer_list<T> initList)
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
            assert(index <= m_Count);
            #endif

            // @Todo: Do insertion along with growth?
            if (m_Count == m_Capacity)
                Reserve(m_Capacity << 1);

            auto dst = m_Data + index + 1;
            auto src = m_Data + index;
            auto size = sizeof(T) * (m_Count - index);

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
                auto size = sizeof(T) * (m_Count - index - 1);

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
            if (capacity == 0)
                capacity = DefaultReserve;

            if (m_Capacity == capacity)
                return;

            auto count = Min(m_Count, capacity);
            auto dst = m_Allocator->Malloc_n<T>(capacity);

            if (count > 0)
            {
                auto dstSize = count * sizeof(T);
                auto src = m_Data;
                auto srcSize = dstSize;

                memcpy_s(dst, dstSize, src, srcSize);
            }
            
            if (m_Data != nullptr)
                m_Allocator->Free(m_Data);

            m_Data = dst;
            m_Count = count;
            m_Capacity = capacity;
        }

        bool CompareTo(const ArrayList<T>& other)
        {
            if (other.m_Count != m_Count)
                return false;

            return 0 == memcmp(m_Data, other.m_Data, sizeof(T) * m_Count);
        }

    private:
        const CountType DefaultReserve = 8;

        IAllocator* m_Allocator = nullptr;
        PtrType<T> m_Data = nullptr;
        CountType m_Count = 0;
        CountType m_Capacity = 0;
    };
}