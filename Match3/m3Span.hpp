#pragma once

#include "m3Types.hpp"

namespace m3
{
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

        // Inclusive.
        inline S Count() const { return m_1 - m_0 + 1; }

        inline bool Contains(S s) const { return s >= m_0 && s <= m_1; }

        inline S operator[] (S s) const 
        {
            assert(Contains(s));
            return m_0 + s;
        }

        inline bool operator == (const Span& other) const
        {
            return m_T == other.m_T
                && m_0 == other.m_0
                && m_1 == other.m_1;
        }

        inline bool operator != (const Span& other) const
        {
            return m_T != other.m_T
                && m_0 != other.m_0
                && m_1 != other.m_1;
        }
    };

    // Does not care if m_T is different.
    template <class T, class S>
    bool HasOverlap(const Span<T, S>& a, const Span<T, S>& b)
    {
        return a.Contains(b.m_0) || a.Contains(b.m_1);
    }

    template <class T>
    T Union(const T& a, const T& b)
    {
        assert(a.m_T == b.m_T);
        assert(HasOverlap(a, b));

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
        ColSpan(col_t c, row_t r0, row_t r1) : 
            Span<col_t, row_t>(c, r0, r1)
        { }

        inline row_t Row_0() const { return m_0; }
        inline col_t Row_1() const { return m_1; }
        inline col_t Col() const { return m_T; }
    };
}

#ifdef CatchAvailable__

TEMPLATE_TEST_CASE("Span overlap", "[span]", m3::RowSpan, m3::ColSpan)
{
    using namespace m3;

    SECTION("Span count")
    {
        TestType a = { 0, 1, 5 };
        TestType b = { 0, -5, -1 };

        REQUIRE(a.Count() == 5);
        REQUIRE(b.Count() == 5);
    }

    SECTION("Value containment")
    {
        TestType rs = { 0, -10, 10 };

        REQUIRE(rs.Contains(-10));
        REQUIRE(rs.Contains(10));
        REQUIRE(rs.Contains(0));
        REQUIRE(rs.Contains(1));
        REQUIRE(rs.Contains(-1));
        REQUIRE(!rs.Contains(20));
        REQUIRE(!rs.Contains(-20));
    }

    SECTION("Overlap when row or column are same, and intervals overlap")
    {
        TestType a = { 0, -10, 3 };
        TestType b = { 0, -3, 10 };

        REQUIRE(HasOverlap(a, b));

        a = { 0, -10, 3 };
        b = { 0, 3, 10 };

        REQUIRE(HasOverlap(a, b));
    }

    SECTION("No overlap when row or column are same, but intervals don't overlap")
    {
        TestType a = { 0, -10, -5 };
        TestType b = { 0, 5, 10 };

        REQUIRE(!HasOverlap(a, b));
    }

    SECTION("Union when overlap")
    {
        TestType a = { 0, -10, 3 };
        TestType b = { 0, -3, 10 };
        TestType u = Union(a, b);

        REQUIRE(u == TestType { 0, -10, 10 });
    }
}

#endif