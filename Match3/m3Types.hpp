#pragma once

#include <cstdint>
#include <cassert>

namespace m3
{
    using row_t = int8_t;
    using col_t = int8_t;

    using count_t = uint16_t;

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

        inline S operator[] (S s) const 
        {
            assert(s < Count());
            return m_0 + s;
        }
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
    };

    struct ColSpan : Span<col_t, row_t>
    {
        ColSpan(row_t r0, row_t r1, col_t c) : 
            Span<col_t, row_t>(c, r0, r1)
        { }

        inline row_t Row_0() const { return m_0; }
        inline col_t Row_1() const { return m_1; }
        inline col_t Col() const { return m_T; }
    };

    struct Matches
    {
        RowSpan Row_0, Row_1;
        ColSpan Col_0, Col_1;
    };
}