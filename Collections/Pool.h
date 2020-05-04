#pragma once

#include <stdint.h>

namespace pfk {

    template <class T, class Size_t = uint32_t>
    class Pool
    {
    private:
        T* m_Pool;
        Size_t m_Capacity;
        Size_t* m_Head;
    };

}