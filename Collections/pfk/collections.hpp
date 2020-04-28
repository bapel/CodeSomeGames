#pragma once

namespace pfk {

template <class T>
class Array
{
public:
    Array() = delete;
    Array(AllocatorImpl* allocator) : m_Allocator(allocator) { pfk_AssertIsPODType(T); }

    Array(AllocatorImpl* allocator, size_t capacity) : 
        m_Allocator(allocator),
        m_First(allocator->Alloc<T>(capacity))
    {
        pfk_AssertIsPODType(T);
        m_Capacity = m_First + capacity;
        m_Last = m_First;
    }

    ~Array()
    {
        m_Allocator->Free(m_First);
    }

    size_t GetCapacity() const;
    void SetCapacity(size_t capacity);

    size_t GetCount() const;

    void Add(const T& value);
    void AddRange(const T* values, size_t numValues);

    void Clear();

    void Insert(size_t index, const T& value);
    void InsertRange(size_t index, const T* values, size_t numValues);

    void RemoveAt(size_t index);
    void RemoveRange(size_t index, size_t count);

    void TrimExcess();

private:
    AllocatorImpl* m_Allocator;
    T* m_First;
    T* m_Capacity;
    T* m_Last;

    // Algorithms.
    // ForEach
    // Find
    // FindAll
    // FindIndex
    // FindLast
    // FindLastIndex
    // ConvertAll
    // Reverse
    // Sort
};

}