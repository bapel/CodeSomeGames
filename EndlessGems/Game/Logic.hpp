#pragma once

#include "BoardChange.hpp"
#include <vector>

namespace Game
{
    class Logic
    {
    public:
        static std::vector<GemLocation::Coord> m_RowsBuffer;

        // Disallow creation.
        Logic() = delete;

        static int ProcessQueue(Board* inOutBoard, BoardChangeQueue* inOutQueue)
        {
            if (inOutQueue->NumRemaining() == 0)
                return 0;

            auto change = inOutQueue->Front();
            switch (change.Type)
            {
                case BoardChangeType::Spawn:
                {
                    auto location = change.Spawn.Location;
                    auto color = change.Spawn.Color;
                    auto gem = inOutBoard->SpawnGem(location, color);

                    inOutQueue->PopFront();

                    if (gem.ID > -1)
                    {
                        // @todo. Side effects.
                    }

                    return 1;
                }

                case BoardChangeType::Clear:
                {
                    auto numClears = 0;

                    // @todo. More robust Fall generation.
                    // Store clear locations into a vector and check above.

                    auto cols_0 = inOutBoard->Cols();
                    auto cols_1 = 0;

                    m_RowsBuffer.resize(inOutBoard->Cols());
                    for (auto& top : m_RowsBuffer)
                        top = -1;

                    while (change.Type == BoardChangeType::Clear 
                        && inOutQueue->NumRemaining() > 0)
                    {
                        auto id = change.Clear.ID;
                        auto location = (*inOutBoard)[id].Location;

                        inOutBoard->ClearGem(id);
                        inOutQueue->PopFront();
                        
                        cols_0 = min(cols_0, location.Col);
                        cols_1 = max(cols_1, location.Col);
                        m_RowsBuffer[location.Col] = max(m_RowsBuffer[location.Col], location.Row);

                        numClears += 1;

                        if (inOutQueue->NumRemaining() == 0)
                            break;

                        change = inOutQueue->Front();
                    }

                    for (auto c = cols_0; c <= cols_1; c++)
                    {
                        GemLocation location = { m_RowsBuffer[c], c };
                        location = location.Above();

                        if (inOutBoard->IsLocationWithinBounds(location))
                        {
                            auto fall = BoardChange();
                            fall.Type = BoardChangeType::Fall;
                            fall.Fall.ID = (*inOutBoard)[location].ID;

                            inOutQueue->PushBack(fall);
                        }
                    }

                    return numClears;
                }

                case BoardChangeType::Fall:
                {
                    auto id = change.Fall.ID;
                    auto from = (*inOutBoard)[id].Location;
                    auto to = from;
                    auto id_1 = -1;

                    do 
                    {
                        to = to.Below();
                    }
                    while (inOutBoard->IsLocationWithinBounds(to.Below())
                        && (*inOutBoard)[to.Below()].ID == -1);
                    
                    if (from != to)
                    {
                        inOutBoard->MoveGem(from, to);

                        auto above = from.Above();
                        if (inOutBoard->IsLocationWithinBounds(above)
                         && ((*inOutBoard)[above].ID != -1))
                        {
                            auto fall = BoardChange();
                            fall.Type = BoardChangeType::Fall;
                            fall.Fall.ID = (*inOutBoard)[above].ID;

                            inOutQueue->PushBack(fall);
                        }
                    }
                        
                    inOutQueue->PopFront();
                    return 1;
                }

                case BoardChangeType::Swap:
                case BoardChangeType::SwapReverse:
                default:
                    // @todo: Implement.
                    inOutQueue->PopFront();
                    return 1;
            }
        }
    };
}