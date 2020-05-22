#define NOMINMAX

#include <SDLGame.hpp>

#include "m3Board.hpp"
#include "m3GemPool.hpp"
//#include "m3GemData.hpp"
#include "m3Match.hpp"
#include "m3BoardView.hpp"

#include <EASTL\vector.h>
#include <EASTL\hash_map.h>
#include <EASTL\algorithm.h>

#include <random>

void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

void* __cdecl operator new[](size_t size, size_t align, size_t offset, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

static const auto BoardRows = 6;
static const auto BoardCols = 6;
static const auto SpriteSize = 64.0f;

struct CameraConstantsBuffer
{
    Matrix model;
    Matrix viewProj;
};

// Order of members is to make it convenient to debug.
struct Tween
{
    float ElapsedMs;
    float Value_0;
    float Value_1;
    float DelayMs;
    float DurationMs;

    inline bool Completed() const { return (ElapsedMs - DelayMs) >= DurationMs; }

    inline float Evaluate()
    {
        auto t = ElapsedMs - DelayMs;
        t = eastl::max(t, 0.0f) / DurationMs;
        t = eastl::clamp(t, 0.0f, 1.0f);
        return Value_0 + t * (Value_1 - Value_0);
    }
};

class Match3Game final : public SDLGame
{
private:
    ComPtr<ID3D11Buffer> m_CameraConstantsBuffer;

    m3::BoardView m_BoardView;

    // Game data.
    using Id = m3::gem_id_t;
    using Index = size_t;
    struct IdIndex { Id Id; Index Index; };
    using Board = m3::Board<Id, BoardRows, BoardCols>;

    std::mt19937 m_RandGenerator;
    std::uniform_int_distribution<uint16_t> m_ColorDistribution;
    m3::GemPool m_GemPool;

    // Board data.
    Board m_Board;
    eastl::hash_map<Id, Index> m_IdToIndex;
    eastl::vector<Id> m_GemIds;
    eastl::vector<m3::row_t> m_GemRows;
    eastl::vector<m3::col_t> m_GemCols;
    eastl::vector<m3::gem_color_t> m_GemColors;

    // Computed stuff.
    eastl::vector<Vector2> m_GemPositions;
    eastl::vector<Vector2> m_GemScales;

    // Tweens.
    eastl::vector<size_t> m_DespawnDstIndices;
    eastl::vector<Tween> m_DespawnTweens;
    eastl::vector<size_t> m_FallDstIndices;
    eastl::vector<Tween> m_FallTweens;

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
        for (auto i = 0; i < m_Board.Count(); i++)
        {
            auto r = i / m_Board.Cols();
            auto c = i % m_Board.Cols();
            auto id = m_GemPool.GetOrCreateGem();
            auto index = m_GemRows.size();

            m_Board(r, c) = id;
            m_IdToIndex.insert(id);
            m_IdToIndex[id] = index;
            m_GemIds.push_back(id);
            m_GemRows.push_back(r);
            m_GemCols.push_back(c);
            m_GemColors.push_back(RandomGemColor());
        }

        m_GemPositions.resize(m_GemRows.size());
        m_GemScales.resize(m_GemRows.size());

        // Init position and scale of all gems.
        for (auto i = 0; i < m_GemPositions.size(); i++)
        {
            auto r = m_GemRows[i];
            auto c = m_GemCols[i];
            auto position = Position(r, c, SpriteSize);
            auto scale = SpriteSize;

            m_GemPositions[i] = position;
            m_GemScales[i] = { scale, scale };
        }

        // destroy a few gems.
        {
            Tween sa0 = { 0.0f, 1.0f, 0.0f, 200, 200 };
            Tween sa1 = { 0.0f, 1.0f, 0.0f, 400, 200 };
            Tween sa2 = { 0.0f, 1.0f, 0.0f, 600, 200 };

            m_DespawnDstIndices.push_back(m_IdToIndex[m_Board(3, 2)]);
            m_DespawnDstIndices.push_back(m_IdToIndex[m_Board(3, 3)]);
            m_DespawnDstIndices.push_back(m_IdToIndex[m_Board(3, 4)]);

            //m_DespawnDstIndices.push_back(m_IdToIndex[m_Board(0, 2)]);
            //m_DespawnDstIndices.push_back(m_IdToIndex[m_Board(1, 2)]);
            //m_DespawnDstIndices.push_back(m_IdToIndex[m_Board(2, 2)]);

            m_DespawnTweens.push_back(sa0);
            m_DespawnTweens.push_back(sa1);
            m_DespawnTweens.push_back(sa2);
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

        auto rows = m_Board.Rows();
        auto cols = m_Board.Cols();
        auto origin = -0.5f * SpriteSize * Vector2((float)cols - 1, (float)rows - 1);

        UpdateDespawnTweens(dtSeconds);

        if (m_DespawnTweens.size() == 0 
         && m_DespawnDstIndices.size() > 0)
        {
            // When all despawn animations are completed. 
            // Destroy gems and make gems above them fall.
            DestroyGemsAndCreateFalls();
        }

        UpdateFallTweens(dtSeconds);
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

        auto rows = m_Board.Rows();
        auto cols = m_Board.Cols();

        m_BoardView.BeginRender();
        m_BoardView.RenderBackground(rows, cols, SpriteSize);
        m_BoardView.RenderGems(m_GemPositions.data(), m_GemScales.data(), m_GemColors.data(), m_GemPositions.size());
        m_BoardView.EndRender();

        m_D3D11.EndFrame();
    }

    void OnDestroy() override final { }

    void OnMouseMove(int x, int y) override final {}

    // Internal functions.
private: 
    inline m3::gem_color_t RandomGemColor()
    {
        return m3::GemColors[m_ColorDistribution(m_RandGenerator)];
    }

    inline Vector2 Position(m3::row_t r, m3::col_t c, float spriteSize)
    {
        const auto cr = Vector2((float)BoardCols - 1, (float)BoardRows - 1);
        const auto origin = -0.5f * spriteSize * cr;
        return origin + spriteSize * Vector2((float)c, (float)r);
    }

    inline float GemY(float ny, float spriteSize)
    {
        const auto cr = (float)BoardRows - 1;
        const auto origin = -0.5f * spriteSize * cr;
        return origin + spriteSize * ny;
    }

    void UpdateDespawnTweens(double dtSeconds)
    {
        eastl::vector<size_t> despawnIndicesToRemove = {};

        // Run scale animations and update gem scale.
        for (auto i = 0; i < m_DespawnTweens.size(); i++)
        {
            auto dst = m_DespawnDstIndices[i];
            auto scale = m_DespawnTweens[i].Evaluate() * SpriteSize;

            m_GemScales[dst] = { scale, scale };
            m_DespawnTweens[i].ElapsedMs += dtSeconds * 1000.0;

            if (m_DespawnTweens[i].Completed())
                despawnIndicesToRemove.push_back(i);
        }

        // Erase completed despawn tweens.
        // @Todo. Optimize? This is a small enough array and probably okay.
        {
            auto numErased = 0;
            for (auto i = 0; i < despawnIndicesToRemove.size(); i++)
            {
                auto indexToRemove = despawnIndicesToRemove[i] - numErased;
                m_DespawnTweens.erase(m_DespawnTweens.begin() + indexToRemove);
                auto dstIndex = m_DespawnDstIndices[indexToRemove];
                m_DespawnDstIndices.erase(m_DespawnDstIndices.begin() + indexToRemove);
                m_DespawnDstIndices.push_back(dstIndex);
                numErased++;
            }
        }
    }

    void DestroyGemsAndCreateFalls()
    {
        // Convert indices to ids.
        auto& despawnIds = m_DespawnDstIndices;
        for (auto i = 0; i < m_DespawnDstIndices.size(); i++)
            m_DespawnDstIndices[i] = m_GemIds[m_DespawnDstIndices[i]];

        for (auto i = 0; i < despawnIds.size(); i++)
        {
            auto id = despawnIds[i];
            auto idx = m_IdToIndex[id];
            auto r = m_GemRows[idx];
            auto c = m_GemCols[idx];

            RemoveGemById(id);
            MakeGemsFall(r + 1, r, c);
        }

        m_DespawnDstIndices.clear();

        // Convert ids to indices.
        for (auto i = 0; i < m_FallDstIndices.size(); i++)
            m_FallDstIndices[i] = m_IdToIndex[m_FallDstIndices[i]];
    }

    void UpdateFallTweens(double dtSeconds)
    {
        eastl::vector<size_t> fallIndicesToRemove = {};

        // Run fall tweens and update gem position.
        for (auto i = 0; i < m_FallTweens.size(); i++)
        {
            auto dst = m_FallDstIndices[i];
            auto y = m_FallTweens[i].Evaluate();

            m_GemPositions[dst].y = GemY(y, SpriteSize);
            m_FallTweens[i].ElapsedMs += dtSeconds * 1000.0;

            if (m_FallTweens[i].Completed())
                fallIndicesToRemove.push_back(i);
        }

        // Erase completed despawn tweens.
        // @Todo. Optimize? This is a small enough array and probably okay.
        {
            auto numErased = 0;
            for (auto i = 0; i < fallIndicesToRemove.size(); i++)
            {
                auto indexToRemove = fallIndicesToRemove[i] - numErased;
                m_FallTweens.erase(m_FallTweens.begin() + indexToRemove);
                auto dstIndex = m_FallDstIndices[indexToRemove];
                m_FallDstIndices.erase(m_FallDstIndices.begin() + indexToRemove);
                m_FallDstIndices.push_back(dstIndex);
                numErased++;
            }
        }
    }

    void RemoveGemById(Id id)
    {
        auto index = m_IdToIndex.at(id);
        auto r = m_GemRows[index];
        auto c = m_GemCols[index];

        auto idBeingMoved = m_GemIds[m_GemIds.size() - 1];
        m_IdToIndex[idBeingMoved] = index;

        m_Board(r, c) = m3::InvalidGemId;

        m_IdToIndex.erase(id);
        m_GemIds.erase_unsorted(m_GemIds.begin() + index);
        m_GemRows.erase_unsorted(m_GemRows.begin() + index);
        m_GemCols.erase_unsorted(m_GemCols.begin() + index);
        m_GemColors.erase_unsorted(m_GemColors.begin() + index);

        m_GemPositions.erase_unsorted(m_GemPositions.begin() + index);
        m_GemScales.erase_unsorted(m_GemScales.begin() + index);
    }

    void MakeGemsFall(m3::row_t r0, m3::row_t rDst, m3::col_t c)
    {
        for (auto r = r0; r < m_Board.Rows(); r++)
        {
            auto id = m_Board(r, c);
            auto index = m_IdToIndex[id];

            Tween fall = {};

            fall.Value_0 = r;
            fall.Value_1 = rDst;
            fall.DurationMs = 500;

            m_Board(rDst, c) = m_Board(r, c);
            m_Board(r, c) = m3::InvalidGemId;

            m_GemRows[index] = rDst;

            m_FallDstIndices.push_back(id);
            m_FallTweens.push_back(fall);

            rDst++;
        }
    }

public:
    Match3Game() :
        m_RandGenerator(0),
        m_ColorDistribution(1, sizeof(m3::GemColors) - 1)
    {}
};

#ifndef Test__
Game__(Match3Game)
#endif