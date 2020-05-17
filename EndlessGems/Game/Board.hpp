#pragma once

#include <VectorMath.hpp>
#include "Constants.hpp"

#include <vector>
#include <string>

namespace Game
{
    using GemID = int32_t;

    union GemLocation
    {
        typedef int32_t Coord;

        int64_t Value;
        struct { Coord Row, Col; };

        GemLocation() = default;

        GemLocation(int row, int col)
        {
            Row = row;
            Col = col;
        }

        inline GemLocation Above (int16_t offset = 1) const { return { Row + offset, Col }; }
        inline GemLocation Below (int16_t offset = 1) const { return { Row - offset, Col }; }
        inline GemLocation Left  (int16_t offset = 1) const { return { Row, Col - offset }; }
        inline GemLocation Right (int16_t offset = 1) const { return { Row, Col + offset }; }

        inline bool operator == (const GemLocation& y) { return Value == y.Value; }
        inline bool operator != (const GemLocation& y) { return Value != y.Value; }

        inline GemLocation operator - (const GemLocation& y) { return { Row - y.Row, Col - y.Col }; }
        inline GemLocation operator + (const GemLocation& y) { return { Row + y.Row, Col + y.Col }; }
    };

    struct Gem
    {
        GemID ID;
        GemColor Color;
        GemLocation Location;

        static inline Gem Default() { return { (GemID)-1, GemColor::None, { -1, -1 } }; }
    };

    class Board
    {
    private:
        union SListNode
        {
            GemID Next;
            Gem Gem;
        };

        static const int k_Rows = Constants::BoardRows;
        static const int k_Cols = Constants::BoardCols;

        GemID m_GemIDs[k_Rows][k_Cols];
        std::unique_ptr<SListNode> m_Gems;
        GemID m_SListHead;
        uint32_t m_NumAliveGems;

    public:
        Board() :
            m_Gems(new SListNode[Rows() * Cols()]),
            m_SListHead(0),
            m_NumAliveGems(0)
        {
            int i = 0;
            SListNode node;

            for (; i < Count() - 1; i++)
            {
                node.Gem = Gem::Default();
                node.Next = i + 1;
                m_Gems.get()[i] = node;
            }

            node.Gem = Gem::Default();
            node.Next = -1;
            m_Gems.get()[i] = node;

            for (auto r = 0; r < Rows(); r++)
            {
                for (auto c = 0; c < Cols(); c++)
                    m_GemIDs[r][c] = -1;
            }
        }

        inline int Rows() const { return k_Rows; }
        inline int Cols() const { return k_Cols; }
        inline int Count() const { return Rows() * Cols(); }

        inline size_t NumAliveGems() const { return m_NumAliveGems; }

        Gem SpawnGem(GemLocation location, GemColor color)
        {
            // Ensure the location is not already taken.
            assert(m_GemIDs[location.Row][location.Col] == -1);

            // Can we spawn another gem.
            // @reconsider: Should this be an assert?
            if (m_SListHead == -1)
                return { -1, GemColor::None };

            auto id = m_SListHead;
            auto& node = m_Gems.get()[id];

            m_SListHead = node.Next;
            m_GemIDs[location.Row][location.Col] = id;
            m_NumAliveGems += 1;

            node.Gem.ID = id;
            node.Gem.Location = location;
            node.Gem.Color = color;

            return node.Gem;
        }

        void ClearGem(GemID id)
        {
            // Ensure valid ID.
            assert(id > -1 && id < Count());

            auto location = m_Gems.get()[id].Gem.Location;

            m_GemIDs[location.Row][location.Col] = -1;
            m_Gems.get()[id].Next = m_SListHead;
            m_SListHead = id;
            m_NumAliveGems -= 1;
        }

        void MoveGem(GemLocation from, GemLocation to)
        {
            // Ensure destination is a "hole".
            assert(m_GemIDs[to.Row][to.Col] == -1);

            auto id = m_GemIDs[from.Row][from.Col];

            m_GemIDs[to.Row][to.Col] = id;
            m_GemIDs[from.Row][from.Col] = -1;
            m_Gems.get()[id].Gem.Location = to;
        }
        
        // Lookup Gem by ID.
        inline Gem operator[] (GemID id) const 
        {
            // Ensure an alive gem is being looked up.
            assert(id > -1 && id < Count());

            return m_Gems.get()[id].Gem;
        }

        inline bool IsLocationWithinBounds(const GemLocation& location) const
        {
            return location.Row > -1 
                && location.Col > -1
                && location.Row < Rows() 
                && location.Col < Cols();
        }

        // Lookup Gem by location.
        inline Gem operator[] (GemLocation location) const 
        {
            // Ensure an the location is within the board bounds.
            assert(IsLocationWithinBounds(location));
            auto id = m_GemIDs[location.Row][location.Col];

            if (id == -1)
                return Gem::Default();

            return m_Gems.get()[id].Gem;
        }

        std::string ToString() const
        {
            std::string str = "";

            // Row 0 is at the bottom.
            for (int r = Rows() - 1; r >= 0; r--)
            {
                for (int c = 0; c < Cols(); c++)
                {
                    auto id = m_GemIDs[r][c];
                    auto data = m_Gems.get()[id];

                    str += Constants::GetGemDebugChar(data.Gem.Color);

                    if (id >= 0)
                    {
                        str += '(';
                        if (id < 10)
                            str += ' ';
                        str += std::to_string(id);
                        str += ") ";
                    }
                    else str += "     ";
                }

                str += '\n';
            }

            return std::move(str);
        }
    };
}