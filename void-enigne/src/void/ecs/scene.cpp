#include "scene.h"
#include "void/memory_system.h"

namespace VoidEngine
{
    namespace ECS
    {
        World::World()
            : m_generatedEntityId(1)
        {
            m_store.reserve(300);
        }
        
        World::~World()
        {
        }

        void Entity::Destroy()
        {
            scene->DestroyEntity(id);
        }
        
        bool Entity::IsAlive()
        {
            if(scene->m_entityRecords.find(id) != scene->m_entityRecords.end())
            {
                return true;
            }

            return false;
        }

        Entity World::CreateEntity()
        {
            EntityId id = GenerateEntityId();

            return Entity(id, this);
        }

        EntityId World::GenerateEntityId()
        {
            EntityId id = nullEntityId;
            if(!m_freeEntityIndex.empty())
            {
                id = m_freeEntityIndex[m_freeEntityIndex.size() - 1]; 
                m_freeEntityIndex.pop_back();
                INCRE_GEN_COUNT(id);
            }
            else
            {
                id = MAKE_ENTITY_ID(m_generatedEntityId++, 0);
            }

            return id;
        }

        void World::DestroyEntity(EntityId id)
        {
            if(m_entityRecords.find(id) != m_entityRecords.end())
            {
                m_entityRecords.erase(id);
                m_freeEntityIndex.push_back(id);
            }
        }
    }
}