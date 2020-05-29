#pragma once

#include "m3Types.hpp"
#include <EASTL\array.h>
#include <EASTL\type_traits.h>

namespace m3
{
    // @Todo: Use a vector instead of an array so size can be set at runtime.
    template <class T, int R, int C>
    struct Board
    {
        static_assert(eastl::is_pod<T>::value);
        using Array = eastl::array<T, R * C>;
        using Type = T;

        Array m_Array;

        // @Todo: Ideally should do nothing. Does this memset to 0?
        Board() = default;

        Board(const void* data, size_t dataSize) 
        {
            dataSize = eastl::min(dataSize, sizeof(m_Array));
            memcpy(m_Array.data(), data, dataSize); 
        }

        // Make with an inverted y-axis
        static void CreateInverted(const void* data, size_t dataSize, Board* outBoard)
        {
            assert(dataSize >= sizeof(m_Array));

            const auto rowSize = sizeof(T) * outBoard->Cols().m_I;

            for (auto r = 0; r < outBoard->Rows().m_I; r++)
            {
                auto src = (T*)data + (outBoard->Cols().m_I * r);
                auto dst = &(*outBoard)((outBoard->Rows().m_I - 1) - r, 0);
                memcpy(dst, src, rowSize);
            }
        }

        inline Row Rows() const { return R; }
        inline Col Cols() const { return C; }
        inline uint32_t Count() const { return R * C; }

        inline uint32_t Index(Row r, Col c) const 
        { 
            assert(IsWithinBounds(r, c));
            // return c * Rows() + r; 
            return r.m_I * Cols().m_I + c.m_I;
        }

        inline       T& operator() (Row r, Col c)       { return m_Array[Index(r, c)]; }
        inline const T& operator() (Row r, Col c) const { return m_Array[Index(r, c)]; }

        inline bool IsWithinBounds(Row r, Col c) const
        {
            return r >= 0 && r < Rows() && c >= 0 && c < Cols();
        }
    };
}

#ifdef CatchAvailable__

#pragma region Rows/Cols
#define _0
#define _1
#define _2
#define _3
#define _4
#define _5
#define _6
#define _7
#define _8
#define _9
#define ___01234567
#pragma endregion

TEST_CASE("Inverted board creation", "[board]")
{
    using namespace m3;

    const int Rows = 5;
    const int Cols = 8;

    const char colors_[] = 
        ___01234567
        _4"BROGYBRO"
        _3"OBROGYBR"
        _2"ROBROGYB"
        _1"BROBROGY"
        _0"YBROBROG";

    using GemColors = Board<GemColor, Rows, Cols>;

    GemColors colors;
    GemColors::CreateInverted(colors_, sizeof(colors_), &colors);

    const char invertedColors[] = 
        ___01234567
        _0"YBROBROG"
        _1"BROBROGY"
        _2"ROBROGYB"
        _3"OBROGYBR"
        _4"BROGYBRO";

    REQUIRE(0 == SDL_memcmp(&colors, invertedColors, sizeof(colors)));
}

#endif