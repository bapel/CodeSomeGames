#pragma once

#include <cstdint>

namespace m3
{
    // These could be 8 bits, but then they would show up as chars in the debugger.
    using row_t = int16_t;
    using col_t = int16_t;

    using count_t = uint16_t;

    using gem_id_t = uint8_t;
    using gem_color_t = uint8_t;

    const gem_color_t GemColors[] = { ' ', 'B', 'R', 'O', 'G', 'Y' };

    struct Gem
    {
        gem_id_t Id;
        gem_color_t Color;
    };
}