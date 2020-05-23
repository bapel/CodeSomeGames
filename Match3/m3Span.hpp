#pragma once

#include "m3Types.hpp"

namespace m3
{
    template <class T, class U>
    struct Span
    {
        T m_T;
        U m_0, m_1;

        Span() = default;
        Span(T r, U s0, U s1) : 
            m_T(r), m_0(s0), m_1(s1) 
        { 
            assert(m_0 <= m_1); 
        }

        // Inclusive.
        inline U Count() const { return m_1 - m_0 + 1; }

        inline bool Contains(U s) const { return s >= m_0 && s <= m_1; }

        inline U operator[] (U s) const 
        {
            assert(Contains(m_0 + s));
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
    template <class T, class U>
    bool HasOverlap(const Span<T, U>& a, const Span<T, U>& b)
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

    using Row_ = Row;
    using Col_ = Col;

    // Span along row. Some col to some other col.
    struct RowSpan : Span<Row, Col>
    {
        RowSpan() = default;
        RowSpan(Row r, Col c0, Col c1) : 
            Span<Row_, Col>(r, c0, c1)
        { }

        inline Row Row() const { return m_T; }
        inline Col Col_0() const { return m_0; }
        inline Col Col_1() const { return m_1; }
    };

    // Span along col. Some row to some other row.
    struct ColSpan : Span<Col_, Row>
    {
        ColSpan() = default;
        ColSpan(Col c, Row r0, Row r1) : 
            Span<Col_, Row>(c, r0, r1)
        { }

        inline Row Row_0() const { return m_0; }
        inline Row Row_1() const { return m_1; }
        inline Col Col() const { return m_T; }
    };
}

#ifdef CatchAvailable__

namespace Catch 
{
    using namespace m3;

    template<>
    struct StringMaker<RowSpan> 
    {
        static std::string convert(const RowSpan rs) 
        {
            std::ostringstream stream;
            stream << "RowSpan (" << rs.Row().m_I << ", " << rs.Col_0().m_I << ", " << rs.Col_1().m_I << ")";
            return stream.str();
        }
    };

    template<>
    struct StringMaker<ColSpan> 
    {
        static std::string convert(const ColSpan cs) 
        {
            std::ostringstream stream;
            stream << "ColSpan (" << cs.Col().m_I << ", " << cs.Row_0().m_I << ", " << cs.Row_1().m_I << ")";
            return stream.str();
        }
    };
}

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