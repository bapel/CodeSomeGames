#include <assert.h>
#include <VectorMath.hpp>

#include <EASTL\vector_multimap.h>

#include "ECS\World.hpp"

void __RunExperiments__();

void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

void* __cdecl operator new[](size_t size, size_t align, size_t offset, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

#include <EASTL\bitvector.h>

struct TransformComponent
{
    Vector3 Position;
    Quaternion Rotation;
    Vector3 Scale;
};



struct MovementComponent
{
    Vector3 Velocity;
    Vector3 Acceleration;
};

struct BounceComponent
{ }; 

struct WrapAroundComponent
{ };

struct RenderComponent {};

ecs_SetComponentTypeId(0, TransformComponent);
ecs_SetComponentTypeId(1, MovementComponent);
ecs_SetComponentTypeId(2, BounceComponent);
ecs_SetComponentTypeId(3, WrapAroundComponent);

class MovementSystem : public ecs::BaseComponentSystem
{
public:
    void Update(ecs::World* world, float deltaTime) override final
    {
        // const auto& entities = world->GetEntitiesWithComponents<TransformComponent, MovementComponent>();
        
    }
};

//template <class ComponentType>
//class DenseComponentDataArray : BaseComponentDataArray
//{
//    
//};

int main()
{
    __RunExperiments__();

    constexpr auto bs = sizeof(BounceComponent);

    ecs::World world;

    world.InitComponentCollection<TransformComponent>();
    world.InitComponentCollection<MovementComponent>();
    world.InitComponentCollection<BounceComponent>();
    world.InitComponentCollection<WrapAroundComponent>();

    auto b_01 = world.CreateEntity("bouncer_01");

    world.AddComponentToEntity<TransformComponent>(b_01);
    world.AddComponentToEntity<MovementComponent>(b_01);
    world.AddComponentToEntity<BounceComponent>(b_01);

    assert(world.EntityHasComponent<TransformComponent>(b_01));
    assert(world.EntityHasComponent<MovementComponent>(b_01));
    assert(world.EntityHasComponent<BounceComponent>(b_01));

    world.RemoveComponentFromEntity<MovementComponent>(b_01);

    assert(world.EntityHasComponent<TransformComponent>(b_01));
    assert(false == world.EntityHasComponent<MovementComponent>(b_01));
    assert(world.EntityHasComponent<BounceComponent>(b_01));

    world.RemoveComponentFromEntity<BounceComponent>(b_01);

    assert(world.EntityHasComponent<TransformComponent>(b_01));
    assert(false == world.EntityHasComponent<MovementComponent>(b_01));
    assert(false == world.EntityHasComponent<BounceComponent>(b_01));

    world.DestroyEntity(b_01);

    return 0;
}