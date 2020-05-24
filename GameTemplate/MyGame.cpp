#define NOMINMAX

#include <SDLGame.hpp>

struct CameraConstantsBuffer
{
    Matrix Model;
    Matrix ViewProj;
};

class MyGame final: public SDLGame
{
private:
    ComPtr<ID3D11Buffer> m_CameraConstantsBuffer;

public:
    void OnCreate() final override
    {
        m_CameraConstantsBuffer = m_D3D11.CreateConstantsBuffer<CameraConstantsBuffer>();
        m_D3D11.SetDebugName(m_CameraConstantsBuffer.Get(), "CameraConstantsBuffer");
    }

    void OnUpdate(double dtSeconds) final override
    { }

    void OnRender(int viewportWidth, int viewportHeight) final override
    {
        CameraConstantsBuffer cameraConstantBufferData =
        {
            Matrix(), // Identity.
            Matrix::CreateOrthographic((float)viewportWidth, (float)viewportHeight, 0.0f, 1.0f)
        };

        m_D3D11.UpdateBufferData(m_CameraConstantsBuffer.Get(), cameraConstantBufferData);

        m_D3D11.BeginFrame();
        m_D3D11.BeginRender();
        m_D3D11.ClearBackBuffer(Color(0.5f, 0.5f, 0.5f, 1.0f));

        // @Todo: Add your rendering here.

        m_D3D11.EndFrame();
    }

    void OnDestroy() final override
    { }

    void OnKeyDown(SDL_Keycode keyCode) final override 
    { };

    void OnKeyUp(SDL_Keycode keyCode) final override 
    { };

    void OnMouseMove(int x, int y) final override 
    { };
};

#ifndef Test__
Game__(MyGame)
#endif