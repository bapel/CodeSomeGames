#define NOMINMAX

#include <SDLGame.hpp>

#include "m3Board.hpp"
#include "m3GemPool.hpp"
#include "m3GemData.hpp"
#include "m3Match.hpp"
#include "m3BoardView.hpp"

#include <EASTL\vector.h>
#include <EASTL\queue.h>

#include <random>

void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

void* __cdecl operator new[](size_t size, size_t align, size_t offset, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

static const auto BoardRows = 12;
static const auto BoardCols = 8;
static const auto SpriteSize = 64.0f;

struct CameraConstantsBuffer
{
    Matrix model;
    Matrix viewProj;
};

class Match3Game final : public SDLGame
{
private:
    ComPtr<ID3D11Buffer> m_CameraConstantsBuffer;

    m3::BoardView m_BoardView;

    using BoardIds = m3::Board<m3::gem_id_t, BoardRows, BoardCols>;
    using BoardIndices = m3::Board<size_t, BoardRows, BoardCols>;

    BoardIds m_BoardIds;
    BoardIndices m_BoardIndices;

    m3::GemPool m_GemPool;
    m3::GemData m_GemData;

    // Game data.
    std::mt19937 m_RandGenerator;
    std::uniform_int_distribution<m3::gem_color_t> m_ColorDistribution;

    // Internal functions.
private: 
    Vector2 Position(m3::row_t r, m3::col_t c, float spriteSize)
    {
        const auto cr = Vector2((float)BoardCols - 1, (float)BoardRows - 1);
        const auto origin = -0.5f * spriteSize * cr;
        return origin + spriteSize * Vector2((float)c, (float)r);
    }

private:
    std::tuple<int, int> GetDesiredWindowSize() override final
    {
        return 
        {
            (int)(16.0f + SpriteSize * BoardCols), 
            (int)(16.0f + SpriteSize * BoardRows) 
        };
    }

    void OnCreate() override final
    {
        m_CameraConstantsBuffer = m_D3D11.CreateConstantsBuffer<CameraConstantsBuffer>();
        m_D3D11.SetDebugName(m_CameraConstantsBuffer.Get(), "CameraConstantsBuffer");

        m_BoardView.Init(m_D3D11, m_ShadersPath);

        // Create and place random gems.
        for (auto i = 0; i < m_BoardIds.Count(); i++)
        {
            auto r = i / m_BoardIds.Cols();
            auto c = i % m_BoardIds.Cols();
            auto color = m3::GemColors[m_ColorDistribution(m_RandGenerator)];
            auto id = m_GemPool.GetOrCreateGem();
            auto index = m_GemData.Add(id, r, c, color);

            m_BoardIds(r, c) = id;
            m_BoardIndices(r, c) = index;
        }

        // Init position and scale of all gems.
        for (auto i = 0; i < m_GemData.Count(); i++)
        {
            auto r = m_GemData.GetRow(i);
            auto c = m_GemData.GetCol(i);
            auto position = Position(r, c, SpriteSize);
            auto scale = SpriteSize;

            m_GemData.SetPosition(i, position);
            m_GemData.SetScale(i, scale);
        }

        // destroy a few gems.
        {
            auto id0 = m_BoardIds(2, 3);
            auto id1 = m_BoardIds(2, 4);
            auto id2 = m_BoardIds(2, 5);
            auto idx0 = m_BoardIndices(2, 3);
            auto idx1 = m_BoardIndices(2, 4);
            auto idx2 = m_BoardIndices(2, 5);

            m_BoardIds(2, 3) = m3::InvalidGemId;
            m_BoardIds(2, 4) = m3::InvalidGemId;
            m_BoardIds(2, 5) = m3::InvalidGemId;

            m3::ScaleAnimation sa0 = { 0.0, 2, 3, 500, 500, 1, 0 };
            m3::ScaleAnimation sa1 = { 0.0, 2, 4, 1000, 500, 1, 0 };
            m3::ScaleAnimation sa2 = { 0.0, 2, 5, 1500, 500, 1, 0 };
            
            m_GemData.AddAnimation(idx0, sa0);
            m_GemData.AddAnimation(idx1, sa1);
            m_GemData.AddAnimation(idx2, sa2);
        }

        /*
                         c           
                +------+------+------+
                |      | cu   |      |
                |      |      |      |
                +------+------+------+
              r | rl   |      | rr   |
                |      |      |      |
                +------+------+------+
                |      | cd   |      |
                |      |      |      |
                +------+------+------+
            0,0                       
        */
        /*
        for (auto i = 0; i < m_Board.Count(); i++)
        {
            m3::row_t r = i / m_Board.Cols();
            m3::col_t c = i % m_Board.Cols();
            auto color = m_Board(c, r);

            auto rl = m3::RowSpan { r, c, m3::GetMatchingColsInRow_L(r, c, color, m_Board) };
            auto rr = m3::RowSpan { r, m3::GetMatchingColsInRow_R(r, c, color, m_Board), c };
            auto cu = m3::ColSpan { c, r, m3::GetMatchingRowsInCol_U(r, c, color, m_Board) };
            auto cd = m3::ColSpan { c, m3::GetMatchingRowsInCol_D(r, c, color, m_Board), r };

            if (rl.Count() >= 3)
            {
                for (auto i = 0; i < rl.Count(); i++)
                {
                    auto rr = rl[i];
                    auto cc = c;

                    auto id = m_Board(rr, cc);
                }
            }
        }
        */
    }

    void OnUpdate(double dtSeconds) override final 
    {
        using namespace m3;

        auto rows = m_BoardIds.Rows();
        auto cols = m_BoardIds.Cols();
        auto origin = -0.5f * SpriteSize * Vector2((float)cols - 1, (float)rows - 1);

        // Run scale animations and update gem scale.
        auto& scaleAnimations = m_GemData.GetScaleAnimations();
        for (auto i = 0; i < scaleAnimations.Data().size(); i++)
        {
            auto& scaleAnimation = scaleAnimations.Data()[i];
            auto scale = scaleAnimation.Current() * SpriteSize;
            auto gemId = scaleAnimations.GemFromDataIndex(i);
            auto index = m_GemData.DataIndexForGem(gemId);

            m_GemData.SetScale(index, scale);
            scaleAnimation.Update(dtSeconds * 1000.0);
        }
    }

    // @Todo. Get width and height from Direct3D11?
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

        ID3D11Buffer* constantsBuffers[] = { m_CameraConstantsBuffer.Get() };
        d3dContext->VSSetConstantBuffers(0, 1, constantsBuffers);

        m_BoardView.BeginRender();
        m_BoardView.RenderBackground(m_BoardIds.Rows(), m_BoardIds.Cols(), SpriteSize);
        m_BoardView.RenderGems(m_GemData, m_BoardIds.Rows(), m_BoardIds.Cols());
        m_BoardView.EndRender();

        m_D3D11.EndFrame();
    }

    void OnDestroy() override final { }

    void OnMouseMove(int x, int y) override final
    {}

public:
    Match3Game() :
        m_BoardIds(),
        m_BoardView(SpriteSize),
        m_RandGenerator(0),
        m_ColorDistribution(1, sizeof(m3::GemColors) - 1)
    {}
};

#ifndef Test__
Game__(Match3Game)
#endif