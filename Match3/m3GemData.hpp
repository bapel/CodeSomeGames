#pragma once

#include <EASTL\vector.h>
#include <EASTL\hash_map.h>

#include "m3Types.hpp"

namespace m3
{
    template <class T>
    class SparseDataArray
    {
    private:
        using Id = gem_id_t;
        using Idx = size_t;

        eastl::hash_map<Id, Idx> m_IdToIndexMap;
        eastl::vector<Id> m_GemIds;
        eastl::vector<T> m_Data;

    public:
        inline size_t DataIndexForGem(Id id) const { return m_IdToIndexMap.at(id); }
        inline gem_id_t GemFromDataIndex(Idx idx) const { return m_GemIds[idx]; }

        inline       eastl::vector<T>& Data()       { return m_Data; }
        inline const eastl::vector<T>& Data() const { return m_Data; }

        inline       eastl::vector<Id>& GemIds()       { return m_GemIds; }
        inline const eastl::vector<Id>& GemIds() const { return m_GemIds; }

        inline void Insert(Id id, const T& data)
        {
            assert(m_IdToIndexMap.find(id) == m_IdToIndexMap.end());

            m_IdToIndexMap.insert(id); // @Todo: Insert data along with key?
            m_IdToIndexMap.at(id) = m_Data.size();
            m_GemIds.push_back(id);
            m_Data.push_back(data);
        }

        inline void Remove(Id id)
        {
            auto iter = m_IdToIndexMap.find(id);
            if (iter == m_IdToIndexMap.end())
                return;

            auto idx = iter->second;
            m_GemIds.erase_unsorted(m_GemIds.begin() + idx);
            m_Data.erase_unsorted(m_Data.begin() + idx);
            m_IdToIndexMap.erase(iter);
            m_IdToIndexMap[m_GemIds[idx]] = idx;
        }
    };

    #include <EASTL\algorithm.h>
    
    struct ScaleAnimation
    {
        double CurrentMs;
        row_t Row;
        col_t Col;
        double DelayMs;
        double DurationMs;
        float From, To;

        inline void Update(double ms) { CurrentMs += ms; }
        inline bool Completed() const { return (CurrentMs - DelayMs) >= DurationMs; }

        inline float Current() const
        {
            auto t = eastl::max(CurrentMs - DelayMs, 0.0);
            t /= DurationMs;
            t = eastl::clamp(t, 0.0, 1.0);
            return From + t * ((double)To - From);
        }
    };

    struct FallAnimation
    { 
        Vector2 Current;
    };

    enum class AnimationType
    {
        None = 0,
        Scale,
        Fall
    };

    class GemData
    {
    private:
        using Id = gem_id_t;
        using Idx = size_t;

        eastl::hash_map<Id, Idx> m_IdToIndexMap;
        eastl::vector<gem_id_t> m_GemIds;
        eastl::vector<row_t> m_GemRows;
        eastl::vector<col_t> m_GemCols;
        eastl::vector<Vector2> m_Position;
        eastl::vector<float> m_Scale;
        eastl::vector<gem_color_t> m_GemColors;

        SparseDataArray<ScaleAnimation> m_ScaleAnimations;
        SparseDataArray<FallAnimation> m_FallAnimations;

    public:
        inline size_t Count() const { return m_GemIds.size(); }

        inline size_t DataIndexForGem(Id id) const { return m_IdToIndexMap.at(id); }

        inline gem_id_t GetGemId(size_t idx) const { return m_GemIds[idx]; }
        inline row_t GetRow(size_t idx) const { return m_GemRows[idx]; }
        inline col_t GetCol(size_t idx) const { return m_GemCols[idx]; }
        inline Vector2 GetPosition(size_t idx) const { return m_Position[idx]; }
        inline float GetScale(size_t idx) const { return m_Scale[idx]; }
        inline gem_color_t GetColor(size_t idx) const { return m_GemColors[idx]; }

        inline       SparseDataArray<ScaleAnimation>& GetScaleAnimations()       { return m_ScaleAnimations; }
        inline const SparseDataArray<ScaleAnimation>& GetScaleAnimations() const { return m_ScaleAnimations; }

        inline const ScaleAnimation& GetScaleAnimation(size_t idx) const 
        {
            idx = m_ScaleAnimations.DataIndexForGem(m_GemIds[idx]);
            return m_ScaleAnimations.Data()[idx]; 
        }

        inline ScaleAnimation& GetScaleAnimation(size_t idx)
        {
            idx = m_ScaleAnimations.DataIndexForGem(m_GemIds[idx]);
            return m_ScaleAnimations.Data()[idx]; 
        }

        inline const FallAnimation& GetFallAnimation(size_t idx) const 
        { 
            idx = m_FallAnimations.DataIndexForGem(m_GemIds[idx]);
            return m_FallAnimations.Data()[idx]; 
        }

        inline FallAnimation& GetFallAnimation(size_t idx)
        { 
            idx = m_FallAnimations.DataIndexForGem(m_GemIds[idx]);
            return m_FallAnimations.Data()[idx]; 
        }

        inline void SetPosition(size_t idx, Vector2 position) { m_Position[idx] = position; }
        inline void SetScale(size_t idx, float scale) { m_Scale[idx] = scale; }

        size_t Add(gem_id_t id, row_t r, col_t c, gem_color_t color)
        {
            m_IdToIndexMap.insert(id);
            m_IdToIndexMap.at(id) = m_GemIds.size();

            m_GemIds.push_back(id);
            m_GemRows.push_back(r);
            m_GemCols.push_back(c);
            m_Position.push_back(Vector2());
            m_Scale.push_back(0.0f);
            m_GemColors.push_back(color);

            return m_GemIds.size() - 1;
        }

        void AddAnimation(size_t idx, ScaleAnimation sa)
        {
            auto id = m_GemIds[idx];
            m_GemRows[idx] = -1;
            m_GemCols[idx] = (col_t)AnimationType::Scale;
            m_ScaleAnimations.Insert(id, sa);
        }

        void RemoveById(m3::gem_id_t id)
        {
            auto iter = m_IdToIndexMap.find(id);
            if (iter == m_IdToIndexMap.end())
                return;

            auto idx = iter->second;

            m_GemIds.erase_unsorted(m_GemIds.begin() + idx);
            m_GemRows.erase_unsorted(m_GemRows.begin() + idx);
            m_GemCols.erase_unsorted(m_GemCols.begin() + idx);
            m_Position.erase_unsorted(m_Position.begin() + idx);
            m_Scale.erase_unsorted(m_Scale.begin() + idx);
            m_GemColors.erase_unsorted(m_GemColors.begin() + idx);

            m_IdToIndexMap.erase(iter);
            m_IdToIndexMap[m_GemIds[idx]] = idx;

            m_ScaleAnimations.Remove(id);
            m_FallAnimations.Remove(id);
        }
    };
}