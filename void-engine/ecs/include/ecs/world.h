#pragma once
#include "ecs_pch.h"
#include "ds/world_allocator.h"
#include "ds/hash_map.h"
#include "entity.h"
#include "ecs_type.h"
#include "system_meta.h"

namespace ECS
{
    struct Allocators
    {
        //BlockAllocator typeInfo;
        BlockAllocator archetypes;
    };

    template<typename T>
    struct Store
    {
        T* store;
        uint32_t count;
        uint32_t capacity;

        void Init(WorldAllocator& wAllocator)
        {
            uint32_t storeCapacity = 8;
            count = 0;
            store =
                PTR_CAST(wAllocator.AllocN(sizeof(T), storeCapacity, storeCapacity), T);
            capacity = storeCapacity;
        }

        void Grow(WorldAllocator& wAllocator)
        {
            uint32_t newStoreCapacity = capacity * 2;
            T* newStore =
                PTR_CAST(wAllocator.AllocN(sizeof(T), newStoreCapacity, newStoreCapacity), T);

            std::memcpy(newStore, store, sizeof(T) * capacity);

            wAllocator.Free(sizeof(T) * capacity, store);

            store = newStore;
            capacity = newStoreCapacity;
        }

        void Add(T id)
        {
            store[count] = id;
            ++count;
        }

        void Destroy(WorldAllocator& wAllocator)
        {
            wAllocator.Free(sizeof(T) * capacity, store);
        }
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
        Entity GetEntity(EntityId id);

        template<typename Component>
        TypeInfoBuilder<Component> RegisterComponent();

        template<typename Component>
        void AddComponent(EntityId id);

        template<typename Component>
        void RemoveComponent(EntityId id);

        void GrowArchetype(Archetype& archetype);

        void SwapBack(EntityRecord& r);

        Archetype* CreateArchetype(ComponentSet&& componentSet);

        Archetype* GetArchetype(const ComponentSet& componentSet);

        Archetype* GetOrCreateArchetype_Add(Archetype* src, ComponentDiff&& componentDiff);

        Archetype* GetOrCreateArchetype_Remove(Archetype* src, ComponentDiff&& componentDiff);

        template<typename Component>
        void Set(EntityId id, Component&& c);

        template<typename Component>
        Component& Get(EntityId id);

        //NOTE: System store list of cache archetypes, but the list can be invalidated at runtime,
        //so I need to find a new way to re-validate this or rewrite this in a different way
        //basically, I have to introduce sync point

        template<typename... Components, typename... FuncArgs>
        void System(void (*func)(FuncArgs...));

        template<typename... FuncArgs>
        void Each(void (*func)(FuncArgs...));

        void Progress(double dt);

        void Destroy();

    public:
        WorldAllocator m_wAllocator;
        Allocators m_allocators;
        SparseSet<EntityRecord> m_entityIndex;
        SparseSet<Archetype> m_archetypes;
        HashMap<ComponentId, ComponentRecord> m_componentRecords;
        HashMap<ComponentId, TypeInfo*> m_typeInfos;
        HashMap<ComponentSet, Archetype*> m_mappedArchetype; //value hold a ref to key, does not change the value's key ref
        Store<ComponentId> m_componentStore;
        Store<SystemCallback> m_systemStore;
        uint32_t m_nextFreeId;
        bool m_isDefered;
    };
}

#include "world.inl"
#include "entity_builder.inl"