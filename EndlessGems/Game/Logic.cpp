#include "Logic.hpp"

const Color Game::Constants::GemDisplayColors[] = 
{
    { 0.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 0.5f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f, 1.0f } 
};

const char Game::Constants::GemDebugChars[] = { ' ', 'B', 'G', 'P', 'O', 'R' };

std::vector<Game::GemLocation::Coord> Game::Logic::m_RowsBuffer;