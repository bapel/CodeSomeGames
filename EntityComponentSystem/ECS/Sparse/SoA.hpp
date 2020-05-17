#pragma once

#include <EASTL\tuple.h>
#include <EASTL\vector.h>

namespace ecs1
{
    template <class ... Types>
    class SoA
    {
    private:
        using Tuple = eastl::tuple<eastl::vector<Types> ...>;

        Tuple m_DataArrays;

    public:
        template <class Type>
        inline eastl::vector<Type>& DataArray() 
        { return eastl::get<eastl::vector<Type>, eastl::vector<Types> ...>(m_DataArrays); }

        template <class Type>
        inline const eastl::vector<Type>& DataArray() const 
        { return eastl::get<eastl::vector<Type>, eastl::vector<Types> ...>(m_DataArrays); }
    };
}