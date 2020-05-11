#pragma once

#include "Logic.hpp"

namespace Game
{
    class BoardView
    {
    private:
        struct Transition
        {
            float Final;
            float Speed;
        };

        struct GemSprite
        {
            float Scale;
            Vector2 Position;
            Color Color;
        };

        std::vector<Transition> m_Transitions;
        std::vector<GemSprite> m_Position;

    public:
        
    };
}