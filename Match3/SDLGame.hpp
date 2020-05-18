#pragma once

#include <SDL.h>
#include <Direct3D11.hpp>
#include <string>

class SDLGame
{
protected:
    Common::Direct3D11 m_D3D11;
    std::string m_ShadersPath;
    SDL_Window* m_Window;
    
public:
    virtual ~SDLGame() {};

    int Run(int argc, char** argv);

    virtual void OnCreate() = 0;
    virtual void OnUpdate(double dtSeconds) = 0;
    virtual void OnRender(int width, int height) = 0;
    virtual void OnDestroy() = 0;

    virtual void OnKeyDown(SDL_Keycode keyCode) {};
    virtual void OnKeyUp(SDL_Keycode keyCode) {};
};

#define RunGame(Game__)\
    int main(int argc, char** argv) {\
        return Game__().Run(argc, argv);\
    }