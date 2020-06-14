#pragma once

#include <assert.h>
#include <stdint.h>
#include <type_traits>

#define pstl_assert_pod__(T__)\
    static_assert(__is_pod(T__), #T__" is not POD")