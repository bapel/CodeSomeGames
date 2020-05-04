#pragma once

#include <assert.h>
#include <stdint.h>
#include <vector>
#include <array>

#include "Direct3D11.hpp"

struct GemsConstants
{
    static const int BoardRows = 8;
    static const int BoardCols = 8;
    static const Color Colors[];
};

const Color GemsConstants::Colors[] = 
{
    Color(0), // 0,
    Color(0xe1b23cff), // 0xffff0000,
    Color(0xdd663cff), // 0xff00ff00,
    Color(0x014025ff), // 0xff800080,
    Color(0x067473ff), // 0xff0066ff,
    Color(0x735c6eff)  // 0xff0000ff
};

enum class GemColor
{
    None = 0,
    Blue,
    Green,
    Purple,
    Orange,
    Red,
    Count
};

union GemLocation
{
    int16_t Value;
    struct { int8_t Row, Col; };

    GemLocation(int row, int col)
    {
        Row = row;
        Col = col;
    }

    inline GemLocation Above() const { return { Row - 1, Col }; }
    inline GemLocation Below() const { return { Row + 1, Col }; }
    inline GemLocation Left() const  { return { Row, Col - 1 }; }
    inline GemLocation Right() const { return { Row, Col + 1 }; }
};

inline bool operator == (const GemLocation& x, const GemLocation& y) { return x.Value == y.Value; }
inline bool operator != (const GemLocation& x, const GemLocation& y) { return x.Value != y.Value; }

inline GemLocation operator - (const GemLocation& x, const GemLocation& y) { return { y.Row - x.Row, y.Col - x.Col }; }
inline GemLocation operator + (const GemLocation& x, const GemLocation& y) { return { y.Row + x.Row, y.Col + x.Col }; }

class GemLocationIterator
{
private:
    const GemLocation& m_From;
    const GemLocation& m_To;
    GemLocation m_Current;
    GemLocation m_Offset;

public:
    GemLocationIterator(const GemLocation& from, const GemLocation& to) :
        m_From(from), m_To(to), m_Current(from), m_Offset(to - from)
    {
        if (m_Offset.Row > 1)
        {
            // Only along row.
            assert(m_Offset.Col == 0);
            m_Offset.Row = 1;
        }

        if (m_Offset.Col > 1) 
        {
            // Only along col.
            assert(m_Offset.Row == 0);
            m_Offset.Col = 1;
        }
    }

    inline GemLocation Current() const { return m_Current; }
    inline bool IsColumn() const { return m_From.Row == m_To.Row; }

    inline void Reset() { m_Current = m_From; }

    inline bool Next() 
    {
        assert(m_Current != m_To);
        m_Current = m_Current + m_Offset;
        return m_Current != m_To;
    }
};

enum class ChangeType
{
    None = 0,
    Swap,
    Spawn,
    Clear,
    Cascade
};

struct GemsBoardChange
{
    ChangeType Type;
    union
    {
        struct { GemLocation At, With; } Swap;
        struct { GemLocation At; GemColor Color; } Spawn;
        struct { GemLocation From, To; inline GemLocationIterator Iterator() const { return { From, To }; } } Cascade;
        struct { GemLocation From, To; inline GemLocationIterator Iterator() const { return { From, To }; } } Clear;
    };

    GemsBoardChange() : Type(ChangeType::None) {}
    GemsBoardChange(ChangeType type, GemLocation l0, GemLocation l1) : Type(type), Cascade({ l0, l1 }) {}

    static GemsBoardChange CreateSwap(GemLocation at, GemLocation with)
    {
        assert(at != with);
        return { ChangeType::Swap, at, with };
    }

    static GemsBoardChange CreateSpawn(GemLocation at)
    {
        // No way to know if this is valid uptil actual logic.
        return { ChangeType::Spawn, at, at };
    }

    static GemsBoardChange CreateCascade(GemLocation from, GemLocation to)
    {
        // Should only be from above.
        assert(from.Col < to.Col);
        assert(from.Row == to.Row);
        return { ChangeType::Cascade, from, to };
    }

