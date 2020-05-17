#pragma once

#include <EASTL\tuple.h>

namespace ecs1
{
    template <class ... Ts>
    using Tuple = eastl::tuple<Ts ...>;

    template <class T, class Tuple>
    struct TupleIndex;

    template <class T, class ... Types>
    struct TupleIndex<T, Tuple<T, Types ...>> {
        static const size_t Index = 0;
    };

    template <class T, class U, class ... Types>
    struct TupleIndex<T, Tuple<U, Types ...>> {
        static const size_t Index = 1 + TupleIndex<T, Tuple<Types...>>::Index;
    };

    template <class Id, class Tuple>
    struct TypeIdsBase
    {
        template <class T>
        static constexpr Id TypeId() 
        { return ecs1::TupleIndex<T, Tuple>::Index; }
    };

    template <class Id, class ... Ts>
    struct TypeIds : TypeIdsBase<Id, eastl::tuple<Ts ...>>
    {
        using Tuple = eastl::tuple<Ts ...>;

        template <class ... OtherTs>
        using Extend = TypeIds<Id, Ts ..., OtherTs ...>;
    };
}