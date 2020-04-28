#pragma once

#include "Platform.hpp"

#include <vector>
#include <tuple>

namespace pfk {

    template <class T>
    class FreeList
    {
    private:
        union Node
        {
            size_t Next;
            T Item;
        };

        // @Todo: remove use of std::vector
        std::vector<Node> m_Pool;
        std::vector<bool> m_InUse;
        size_t m_FreeListHead = -1;

    public:
        void Create(size_t startingCapacity)
        {
            m_Pool.resize(startingCapacity);
            m_InUse.resize(startingCapacity);

            for (auto i = 0; i < m_Pool.size() - 1; i++)
            {
                m_Pool[i].Next = i + 1;
                m_InUse[i] = false;
            }
                
            m_Pool[m_Pool.size() - 1].Next = -1;
            m_InUse[m_Pool.size() - 1] = false;
            m_FreeListHead = 0;
        }

        void Destroy() {}

        size_t Acquire()
        {
            // Forgor to call Create?
            assert(m_Pool.size() > 0);

            if (m_FreeListHead == -1)
            {
                auto oldSize = m_Pool.size();
                auto capacity = m_Pool.size() << 1;

                m_Pool.resize(capacity);
                m_InUse.resize(capacity);

                for (auto i = oldSize; i < m_Pool.size() - 1; i++)
                {
                    m_Pool[i].Next = i + 1;
                    m_InUse[i] = false;
                }

                m_Pool[m_Pool.size() - 1].Next = -1;
                m_InUse[m_Pool.size() - 1] = false;
                m_FreeListHead = oldSize;
            }

            auto idx = m_FreeListHead;
            
            m_FreeListHead = m_Pool[idx].Next;
            m_Pool[idx].Item = T();
            m_InUse[idx] = true;

            return idx;
        }

        void Release(size_t index)
        {
            // Attempting to free an index that was never Acquired?
            // Attempting to free an index twice?
            assert(m_InUse[index]);

            m_Pool[index].Next = m_FreeListHead;
            m_InUse[index] = false;
            m_FreeListHead = index;
        }

        inline size_t Count() { return m_Pool.size(); }

        inline bool Get(size_t idx, T* outItem) const 
        { 
            *outItem = m_Pool[idx].Item;
            return m_InUse[idx];
        }

        inline       T& operator[](size_t index)       { return m_Pool[index].Item; }
        inline const T& operator[](size_t index) const { return m_Pool[index].Item; }
    };

}