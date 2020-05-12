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
}