#pragma once
#include "ecs_pch.h"
#include "ecs_type.h"
#include "ds/world_allocator.h"
#include "type_info_builder.h"
#include "ds/hash_map.h"
#include "entity.h"
#include "system_meta.h"

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
            : m_nextFreeId(200), m_isDefered(false)
        {
        }

        void Init();

        void InitAllocators();

        void RegisterInternalComponents();

        Entity CreateEntity(EntityId parent);

        Entity CreateEntity(const char* name, EntityId parent);

        Entity CreateEntity(LoEntityId id, EntityId parent);
        Entity CreateEntity(LoEntityId id, const char* name, EntityId parent);

        Entity CreateEntity(EntityDesc& desc);

        EntityId GetFreeId();

        EntityRecord* GetEntityRecord(EntityId id);
        Entity GetEntity(EntityId id);

        void ResolveEntityDesc(EntityRecord& r, EntityDesc& desc);

        template<typename Component>
        TypeInfoBuilder<Component> Component();

        template<typename Tag>
        TypeInfoBuilder<Tag> Tag();
        
        template<typename Pair>
        TypeInfoBuilder<Pair> Pair(bool isExclusive, bool isToggle = false);

        template<typename Component>
        void AddComponent(EntityId id);

        template<typename Component>
        void RemoveComponent(EntityId id);

        template<typename Component>
        void AddPair(EntityId id);

        template<typename Component>
        void AddTag(EntityId id);

        void GrowArchetype(Archetype& archetype);

        void SwapBack(EntityRecord& r);

        Archetype* CreateArchetype(ComponentSet&& componentSet);

        Archetype* GetArchetype(const ComponentSet& componentSet);

        Archetype* GetOrCreateArchetype_Add(Archetype* src, ComponentId id);

        Archetype* GetOrCreateArchetype_Remove(Archetype* src, ComponentId id);


        template<typename T>
        void Set(EntityId id, T&& c);

        template<typename Component>
        Component& Get(EntityId id);

        //NOTE: System store list of cache archetypes, but the list can be invalidated at runtime,
        //so I need to find a new way to re-validate this or rewrite this in a different way
        //basically, I have to introduce sync point

        template<typename... Components, typename... FuncArgs>
        void System(void (*func)(FuncArgs...));

        template<typename... Components, typename... FuncArgs>
        void Each(void (*func)(FuncArgs...));

        void Progress(double dt);

        void Destroy();

    public:
        WorldAllocator m_wAllocator;
        Allocators m_allocators;
        SparseSet<EntityRecord> m_entityIndex;
        SparseSet<Archetype> m_archetypes;
        HashMap<ComponentId, ComponentRecord> m_componentIndex;
        HashMap<ComponentId, TypeInfo*> m_typeInfos;
        HashMap<ComponentSet, Archetype*> m_mappedArchetype; //value hold a ref to key, does not change the value's key ref
        Store<ComponentId> m_componentStore;
        Store<SystemCallback> m_systemStore;
        uint32_t m_nextFreeId;
        bool m_isDefered;
    };
}

#include "type_info_builder.inl"
#include "world.inl"
#include "entity_builder.inl"