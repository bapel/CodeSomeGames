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
#include <EASTL\hash_set.h>

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
    std::mt19937 m_RandGenerator;
    std::uniform_int_distribution<uint16_t> m_ColorDistribution;
    m3::GemPool m_GemPool;

    // Board data.
    using Board = m3::Board<m3::GemId, BoardRows, BoardCols>;

    Board m_Board;
    eastl::hash_map<m3::GemId, uint32_t> m_IdToIndex;
    eastl::vector<m3::GemId> m_GemIds;
    eastl::vector<m3::Row> m_GemRows;
    eastl::vector<m3::Col> m_GemCols;
    eastl::vector<m3::GemColor> m_GemColors;

    // Computed stuff.
    eastl::vector<Vector2> m_GemPositions;
    eastl::vector<Vector2> m_GemScales;

    // Tweens.
    eastl::vector<uint32_t> m_DespawnDstIndices;
    eastl::vector<Tween> m_DespawnTweens;
    eastl::vector<uint32_t> m_FallDstIndices;
    eastl::vector<Tween> m_FallTweens;

    // Temp buffers.
    eastl::vector<uint32_t> m_DespawnIndicesToRemove;
    eastl::vector<uint32_t> m_FallIndicesToRemove;

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
        m3::Row r1, r2;

        auto r = r1 <= r2;

        m_CameraConstantsBuffer = m_D3D11.CreateConstantsBuffer<CameraConstantsBuffer>();
        m_D3D11.SetDebugName(m_CameraConstantsBuffer.Get(), "CameraConstantsBuffer");

        m_BoardView.Init(m_D3D11, m_ShadersPath);

        // Create and place random colored gems.
        for (auto i = 0U; i < m_Board.Count(); i++)
        {
            m3::Row r = i / m_Board.Cols().m_I;
            m3::Col c = i % m_Board.Cols().m_I;
            auto id = m_GemPool.GetOrCreateGem();
            auto index = (uint32_t)m_GemRows.size();

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

        FindAndClearFromWholeBoard();

         // @Todo: Remove test. 
         // Despawn a few gems.
        //DespawnGem(3, 2);
        //DespawnGem(3, 3);
        //DespawnGem(3, 4);
        //DespawnGem(2, 1);
        //DespawnGem(2, 2);
        //DespawnGem(2, 3);
        //DespawnGem(1, 2);
        //DespawnGem(2, 2);
        //DespawnGem(3, 2);
    }

    m3::GemColor GetColor(m3::Row r, m3::Col c)
    {
        auto id = m_Board(r, c);
        if (id == m3::InvalidGemId)
            return m3::InvalidColor;

        auto index = m_IdToIndex.at(id);
        return m_GemColors[index];
    }

    // @Todo: float instead of double is okay?
    void OnUpdate(double dtSeconds) override final
    {
        dtSeconds = eastl::clamp(dtSeconds, 0.0, 1.0 / 60.0);
        UpdateDespawnTweens((float)dtSeconds);
        UpdateFallTweens((float)dtSeconds);

        // If no tweens are in progress, we look for more matches on the whole board.
        if (m_DespawnTweens.size() == 0 &&
            m_DespawnDstIndices.size() == 0 &&
            m_FallTweens.size() == 0 && 
            m_FallDstIndices.size() == 0)
        {
            FindAndClearFromWholeBoard();
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

        auto rows = m_Board.Rows();
        auto cols = m_Board.Cols();

        m_BoardView.BeginRender();
        m_BoardView.RenderBackground(rows.m_I, cols.m_I, SpriteSize);
        m_BoardView.RenderGems(m_GemPositions, m_GemScales, m_GemColors);
        m_BoardView.EndRender();

        m_D3D11.EndFrame();
    }

    void OnDestroy() override final { }

    void OnMouseMove(int x, int y) override final {}

    // Internal functions.
private: 
    inline m3::GemColor RandomGemColor()
    {
        return m3::GemColors[m_ColorDistribution(m_RandGenerator)];
    }

    inline Vector2 Position(m3::Row r, m3::Col c, float spriteSize)
    {
        const auto cr = Vector2((float)BoardCols - 1, (float)BoardRows - 1);
        const auto origin = -0.5f * spriteSize * cr;
        return origin + spriteSize * Vector2((float)c.m_I, (float)r.m_I);
    }

    inline float GemY(float ny, float spriteSize)
    {
        const auto cr = (float)BoardRows - 1;
        const auto origin = -0.5f * spriteSize * cr;
        return origin + spriteSize * ny;
    }

    void FindAndClearFromWholeBoard()
    {
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
        eastl::hash_set<m3::GemId> idsToRemove;

        for (auto i = 0; i < m_Board.Count(); i++)
        {
            m3::Row r = i / m_Board.Cols().m_I;
            m3::Col c = i % m_Board.Cols().m_I;

            auto colors = [this](m3::Row r, m3::Col c) { return this->GetColor(r, c); };
            auto color = colors(r, c);
            if (color == m3::InvalidColor)
                continue;

            auto rl = m3::RowSpan { r, m3::GetMatchingColsInRow_L(r, c, color, colors), c };
            auto rr = m3::RowSpan { r, c, m3::GetMatchingColsInRow_R(r, c, color, colors, m_Board.Cols() - 1) };
            auto cu = m3::ColSpan { c, r, m3::GetMatchingRowsInCol_U(r, c, color, colors, m_Board.Rows() - 1) };
            auto cd = m3::ColSpan { c, m3::GetMatchingRowsInCol_D(r, c, color, colors), r };

            const auto n = 3;

            if (rl.Count() >= n)
            {
                for (auto i = 0; i < rl.Count(); i++)
                {
                    auto r = rl.Row();
                    auto c = rl[i];
                    auto id = m_Board(r, c);
                    idsToRemove.insert(id);
                }
            }

            if (rr.Count() >= n)
            {
                for (auto i = 0; i < rr.Count(); i++)
                {
                    auto r = rr.Row();
                    auto c = rr[i];
                    auto id = m_Board(r, c);
                    idsToRemove.insert(id);
                }
            }

            if (cu.Count() >= n)
            {
                for (auto i = 0; i < cu.Count(); i++)
                {
                    auto r = cu[i];
                    auto c = cu.Col();
                    auto id = m_Board(r, c);
                    idsToRemove.insert(id);
                }
            }

            if (cd.Count() >= n)
            {
                for (auto i = 0; i < cd.Count(); i++)
                {
                    auto r = cd[i];
                    auto c = cu.Col();
                    auto id = m_Board(r, c);
                    idsToRemove.insert(id);
                }
            }
        }

        if (idsToRemove.size() > 0)
        {
            for (auto iter = idsToRemove.begin(); iter != idsToRemove.end(); iter++)
            {
                auto id = *iter;
                auto idx = m_IdToIndex[id];
                auto r = m_GemRows[idx];
                auto c = m_GemCols[idx];

                DespawnGem(r, c);
            }
        }
    }

    void DespawnGem(m3::Row r, m3::Col c)
    {
        auto id = m_Board(r, c);
        auto idx = m_IdToIndex[id];
        
        Tween despawnTween = {};

        despawnTween.Value_0 = 1.0f;
        despawnTween.Value_1 = 0.0f;
        despawnTween.DelayMs = 200;
        despawnTween.DurationMs = 200;

        m_DespawnDstIndices.push_back(idx);
        m_DespawnTweens.push_back(despawnTween);
    }

    void UpdateDespawnTweens(float dtSeconds)
    {
        m_DespawnIndicesToRemove.clear();

        // Run scale animations and update gem scale.
        for (auto i = 0; i < m_DespawnTweens.size(); i++)
        {
            auto dst = m_DespawnDstIndices[i];
            auto scale = m_DespawnTweens[i].Evaluate() * SpriteSize;

            m_GemScales[dst] = { scale, scale };
            m_DespawnTweens[i].ElapsedMs += dtSeconds * 1000.0f;

            if (m_DespawnTweens[i].Completed())
                m_DespawnIndicesToRemove.push_back(i);
        }

        // Erase completed despawn tweens.
        // @Todo. Optimize? This is a small enough array and probably okay.
        {
            auto numErased = 0;
            for (auto i = 0; i < m_DespawnIndicesToRemove.size(); i++)
            {
                auto indexToRemove = m_DespawnIndicesToRemove[i] - numErased;
                m_DespawnTweens.erase(m_DespawnTweens.begin() + indexToRemove);
                auto dstIndex = m_DespawnDstIndices[indexToRemove];
                m_DespawnDstIndices.erase(m_DespawnDstIndices.begin() + indexToRemove);
                m_DespawnDstIndices.push_back(dstIndex);
                numErased++;
            }
        }

        if (m_DespawnTweens.size() == 0 && 
            m_DespawnDstIndices.size() > 0)
        {
            // These are just for readability, and we want to avoid copying.
            auto& despawnIds = m_DespawnDstIndices;

            // Convert indices to ids.
            for (auto i = 0; i < despawnIds.size(); i++)
                despawnIds[i] = m_GemIds[m_DespawnDstIndices[i]].m_I;

            // Destroy despawned gems.
            // Also create a RowSpan above which gems will fall.
            m3::RowSpan fallSpan;
            {
                auto id = despawnIds[0];
                auto idx = m_IdToIndex[id];
                auto r = m_GemRows[idx];
                auto c = m_GemCols[idx];
                fallSpan = { r, c, c };

                RemoveGemById(id);

                for (auto i = 1; i < despawnIds.size(); i++)
                {
                    id = despawnIds[i];
                    idx = m_IdToIndex[id];
                    r = m_GemRows[idx];
                    c = m_GemCols[idx];

                    fallSpan = 
                    {
                        eastl::min(fallSpan.Row(), r),
                        eastl::min(fallSpan.Col_0(), c),
                        eastl::max(fallSpan.Col_1(), c)
                    };

                    RemoveGemById(id);
                }
            }

            for (auto i = 0; i < fallSpan.Count(); i++)
            {
                auto r = fallSpan.Row();
                auto c = fallSpan[i];
                MakeGemsFall(r, c);
            }

            m_DespawnDstIndices.clear();
        }
    }

    void UpdateFallTweens(float dtSeconds)
    {
        m_FallIndicesToRemove.clear();

        // Run fall tweens and update gem position.
        for (auto i = 0; i < m_FallTweens.size(); i++)
        {
            auto dst = m_FallDstIndices[i];
            auto y = m_FallTweens[i].Evaluate();

            m_GemPositions[dst].y = GemY(y, SpriteSize);
            m_FallTweens[i].ElapsedMs += dtSeconds * 1000.0f;

            if (m_FallTweens[i].Completed())
                m_FallIndicesToRemove.push_back(i);
        }

        // Erase completed despawn tweens.
        // @Todo. Optimize? This is a small enough array and probably okay.
        {
            auto numErased = 0;
            for (auto i = 0; i < m_FallIndicesToRemove.size(); i++)
            {
                auto indexToRemove = m_FallIndicesToRemove[i] - numErased;
                m_FallTweens.erase(m_FallTweens.begin() + indexToRemove);
                auto dstIndex = m_FallDstIndices[indexToRemove];
                m_FallDstIndices.erase(m_FallDstIndices.begin() + indexToRemove);
                m_FallDstIndices.push_back(dstIndex);
                numErased++;
            }
        }

        if (m_FallTweens.size() == 0 && 
            m_FallDstIndices.size() > 0)
        {
            m_FallDstIndices.clear();
        }
    }

    void RemoveGemById(m3::GemId id)
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

    void MakeGemsFall(m3::Row r, m3::Col c)
    {
        auto dr = 0;
        for (; r < m_Board.Rows(); r = r + 1)
        {
            auto id = m_Board(r, c);
            
            if (id == m3::InvalidGemId)
                dr++;
            else if (dr > 0)
            {
                auto index = m_IdToIndex[id];

                m_Board(r - dr, c) = m_Board(r, c);
                m_Board(r, c) = m3::InvalidGemId;
                m_GemRows[index] = r - dr;

                Tween fall = {};

                fall.Value_0 = r.m_I;
                fall.Value_1 = (r - dr).m_I;
                fall.DurationMs = dr * 500;

                m_FallDstIndices.push_back(index);
                m_FallTweens.push_back(fall);
            }
        }
    }

public:
    Match3Game() :
        m_Board(),
        m_RandGenerator(0),
        m_ColorDistribution(1, sizeof(m3::GemColors) - 1)
    {}
};

#ifndef Test__
Game__(Match3Game)
#endif