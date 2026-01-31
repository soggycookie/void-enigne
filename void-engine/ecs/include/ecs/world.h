#pragma once
#include "ecs_pch.h"
#include "ds/world_allocator.h"
#include "entity.h"

namespace ECS
{

    class World
    {
    public:
        World()
            : m_nextFreeId(0)
        {
            m_allocator.Init();
            m_entityIndex.Init(&m_allocator, nullptr, 8, true);
        }

        Entity CreateEntity();

        Entity CreateEntity(const char* name);

        Entity CreateEntity(LoEntityId id);
        Entity CreateEntity(LoEntityId id, const char* name);

        const char* GetEntityName(EntityId id);
        Entity* GetEntity(EntityId id);

    public:
        WorldAllocator m_allocator;        
        SparseSet<Entity> m_entityIndex;
        uint32_t m_nextFreeId;
    };
}

#include "entity_builder.inl"