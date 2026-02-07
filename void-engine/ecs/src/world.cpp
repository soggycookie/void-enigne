#include "world.h"
#include "internal_component.h"

namespace ECS
{

    World* CreateWorld()
    {
        auto w = new World();
        w->Init();
        //w->CreateInternalEntity();
        w->RegisterInternalComponents();
        return w;
    }

    void World::RegisterInternalComponents()
    {
        RegisterComponent<EcsName>().Id(EcsNameId).Register();
        RegisterComponent<EcsSystem>().Id(EcsSystemId).Register();

        RegisterTag<EcsPhase>().Id(EcsPhaseId).Register();
        RegisterTag<EcsArchetype>().Id(EcsArchetypeId).Register();
        RegisterTag<EcsPipeline>().Id(EcsPipelineId).Register();

        RegisterPair<ChildOf>(true).Id(ChildOfId).Register();
        RegisterPair<DependOn>(false).Id(DependOnId).Register();
        RegisterPair<Toggle>(false, true).Id(ToggleId).Register();
    }


    void DestroyWorld(World* world)
    {
        world->Destroy();
        delete world;
    }

    Entity World::CreateEntity(EntityId parent)
    {
        uint64_t id = m_entityIndex.GetReservedFreeId();

        bool newId = false;
        if(id == 0)
        {
            newId = true;

            while(m_entityIndex.isValidDense(++m_nextFreeId));

            id = m_nextFreeId;
        }
        Entity e(id, this);

        if(!newId)
        {
            e.IncreGenCount();
        }

        EntityRecord r;
        r.archetype = nullptr;
        r.dense = 0;
        r.row = 0;
        std::snprintf(r.name, 16, "Entity %u", e.GetLowId());

        r.dense = m_entityIndex.PushBack(e.GetFullId(), r, newId);
        GetEntityRecord(e.GetFullId())->dense = r.dense;



        return e;
    }

    Entity World::CreateEntity(const char* name, EntityId parent)
    {
        uint64_t id = m_entityIndex.GetReservedFreeId();
        bool newId = false;
        if(id == 0)
        {
            newId = true;

            while(m_entityIndex.isValidDense(++m_nextFreeId));

            id = m_nextFreeId;
        }
        Entity e(id, this);

        if(!newId)
        {
            e.IncreGenCount();
        }

        EntityRecord r;
        r.archetype = nullptr;
        r.dense = 0;
        r.row = 0;
        std::snprintf(r.name, 16, name);

        r.dense = m_entityIndex.PushBack(e.GetFullId(), r, newId);
        GetEntityRecord(e.GetFullId())->dense = r.dense;

        return e;
    }

    Entity World::CreateEntity(LoEntityId id, EntityId parent)
    {
        if(!m_entityIndex.isValidDense(id))
        {
            Entity e(id, this);

            EntityRecord r;
            r.archetype = nullptr;
            r.dense = 0;
            r.row = 0;
            std::snprintf(r.name, 16, "Entity");

            r.dense = m_entityIndex.PushBack(e.GetFullId(), r, true);
            GetEntityRecord(e.GetFullId())->dense = r.dense;

            return e;
        }
        else
        {
            uint64_t freeId = m_entityIndex.GetReservedFreeId();
            bool newId = false;
            if(freeId == 0)
            {
                newId = true;

                while(m_entityIndex.isValidDense(++m_nextFreeId));

                freeId = m_nextFreeId;
            }

            Entity e(freeId, this);

            if(!newId)
            {
                e.IncreGenCount();
            }

            EntityRecord r;
            r.archetype = nullptr;
            r.dense = 0;
            r.row = 0;
            std::snprintf(r.name, 16, "Entity");

            r.dense = m_entityIndex.PushBack(e.GetFullId(), r, newId);
            GetEntityRecord(e.GetFullId())->dense = r.dense;

            return e;
        }
    }

    Entity World::CreateEntity(LoEntityId id, const char* name, EntityId parent)
    {
        if(!m_entityIndex.isValidDense(id))
        {
            Entity e(id, this);

            EntityRecord r;
            r.archetype = nullptr;
            r.dense = 0;
            r.row = 0;
            std::snprintf(r.name, 16, name);

            r.dense = m_entityIndex.PushBack(e.GetFullId(), r, true);
            GetEntityRecord(e.GetFullId())->dense = r.dense;

            return e;
        }
        else
        {
            uint64_t freeId = m_entityIndex.GetReservedFreeId();
            bool newId = false;
            if(freeId == 0)
            {
                newId = true;

                while(m_entityIndex.isValidDense(++m_nextFreeId));

                freeId = m_nextFreeId;
            }
            Entity e(freeId, this);

            if(!newId)
            {
                e.IncreGenCount();
            }

            EntityRecord r;
            r.archetype = nullptr;
            r.dense = 0;
            r.row = 0;
            std::snprintf(r.name, 16, name);

            r.dense = m_entityIndex.PushBack(e.GetFullId(), r, newId);
            GetEntityRecord(e.GetFullId())->dense = r.dense;

            return e;
        }
    }

