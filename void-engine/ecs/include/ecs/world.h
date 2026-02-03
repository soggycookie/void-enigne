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

    struct ComponentStore
    {
        ComponentId* storeArr;
        uint32_t count;
        uint32_t capacity;

        void Init(WorldAllocator& wAllocator)
        {
            uint32_t storeCapacity = 8;
            count = 0;
            storeArr =
                PTR_CAST(wAllocator.AllocN(sizeof(ComponentId), storeCapacity, storeCapacity), ComponentId);
            capacity = storeCapacity;
        }

        void Grow(WorldAllocator& wAllocator)
        {
            uint32_t newStoreCapacity = capacity * 2;
            ComponentId* newStoreArr =
                PTR_CAST(wAllocator.AllocN(sizeof(ComponentId), newStoreCapacity, newStoreCapacity), ComponentId);

            std::memcpy(newStoreArr, storeArr, sizeof(ComponentId) * capacity);

            wAllocator.Free(sizeof(ComponentId) * capacity, storeArr);

            storeArr = newStoreArr;
            capacity = newStoreCapacity;
        }

        void Add(ComponentId id)
        {
            storeArr[count] = id;        
            ++count;
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
        TypeInfoBuilder<Component> RegisterComponent()
        {
            TypeInfo* ti = new (m_wAllocator.Alloc(sizeof(TypeInfo))) TypeInfo();
            ti->size = sizeof(Component);
            ti->alignment = alignof(Component);
            ti->id = GetComponentId<Component>();
            ti->entityId = 0;
            ti->name = typeid(Component).name();

            auto e = CreateEntity(ti->name);
            ti->entityId = e.GetFullId();

            assert(std::is_destructible_v<Component>);
            assert(std::is_trivially_constructible_v<Component>);

            TypeInfoBuilder<Component> tiBuilder{*ti};

            tiBuilder.Ctor(
                [](void* dest)
                {
                    new (dest) Component();
                }
            );

            if constexpr(std::is_move_constructible_v<Component>)
            {
                tiBuilder.MoveCtor(
                    [](void* dest, void* src)
                    {
                        new (dest) Component(std::move(*PTR_CAST(src, Component)));
                    }
                );
            }
            else if constexpr(!std::is_trivially_constructible_v<Component>)
            {
                tiBuilder.CopyCtor(
                    [](void* dest, const void* src)
                    {
                        new (dest) Component(*PTR_CAST(src, Component));
                    }
                );
            }

            if constexpr(!std::is_trivially_destructible_v<Component>)
            {
                tiBuilder.Dtor(
                    [](void* src)
                    {
                        Component* c = PTR_CAST(src, Component);
                        c->~Component();
                    }
                );
            }


            tiBuilder.AddEvent(
                []()
                {
                    std::cout << "Add component " << ComponentName<Component>::name << std::endl;
                }
            );

            tiBuilder.RemoveEvent(
                []()
                {
                    std::cout << "Remove component" << ComponentName<Component>::name << std::endl;
                }
            );

            tiBuilder.SetEvent(
                [](void* dest)
                {
                    std::cout << "Set component" << ComponentName<Component>::name << std::endl;
                }
            );

            if(m_componentStore.capacity == m_componentStore.count)
            {
                m_componentStore.Grow(m_wAllocator);
            }
            m_componentStore.Add(ti->id);

            ComponentRecord cr;
            cr.id = ti->id;
            cr.typeInfo = ti;
            cr.archetypeCache.capacity = 4;
            cr.archetypeCache.archetypeArr = PTR_CAST(m_wAllocator.Alloc(sizeof(Archetype*) * cr.archetypeCache.capacity), Archetype*);
            cr.archetypeCache.count = 0;

            assert(cr.archetypeCache.archetypeArr);

            m_componentRecords.Insert(ti->id, std::move(cr));
            m_typeInfos.Insert(ti->id, ti);
            return tiBuilder;
        }

        template<typename Component>
        void AddComponent(EntityId id)
        {
            EntityRecord* r = m_entityIndex.GetPageData(id);

            assert(r);

            if(r->archetype)
            {
                int32_t s = r->archetype->components.Search(GetComponentId<Component>());
                
                assert(s == -1);
            }

            uint32_t count = 1;
            uint32_t* arr = PTR_CAST(m_wAllocator.Alloc(sizeof(ComponentId) * count), ComponentId);

            ComponentDiff cdiff;
            cdiff.idArr = arr;
            cdiff.count = count;
            cdiff.idArr[0] = GetComponentId<Component>();

            Archetype* destArchetype = GetOrCreateArchetype_Add(r->archetype, std::move(cdiff));

            if(destArchetype->count == destArchetype->capacity)
            {
                GrowArchetype(*destArchetype);
            }

            //empty entity
            if(!r->archetype)
            {
                for(uint32_t i = 0; i < destArchetype->components.count; i++)
                {
                    TypeInfo& ti = *(destArchetype->columns[i].typeInfo);
                    void* dataAddr = OFFSET(destArchetype->columns[i].data, destArchetype->count * ti.size);
                    ti.hook.ctor(dataAddr);
                }
            }
            //at least 1 component
            else
            {
                //SWAP BACK IN SRC ARCHETYPE
                SwapBack(*r);

                for(uint32_t i = 0; i < destArchetype->components.count; i++)
                {
                    Column& destCol = destArchetype->columns[i];
                    TypeInfo& ti = *destCol.typeInfo;

                    void* dest = OFFSET(destCol.data, ti.size * destArchetype->count);
                    
                    int32_t srcIndex = r->archetype->components.Search(destArchetype->components.idArr[i]);

                    if(srcIndex == -1)
                    {
                        ti.hook.ctor(dest);
                    }
                    else
                    {
                        Column& srcCol = r->archetype->columns[srcIndex];
                        void* src = OFFSET(srcCol.data, ti.size * r->row);

                        if(ti.hook.moveCtor)
                        {
                            ti.hook.moveCtor(dest, src);
                        }
                        else if(ti.hook.copyCtor)
                        {
                            ti.hook.copyCtor(dest, src);
                        }
                        else
                        {
                            std::memcpy(dest, src, ti.size);
                        }

                        if(ti.hook.dtor)
                        {
                            ti.hook.dtor(src);
                        }
                    }
                }

            }

            destArchetype->entities[destArchetype->count] = id;
            r->archetype = destArchetype;
            r->row = destArchetype->count;
            ++destArchetype->count;

            TypeInfo& ti = *m_typeInfos[GetComponentId<Component>()];
            ti.hook.onAdd();
        }

        template<typename Component>
        void RemoveComponent(EntityId id)
        {
            EntityRecord* r = m_entityIndex.GetPageData(id);

            assert(r);
            assert(r->archetype);
            Archetype* srcArchetype = r->archetype;
            int32_t s = srcArchetype->components.Search(GetComponentId<Component>());

            assert(s != -1);

            uint32_t count = 1;
            uint32_t* arr = PTR_CAST(m_wAllocator.Alloc(sizeof(ComponentId) * count), ComponentId);

            ComponentDiff cdiff;
            cdiff.idArr = arr;
            cdiff.count = count;
            cdiff.idArr[0] = GetComponentId<Component>();

            Archetype* destArchetype = GetOrCreateArchetype_Remove(srcArchetype, std::move(cdiff));

            if(destArchetype->count == destArchetype->capacity)
            {
                GrowArchetype(*destArchetype);
            }

            //SWAP BACK IN SRC ARCHETYPE
            SwapBack(*r);

            for(uint32_t i = 0; i < srcArchetype->components.count; i++)
            {
                Column& srcCol = srcArchetype->columns[i];
                TypeInfo& ti = *srcCol.typeInfo;
                void* src = OFFSET(srcCol.data, ti.size * r->row);

                int32_t destIndex = destArchetype->components.Search(srcArchetype->components.idArr[i]);

                if(destIndex != -1)
                {
                    Column& destCol = destArchetype->columns[destIndex];
                    void* dest = OFFSET(destCol.data, ti.size * destArchetype->count);

                    if(ti.hook.moveCtor)
                    {
                        ti.hook.moveCtor(dest, src);
                    }
                    else if(ti.hook.copyCtor)
                    {
                        ti.hook.copyCtor(dest, src);
                    }
                    else
                    {
                        std::memcpy(dest, src, ti.size);
                    }
                }

                if(ti.hook.dtor)
                {
                    ti.hook.dtor(src);
                }
            }

            destArchetype->entities[destArchetype->count] = id;
            r->archetype = destArchetype;
            r->row = destArchetype->count;
            ++destArchetype->count;

            TypeInfo& ti = *m_typeInfos[GetComponentId<Component>()];
            ti.hook.onRemove();
        }

        void GrowArchetype(Archetype& archetype)
        {
            uint32_t newCapacity = archetype.capacity * 2;

            //grow entities
            EntityId* newEntities =
                PTR_CAST(m_wAllocator.Alloc(sizeof(EntityId) * newCapacity), EntityId);

            std::memcpy(newEntities, archetype.entities, sizeof(EntityId) * archetype.capacity);
            m_wAllocator.Free(sizeof(EntityId) * archetype.capacity, archetype.entities);
            
            archetype.entities = newEntities;

            //grow column data
            for(uint32_t i = 0; i < archetype.components.count; i++)
            {
                Column& col = archetype.columns[i];
                auto ti = col.typeInfo;
                void* newColData = m_wAllocator.Alloc(ti->size * newCapacity);

                if(ti->hook.moveCtor || ti->hook.copyCtor)
                {
                    for(uint32_t row = 0; row < archetype.count; row++)
                    {
                        void* dest = OFFSET(newColData, ti->size * row);
                        void* src = OFFSET(col.data, ti->size * row);

                        if(ti->hook.moveCtor)
                        {
                            ti->hook.moveCtor(dest, src);
                        }
                        else
                        {
                            ti->hook.copyCtor(dest, src);
                        }

                        if(ti->hook.dtor)
                        {
                            ti->hook.dtor(src);
                        }
                    }
                }
                else
                {
                    std::memcpy(newColData, col.data, ti->size * archetype.count);
                }

                m_wAllocator.Free(ti->size * archetype.capacity, col.data);

                col.data = newColData;
            }

            archetype.capacity = newCapacity;
        }

        void SwapBack(EntityRecord& r)
        {
            assert(r.archetype);
            assert(r.dense);

            Archetype& archetype = *r.archetype;

            if(archetype.count == 0 || (archetype.count == r.row + 1))
            {
                return;
            }

            EntityId swapId = archetype.entities[r.row];
            EntityId backId = archetype.entities[archetype.count - 1];

            EntityRecord* backRecord = m_entityIndex.GetPageData(backId);
            assert(backRecord);

            archetype.entities[r.row] = backId;
            archetype.entities[archetype.count - 1] = swapId;

            for(uint32_t i = 0; i < r.archetype->components.count; i++)
            {
                Column& col = r.archetype->columns[i];
                TypeInfo& ti = *col.typeInfo;
                void* swap = OFFSET(col.data, ti.size * r.row);
                void* back = OFFSET(col.data, ti.size * (archetype.count - 1));

                void* temp = m_wAllocator.Alloc(ti.size);

                if(ti.hook.moveCtor)
                {
                    ti.hook.moveCtor(temp, swap);
                    ti.hook.moveCtor(swap, back);
                    ti.hook.moveCtor(back, temp);
                }
                else if(ti.hook.copyCtor)
                {
                    ti.hook.copyCtor(temp, swap);
                    ti.hook.copyCtor(swap, back);
                    ti.hook.copyCtor(back, temp);
                }
                else
                {
                    std::memcpy(temp, swap, ti.size);
                    std::memcpy(swap, back, ti.size);
                    std::memcpy(back, temp, ti.size);
                }

                if(ti.hook.dtor)
                {
                    ti.hook.dtor(temp);
                }

                m_wAllocator.Free(ti.size, temp);
            }

            backRecord->row = r.row;
            r.row = archetype.count - 1;
        }

        Archetype* CreateArchetype(ComponentSet&& componentSet)
        {
            ArchetypeId id = GetArchetypeId();
            Archetype archetype;
            archetype.id = id;
            archetype.count = 0;
            archetype.capacity = DefaultArchetypeCapacity;
            archetype.components = componentSet;
            archetype.addEdges.Init(&m_wAllocator, DefaultArchetypeCapacity);
            archetype.removeEdges.Init(&m_wAllocator, DefaultArchetypeCapacity);

            archetype.columns =
                PTR_CAST(m_wAllocator.Alloc(sizeof(Column) * componentSet.count), Column);
            archetype.entities =
                PTR_CAST(m_wAllocator.Alloc(sizeof(EntityId) * DefaultArchetypeCapacity), EntityId);

            for(uint32_t idx = 0; idx < componentSet.count; idx++)
            {
                TypeInfo* ti = m_typeInfos[componentSet.idArr[idx]];
                archetype.columns[idx].typeInfo = ti;
                archetype.columns[idx].data =
                    m_wAllocator.Alloc(ti->size * DefaultArchetypeCapacity);
            }

            m_archetypes.PushBack(id, std::move(archetype));
            Archetype* rArchetype = m_archetypes.GetPageData(id);
            
            //NOTE: Test this
            for(uint32_t idx = 0; idx < componentSet.count; idx++)
            {
                ComponentRecord& cr = m_componentRecords[componentSet.idArr[idx]];
                
                if(cr.archetypeCache.count == cr.archetypeCache.capacity)
                {
                    uint32_t newCapacity = cr.archetypeCache.capacity * 2;
                    Archetype** newArchetypeArr = PTR_CAST(m_wAllocator.Alloc(sizeof(Archetype*) * newCapacity), Archetype*);
                    std::memcpy(newArchetypeArr, cr.archetypeCache.archetypeArr, sizeof(Archetype*) * cr.archetypeCache.capacity);

                    m_wAllocator.Free(sizeof(Archetype*) * cr.archetypeCache.capacity, cr.archetypeCache.archetypeArr);
                    cr.archetypeCache.archetypeArr = newArchetypeArr;
                }
                
                cr.archetypeCache.archetypeArr[cr.archetypeCache.count] = rArchetype;
                ++cr.archetypeCache.count;
            }


            m_mappedArchetype.Insert(componentSet, rArchetype);

            return rArchetype;
        }

        Archetype* GetArchetype(const ComponentSet& componentSet)
        {
            if(m_mappedArchetype.ContainsKey(componentSet))
            {
                return m_mappedArchetype[componentSet];
            }

            return nullptr;
        }

        Archetype* GetOrCreateArchetype_Add(Archetype* src, ComponentDiff&& componentDiff)
        {
            uint32_t srcCount = 0;
            if(src)
            {
                srcCount = src->components.count;
            }

            Archetype* dest = nullptr;

            //find or create edge
            if(src)
            {
                if(src->addEdges.ContainsKey(componentDiff))
                {
                    dest = src->addEdges[componentDiff];
                    m_wAllocator.Free(sizeof(ComponentId) * componentDiff.count, componentDiff.idArr);
                }
                else
                {
                    //CREATE EDEGE
                    uint32_t count = srcCount + componentDiff.count;
                    uint32_t* arr = PTR_CAST(m_wAllocator.Alloc(sizeof(ComponentId) * count), ComponentId);

                    ComponentSet cs;
                    cs.idArr = arr;
                    cs.count = count;
                    std::memcpy(cs.idArr, src->components.idArr, srcCount * sizeof(ComponentId));
                    std::memcpy((cs.idArr + srcCount), componentDiff.idArr, componentDiff.count * sizeof(ComponentId));
                    cs.Sort();

                    dest = GetArchetype(cs);

                    if(!dest)
                    {
                        dest = CreateArchetype(std::move(cs));
                    }
                    else
                    {
                        m_wAllocator.Free(sizeof(ComponentId) * cs.count, cs.idArr);
                    }

                    src->addEdges.Insert(std::move(componentDiff), dest);
                }
            }
            else
            {
                componentDiff.Sort();

                dest = GetArchetype(componentDiff);

                if(!dest)
                {
                    dest = CreateArchetype(std::move(componentDiff));
                }
                else
                {
                    m_wAllocator.Free(sizeof(ComponentId) * componentDiff.count, componentDiff.idArr);
                }
            }

            assert(dest);

            return dest;
        }

        Archetype* GetOrCreateArchetype_Remove(Archetype* src, ComponentDiff&& componentDiff)
        {
            assert(src);

            uint32_t srcCount = 0;
            srcCount = src->components.count;
            
            Archetype* dest = nullptr;

            if(src->removeEdges.ContainsKey(componentDiff))
            {
                dest = src->removeEdges[componentDiff];
                m_wAllocator.Free(sizeof(ComponentId) * componentDiff.count, componentDiff.idArr);
            }
            else
            {
                //CREATE EDEGE
                uint32_t count = srcCount - componentDiff.count;
                assert(count >= 0);
                ComponentId* arr = PTR_CAST(m_wAllocator.Alloc(sizeof(ComponentId) * count), ComponentId);

                ComponentSet cs;
                cs.idArr = arr;
                cs.count = count;

                //TODO: Review this later
                uint32_t csIdx = 0;

                for(uint32_t sIdx = 0; sIdx < src->components.count; sIdx++)
                {
                    bool removed = false;
                    for(uint32_t cdIdx = 0; cdIdx < componentDiff.count; cdIdx++)
                    {
                        if(src->components.idArr[sIdx] == componentDiff.idArr[cdIdx])
                        {
                            removed = true;
                        }
                    }

                    if(!removed)
                    {
                        cs.idArr[csIdx] = src->components.idArr[sIdx];
                        ++csIdx;
                    }
                }

                cs.Sort();

                dest = GetArchetype(cs);

                if(!dest)
                {
                    dest = CreateArchetype(std::move(cs));
                }
                else
                {
                    m_wAllocator.Free(sizeof(ComponentId) * cs.count, cs.idArr);
                }

                src->removeEdges.Insert(std::move(componentDiff), dest);
            }
            
            assert(dest);

            return dest;
        }

        template<typename Component>
        void Set(EntityId id, Component&& c)
        {
            EntityRecord* r = m_entityIndex.GetPageData(id);
            assert(r);
            assert(r->archetype);
            assert(r->dense);

            int32_t idx = r->archetype->components.Search(GetComponentId<Component>());

            assert(idx != -1);

            Component& component = *CAST_OFFSET_ELEMENT(r->archetype->columns[idx].data, Component, sizeof(Component), r->row);

            component = std::move(c);
        }

        template<typename Component>
        Component& Get(EntityId id)
        {
            EntityRecord* r = m_entityIndex.GetPageData(id);
            assert(r);
            assert(r->archetype);
            assert(r->dense);

            int32_t idx = r->archetype->components.Search(GetComponentId<Component>());

            assert(idx != -1);

            Component& component = *CAST_OFFSET_ELEMENT(r->archetype->columns[idx].data, Component, sizeof(Component), r->row);

            return component;
        }

        void Destroy()
        {
            //clear archetype
            for(uint32_t aIdx = 1; aIdx <= m_archetypes.GetCount(); aIdx++)
            {
                Archetype* archetype = m_archetypes.GetPageData(m_archetypes.GetId(aIdx));
                assert(archetype);

                for(uint32_t cIdx = 0; cIdx < archetype->components.count; cIdx++)
                {
                    Column& col = archetype->columns[cIdx];
                    TypeInfo& ti = *col.typeInfo;

                    if(ti.hook.dtor)
                    {
                        for(uint32_t row = 0; row < archetype->count; row++)
                        {
                            void* component = OFFSET(col.data, (ti.size * row));

                            ti.hook.dtor(component);
                        }
                    }

                    m_wAllocator.Free(ti.size * archetype->capacity, col.data);
                }

                m_wAllocator.Free(sizeof(EntityId) * archetype->capacity, archetype->entities);
                m_wAllocator.Free(sizeof(Column) * archetype->components.count, archetype->columns);
                archetype->addEdges.Destroy();
                archetype->removeEdges.Destroy();
            }



            m_archetypes.Destroy();
            m_entityIndex.Destroy();

            //NOTE: should clear the data if keeping metadata between world is favorable 
            m_componentRecords.Destroy();

            m_mappedArchetype.Destroy();
            m_typeInfos.Destroy();

            m_allocators.archetypes.Destroy();

            for(uint32_t bIdx = 1; bIdx <= m_wAllocator.m_sparse.GetCount(); bIdx++)
            {
                BlockAllocator* ba = m_wAllocator.m_sparse.GetPageData(m_wAllocator.m_sparse.GetId(bIdx));
                assert(ba);

                ba->Destroy();
            }

            m_wAllocator.m_sparse.Destroy();
            m_wAllocator.m_chunks.Destroy();
        }



    public:
        WorldAllocator m_wAllocator;
        Allocators m_allocators;
        SparseSet<EntityRecord> m_entityIndex;
        SparseSet<Archetype> m_archetypes;
        HashMap<ComponentId, ComponentRecord> m_componentRecords;
        HashMap<ComponentId, TypeInfo*> m_typeInfos;
        HashMap<ComponentSet, Archetype*> m_mappedArchetype; //value hold a ref to key, does not change the value's key ref
        ComponentStore m_componentStore;
        uint32_t m_nextFreeId;
        bool m_isDefered;
    };
}

#include "entity_builder.inl"