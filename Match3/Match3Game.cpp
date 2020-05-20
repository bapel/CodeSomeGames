#define NOMINMAX

#include <SDLGame.hpp>

#include "m3Board.hpp"
#include "m3BoardView.hpp"

#include <EASTL\vector.h>

struct CameraConstantsBuffer
{
    Matrix model;
    Matrix viewProj;
};

class Match3Game final : public SDLGame
{
    ComPtr<ID3D11Buffer> m_CameraConstantsBuffer;

    static const auto BoardRows = 12;
    static const auto BoardCols = 8;
    using Board = m3::Board<m3::gem_id_t, BoardRows, BoardCols>;

    m3::BoardView m_BoardView;

    eastl::vector<m3::gem_id_t> m_GemIds;
    eastl::vector<m3::row_t> m_GemRows;
    eastl::vector<m3::col_t> m_GemCols;
    eastl::vector<m3::gem_color_t> m_GemColors;

    Board m_Board;

    std::tuple<int, int> GetDesiredWindowSize()
    {
        return 
        {
            16.0f + 64.0f * BoardCols, 
            16.0f + 64.0f * BoardRows 
        };
    }
    
    void OnCreate() 
    {
        m_CameraConstantsBuffer = m_D3D11.CreateConstantsBuffer<CameraConstantsBuffer>();
        m_D3D11.SetDebugName(m_CameraConstantsBuffer.Get(), "CameraConstantsBuffer");

        m_BoardView.Init(m_D3D11, m_ShadersPath);
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
        m_D3D11.ClearBackBuffer(Color(0.5f, 0.5f, 0.5f, 1.0f));

        ID3D11Buffer* constantsBuffers[] = { m_CameraConstantsBuffer.Get() };
        d3dContext->VSSetConstantBuffers(0, 1, constantsBuffers);

        m_BoardView.BeginRender();
        m_BoardView.RenderBackground(m_Board.Rows(), m_Board.Cols());
        m_BoardView.EndRender();
        
        m_D3D11.EndFrame();
    }

    void OnDestroy()
    { }
};

#ifndef Test__
Game__(Match3Game)
#endif