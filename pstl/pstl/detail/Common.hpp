#pragma once

#define WIN32_LEAN_AND_MEAN

#include <assert.h>
#include <stdint.h>
#include <type_traits>

#define pstl_is_debug__ _DEBUG
#define pstl_inline__ __inline

namespace pstl {

    template <typename T>
    constexpr bool IsTrivialType()
    {
        return
            std::is_trivially_constructible_v<T>() && 
            std::is_trivially_destructible_v<T>() && 
            std::is_trivially_assignable_v<T>() && 
            std::is_trivially_copyable_v<T>();
    }

    template <class T, class U>
    pstl_inline__ constexpr T Min(const T& a, const U& b)
    { return (a < b) ? a : b; }

    template <class T, class U>
    pstl_inline__ constexpr T Max(const T& a, const U& b)
    { return (a > b) ? a : b; }

}

#define pstl_assert_pod__(T__)\
    static_assert(pstl::IsTrivialType<T__>(), #T__" must be POD")