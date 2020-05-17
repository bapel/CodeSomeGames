#include <stdint.h>
#include <eastl\vector.h>
#include <eastl\unique_ptr.h>
#include <eastl\hash_map.h>

namespace ECS
{
    using EntityId = uint32_t;
    using ComponentTypeId = uint32_t;
    using ComponentId = uint32_t;

    class ISystem
    {
    public:
        virtual void Update(float dt) = 0;
    };
    
    class World
    {
        struct Entity
        {
            char Name[64];
            eastl::hash_map<ComponentTypeId, ComponentId> Components;
        };

        using Ptr_IComponentSystem = eastl::unique_ptr<ISystem>;
        
    private:
        eastl::vector<Ptr_IComponentSystem>  m_Systems;
        eastl::hash_map<EntityId, Entity> m_Entities;
    };
}