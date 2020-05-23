#pragma once

#include "m3Span.hpp"
#include "m3Board.hpp"

namespace m3
{
    struct Matches
    {
        union
        {
            struct { RowSpan Row_0, Row_1; };
            struct { RowSpan Row_L, Row_R; };
        };
        union
        {
            struct { ColSpan Col_0, Col_1; };
            struct { ColSpan Col_D, Col_U; };
        };

        Matches() = default;
    };

    template <class T, class Values>
    inline Row GetMatchingRowsInCol_U(Row r, Col c, const T& value, const Values& values)
    {
        while (values.IsWithinBounds(r + 1, c) && (values(r + 1, c) == value)) 
            r = r + 1;
        return r;
    }

    template <class T, class Values>
    inline Row GetMatchingRowsInCol_D(Row r, Col c, const T& value, const Values& values)
    {
        while (values.IsWithinBounds(r - 1, c) && (values(r - 1, c) == value)) 
            r = r - 1;
        return r;
    }

    template <class T, class Values>
    inline Col GetMatchingColsInRow_L(Row r, Col c, const T& value, const Values& values)
    {
        while (values.IsWithinBounds(r, c - 1) && (values(r, c - 1) == value)) 
            c = c - 1;
        return c;
    }

    template <class T, class Values>
    inline Col GetMatchingColsInRow_R(Row r, Col c, const T& value, const Values& values)
    {
        while (values.IsWithinBounds(r, c + 1) && (values(r, c + 1) == value)) 
            c = c + 1;
        return c;
    }

    /*
                      c                               c
            +------+------+------+          +------+------+------+
            |      |      |      |          |      | mru  |      |
            |      |      |      |          |      |      |      |
            +------+------+------+          +------+------+------+
         r1 |      | col1 |      |          | mcl1 | col0 | mcr1 | r1
            |      |      |      |          |      |      |      |
            +------+------+------+  Swap >  +------+------+------+
         r0 |      | col0 |      |          | mcl0 | col1 | mcr0 | r0 
            |      |      |      |          |      |      |      |
            +------+------+------+          +------+------+------+
            |      |      |      |          |      | mrd  |      |
            |      |      |      |          |      |      |      |
            +------+------+------+          +------+------+------+
        0,0                             0,0
    */
    template <class Values>
    Matches GetMatchesForSwap_Col(Col c, Row r0, Row r1, const Values& values)
    {
        assert(r0 < r1);

        using T = typename Values::Type;
        T col0 = values(r0, c);
        T col1 = values(r1, c);

        // Use swapped colors here.

        auto mcl0 = GetMatchingColsInRow_L(r0, c, col1, values);
        auto mcr0 = GetMatchingColsInRow_R(r0, c, col1, values);

        auto mcl1 = GetMatchingColsInRow_L(r1, c, col0, values);
        auto mcr1 = GetMatchingColsInRow_R(r1, c, col0, values);

        auto mru = GetMatchingRowsInCol_U(r1, c, col0, values);
        auto mrd = GetMatchingRowsInCol_D(r0, c, col1, values);

        Matches m;

        m.Row_1 = { r1, mcl1, mcr1 };
        m.Row_0 = { r0, mcl0, mcr0 };

        if (col0 != col1)
        {
            m.Col_U = { c, r1, mru };
            m.Col_D = { c, mrd, r0 };
        }
        else
            m.Col_1 = m.Col_0 = { c, mrd, mru };

        return m;
    }

