#pragma once

#include <cstdint>
#include <EASTL\numeric_limits.h>
#include <EASTL\type_traits.h>
#include <EASTL\functional.h>

namespace m3
{
    // This is an internal type.
    template <class T>
    struct Int_
    {
        static_assert(eastl::is_integral<T>());

        T m_I;

        Int_() = default;
        Int_(const T& i) : m_I(i) {}

        inline T Int() const { return m_I; }
    };

    #define IntType__(T__, I__)\
        struct T__ : public Int_<I__> { T__() = default; T__(int i) : Int_<I__>(i) {} }

    // @Todo: Not in use. Keep?
    #define UnaryOp__(T__, Op__)\
        inline T__ operator Op__ (const T__& a) { return Op__(a.m_I); }

    #define BinaryOp__(R__, T__, Op__)\
        inline R__ operator Op__ (const T__& a, const T__& b) { return a.m_I Op__ b.m_I; }\
        inline R__ operator Op__ (const int& a, const T__& b) { return a Op__ b.m_I; }\
        inline R__ operator Op__ (const T__& a, const int& b) { return a.m_I Op__ b; }

    #define AllOps__(T__)\
        BinaryOp__(T__, T__, +)\
        BinaryOp__(T__, T__, -)\
        BinaryOp__(T__, T__, *)\
        BinaryOp__(T__, T__, /)\
        BinaryOp__(T__, T__, %)\
        BinaryOp__(bool, T__, <)\
        BinaryOp__(bool, T__, >)\
        BinaryOp__(bool, T__, <=)\
        BinaryOp__(bool, T__, >=)\
        BinaryOp__(bool, T__, ==)\
        BinaryOp__(bool, T__, !=)
        /*BinaryOp__(T__, T__, &)\
        BinaryOp__(T__, T__, |)*/

    // These are distinct types to help avoid bugs related to mixing them up.
    IntType__(GemId, uint16_t);
    IntType__(GemColor, uint8_t);

    // These could be 8 bits, but then they show up as char in the debugger.
    IntType__(Row, int16_t);
    IntType__(Col, int16_t);
    
    // Id and Color only require comparison ops.
    BinaryOp__(bool, GemId, ==)
    BinaryOp__(bool, GemId, !=)
    BinaryOp__(bool, GemColor, ==)
    BinaryOp__(bool, GemColor, !=)

    // Row and Col require all ops.
    AllOps__(Row);
    AllOps__(Col);

    #undef AllOps__
    #undef UnaryOp__
    #undef BinaryOp__
    #undef IntType__
}

// Constants, invalid values and pre-defined colors.
namespace m3
{
    const GemId InvalidGemId = 0xFFFFFFFF;//eastl::numeric_limits<uint16_t>::max();

    const GemColor InvalidColor = ' ';
    const GemColor Blue = GemColor('B');
    const GemColor Red = GemColor('R');
    const GemColor Orange = GemColor('O');
    const GemColor Green = GemColor('G');
    const GemColor Yellow = GemColor('Y');

    const GemColor GemColors[] = 
    { InvalidColor, Blue, Red, Orange, Green, Yellow };
}

namespace eastl
{
    template <> 
    struct hash<m3::GemId>
    { 
        size_t operator()(m3::GemId id) const 
        { return static_cast<size_t>(id.Int()); }
    };
}