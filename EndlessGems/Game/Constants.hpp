#pragma once

#include <SimpleMath.hpp>

namespace Game
{
    enum class GemColor
    {
        None = 0,
        Blue,
        Green,
        Purple,
        Orange,
        Red,
        Count
    };

    struct Constants
    {
        static const int BoardRows = 8;
        static const int BoardCols = 8;
        static const Color GemDisplayColors[];
        static const char GemDebugChars[];

        inline static Color GetGemDisplayColor(GemColor color) { return GemDisplayColors[(int)color]; }
        inline static char GetGemDebugChar(GemColor color) { return GemDebugChars[(int)color]; }
    };

    const Color Constants::GemDisplayColors[] = 
    {
        ToColor(0), // 0,
        ToColor(0xe1b23cff), // 0xffff0000,
        ToColor(0xdd663cff), // 0xff00ff00,
        ToColor(0x014025ff), // 0xff800080,
        ToColor(0x067473ff), // 0xff0066ff,
        ToColor(0x735c6eff)  // 0xff0000ff
    };

    const char Constants::GemDebugChars[] = { ' ', 'B', 'G', 'P', 'O', 'R' };
}