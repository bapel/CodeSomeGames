#pragma once

#include <cstdint>
#include <EASTL\numeric_limits.h>
#include <EASTL\type_traits.h>

namespace m3
{
    template <class T>
    struct Int
    {
        static_assert(eastl::is_integral<T>());

        T m_I;

        Int() = default;
        Int(const T& i) : m_I(i) {}
    };

    #define UnaryOp__(T__, Op__)\
        inline T__ operator Op__ (const T__& a) { return Op__(a.m_I); }
            
    #define BinaryOp__(R__, T__, Op__)\
        inline R__ operator Op__ (const T__& a, const T__& b) { return a.m_I Op__ b.m_I; }\
        inline R__ operator Op__ (const int& a, const T__& b) { return a Op__ b.m_I; }\
        inline R__ operator Op__ (const T__& a, const int& b) { return a.m_I Op__ b; }

    #define DeclareOps__(T__)\
        BinaryOp__(T__, T__, +)\
        BinaryOp__(T__, T__, -)\
        BinaryOp__(T__, T__, *)\
        BinaryOp__(T__, T__, /)\
        BinaryOp__(T__, T__, %)\
        BinaryOp__(T__, T__, &)\
        BinaryOp__(T__, T__, |)\
        BinaryOp__(bool, T__, <)\
        BinaryOp__(bool, T__, >)\
        BinaryOp__(bool, T__, <=)\
        BinaryOp__(bool, T__, >=)\
        BinaryOp__(bool, T__, ==)\
        BinaryOp__(bool, T__, !=)

    // These could be 8 bits, but then they would show up as chars in the debugger.
    struct Row : public Int<int16_t> { Row() = default; Row(int i) : Int<int16_t>(i) {} };
    struct Col : public Int<int16_t> { Col() = default; Col(int i) : Int<int16_t>(i) {} };

    DeclareOps__(Row);
    DeclareOps__(Col);

    #undef UnaryOp__
    #undef BinaryOp__
    #undef DeclareOps__
    
    using gem_id_t = uint16_t;
    constexpr gem_id_t InvalidGemId = eastl::numeric_limits<gem_id_t>::max();
    
    using gem_color_t = uint8_t;
    const gem_color_t InvalidColor = ' ';
    constexpr gem_color_t GemColors[] = { InvalidColor, 'B', 'R', 'O', 'G', 'Y' };

    struct RowCol
    {
        Row Row;
        Col Column;
    };
}