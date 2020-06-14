#pragma once

#include "Common.hpp"
#include "Enumerator.hpp"

namespace pstl {
namespace detail {

    template <
        template <class, class> class Store,
        template <class, bool> class Access,
        class T, class C, bool AssertBounds = true>
    class ArrayBase : 
        public Store<T, C>,
        public Access<T, AssertBounds>
    {
    public:
        using StoreBase = Store<T, C>;
        using AccessBase = Access<T, AssertBounds>;

        using StoreBase::StoreBase;

        __inline T& operator[](uint32_t index) 
        { return At(index, m_start, Count()); }

        __inline const T& operator[](uint32_t index) const
        { return At(index, m_start, Count()); }

    private:
        using AccessBase::At;
        using StoreBase::m_start;
        using StoreBase::Count;
    };

    template <class T, bool AssertBounds = true>
    struct RandomAccess;

    template <class T>
    struct RandomAccess<T, true>
    {
        __inline static T& At(uint32_t index, T* start, uint32_t count)
        {
            assert(index < count);
            return start[index];
        }
    };

    template <class T>
    struct RandomAccess<T, false>
    {
        __inline static T& At(uint32_t index, T* start, uint32_t count)
        { return start[index]; }
    };

}}