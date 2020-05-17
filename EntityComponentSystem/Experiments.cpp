// #include <tuple>
#include "ECS\Archetype\TypeIds.hpp"
#include "ECS\Archetype\SoA.hpp"

#include <stdint.h>
#include <EASTL\vector.h>

namespace ecs1
{
    using ComponentTypeId = uint32_t;

    template <class ... ComponentTypes>
    using ComponentTypeIds = TypeIds<ComponentTypeId, ComponentTypes ...>;

    using EntityId = uint64_t;

    template <class ... ComponentTypes>
    class ComponentGroup
    {
    private:
        SoA<ComponentTypes ...> m_SoA;
        eastl::vector<EntityId> m_Entities;

    public:
        template <class Type>
        inline eastl::vector<Type>& ComponentDataArray() 
        { return m_SoA.DataArray<Type>(); }

        template <class Type>
        inline const eastl::vector<Type>& ComponentDataArray() const 
        { return m_SoA.DataArray<Type>(); }
    };

    using ComponentGroupTypeId = uint16_t;

    template <class ... ComponentGroupTypes>
    using ComponentGroupTypeIds = TypeIds<ComponentGroupTypeId, ComponentGroupTypes ...>;
}

#pragma region Engine Components

struct TransformComponent
{
    struct { float x, y, z; } Position;
    struct { float a, b, c, d; } Rotation;
    struct { float x, y, z; } Scale;
};

struct RenderComponent {};

#pragma endregion
#pragma region Game/Demo Components

struct MovementComponent
{
    struct { float x, y, z; } Velocity;
    struct { float x, y, z; } Acceleration;
};

struct BounceComponent
{ }; 

struct WrapAroundComponent
{ };

#pragma endregion

using EngineComponentTypeIds = ecs1::ComponentTypeIds<
    TransformComponent,
    RenderComponent>;

using GameComponentTypeIds = EngineComponentTypeIds::Extend<
    MovementComponent,
    BounceComponent,
    WrapAroundComponent>;

constexpr auto tcId = GameComponentTypeIds::TypeId<TransformComponent>();
constexpr auto rcId = GameComponentTypeIds::TypeId<RenderComponent>();
constexpr auto mcId = GameComponentTypeIds::TypeId<MovementComponent>();
constexpr auto bcId = GameComponentTypeIds::TypeId<BounceComponent>();
constexpr auto wcId = GameComponentTypeIds::TypeId<WrapAroundComponent>();

using BounceGroup = ecs1::ComponentGroup<
    TransformComponent, 
    RenderComponent, 
    MovementComponent, 
    BounceComponent>;

using WrapAroundGroup = ecs1::ComponentGroup<
    TransformComponent, 
    RenderComponent, 
    MovementComponent, 
    WrapAroundComponent>;

using MyComponentGroupTypeIds = ecs1::ComponentGroupTypeIds<
    BounceGroup, 
    WrapAroundGroup>;

constexpr auto bgId = MyComponentGroupTypeIds::TypeId<BounceGroup>();
constexpr auto wgId = MyComponentGroupTypeIds::TypeId<WrapAroundGroup>();

// constexpr auto _id = MyComponentGroupTypeIds::TypeId<int>();

void __RunExperiments__()
{
    BounceGroup bounceGroup;

    auto& tcs_1 = bounceGroup.ComponentDataArray<TransformComponent>();
    auto& rcs_1 = bounceGroup.ComponentDataArray<RenderComponent>();
    auto& mcs_1 = bounceGroup.ComponentDataArray<MovementComponent>();
    auto& bcs_1 = bounceGroup.ComponentDataArray<BounceComponent>();

    WrapAroundGroup wrapAroundGroup;

    auto& tcs_2 = wrapAroundGroup.ComponentDataArray<TransformComponent>();
    auto& rcs_2 = wrapAroundGroup.ComponentDataArray<RenderComponent>();
    auto& mcs_2 = wrapAroundGroup.ComponentDataArray<MovementComponent>();
    auto& bcs_2 = wrapAroundGroup.ComponentDataArray<BounceComponent>();
}