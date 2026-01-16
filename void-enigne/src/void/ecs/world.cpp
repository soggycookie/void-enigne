#include "world.h"
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
            return scene->isEntityAlive(id);
        }

        Entity World::CreateEntity()
        {
            EntityId id = GenerateEntityId();
            m_entityRecords.insert({id, EntityRecord{nullptr, 0}});
            return Entity(id, this);
        }

        EntityId World::GenerateEntityId()
        {
            EntityId id = nullEntityId;
            if(!m_freeEntityIndex.empty())
            {
                id = m_freeEntityIndex[m_freeEntityIndex.size() - 1]; 
                m_freeEntityIndex.pop_back();
                id = ECS_INCRE_GEN_COUNT(id);
            }
            else
            {
                id = ECS_MAKE_ENTITY_ID(m_generatedEntityId++, 0);
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

        bool World::isEntityAlive(EntityId id)
        {
            if(m_entityRecords.find(id) != m_entityRecords.end())
            {
                return true;
            }

            return false;        
        }

    }
}