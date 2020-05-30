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

            auto dst = m_Allocator->Malloc_n<T>(capacity);
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

TEST_CASE("List construction", "[list]")
{
    using namespace NamespaceName__;

    SECTION("List must be initially empty and unallocated")
    {
        ArrayList<int> ints;

        REQUIRE(0 == ints.Count());
        REQUIRE(0 == ints.Capacity());
        REQUIRE(nullptr == ints.Data());
        REQUIRE(0 == ints.DataSize());
        REQUIRE(0 == ints.AllocatedSize());
    }

    SECTION("List init with capacity should adequately allocate")
    {
        const auto n = 200;
        ArrayList<int> ints(n);

        REQUIRE(0 == ints.Count());
        REQUIRE(n == ints.Capacity());
        REQUIRE(nullptr != ints.Data());
        REQUIRE(0 == ints.DataSize());
        REQUIRE(n * sizeof(int) == ints.AllocatedSize());
    }
}

TEST_CASE("List add and retrieval", "[list]")
{
    using namespace NamespaceName__;

    SECTION("List Add and items retrieval (no growth)")
    {
        const auto n = 100;
        ArrayList<int> ints(n);

        for (auto i = 0; i < n; i++)
            ints.Add(2 * i);

        REQUIRE(n == ints.Count());
        REQUIRE(n == ints.Capacity());
        REQUIRE(n * sizeof(int) == ints.DataSize());
        REQUIRE(n * sizeof(int) == ints.AllocatedSize());

        for (auto i = 0; i < n; i++)
            REQUIRE(ints[i] == (2 * i));
    }

    SECTION("List Add and items retrieval (with growth)")
    {
        const auto n0 = 10;
        const auto n = 200;
        ArrayList<int> ints(10);

        for (auto i = 0; i < n; i++)
            ints.Add(2 * i);

        REQUIRE(n == ints.Count());
        REQUIRE(n <= ints.Capacity());
        REQUIRE(n * sizeof(int) == ints.DataSize());
        REQUIRE(n * sizeof(int) <= ints.AllocatedSize());

        for (auto i = 0; i < n; i++)
            REQUIRE(ints[i] == (2 * i));
    }
}

TEST_CASE("List insert and retrieval", "[list]")
{
    using namespace NamespaceName__;

    SECTION("List insert and items retrieval (no growth)")
    {
        const auto n = 99;
        ArrayList<int> ints(n + 1);

        for (auto i = 0; i < n; i++)
            ints.Add(2 * i);

        ints.Insert(15, -15);

        REQUIRE((n + 1) == ints.Count());
        REQUIRE((n + 1) == ints.Capacity());
        REQUIRE((n + 1) * sizeof(int) == ints.DataSize());
        REQUIRE((n + 1) * sizeof(int) <= ints.AllocatedSize());

        for (auto i = 0; i < 15; i++)
            REQUIRE(ints[i] == (2 * i));

        REQUIRE(-15 == ints[15]);

        for (auto i = 17; i < n + 1; i++)
            REQUIRE(ints[i] == (2 * (i - 1)));
    }

    SECTION("List many insert and items retrieval (with growth)")
    {
        const auto n = 50;
        ArrayList<int> ints(n);

        for (auto i = 0; i < n; i++)
            ints.Add(2 * i + 1);

        for (auto j = 0; j < n; j++)
            ints.Insert(2 * j, 2 * j);

        REQUIRE(2 * n == ints.Count());
        REQUIRE(2 * n == ints.Capacity());
        REQUIRE(2 * n * sizeof(int) == ints.DataSize());
        REQUIRE(2 * n * sizeof(int) <= ints.AllocatedSize());

        for (auto i = 0; i < ints.Count(); i++)
            REQUIRE(i == ints[i]);
    }
}

TEST_CASE("List remove and retrieval")
{
    
}

#endif