    static GemsBoardChange CreateClear(GemLocation from, GemLocation to)
    {
        auto difference = to - from;

        // Only along row.
        if (difference.Row > 1)
            assert(difference.Col == 0);

        // Only along col.
        if (difference.Col > 1)
            assert(difference.Row == 0);

        return { ChangeType::Cascade, from, to };
    }
};

struct GemsBoardState
{
    static const int Rows = GemsConstants::BoardRows;
    static const int Cols = GemsConstants::BoardCols;

    GemColor m_Colors[Rows][Cols];

    inline       GemColor& operator[](GemLocation position)       { return m_Colors[position.Row][position.Col]; }
    inline const GemColor& operator[](GemLocation position) const { return m_Colors[position.Row][position.Col]; }

    inline bool IsLocationValid(GemLocation location) const
    {
        return 
            location.Row > 0 
         && location.Row < Rows
         && location.Col > 0 
         && location.Col < Cols;
    }
};

struct GemsLogic
{
    static bool ApplyChange(const GemsBoardState& currentState, GemsBoardChange change, GemsBoardState& outNextState)
    {
        switch (change.Type)
        {
            case ChangeType::Swap:
                outNextState[change.Swap.At] = currentState[change.Swap.With];
                outNextState[change.Swap.With] = currentState[change.Swap.At];
                return true;

            case ChangeType::Spawn:
                // Reject if a color is already present at the location.
                // aka should not be able to if there is no "hole".
                if (currentState[change.Spawn.At] != GemColor::None)
                    return false;
                outNextState[change.Spawn.At] = change.Spawn.Color;
                return true;

            case ChangeType::Clear:
            {
                auto iter = change.Clear.Iterator();
                outNextState[iter.Current()] = GemColor::None;
                while (iter.Next())
                    outNextState[iter.Current()];
                return true;
            }

            case ChangeType::Cascade:
                outNextState[change.Cascade.To] = currentState[change.Cascade.From];
                outNextState[change.Cascade.From] = GemColor::None;
                return true;

            case ChangeType::None:
                return false;
        }
    }

    static int GetSideEffects(const GemsBoardState& currentState, GemsBoardChange change, std::vector<GemsBoardChange>& outSideEffects)
    {
        outSideEffects.clear();

        switch (change.Type)
        {
            case ChangeType::Clear:
            {
                auto locationIterator = change.Clear.Iterator();

                if (locationIterator.IsColumn())
                {
                    if (!currentState.IsLocationValid(change.Clear.From.Above()))
                        return 0;

                    auto from = change.Clear.To.Above();
                    auto to = change.Clear.From;
                    auto sideEffect = GemsBoardChange::CreateCascade(from, to);

                    outSideEffects.push_back(sideEffect);
                    return 1;
                }

                if (!currentState.IsLocationValid(locationIterator.Current().Above()))
                    return 0;

                int count = 0;

                do
                {
                    auto from = locationIterator.Current().Above();
                    auto to = locationIterator.Current();
                    auto sideEffect = GemsBoardChange::CreateCascade(from, to);
                    outSideEffects.push_back(sideEffect);
                    count++;
                }
                while (locationIterator.Next());

                return count;
            }

            case ChangeType::Cascade:
            {
                // @TODO: Produce clears. For now, simply cause another cascade.
                GemsBoardChange sideEffect;
                sideEffect.Type = ChangeType::Cascade;

                if (!currentState.IsLocationValid(change.Cascade.From.Above()))
                    return 0;

                sideEffect.Cascade.From = change.Cascade.From.Above();
                sideEffect.Cascade.To = change.Cascade.From;
            }

            case ChangeType::Swap:
            case ChangeType::Spawn:
            case ChangeType::None:
                return 0;        
        }
    }
};

class GemsBoardView
{
private:
    std::vector<float> m_Positions;
    std::vector<uint32_t> m_Colors;

    ComPtr<ID3D11Buffer> m_PosBuffer;
    ComPtr<ID3D11Buffer> m_ColBuffer;

public:
    
};