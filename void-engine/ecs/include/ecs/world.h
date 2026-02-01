#pragma once
#include "ecs_pch.h"
#include "ds/world_allocator.h"
#include "ds/hash_map.h"
#include "entity.h"
#include "ecs_type.h"

namespace ECS
{
    struct Allocators
    {
        //BlockAllocator typeInfo;
        BlockAllocator archetypes;
    };


    class World
    {
    public:
        World()
            : m_nextFreeId(0), m_isDefered(false)
        {
        }

        void Init();

        void InitAllocators();

        Entity CreateEntity();

        Entity CreateEntity(const char* name);

        Entity CreateEntity(LoEntityId id);
        Entity CreateEntity(LoEntityId id, const char* name);

        EntityRecord* GetEntityRecord(EntityId id);
        Entity* GetEntity(EntityId id);


        //NOTE: Add type, event hook
        template<typename Component>
        TypeInfo& RegisterComponent()
        {
            //Entity e = CreateEntity(typeid(Component).name());
            TypeInfo* ti = new (m_wAllocator.Alloc(sizeof(TypeInfo))) TypeInfo();
            ti->size = sizeof(Component);
            ti->alignment = alignof(Component);
            ti->id = GetComponentId<Component>();
            ti->entityId = 0;
            ti->name = typeid(Component).name();

            m_typeInfos.Insert(ti->id, ti);
            return *ti;        
        }

    public:
        WorldAllocator m_wAllocator;        
        Allocators m_allocators;        
        SparseSet<EntityRecord> m_entityIndex;
        SparseSet<Archetype> m_archetypes;
        HashMap<ComponentId, ComponentRecord> m_componentRecords;
        HashMap<ComponentId, TypeInfo*> m_typeInfos;
        HashMap<ComponentSet, Archetype*> m_mappedArchetype;
        uint32_t m_nextFreeId;
        bool m_isDefered;
    };
}

#include "entity_builder.inl"