#pragma once

#include <SDL.h>
#include <string>

#include "Direct3D11.hpp"

class SDLGame
{
protected:
    Common::Direct3D11 m_D3D11;
    std::string m_ShadersPath;
    SDL_Window* m_Window = nullptr;
    
public:
    virtual ~SDLGame() {};

    virtual std::tuple<int, int> GetDesiredWindowSize() { return { 1024, 768 }; }

    int Run(int argc, char** argv);

    virtual void OnCreate() = 0;
    virtual void OnUpdate(double dtSeconds) = 0;
    virtual void OnRender(int width, int height) = 0;
    virtual void OnDestroy() = 0;

    virtual void OnKeyDown(SDL_Keycode keyCode) {};
    virtual void OnKeyUp(SDL_Keycode keyCode) {};
    virtual void OnMouseMove(int x, int y) {};
};

#define Game__(in_Game__)\
    int main(int argc, char** argv) {\
        auto* p = new in_Game__();\
        auto r = p->Run(argc, argv);\
        delete p;\
        return r;\
    }