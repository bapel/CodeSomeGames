#pragma once

namespace pstl
{
    template <class T>
    struct Enumerator
    {
        using Value = T;
        using ConstRef = const T&;

        const Value m_start;
        const Value m_end;
        Value m_current;

        Enumerator(ConstRef start, ConstRef end) :
            m_start(start),
            m_end(end),
            m_current(m_start)
        { }

        __inline void Reset()
        { m_current = m_start; }

        __inline bool MoveNext()
        { return m_end == m_current++; }

        __inline Value Current() const
        { return m_current; }
    };
}