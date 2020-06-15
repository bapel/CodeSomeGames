#pragma once

namespace pstl
{
    template <class T>
    struct Range
    {
        using Value = T;
        using ConstRef = const T&;
        using Enumerator = Enumerator<T>;

        const Value m_start;
        const Value m_end;

        __inline Range(ConstRef start, ConstRef end) : 
            m_start(start),
            m_end(end)
        { }

        __inline Enumerator GetEnumerator() const
        { return { m_start, m_end }; }
    };
}