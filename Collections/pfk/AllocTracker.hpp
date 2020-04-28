#pragma once

#include "FreeList.hpp"

namespace pfk {

    class AllocTracker
    {
    private:
        struct Tracking
        {
            static const size_t k_FileNameLength = 128;

            size_t Line;
            size_t Size;
            size_t Align;
            char File[k_FileNameLength];
        };

        const char* m_Name;
        FreeList<Tracking> m_Tracking;
        size_t m_NumAllocations = 0;

    public:
        inline void Init(const char* name)
        {
            m_Name = name;
            m_Tracking.Create(64);
        }

        inline void Shutdown()
        {
            if (m_NumAllocations > 0)
                ReportUnallocated();

            assert(m_NumAllocations == 0);
            m_Tracking.Destroy();
        }

        inline size_t Track(size_t size, size_t align, const char* file, uint32_t line)
        {
            auto t = Tracking { line, size, align };
            auto len = strlen(file);

            strncpy_s(t.File, file, pfk_min(len, Tracking::k_FileNameLength));

            auto idx = m_Tracking.Acquire();

            m_Tracking[idx] = t;
            m_NumAllocations++;

            return idx;
        }

        inline void Untrack(size_t idx)
        {
            m_Tracking.Release(idx);
            m_NumAllocations--;
        }

        inline size_t NumAllocations() const { return m_NumAllocations; }

    private:
        void ReportUnallocated()
        {
            for (auto i = 0; i < m_Tracking.Count(); i++)
            {
                Tracking tracking;
                if (!m_Tracking.Get(i, &tracking)) continue;
                printf("Allocation not freed: size %zu, align %zu, file %s, line %zu \n", 
                    tracking.Size,
                    tracking.Align,
                    tracking.File,
                    tracking.Line);
            }
        }
    };

}