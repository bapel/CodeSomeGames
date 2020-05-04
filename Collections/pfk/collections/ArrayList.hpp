#pragma once

#include "..\Memory.hpp"

namespace pfk {
namespace collections {

    template <class T>
    struct ArrayList
    {
        IAllocator* m_Allocator = nullptr;
        T* m_Start = nullptr;
        T* m_End = nullptr;
        T* m_Capacity = nullptr;

        static ArrayList<T> Create__(IAllocator* allocator, size_t capacity, const char* file, uint32_t line)
        {
            m_Allocator = allocator;
            m_Start = allocator->Alloc(capacity, alignof(T), file, line);
            m_End = m_Start;
            m_Capacity = m_Start + capacity;
        }



        void Destroy()
        {
            m_Allocator->Free(m_Start);
            *this = {};
        }

        inline       T& operator[](size_t index)       { return *(m_Start + index); }
        inline const T& operator[](size_t index) const { return *(m_Start + index); }
    };
}}

#define pfk_array_list_create(T__, allocator__, capacity__)\
    pstl::collections::ArrayList<T__>::Create(allocator__, capacity__, __FILE__, __LINE__)
    