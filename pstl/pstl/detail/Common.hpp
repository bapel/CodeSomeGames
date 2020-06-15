#pragma once

#define WIN32_LEAN_AND_MEAN

#include <assert.h>
#include <stdint.h>
#include <type_traits>

#define pstl_assert_pod__(T__)\
    static_assert(std::is_pod_v(T__)(), #T__" must be POD")