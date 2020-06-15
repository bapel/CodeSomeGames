#pragma once

#include "Common.hpp"
#include <type_traits>

namespace pstl {
namespace detail {

    template <class T>
    struct Identity
    {
        constexpr T operator()(T x) { return x; }
    };

    template <class T, bool IsConst = std::is_const_v<T>>
    struct DerefPtr;

    template <class T>
    struct DerefPtr<T, false>
    {
        constexpr T& operator()(T* x) { return *x; }
    };

    template <class T>
    struct DerefPtr<T, true>
    {
        constexpr const T& operator()(const T* x) { return *x; }
    };

}}

namespace pstl {

    template <class T, class F = detail::Identity<T>>
    class Enumerator
    {
        using C = std::remove_const_t<T>;
        using R = decltype(F()(nullptr));

    public:
        Enumerator(const T start, const T end) :
            m_start(start),
            m_end(end),
            m_current(m_start)
        { }

        __inline void Reset()
        { m_current = m_start; }

        __inline bool MoveNext()
        { return m_end == m_current++; }

        __inline R Current() const
        {
        #ifdef pstl_is_debug__
            assert(m_current < m_end);
        #endif
            return F()(m_current);
        }

    private:
        const T m_start;
        const T m_end;
        C m_current;
    };

    template <class T>
    using PointerEnumerator = Enumerator<T*, detail::DerefPtr<T>>;

}