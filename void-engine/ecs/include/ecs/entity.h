#pragma once
#include "entity_builder.h"

namespace ECS
{
    class Entity : public EntityBuilder
    {
    public:

        static constexpr size_t NameCapacity = 16;

        explicit Entity(EntityId id, World* world, const char* name)
            : EntityBuilder(id, world)
        {
            if(name)
            {
                int32_t r = std::snprintf(m_name, NameCapacity, name);
            
                if(r < 0)
                {
                    SetDefaultName();
                }
            }
            else
            {
                SetDefaultName();
                //m_name[0] = '\0';
            }
        }
        virtual ~Entity() = default;

        Entity(Entity&& other) = default;
        Entity& operator=(Entity&& other) = default;

        Entity(const Entity& other) = default;
        Entity& operator=(const Entity& other) = default;

        const char* GetName() const
        {
            return m_name;
        }

    private:
        void SetDefaultName();


    private:
        char m_name[NameCapacity];
    };
}
