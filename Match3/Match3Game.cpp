#define NOMINMAX

#include <SDLGame.hpp>
#include "Match3BoardView.hpp"

struct CameraConstantsBuffer
{
    Matrix model;
    Matrix viewProj;
};

class Match3Game final : public SDLGame
{
    Match3BoardView m_BoardView;
    ComPtr<ID3D11Buffer> m_CameraConstantsBuffer;
    
    void OnCreate() 
    {
        // Load resources.

        m_BoardView.Init(m_D3D11, m_ShadersPath);

        m_CameraConstantsBuffer = m_D3D11.CreateConstantsBuffer<CameraConstantsBuffer>();
        m_D3D11.SetDebugName(m_CameraConstantsBuffer.Get(), "CameraConstantsBuffer");

        const auto d3dDevice = m_D3D11.GetDevice().Get();

        // Init game.
        // @Todo.
    }

    void OnUpdate(double dtSeconds) { }

    // @Todo. Get width and height from Direct3D11?
    void OnRender(int viewportWidth, int viewportHeight)
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
        m_D3D11.ClearBackBuffer(Color(0x2a2b3eff));

        ID3D11Buffer* constantsBuffers[] = { m_CameraConstantsBuffer.Get() };
        d3dContext->VSSetConstantBuffers(0, 1, constantsBuffers);

        m_BoardView.Begin();
        m_BoardView.RenderGem({ 0.0f, 0.0f }, { 100.0f, 100.0f }, Color(1, 1, 1, 1));
        m_BoardView.End();

        m_D3D11.EndFrame();
    }

    void OnDestroy()
    { }
};

#ifndef Test__
Game__(Match3Game)
#endif