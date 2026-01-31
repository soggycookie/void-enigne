#include "entity.h"
#include "world.h"

namespace ECS
{

    void Entity::SetDefaultName()
    {
        std::snprintf(m_name, NameCapacity, "Entity %u", m_world->m_entityIndex.GetCount() + 1);
    }
}