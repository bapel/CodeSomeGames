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