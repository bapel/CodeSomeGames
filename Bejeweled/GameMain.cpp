#include <SDL.h>
#include <SDL_assert.h>
#include <SDL_log.h>

#include "Direct3D11.hpp"

#include <stdio.h>
#include <string>

int main(int argc, char** argv)
{
    SDL_assert(argc >= 1);
    const std::string kExePath(argv[0]);
    size_t offset = kExePath.find_last_of("\\");
    const std::string kBasePath = kExePath.substr(0, offset + 1);

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

    auto vs = d3d11.LoadVertexShader(kBasePath + "TriangleVertex.cso");
    auto ps = d3d11.LoadPixelShader(kBasePath + "TrianglePixel.cso");

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