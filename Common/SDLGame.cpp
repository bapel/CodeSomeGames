#include "SDLGame.hpp"

#include <SDL_assert.h>
#include <SDL_image.h>
#include <SDL_events.h>

int SDLGame::Run(int argc, char** argv)
{
    SDL_assert_release(0 == SDL_Init(0));

    SDL_assert_release(argc >= 1);
    const std::string kExePath(argv[0]);
    size_t offset = kExePath.find_last_of("\\");
    m_ShadersPath = kExePath.substr(0, offset + 1);

    auto flags = IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    if (IMG_INIT_JPG != (flags & IMG_INIT_JPG))
        SDL_Log("Failed to load **jpeg** module");
    if (IMG_INIT_PNG != (flags & IMG_INIT_PNG))
        SDL_Log("Failed to load **png** module");

    m_Window = SDL_CreateWindow(
        "Bejeweled Clone",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1024, 768,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    SDL_assert_release(nullptr != m_Window);

    m_D3D11.Init(m_Window);
    OnCreate();

    bool quit = false;
    SDL_Event event = {};
    uint32_t lastMS = 0;
    float rotationAngle = 0.0f;

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (SDL_QUIT == event.type)
            {
                quit = true;
                break;
            }
            else if (SDL_KEYDOWN == event.type)
            {
                OnKeyDown(event.key.keysym.sym);
                break;
            }
            else if (SDL_KEYUP == event.type)
            {
                OnKeyUp(event.key.keysym.sym);
                break;
            }
        }

        if (quit)
            break;

        auto currentMS = SDL_GetTicks();
        auto elapsedMS = currentMS - lastMS;
        lastMS = currentMS;
        OnUpdate(elapsedMS / 1000.0);

        int w, h;
        SDL_GetWindowSize(m_Window, &w, &h);
        OnRender(w, h);

    }

    OnDestroy();
    SDL_DestroyWindow(m_Window);

    return 0;
}