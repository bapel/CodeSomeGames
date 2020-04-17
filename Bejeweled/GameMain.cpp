#include <SDL.h>
#include <SDL_assert.h>

#include "Direct3D11.hpp"

int main(int argc, char** argv)
{
    SDL_assert(0 == SDL_Init(0));

    SDL_Window* window = SDL_CreateWindow(
        "Bejeweled Clone",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1024, 768,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    SDL_assert(nullptr != window);

    Direct3D11 d3d11;

    d3d11.Init(window);

    bool quit = false;
    SDL_Event event = {};

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (SDL_QUIT == event.type)
            {
                quit = true;
                break;
            }
        }

        if (quit)
            break;

        // Main Loop.
        d3d11.FrameStart(window);
        d3d11.FrameEnd();
    }

    SDL_DestroyWindow(window);
    return 0;
}