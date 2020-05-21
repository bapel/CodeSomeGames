#pragma once

#include <cstdint>
#include <EASTL\numeric_limits.h>

namespace m3
{
    // These could be 8 bits, but then they would show up as chars in the debugger.
    using row_t = int16_t;
    using col_t = int16_t;

    using count_t = uint16_t;

    using gem_id_t = uint16_t;
    constexpr gem_id_t InvalidGemId = eastl::numeric_limits<gem_id_t>::max();
    
    using gem_color_t = uint8_t;
    constexpr gem_color_t GemColors[] = { ' ', 'B', 'R', 'O', 'G', 'Y' };

    struct RowCol
    {
        row_t Row;
        col_t Column;
    };
}