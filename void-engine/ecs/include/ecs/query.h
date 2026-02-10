#pragma once
#include "ecs_type.h"


namespace ECS
{
    struct ArchetypeLinkedList
    {
        Archetype* archetype;
        ArchetypeLinkedList* next;

        static ArchetypeLinkedList* Alloc(WorldAllocator& wAllocator)
        {
            ArchetypeLinkedList* all = PTR_CAST(wAllocator.Calloc(sizeof(ArchetypeLinkedList)), ArchetypeLinkedList);
        
            return all;
        }

        static void Free(WorldAllocator& wAllocator, void* addr)
        {
            wAllocator.Free(sizeof(ArchetypeLinkedList), addr); 
        }
    };

    class World;

    struct QueryIterator
    {
        World* world;
        Archetype* archetype;
        uint32_t row;
        //double deltaTime;

        Entity GetEntity()
        {
            EntityId id = archetype->entities[row];

            return Entity(id, world);
        }

        template<typename Component>
        Component& Get()
        {
            uint32_t colIdx = archetype->components.Search(ComponentTypeId<EntityId>::id);

            assert(colIdx != -1);

            Column& col = archetype->columns[colIdx];
            TypeInfo& ti = *col.typeInfo;
            void* comData = OFFSET(col.data, ti.size * row);
            
            return *PTR_CAST(comData, Component);
        }
    };

    struct Query
    {
        ArchetypeLinkedList* head;
    };
}
