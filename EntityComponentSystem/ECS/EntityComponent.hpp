#pragma once

#include "Config.hpp"
#include <EASTL\vector.h>

namespace ecs
{
    template <class ComponentType>
    constexpr ComponentTypeId GetComponentTypeId();

    class EntityComponentCollection
    {
    public:
        virtual ~EntityComponentCollection() = default;
    };

    template <class ComponentType>
    class ComponentCollection : public EntityComponentCollection
    {
    private:
        eastl::vector<ComponentType> m_Components;
        eastl::vector<EntityId> m_Entities;

    public:
        ~ComponentCollection() {}

        ComponentId CreateComponentForEntity(EntityId parentId)
        {
            m_Components.emplace_back();
            m_Entities.push_back(parentId);
            return (ComponentId)(m_Components.size() - 1);
        }

        EntityId DestroyComponent(ComponentId id)
        {
            if (m_Components.size() == 1)
            {
                m_Components.clear();
                m_Entities.clear();
                return InvalidEntityId;
            }

            m_Components.erase_unsorted(m_Components.begin() + id);
            m_Entities.erase_unsorted(m_Entities.begin() + id);

            return m_Entities[id];
        }

        inline EntityId GetEntity(ComponentId id) const
        {
            return m_Entities[id];
        }
    };
}

#define ecs_SetComponentTypeId(TypeId__, ComponentType__)\
    template <>\
    constexpr ecs::ComponentTypeId ecs::GetComponentTypeId<ComponentType__>() {\
        return TypeId__;\
    }