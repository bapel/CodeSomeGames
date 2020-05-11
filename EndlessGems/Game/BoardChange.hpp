#pragma once

#include <stdint.h>
#include <array>
#include <vector>
#include <unordered_map>

#include "Board.hpp"

namespace Game
{
    using ChangeType = uint64_t;

    enum class BoardChangeType
    {
        None,
        Swap,
        SwapReverse,
        Spawn,
        Clear,
        Fall
    };

    struct BoardChange
    {
        BoardChangeType Type;
        union
        {
            struct { GemID From, To; } Swap;
            struct { GemID From, To; } SwapReverse;
            struct { GemLocation Location; GemColor Color; } Spawn;
            struct { GemID ID; } Clear;
            struct { GemID ID; } Fall;
        };
    };

    class BoardChangeQueue
    {
    private:
        std::vector<BoardChange> m_Changes;
        uint32_t m_Front;

    public:
        BoardChangeQueue() :
            m_Front(0)
        {}

        inline void PushBack(BoardChange change)
        {
            m_Changes.push_back(change);
        }

        inline BoardChange Front()
        {
            assert(m_Front < m_Changes.size());
            return m_Changes[m_Front];
        }

        inline void PopFront()
        {
            assert(m_Front < m_Changes.size());
            m_Front++;
        }

        inline uint32_t NumRemaining() const
        {
            return m_Changes.size() - m_Front;
        }

        inline void Clear()
        {
            m_Changes.clear();
            m_Front = 0;
        }

        inline const std::vector<BoardChange>& GetChanges() const
        {
            return m_Changes;
        }
    };
}