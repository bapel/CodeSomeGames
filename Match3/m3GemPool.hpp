#pragma once

#include <EASTL\queue.h>

#include "m3Types.hpp"

namespace m3
{
    class GemPool
    {
    private:
        eastl::queue<gem_id_t> m_GemPool;
        gem_id_t m_MaxGemId = 0;

    public:
        gem_id_t GetOrCreateGem()
        {
            if (m_GemPool.size() == 0)
                return m_MaxGemId++;

            auto id = m_GemPool.front();
            m_GemPool.pop();

            return id;
        }

        void ReleaseGem(gem_id_t id)
        {
            m_GemPool.push(id);
        }
    };
}

#include <EASTL\vector.h>
#include <EASTL\hash_map.h>

namespace m3
{
    template <int R, int C, class ... Ts>
    class GemPool1
    {
    private:
        using Id = m3::gem_id_t;
        using Index = size_t;
        struct IdIndex { Id Id; Index Index; };
        using Board = m3::Board<Id, R, C>;

        eastl::vector<Id> m_GemIds;
        size_t m_NumAliveGems;

        Board m_Board;
        eastl::hash_map<Id, Index> m_IdToIndex;
        eastl::tuple<eastl::vector<Ts> ...> m_DataArrays;

    public:
        template <class T>
        inline eastl::vector<T>& DataArray()
        { return eastl::get<eastl::vector<T>, eastl::vector<Ts> ...>(m_DataArrays); }

        template <class T>
        inline const eastl::vector<T>& DataArray() const
        { return eastl::get<eastl::vector<T>, eastl::vector<Ts> ...>(m_DataArrays); }

        IdIndex CreateGem(row_t r, col_t c, gem_color_t color)
        {
            auto id = m_GemIds.size();
            auto index = id;
            
            m_GemIds.push_back(id);
            m_NumAliveGems++;

            m_Board(r, c) = id;
            m_IdToIndex.insert(id);
            m_IdToIndex[id] = index;

            (DataArray<Ts>().push_back({}), ...);
        }

        void ReleaseGem(Id id)
        {
            auto index = m_IdToIndex[id];
            auto r = DataArray<row_t>()[index];
            auto c = DataArray<col_t>()[index];

            eastl::swap(m_GemIds[index], m_GemIds[m_GemIds.size() - 1]);
            m_NumAliveGems--;

            m_Board(r, c).Id = InvalidGemId;
            m_IdToIndex.erase(id);

            (DataArray<Ts>().erase_unordered(DataArray<Ts>().begin() + index), ...);
        }
    };
}