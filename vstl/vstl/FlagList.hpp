#pragma once

#include "..\vx\common.hpp"

namespace vstl {

    template <class Enum>
    class PackedEnumList
    {
    public:
        using count_type = decltype(Enum::count_);

        constexpr auto max_value = std::numeric_limits<count_type>::max();
        // constexpr auto bits_per_value = log2();
    };

}