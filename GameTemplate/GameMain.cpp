#define NOMINMAX

#include <SDL.h>

class Game final : public SDLGame
{
private:
    ComPtr<ID3D11Buffer> m_CameraConstantsBuffer;

private:
    std::tuple<int, int> GetDesiredWindowSize() override final { return { 1024, 768 } }

    void OnCreate() override final
    { }

    void OnUpdate(double dtSeconds) override final
    { }

    void OnRender(int viewportWidth, int viewportHeight) override final
    {
        CameraConstantsBuffer cameraConstantBufferData =
        {
            Matrix::CreateRotationX(0.0f), // @Todo. Matrix::Identity is undefined???
            Matrix::CreateOrthographic((float)viewportWidth, (float)viewportHeight, 0.0f, 1.0f)
        };

        m_D3D11.UpdateBufferData(m_CameraConstantsBuffer.Get(), cameraConstantBufferData);

        const auto d3dContext = m_D3D11.GetDeviceContext();

        m_D3D11.BeginFrame();
        m_D3D11.BeginRender();
        m_D3D11.ClearBackBuffer(Color(0.5f, 0.5f, 0.5f, 1.0f));

        // @Todo: Add rendering.

        m_D3D11.EndFrame();
    }

    void OnDestroy() override final 
    { }

    void OnMouseMove(int x, int y) override final 
    { }

public:
    Game()
    { }
};

#ifndef Test__
Game__(Game)
#endif