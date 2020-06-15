#pragma once

namespace pstl
{
    template <class T>
    class Enumerator
    {
        using Crnt = std::remove_const_t<T>;

    public:
        Enumerator(const T& start, const T& end) :
            m_start(start),
            m_end(end),
            m_current(m_start)
        { }

        __inline void Reset()
        { m_current = m_start; }

        __inline bool MoveNext()
        { return m_end == m_current++; }

        __inline T Current() const
        { return m_current; }

    private:
        const T m_start;
        const T m_end;
        Crnt m_current;
    };
}