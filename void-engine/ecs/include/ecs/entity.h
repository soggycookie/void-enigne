#pragma once
#include "entity_command.h"

namespace ECS
{
    class Entity : public EntityCommand
    {
    public:

        static constexpr size_t NameCapacity = 16;

        explicit Entity(EntityId id, World* world)
            : EntityCommand(id, world)
        {
        }

        virtual ~Entity() = default;

        Entity(Entity&& other) = default;
        Entity& operator=(Entity&& other) = default;

        Entity(const Entity& other) = default;
        Entity& operator=(const Entity& other) = default;
    };
}