    /*      
                      c0     c1                              c0     c1
            +------+------+------+------+          +------+------+------+------+
            |      |      |      |      |          |      | mru0 | mru1 |      |
            |      |      |      |      |          |      |      |      |      |
            +------+------+------+------+          +------+------+------+------+
          r |      | col0 | col1 |      |          | mcl  | col1 | col0 | mcr  | r
            |      |      |      |      |  Swap >  |      |      |      |      |
            +------+------+------+------+          +------+------+------+------+
            |      |      |      |      |          |      | mrd0 | mrd1 |      | 
            |      |      |      |      |          |      |      |      |      |
            +------+------+------+------+          +------+------+------+------+
        0,0                                    0,0
    */
    template <class Values>
    Matches GetMatchesForSwap_Row(Row r, Col c0, Col c1, const Values& values)
    {
        assert(c0 < c1);

        using T = typename Values::Type;
        T col0 = values(r, c0);
        T col1 = values(r, c1);

        // Use swapped colors here.

        auto mru0 = GetMatchingRowsInCol_U(r, c0, col1, values);
        auto mrd0 = GetMatchingRowsInCol_D(r, c0, col1, values);

        auto mru1 = GetMatchingRowsInCol_U(r, c1, col0, values);
        auto mrd1 = GetMatchingRowsInCol_D(r, c1, col0, values);

        auto mcl = GetMatchingColsInRow_L(r, c0, col1, values);
        auto mcr = GetMatchingColsInRow_R(r, c1, col0, values);

        Matches m;

        m.Col_0 = { c0, mrd0, mru0 };
        m.Col_1 = { c1, mrd1, mru1 };

        if (col0 != col1)
        {
            m.Row_L = { r, mcl, c0 };
            m.Row_R = { r, c1, mcr };
        }
        else
            m.Row_1 = m.Row_0 = { r, mcl, mcr };

        return m;
    }
}

#ifdef CatchAvailable__

TEST_CASE("Matching functions", "[matching]")
{
    using namespace m3;

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

        using GemColors = m3::Board<gem_color_t, Rows, Cols>;

        GemColors colors;
        GemColors::CreateInverted(colors_, sizeof(colors_), &colors);

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

        using GemColors = Board<gem_color_t, Rows, Cols>;

        GemColors colors;
        GemColors::CreateInverted(colors_, sizeof(colors_), &colors);

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

    SECTION("Swap along column")
    {
        const int Rows = 5;
        const int Cols = 8;

        const char colors_[] = 
            /*  |        */
            ___01234567
            _4"BOOGYBRO"
            _3"OBOOGYBR" // -
            _2"ROBBOGYB" // -
            _1"BBOBROGY"
            _0"YBROBROG";

        using GemColors = m3::Board<gem_color_t, Rows, Cols>;

        GemColors colors;
        GemColors::CreateInverted(colors_, sizeof(colors_), &colors);

        auto c = 1;
        auto r0 = 2;
        auto r1 = 3;

        auto m = GetMatchesForSwap_Col(c, r0, r1, colors);

        REQUIRE(m.Row_0 == RowSpan { 2, 1, 3 });
        REQUIRE(m.Row_1 == RowSpan { 3, 0, 3 });
        REQUIRE(m.Col_0 == ColSpan { 1, 0, 2 });
        REQUIRE(m.Col_1 == ColSpan { 1, 3, 4 });
    }

    SECTION("Swap along row")
    {
        const int Rows = 5;
        const int Cols = 8;

        const char colors_[] = 
            /*  |        */
            ___01234567
            _4"BROGRBRO"
            _3"OBRORYBR" // -
            _2"ROORORRB" // -
            _1"BRBOROGY"
            _0"YBROYGOG";

        using GemColors = m3::Board<gem_color_t, Rows, Cols>;

        GemColors colors;
        GemColors::CreateInverted(colors_, sizeof(colors_), &colors);

        auto r = 2;
        auto c0 = 3;
        auto c1 = 4;

        auto m = GetMatchesForSwap_Row(r, c0, c1, colors);

        REQUIRE(m.Row_0 == RowSpan { 2, 1, 3 });
        REQUIRE(m.Row_1 == RowSpan { 2, 4, 6 });
        REQUIRE(m.Col_0 == ColSpan { 3, 0, 3 });
        REQUIRE(m.Col_1 == ColSpan { 4, 1, 4 });
    }
}

#endif