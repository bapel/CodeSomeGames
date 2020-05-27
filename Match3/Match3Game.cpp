#define NOMINMAX

#include <SDLGame.hpp>

#include "m3Board.hpp"
#include "m3GemPool.hpp"
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

static const auto BoardRows = 64;
static const auto BoardCols = 64;
static const auto SpriteSize = 16.0f;

struct CameraConstantsBuffer
{
    Matrix Model;
    Matrix ViewProj;
};

enum EaseType
{
    InBack,
    OutBounce
};

// Order of members is to make it convenient to debug.
inline float EaseInBack(float x) 
{
    const auto c1 = 1.70158f;
    const auto c3 = c1 + 1;
    return c3 * x * x * x - c1 * x * x;
}

inline float EaseOutBounce(float x) 
{
    const auto n1 = 7.5625f;
    const auto d1 = 2.75f;

    if (x < 1 / d1) {
        return n1 * x * x;
    } else if (x < 2.0f / d1) {
        return n1 * (x -= 1.5f / d1) * x + 0.75f;
    } else if (x < 2.5f / d1) {
        return n1 * (x -= 2.25f / d1) * x + 0.9375f;
    } else {
        return n1 * (x -= 2.625f / d1) * x + 0.984375f;
    }
}

struct Tween
{
    float ElapsedMs;
    float Value_0;
    float Value_1;
    float DelayMs;
    float DurationMs;
    EaseType EaseType = InBack;

    inline bool Completed() const { return (ElapsedMs - DelayMs) >= DurationMs; }

