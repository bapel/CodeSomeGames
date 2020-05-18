#pragma once

#include <EASTL\array.h>
#include <EASTL\type_traits.h>
#include <EASTL\algorithm.h>
#include <EASTL\tuple.h>

#include <cstdint>
#include <cassert>

namespace m3
{
    using row_t = int8_t;
    using col_t = int8_t;

    using count_t = uint16_t;

    namespace private_
    {
        // @Todo: Use a vector instead of an array so size can be set at runtime.
        template <class T, int R, int C>
        struct Board
        {
            static_assert(eastl::is_pod<T>::value);
            using Array = eastl::array<T, R * C>;

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

                const auto rowSize = sizeof(T) * outBoard->Cols();

                for (auto r = 0; r < outBoard->Rows(); r++)
                {
                    auto src = (T*)data + (r * outBoard->Cols());
                    auto dst = &(*outBoard)((outBoard->Rows() - 1) - r, 0);
                    memcpy(dst, src, rowSize);
                }
            }

            inline row_t Rows() const { return R; }
            inline col_t Cols() const { return C; }
            inline count_t Count() const { return R * C; }

            inline count_t Index(row_t r, col_t c) const 
            { 
                assert(IsWithinBounds(r, c));
                return r * Cols() + c; 
            }

            inline       T& operator() (row_t r, col_t c)       { return m_Array[Index(r, c)]; }
            inline const T& operator() (row_t r, col_t c) const { return m_Array[Index(r, c)]; }

            inline bool IsWithinBounds(row_t r, col_t c) const
            {
                return r >= 0 && r < Rows() && c >= 0 && c < Cols();
            }
        };
    }

    using gem_id_t = uint8_t;
    using gem_color_t = uint8_t;

    const gem_color_t GemColors[] = { ' ', 'B', 'R', 'O', 'G', 'Y' };

    struct Gem
    {
        gem_id_t Id;
        gem_color_t Color;
    };

    template <class T, class S>
    struct Span
    {
        T m_T;
        S m_0, m_1;

        Span(T r, S s0, S s1) : 
            m_T(r), m_0(s0), m_1(s1) 
        { 
            assert(m_0 <= m_1); 
        }

        inline S Count() const { return m_1 - m_1; }
        inline bool Contains(S s) const { return s >= m_0 || s <= m_1; }
    };

    template <class T, class S>
    Span<T, S> Union(const Span<T, S>& a, const Span<T, S>& b)
    {
        assert(a.m_0 == b.m_0);
        assert(a.Contains(b.m_0) || a.Contains(b.m_1));
        return { a.m_T, SDL_min(a.m_0, b.m_0), SDL_max(a.m_1, b.m_1) };
    }

    struct RowSpan : Span<row_t, col_t>
    {
        RowSpan(row_t r, col_t c0, col_t c1) : 
            Span<row_t, col_t>(r, c0, c1)
        { }

        inline row_t Row() const { return m_T; }
        inline col_t Col_0() const { return m_0; }
        inline col_t Col_1() const { return m_1; }

        inline eastl::tuple<row_t, col_t> operator[] (col_t c) const 
        {
            assert(c < Count());
            return { Row(), Col_0() + c };
        }
    };

    struct ColSpan : Span<col_t, row_t>
    {
        ColSpan(row_t r0, row_t r1, col_t c) : 
            Span<col_t, row_t>(c, r0, r1)
        { }

        inline row_t Row_0() const { return m_0; }
        inline col_t Row_1() const { return m_1; }
        inline col_t Col() const { return m_T; }

        inline eastl::tuple<row_t, col_t> operator[] (row_t r) const 
        {
            assert(r < Count());
            return { Row_0() + r, Col() };
        }
    };

    struct Matches
    {
        RowSpan Row_0, Row_1;
        ColSpan Col_0, Col_1;
    };

    const row_t BoardRows = 8;
    const col_t BoardCols = 8;

    template <class T>
    using Board = private_::Board<T, BoardRows, BoardCols>;
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

    using GemColors = private_::Board<gem_color_t, Rows, Cols>;

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