    EntityId World::GetFreeId()
    {
        return ++m_nextFreeId;
    }


    //Entity World::CreateEntity(EntityDesc& desc)
    //{
    //    if(!m_entityIndex.isValidDense(desc.id))
    //    {
    //        Entity e(desc.id, this);

    //        EntityRecord r;
    //        r.archetype = nullptr;
    //        r.dense = 0;
    //        r.row = 0;
    //        std::snprintf(r.name, 16, desc.name);

    //        r.dense = m_entityIndex.PushBack(e.GetFullId(), r, true);
    //        EntityRecord& er = *GetEntityRecord(e.GetFullId());
    //        er.dense = r.dense;

    //        ResolveEntityDesc(er, desc);

    //        return e;
    //    }
    //    else
    //    {
    //        uint64_t freeId = m_entityIndex.GetReservedFreeId();
    //        bool newId = false;
    //        if(freeId == 0)
    //        {
    //            newId = true;

    //            while(m_entityIndex.isValidDense(++m_nextFreeId));

    //            freeId = m_nextFreeId;
    //        }
    //        Entity e(freeId, this);

    //        if(!newId)
    //        {
    //            e.IncreGenCount();
    //        }

    //        EntityRecord r;
    //        r.archetype = nullptr;
    //        r.dense = 0;
    //        r.row = 0;
    //        std::snprintf(r.name, 16, desc.name);

    //        r.dense = m_entityIndex.PushBack(e.GetFullId(), r, newId);
    //        EntityRecord& er = *GetEntityRecord(e.GetFullId());
    //        er.dense = r.dense;

    //        ResolveEntityDesc(er, desc);

    //        return e;
    //    }
    //}

    //void World::ResolveEntityDesc(EntityRecord& r, EntityDesc& desc)
    //{
    //    ComponentSet cs;
    //    Archetype* destArchetype = r.archetype;
    //    if(desc.parent != 0){
    //        ComponentDiff cdiff;
    //        cdiff.Alloc(m_wAllocator, 1);
    //        cdiff.idArr[0] = MakePair(desc.id, desc.parent);
    //    }
    //    if(desc.name)
    //    {
    //        ComponentDiff cdiff;
    //        cdiff.Alloc(m_wAllocator, 1);
    //        cdiff.idArr[0] = EcsNameId;
    //        destArchetype = GetOrCreateArchetype_Add(destArchetype, std::move(cdiff));
    //    }

    //    destArchetype = GetOrCreateArchetype_Add(destArchetype, std::move(desc.add));

    //    r.archetype = destArchetype;
    //    
    //    if(destArchetype->count == destArchetype->capacity)
    //    {
    //        destArchetype->
    //    }
    //    //Archetype* destArchetype = GetOrCreateArchetype_Add(r.archetype, );
    //}

    EntityRecord* World::GetEntityRecord(EntityId id)
    {
        EntityRecord* e = m_entityIndex.GetPageData(id);

        if(!e)
        {
            return nullptr;
        }
        else
        {
            return e;
        }
    }

    void World::Init()
    {
        m_wAllocator.Init();
        InitAllocators();
        m_entityIndex.Init(&m_wAllocator, nullptr, 8, true);
        m_archetypes.Init(&m_wAllocator, &m_allocators.archetypes, 8, false);
        m_componentIndex.Init(&m_wAllocator, 8);
        m_typeInfos.Init(&m_wAllocator, 8);
        m_mappedArchetype.Init(&m_wAllocator, 8);

        m_systemStore.Init(m_wAllocator);
        m_componentStore.Init(m_wAllocator);
        m_isDefered = false;
    }

    void World::InitAllocators()
    {
        m_allocators.archetypes.Init(SparsePageCount * sizeof(Archetype));
    }

    Entity World::GetEntity(EntityId id)
    {
        EntityRecord* r = m_entityIndex.GetPageData(id);
        uint32_t dense = r->dense;
        uint64_t versionedId = m_entityIndex.GetDenseArr()[dense];
        if(!r || versionedId != id)
        {
            return Entity(0, this);
        }

        return Entity(id, this);
    }

    void World::GrowArchetype(Archetype& archetype)
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

    void World::SwapBack(EntityRecord& r)
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

    Archetype* World::CreateArchetype(ComponentSet&& componentSet)
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
            ComponentRecord& cr = m_componentIndex[componentSet.idArr[idx]];

            if(cr.archetypeStore.count == cr.archetypeStore.capacity)
            {
                cr.archetypeStore.Grow(m_wAllocator);
            }

