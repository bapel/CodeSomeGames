#pragma once

#include "Collections.hpp"
#include <limits>

Namespace__
{
    namespace internal_
    {
        constexpr uint32_t NextPow2(uint32_t v)
        {
            v--;
            v |= v >> 1;
            v |= v >> 2;
            v |= v >> 4;
            v |= v >> 8;
            v |= v >> 16;
            return ++v;
        }

        template <unsigned int N>
        struct BitSet_T
        { using T = uint64_t; };

        template <> struct BitSet_T<8>  { using T = uint8_t;  };
        template <> struct BitSet_T<16> { using T = uint16_t; };
        template <> struct BitSet_T<32> { using T = uint32_t; };

        template <unsigned int N, typename T = uint64_t>
        struct BitSetBase
        {
        public:
            using ThisType = BitSetBase<N, T>;
            using ValueType = T;
            static const auto One = (T)1;

            BitSetBase()
            { memset(m_Ints, 0, DataSize()); }

            BitSetBase(ConstRefType<ValueType> value) : BitSetBase()
            { m_Ints[0] = value; }

            template <class U>
            BitSetBase(std::initializer_list<U> values)
            {
                auto vs = sizeof(U) * values.size();
                auto s = Min(DataSize(), vs);
                
                memcpy(m_Ints, values.begin(), s);

                if (DataSize() > vs)
                {
                    auto r = DataSize() - vs;
                    memset((uint8_t*)m_Ints + s, 0, r);
                }
            }

            BitSetBase(std::initializer_list<ValueType> values)
            {
                auto i = 0U;
                for (; i < Min(NumInts, values.size()); i++)
                    m_Ints[i] = *(values.begin() + i);
                for (; i < NumInts; i++)
                    m_Ints[i] = 0;
            }

            template <class U>
            BitSetBase(const U* values, CountType n)
            {
                auto vs = sizeof(U) * n;
                auto s = Min(DataSize(), vs);

                memcpy(m_Ints, values, s);

                if (DataSize() > vs)
                {
                    auto r = DataSize() - vs;
                    memset((uint8_t*)m_Ints + s, 0, r);
                }
            }

            constexpr CountType Count() const { return sizeof(m_Ints) * 8; }
            constexpr SizeType DataSize() const { return sizeof(m_Ints); }

            ValueType* Ints() { return m_Ints; };
            const ValueType* Ints() const { return m_Ints; }
            CountType IntsCount() const { return NumInts; }

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

            void Set(IndexType index)
            {
                #ifdef BoundsCheck__
                assert(index < Count());
                #endif

                const auto i = index / BitsInValue;
                const auto mod = index & MaxInt;
                m_Ints[i] |= (One << mod);
            }

            void Unset(IndexType index)
            {
                #ifdef BoundsCheck__
                assert(index < Count());
                #endif

                const auto i = index / BitsInValue;
                const auto mod = index & MaxInt;
                m_Ints[i] &= ~(One << mod);
            }

            void Clear()
            { memset(m_Ints, 0, DataSize()); }

            bool operator[] (IndexType index) const
            { return Get(index); }

            bool operator == (const ThisType& other) const
            { return 0 == memcmp(m_Ints, other.m_Ints, DataSize()); };

            bool operator != (const ThisType& other) const
            { return 0 != memcmp(m_Ints, other.m_Ints, DataSize()); };

            bool operator && (const ThisType& other)
            {
                bool result = m_Ints[0] && other.m_Ints[0];
                for (auto i = 1U; i < NumInts; i++)
                    result &= m_Ints[i] && other.m_Ints[i];
                return result;
            }

            ThisType operator & (const ThisType& other) const
            {
                ThisType result;
                for (auto i = 0U; i < result.NumInts; i++)
                    result.m_Ints[i] = m_Ints[i] & other.m_Ints[i];
                return result;
            }

            ThisType operator | (const ThisType& other) const
            {
                ThisType result;
                for (auto i = 0U; i < result.NumInts; i++)
                    result.m_Ints[i] = m_Ints[i] | other.m_Ints[i];
                return result;
            }

            ThisType operator ^ (const ThisType& other) const
            {
                ThisType result;
                for (auto i = 0U; i < result.NumInts; i++)
                    result.m_Ints[i] = m_Ints[i] ^ other.m_Ints[i];
                return result;
            }

            ThisType operator ~ ()
            {
                ThisType result;
                for (auto i = 0U; i < result.NumInts; i++)
                    result.m_Ints[i] = ~m_Ints[i];
                return result;
            }

            ThisType operator << (int n)
            {
                ThisType result;
                ValueType carry = 0;
                auto i = 0U;

                while (((n - (int)BitsInValue) >= 0) 
                    && (i < NumInts))
                {
                    m_Ints[i++] = 0;
                    n -= BitsInValue;
                }

                for (; i < NumInts; i++)
                {
                    auto v = m_Ints[i];
                    result.m_Ints[i] = (v << n) | carry;
                    carry = v >> (BitsInValue - n);
                }

                return result;
            }

            ThisType operator >> (int n)
            {
                ThisType result;
                ValueType carry = 0;
                int i = NumInts - 1;

                while (((n - (int)BitsInValue) >= 0)
                    && (i >= 0))
                {
                    m_Ints[i--] = 0;
                    n -= BitsInValue;
                }

                for (; i >= 0; i--)
                {
                    auto v = m_Ints[i];
                    result.m_Ints[i] = (v >> n) | carry;
                    carry = v << (BitsInValue - n);
                }

                return result;
            }

        private:
            static const auto BitsInValue = sizeof(ValueType) * 8;
            static const auto Remainder = N % BitsInValue;
            static const auto NumInts = (N / BitsInValue) + (Remainder > 0);
            static const auto MaxInt = std::numeric_limits<ValueType>::max();

            ValueType m_Ints[NumInts];
        }; 
    }

    template <unsigned int N>
    using BitSet_T = typename internal_::BitSet_T<internal_::NextPow2(N)>::T;

    template <unsigned int N>
    using BitSet = internal_::BitSetBase<N, BitSet_T<N>>;
}