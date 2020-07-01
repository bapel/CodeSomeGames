#pragma once

#define WIN32_LEAN_AND_MEAN

#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>

#include <type_traits>
#include <initializer_list>

#define vx_is_debug__ _DEBUG
#define vx_inline__ __inline

namespace vx {

    template <class T, class U>
    vx_inline__ constexpr T min(const T& a, const U& b)
    { return (a < b) ? a : b; }

    template <class T, class U>
    vx_inline__ constexpr T max(const T& a, const U& b)
    { return (a > b) ? a : b; }

}

#define vx_assert_trivial__(T__)\
    static_assert(std::is_pod_v<T__>, #T__" must be POD")