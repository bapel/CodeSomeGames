#pragma once

namespace ecs
{
    class World;

    class BaseComponentSystem
    {
    public:
        ~BaseComponentSystem() = default;
        virtual void Update(World* world, float dt) = 0;
    };
}