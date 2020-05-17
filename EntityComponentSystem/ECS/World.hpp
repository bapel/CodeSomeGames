#pragma once

#include "Config.hpp"
#include "ComponentSystem.hpp"
#include "EntityComponent.hpp"

#include <assert.h>
#include <EASTL\string.h>
#include <EASTL\fixed_string.h>
#include <EASTL\fixed_hash_map.h>
#include <EASTL\unique_ptr.h>
#include <EASTL\vector.h>
#include <EASTL\queue.h>
#include <EASTL\hash_map.h>

#define ecs_ctid(ComponentType__) GetComponentTypeId<ComponentType__>()

namespace ecs
{
    class World
    {
        class Entity
        {
        public:
            eastl::string Name;
            eastl::fixed_hash_map<ComponentTypeId, ComponentId, MaxComponentsPerEntity> Components;

            Entity(const eastl::string& name) : Name(name) {}
        };

        using PtrSystem = eastl::unique_ptr<BaseComponentSystem>;
        using PtrComponents = eastl::unique_ptr<EntityComponentCollection>;

        using Systems = eastl::vector<PtrSystem>;
        using Entities = eastl::hash_map<EntityId, Entity>;
        using EntityPool = eastl::queue<EntityId>;
        using Components = eastl::hash_map<ComponentTypeId, PtrComponents>;

    private:
        Systems m_Systems;
        Entities m_Entities;
        EntityPool m_EntityPool;
        Components m_Components;

        EntityId m_LargestId;

    public:
        World() :
            m_LargestId(0)
        {}

        EntityId CreateEntity(const eastl::string& name)
        {
            EntityId id = InvalidEntityId;

            if (!m_EntityPool.empty())
            {
                id = m_EntityPool.front();
                m_EntityPool.pop();
            }
            else
                id = m_LargestId++;

            assert(id != InvalidEntityId);
            m_Entities.emplace(id, name);

            return id;
        }

        void DestroyEntity(EntityId id)
        {
            auto& entity = m_Entities.at(id);
            auto& componentMap = m_Entities.at(id).Components;
            auto b = componentMap.begin();
            auto e = componentMap.end();

            for (auto i = b; i != e; i++)
            {
                auto ctid = i->first;
                auto cid = i->second;


            }

            m_Entities.erase(id);
        }

        template <class ComponentType>
        void AddComponentToEntity(EntityId id, const ComponentType& initial = {})
        {
            auto ctid = ecs_ctid(ComponentType);
            auto collection = GetComponentCollection<ComponentType>();
            auto componentId = collection->CreateComponentForEntity(id);

            m_Entities.at(id).Components.emplace(ctid, componentId);
        }

        template <class ComponentType>
        bool EntityHasComponent(EntityId id)
        {
            auto ctid = ecs_ctid(ComponentType);
            auto& componentMap = m_Entities.at(id).Components;

            return componentMap.find(ctid) != componentMap.end();
        }

        template <class ComponentType>
        void RemoveComponentFromEntity(EntityId id)
        {
            auto ctid = ecs_ctid(ComponentType);
            auto& entity = m_Entities.at(id);
            auto collection = GetComponentCollection<ComponentType>();
            auto componentId = entity.Components.at(ctid);
            auto movedEntityId = collection->DestroyComponent(componentId);

            entity.Components.erase(ctid);

            if (movedEntityId != InvalidEntityId)
                m_Entities.at(movedEntityId).Components.at(ctid) = componentId;
        }

        template <class ComponentType>
        void InitComponentCollection()
        {
            auto c = eastl::make_unique<ComponentCollection<ComponentType>>();
            m_Components[ecs_ctid(ComponentType)] = eastl::move(c);
        }

        template <class ComponentType>
        const ComponentCollection<ComponentType>* GetComponentCollection() const
        {
            auto ptr = m_Components.at(ecs_ctid(ComponentType)).get();
            return (const ComponentCollection<ComponentType>*)ptr;
        }

        template <class ComponentType>
        ComponentCollection<ComponentType>* GetComponentCollection()
        {
            auto ptr = m_Components.at(ecs_ctid(ComponentType)).get();
            return (ComponentCollection<ComponentType>*)ptr;
        }
    };
}

#undef ecs_ctid
