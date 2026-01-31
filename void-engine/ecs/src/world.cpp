#include "world.h"

namespace ECS
{

    World* CreateWorld()
    {
        return new World();
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
        Entity e(id, this, nullptr);
        e.IncreGenCount();

        m_entityIndex.PushBack(id, std::move(e), newId);

        return *m_entityIndex.GetPageData(id);
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
        Entity e(id, this, name);
        e.IncreGenCount();

        m_entityIndex.PushBack(id, std::move(e), newId);

        return *m_entityIndex.GetPageData(id);
    }

    Entity World::CreateEntity(LoEntityId id)
    {
        if(!m_entityIndex.isValidDense(id))
        {
            Entity e(id, this, nullptr);
            e.IncreGenCount();

            m_entityIndex.PushBack(id, std::move(e), true);
            
            return *m_entityIndex.GetPageData(id);
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
            Entity e(freeId, this, nullptr);
            e.IncreGenCount();

            m_entityIndex.PushBack(freeId, std::move(e), newId);
            
            return *m_entityIndex.GetPageData(freeId);
        }
    }

    Entity World::CreateEntity(LoEntityId id, const char* name)
    {
        if(!m_entityIndex.isValidDense(id))
        {
            Entity e(id, this, name);
            e.IncreGenCount();

            m_entityIndex.PushBack(id, std::move(e), true);
            
            return *m_entityIndex.GetPageData(id);
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
            Entity e(freeId, this, name);
            e.IncreGenCount();

            m_entityIndex.PushBack(freeId, std::move(e), newId);
            
            return *m_entityIndex.GetPageData(freeId);
        }
    }

    const char* World::GetEntityName(EntityId id)
    {
        Entity* e = m_entityIndex.GetPageData(id);
        
        if(!e)
        {
            return nullptr;
        }
        else
        {
            return e->GetName();
        }
    }

    Entity* World::GetEntity(EntityId id)
    {
        return m_entityIndex.GetPageData(id);
    }

}