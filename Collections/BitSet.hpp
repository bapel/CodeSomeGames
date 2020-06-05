#pragma once

#include "Collections.hpp"
#include <limits>

Namespace__
{
    template <uint32_t N>
    struct BitSet
    {
    private:
        // @Todo: Parameterize using templates?
        using Value_T = uint64_t;
        static const auto One = 1ULL;

    public:
        BitSet()
        { memset(m_Ints, 0, DataSize()); }

        BitSet(std::initializer_list<Value_T> values)
        {
            auto i = 0U;
            for (; i < Min(NumInts, values.size()); i++)
                m_Ints[i] = *(values.begin() + i);
            for (; i < NumInts; i++)
                m_Ints[i] = 0;
        }

        BitSet(const Value_T* ints, CountType n)
        {
            auto i = 0U;
            for (; i < Min(NumInts, n); i++)
                m_Ints[i] = ints[i];
            for (; i < NumInts; i++)
                m_Ints[i] = 0;
        }

        constexpr CountType Count() const { return sizeof(m_Ints) * 8; }
        constexpr SizeType DataSize() const { return sizeof(m_Ints); }

        // true is all bits are set to true.
        bool All() const
        {
            for (auto i = 0U; i < NumInts; i++)
            {
                if (m_Ints[i] != MaxInt)
                    return false;
            }

            return true;
        }

        // true if any bit is set to true.
        bool Any() const
        {
            // @Todo: Could be faster.
            for (auto i = 0U; i < NumInts; i++)
            {
                if (m_Ints[i] > 0)
                    return true;
            }

            return false;
        }

        // true is no bits are true.
        bool None() const
        {
            for (auto i = 0U; i < NumInts; i++)
            {
                if (m_Ints[i] != 0)
                    return false;
            }

            return true;
        }

        bool Get(IndexType index) const
        {
            #ifdef BoundsCheck__
            assert(index < Count());
            #endif

            const auto i = index / BitsInValue;
            const auto mod = index & MaxInt;
            return (One << mod) & m_Ints[i];
        }

        void Set(IndexType index, bool value)
        {
            #ifdef BoundsCheck__
            assert(index < Count());
            #endif

            const auto i = index / BitsInValue;
            const auto mod = index & MaxInt;
            m_Ints[i] |= (One << mod);
        }

        void Clear()
        { memset(m_Ints, 0, DataSize()); }

        bool operator[] (IndexType index) const
        { return Get(index); }

        bool operator ! () const 
        {
            bool result = !m_Ints[0];
            for (auto i = 1U; i < NumInts; i++)
                result &= !m_Ints[i];
            return result;
        }

        bool operator && (const BitSet<N>& other)
        {
            bool result = m_Ints[0] && other.m_Ints[0];
            for (auto i = 1U; i < NumInts; i++)
                result &= m_Ints[i] && other.m_Ints[i];
            return result;
        }

        template <int M>
        auto operator & (const BitSet<M>& other) const
            -> BitSet<Max(N, M)>
        {
            BitSet<Max(N, M)> result;
            for (auto i = 0U; i < result.NumInts; i++)
                result.m_Ints[i] = m_Ints[i] & other.m_Ints[i];
        }

        template <int M>
        auto operator | (const BitSet<M>& other) const
            -> BitSet<Max(N, M)>
        {
            BitSet<Max(N, M)> result;
            for (auto i = 0U; i < result.NumInts; i++)
                result.m_Ints[i] = m_Ints[i] | other.m_Ints[i];
        }

        template <int M>
        auto operator ^ (const BitSet<M>& other) const
            -> BitSet<Max(N, M)>
        {
            BitSet<Max(N, M)> result;
            for (auto i = 0U; i < result.NumInts; i++)
                result.m_Ints[i] = m_Ints[i] ^ other.m_Ints[i];
        }

        BitSet<N> operator ~()
        {
            BitSet<N> result;
            for (auto i = 0U; i < result.NumInts; i++)
                result.m_Ints[i] = ~m_Ints[i];
        }

    private:
        static const auto BitsInValue = sizeof(Value_T) * 8;
        static const auto Remainder = N % BitsInValue;
        static const auto NumInts = (N / BitsInValue) + (Remainder > 0);
        static const auto MaxInt = std::numeric_limits<Value_T>::max();

        Value_T m_Ints[NumInts];
    }; 
}