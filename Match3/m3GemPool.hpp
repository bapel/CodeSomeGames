#pragma once

#include <EASTL\queue.h>

#include "m3Types.hpp"

namespace m3
{
    class GemPool
    {
    private:
        // @Todo: Could be more performant.
        eastl::queue<GemId> m_GemPool;
        GemId m_MaxGemId = 0;

    public:
        GemId GetOrCreateGem()
        {
            if (m_GemPool.size() == 0)
            {
                m_MaxGemId.m_I++;
                return m_MaxGemId;
            }

            auto id = m_GemPool.front();
            m_GemPool.pop();

            return id;
        }

        void ReleaseGem(GemId id)
        {
            m_GemPool.push(id);
        }
    };
}