            cr.archetypeStore.store[cr.archetypeStore.count] = rArchetype;
            ++cr.archetypeStore.count;
        }


        m_mappedArchetype.Insert(componentSet, rArchetype);

        return rArchetype;
    }

    Archetype* World::GetArchetype(const ComponentSet& componentSet)
    {
        if(m_mappedArchetype.ContainsKey(componentSet))
        {
            return m_mappedArchetype[componentSet];
        }

        return nullptr;
    }

    Archetype* World::GetOrCreateArchetype_Add(Archetype* src, ComponentId id)
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
            if(src->addEdges.ContainsKey(id))
            {
                dest = src->addEdges[id];
            }
            else
            {
                //CREATE EDEGE
                uint32_t count = srcCount + 1;

                ComponentSet cs;
                cs.Alloc(m_wAllocator, count);
                std::memcpy(cs.idArr, src->components.idArr, srcCount * sizeof(ComponentId));
                cs.idArr[count - 1] = id;
                cs.count = count;
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

                src->addEdges.Insert(id, dest);
            }
        }
        else
        {
            ComponentSet cs;
            cs.Alloc(m_wAllocator, 1);
            cs.idArr[0] = id;

            dest = GetArchetype(cs);

            if(!dest)
            {
                dest = CreateArchetype(std::move(cs));
            }
            else
            {
                m_wAllocator.Free(sizeof(ComponentId), cs.idArr);
            }
        }

        assert(dest);

        return dest;
    }

    Archetype* World::GetOrCreateArchetype_Remove(Archetype* src, ComponentId id)
    {
        assert(src);

        uint32_t srcCount = 0;
        srcCount = src->components.count;

        Archetype* dest = nullptr;

        if(src->removeEdges.ContainsKey(id))
        {
            dest = src->removeEdges[id];
        }
        else
        {
            //CREATE EDEGE
            uint32_t count = srcCount - 1;
            assert(count >= 0);

            ComponentSet cs;
            cs.Alloc(m_wAllocator, count);
            cs.count = count;

            //TODO: Review this later
            uint32_t rIdx = src->components.Search(id);
            assert(rIdx != -1);

            std::memcpy(cs.idArr, src->components.idArr, rIdx * sizeof(ComponentId));
            std::memcpy(cs.idArr + rIdx, src->components.idArr + rIdx + 1, (count - rIdx) * sizeof(ComponentId));
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

            src->removeEdges.Insert(id, dest);
        }

        assert(dest);

        return dest;
    }

    void World::Progress(double dt)
    {
        if(m_isDefered == false)
        {
            m_isDefered = true;

            for(uint32_t idx = 0; idx < m_systemStore.count; idx++)
            {
                SystemCallback& sc = m_systemStore.store[idx];

                ArchetypeLinkedList* head = sc.archetypeList;

                void** componentsData = PTR_CAST(m_wAllocator.Alloc(sizeof(void*) * sc.components.count), void*);

                while(head->archetype)
                {
                    Archetype* archetype = head->archetype;

                    for(uint32_t row = 0; row < archetype->count; row++)
                    {
                        QueryIterator it;
                        it.archetype = archetype;
                        it.world = this;
                        it.row = row;

                        for(uint32_t idx = 0; idx < sc.components.count; idx++)
                        {
                            uint32_t colIdx = archetype->components.Search(sc.components.idArr[idx]);

                            assert(colIdx != -1);

                            Column& col = archetype->columns[colIdx];
                            TypeInfo& ti = *col.typeInfo;
                            void* comData = OFFSET(col.data, ti.size * row);
                            componentsData[idx] = comData;
                        }

                        //EXECUTE
                        sc.Execute(&it, componentsData);
                    }

                    head = head->next;
                }

                m_wAllocator.Free(sizeof(void*) * sc.components.count, componentsData);
            }

            m_isDefered = false;
        }
    }

    void World::Destroy()
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
        m_componentIndex.Destroy();

        m_mappedArchetype.Destroy();
        m_typeInfos.Destroy();

        m_allocators.archetypes.Destroy();
        m_componentStore.Destroy(m_wAllocator);

        for(uint32_t sIdx = 0; sIdx < m_systemStore.count; sIdx++)
        {
            SystemCallback& sc = m_systemStore.store[sIdx];
            ArchetypeLinkedList* head = sc.archetypeList;

            while(head->archetype)
            {
                ArchetypeLinkedList* freeNode = head;
                head = head->next;

                ArchetypeLinkedList::Free(m_wAllocator, freeNode);
            }

            sc.components.Free(m_wAllocator);
        }
        m_systemStore.Destroy(m_wAllocator);

        for(uint32_t bIdx = 1; bIdx <= m_wAllocator.m_sparse.GetCount(); bIdx++)
        {
            BlockAllocator* ba = m_wAllocator.m_sparse.GetPageData(m_wAllocator.m_sparse.GetId(bIdx));
            assert(ba);

            ba->Destroy();
        }

        m_wAllocator.m_sparse.Destroy();
        m_wAllocator.m_chunks.Destroy();
    }

}