    inline float Evaluate()
    {
        auto t = ElapsedMs - DelayMs;
        t = eastl::max(t, 0.0f) / DurationMs;
        t = eastl::clamp(t, 0.0f, 1.0f);

        switch (EaseType)
        {
            case InBack: t = EaseInBack(t); break;
            case OutBounce: t = EaseOutBounce(t); break;
        }

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
    // @Todo: Everything might benefit from this being column-major
    // We hardly iterate over a row, except when matching. But we iterate over columns a lot, 
    // like when generating falls. So having the column be in a cache-line would be great.
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
    // Double-buffered to eliminate cost of erase for completed tweens.
    // We simply move incomplete tweens to the other vector and clear-swap.
    // @Todo: Begging to be abstracted to it's own class.
    eastl::vector<m3::GemId> m_DespawnGemIds;
    eastl::vector<uint32_t> m_DespawnDstIndices;
    eastl::vector<uint32_t> m_DespawnDstIndices_1;
    eastl::vector<Tween> m_DespawnTweens;
    eastl::vector<Tween> m_DespawnTweens_1;

    eastl::vector<m3::GemId> m_FallGemIds;
    eastl::vector<uint32_t> m_FallDstIndices;
    eastl::vector<uint32_t> m_FallDstIndices_1;
    eastl::vector<Tween> m_FallTweens;
    eastl::vector<Tween> m_FallTweens_1;

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
        assert((BoardRows * BoardCols) < m3::InvalidGemId.Int());

        m_CameraConstantsBuffer = m_D3D11.CreateConstantsBuffer<CameraConstantsBuffer>();
        m_D3D11.SetDebugName(m_CameraConstantsBuffer.Get(), "CameraConstantsBuffer");

        auto rows = m_Board.Rows();
        auto cols = m_Board.Cols();

        m_BoardView.Init(m_D3D11, m_ShadersPath);
        m_BoardView.InitBackgroundBatch(rows.m_I, cols.m_I, SpriteSize);

        m_IdToIndex.reserve(m_Board.Count());

        // Create and place random colored gems.
        for (auto i = 0U; i < m_Board.Count(); i++)
        {
            m3::Row r = i / cols.m_I;
            m3::Col c = i % cols.m_I;
            auto id = m_GemPool.GetOrCreateGem();
            auto index = (uint32_t)m_GemRows.size();
            auto color = RandomGemColor();

            m_Board(r, c) = id;
            m_IdToIndex.insert(id);
            m_IdToIndex[id] = index;
            m_GemIds.emplace_back(id);
            m_GemRows.emplace_back(r);
            m_GemCols.emplace_back(c);
            m_GemColors.emplace_back(color);
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
        m_BoardView.RenderBackground();
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

                // @Todo: Compact to front?
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

        m_DespawnGemIds.emplace_back(id);
        m_DespawnDstIndices.emplace_back(idx);
        m_DespawnTweens.emplace_back(despawnTween);
    }

    void UpdateDespawnTweens(float dtSeconds)
    {
        // Run scale animations and update gem scale.
        for (auto i = 0; i < m_DespawnTweens.size(); i++)
        {
            auto dst = m_DespawnDstIndices[i];
            auto& tween = m_DespawnTweens[i];

            auto scale = tween.Evaluate() * SpriteSize;
            m_GemScales[dst] = { scale, scale };
            m_DespawnTweens[i].ElapsedMs += dtSeconds * 1000.0f;

            if (!m_DespawnTweens[i].Completed())
            {
                m_DespawnDstIndices_1.emplace_back(dst);
                m_DespawnTweens_1.emplace_back(tween);
            }
        }

        m_DespawnDstIndices.clear();
        m_DespawnTweens.clear();

        eastl::swap(m_DespawnDstIndices, m_DespawnDstIndices_1);
        eastl::swap(m_DespawnTweens, m_DespawnTweens_1);

        if (m_DespawnGemIds.size() > 0 && m_DespawnTweens.size() == 0)
        {
            // These are just for readability, and we want to avoid copying.
            auto& despawnIds = m_DespawnGemIds;

            // Destroy despawned gems.
            // Also create a RowSpan above which gems are going to fall.
            auto id = despawnIds[0];
            auto idx = m_IdToIndex[id];
            auto r = m_GemRows[idx];
            auto c = m_GemCols[idx];
            m3::RowSpan fallSpan = { r, c, c };

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

            for (auto i = 0; i < fallSpan.Count(); i++)
            {
                auto r = fallSpan.Row();
                auto c = fallSpan[i];
                MakeGemsFall(r, c);
            }

            m_DespawnGemIds.clear();
        }
    }

    void UpdateFallTweens(float dtSeconds)
    {
        // Run fall tweens and update gem position.
        for (auto i = 0; i < m_FallTweens.size(); i++)
        {
            auto dst = m_FallDstIndices[i];
            auto y = m_FallTweens[i].Evaluate();

            m_GemPositions[dst].y = GemY(y, SpriteSize);
            m_FallTweens[i].ElapsedMs += dtSeconds * 1000.0f;

            if (!m_FallTweens[i].Completed())
            {
                // If the tween isn't completed tranfer it to the other vector.
                m_FallTweens_1.emplace_back(m_FallTweens[i]);
                m_FallDstIndices_1.emplace_back(m_FallDstIndices[i]);
            }
        }

        m_FallDstIndices.clear();
        m_FallTweens.clear();

        eastl::swap(m_FallDstIndices, m_FallDstIndices_1);
        eastl::swap(m_FallTweens, m_FallTweens_1);

        if (m_FallGemIds.size() > 0 && m_FallTweens.size() == 0)
        {
            m_FallDstIndices.clear();
            m_FallDstIndices_1.clear();
            m_FallTweens.clear();
            m_FallTweens_1.clear();

            // @Todo: Ideally we should only be checking in the neighborhood of gems that fell into place.
            FindAndClearFromWholeBoard();

            // @Todo: We should only use these ids to check for matches. 
            // Right now this just helps signal completion of all fall tweens.
            m_FallGemIds.clear();
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
                fall.DurationMs = dr * 100;
                fall.EaseType = OutBounce;

                m_FallGemIds.emplace_back(id);
                m_FallDstIndices.emplace_back(index);
                m_FallTweens.emplace_back(fall);
            }
        }
    }

public:
    Match3Game() :
        m_Board(),
        m_RandGenerator(0),
        m_ColorDistribution(1, sizeof(m3::GemColors) - 1)
    {
        auto despawnReserve = m_Board.Count() / 4;
        m_DespawnGemIds.reserve(despawnReserve);
        m_DespawnDstIndices.reserve(despawnReserve);
        m_DespawnDstIndices_1.reserve(despawnReserve);
        m_DespawnTweens.reserve(despawnReserve);
        m_DespawnTweens_1.reserve(despawnReserve);

        auto fallReserve = m_Board.Count() / 2;
        m_FallGemIds.reserve(fallReserve);
        m_FallDstIndices.reserve(fallReserve);
        m_FallDstIndices_1.reserve(fallReserve);
        m_FallTweens.reserve(fallReserve);
        m_FallTweens_1.reserve(fallReserve);
    }
};

#ifndef Test__
Game__(Match3Game)
#endif