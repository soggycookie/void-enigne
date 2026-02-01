#include "world.h"

namespace ECS
{

    World* CreateWorld()
    {
        auto w = new World();
        w->Init();

        return w;
    }

    Entity World::CreateEntity()
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
        e.IncreGenCount();

        EntityRecord r = {};
        std::snprintf(r.name, 16, "Entity");

        m_entityIndex.PushBack(e.GetFullId(), r, newId);

        return e;
    }

    Entity World::CreateEntity(const char* name)
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
        e.IncreGenCount();

        EntityRecord r = {};
        std::snprintf(r.name, 16, name);

        m_entityIndex.PushBack(e.GetFullId(), r, newId);

        return e;
    }

    Entity World::CreateEntity(LoEntityId id)
    {
        if(!m_entityIndex.isValidDense(id))
        {
            Entity e(id, this);
            e.IncreGenCount();

            EntityRecord r = {};
            std::snprintf(r.name, 16, "Entity");

            m_entityIndex.PushBack(e.GetFullId(), r, true);
            
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
            e.IncreGenCount();
            
            EntityRecord r = {};
            std::snprintf(r.name, 16, "Entity");

            m_entityIndex.PushBack(e.GetFullId(), r, newId);
            
            return e;
        }
    }

    Entity World::CreateEntity(LoEntityId id, const char* name)
    {
        if(!m_entityIndex.isValidDense(id))
        {
            Entity e(id, this);
            e.IncreGenCount();

            EntityRecord r = {};
            std::snprintf(r.name, 16, name);

            m_entityIndex.PushBack(e.GetFullId(), r, true);
            
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
            e.IncreGenCount();
            
            EntityRecord r = {};
            std::snprintf(r.name, 16, name);

            m_entityIndex.PushBack(e.GetFullId(), r, newId);
            
            return e;
        }
    }

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
        m_componentRecords.Init(&m_wAllocator, 8);
        m_typeInfos.Init(&m_wAllocator, 8);
        m_mappedArchetype.Init(&m_wAllocator, 8);
    }

    void World::InitAllocators()
    {
        m_allocators.archetypes.Init(SparsePageCount * sizeof(Archetype));
    }

    //Entity* World::GetEntity(EntityId id)
    //{
    //    return m_entityIndex.GetPageData(id);
    //}




}