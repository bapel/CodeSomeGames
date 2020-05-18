#pragma once

#include "m3Types.hpp"
using namespace m3;

namespace m3
{
    template <class T, class Values>
    inline row_t GetMatchingRowsInCol_U(row_t r, col_t c, const T& value, const Values& values)
    {
        while (values.IsWithinBounds(r + 1, c) && (values(r + 1, c) == value)) 
            r = r + 1;
        return r;
    }

    template <class T, class Values>
    inline row_t GetMatchingRowsInCol_D(row_t r, col_t c, const T& value, const Values& values)
    {
        while (values.IsWithinBounds(r - 1, c) && (values(r - 1, c) == value)) 
            r = r - 1;
        return r;
    }

    template <class T, class Values>
    inline col_t GetMatchingColsInRow_L(row_t r, col_t c, const T& value, const Values& values)
    {
        while (values.IsWithinBounds(r, c - 1) && (values(r, c - 1) == value)) 
            c = c - 1;
        return c;
    }

    template <class T, class Values>
    inline col_t GetMatchingColsInRow_R(row_t r, col_t c, const T& value, const Values& values)
    {
        while (values.IsWithinBounds(r, c + 1) && (values(r, c + 1) == value)) 
            c = c + 1;
        return c;
    }

    /*      
        0,0           c                 0,0           c
            +------+------+------+          +------+------+------+
            |      |      |      |          |      | mru  |      |
            |      |      |      |          |      |      |      |
            +------+------+------+          +------+------+------+
         r0 |      | col0 |      |          | mcl0 | col1 | mcr0 | r0 
            |      |      |      |          |      |      |      |
            +------+------+------+  Swap >  +------+------+------+
         r1 |      | col1 |      |          | mcl1 | col0 | mcr1 | r1 
            |      |      |      |          |      |      |      |
            +------+------+------+          +------+------+------+
            |      |      |      |          |      | mrd  |      |
            |      |      |      |          |      |      |      |
            +------+------+------+          +------+------+------+
    */
    template <class T, class Values>
    Matches TryGetMatchesForSwapAlongCol(col_t c, row_t r0, row_t r1, const Values& values)
    {
        assert(r0 < r1);

        Matches m;

        // Post swap colors.
        T c0 = values(r1, c);
        T c1 = values(r0, c);

        auto mcl0 = GetMatchingColsInRow_L(r0, c, c1);
        auto mcr0 = GetMatchingColsInRow_R(r0, c, c0);
        auto mcl1 = GetMatchingColsInRow_L(r1, c, c1);
        auto mcr1 = GetMatchingColsInRow_R(r1, c, c0);

        auto mru = GetMatchingRowsInCol_U(r0, c, c1);
        auto mrd = GetMatchingRowsInCol_D(r1, c, c0);

        m.Row_0 = { r0, mcl0, mcr0 };
        m.Row_1 = { r1, mcl1, mcr1 };

        if (c0 != c1)
        {
            m.Col_0 = { c, r0, mru };
            m.Col_1 = { c, mrd, r1};
        }
        else
            m.Col_0 = { c, mrd, mru };

        return m;
    }
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

TEST_CASE("Matching functions", "[matching]")
{
    SECTION("Should find no matches")
    {
        const int Rows = 5;
        const int Cols = 8;

        const char colors_[] = 
            ___01234567
            _4"BROGYBRO"
            _3"OBROGYBR"
            _2"ROBROGYB"
            _1"BROBROGY"
            _0"YBROBROG";

        using GemColors = private_::Board<gem_color_t, Rows, Cols>;

        GemColors colors;
        GemColors::MakeInverted(colors_, sizeof(colors_), &colors);

        for (auto i = 0; i < colors.Count(); i++)
        {
            auto r = i / colors.Cols();
            auto c = i % colors.Cols();
            auto cl = colors(r, c);

            DYNAMIC_SECTION("Match check (" << r << ", " << c << ")")
            {
                REQUIRE(r == GetMatchingRowsInCol_U(r, c, cl, colors));
                REQUIRE(r == GetMatchingRowsInCol_D(r, c, cl, colors));
                REQUIRE(c == GetMatchingColsInRow_L(r, c, cl, colors));
                REQUIRE(c == GetMatchingColsInRow_R(r, c, cl, colors));
            }
        }
    }

    SECTION("Should find matches (+ve)")
    {
        const int Rows = 5;
        const int Cols = 8;

        const char colors_[] = 
               /*|*/
            ___01234567
            _4"BRBGYBRO"
            _3"OYBOGYBR"
            _2"BBBBBBYB" //--
            _1"BRBBROGY"
            _0"YBROBROG";

        using GemColors = private_::Board<gem_color_t, Rows, Cols>;

        GemColors colors;
        GemColors::MakeInverted(colors_, sizeof(colors_), &colors);

        auto r = 2;
        auto c = 2;
        auto cl = colors(r, c);

        DYNAMIC_SECTION("Match check (" << r << ", " << c << ") Col_U") 
        {
            REQUIRE(GetMatchingRowsInCol_U(r, c, cl, colors) == 4);
        }

        DYNAMIC_SECTION("Match check (" << r << ", " << c << ") Col_D") 
        {
            REQUIRE(GetMatchingRowsInCol_D(r, c, cl, colors) == 1);
        }

        DYNAMIC_SECTION("Match check (" << r << ", " << c << ") Row_L") 
        {
            REQUIRE(GetMatchingColsInRow_L(r, c, cl, colors) == 0);
        }

        DYNAMIC_SECTION("Match check (" << r << ", " << c << ") Row_R") 
        {
            REQUIRE(GetMatchingColsInRow_R(r, c, cl, colors) == 5);
        }
    }
}

#endif