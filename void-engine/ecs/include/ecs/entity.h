#pragma once
#include "entity_builder.h"

namespace ECS
{
    class Entity : public EntityBuilder
    {
    public:

        static constexpr size_t NameCapacity = 16;

        explicit Entity(EntityId id, World* world)
            : EntityBuilder(id, world)
        {
        }

        virtual ~Entity() = default;

        Entity(Entity&& other) = default;
        Entity& operator=(Entity&& other) = default;

        Entity(const Entity& other) = default;
        Entity& operator=(const Entity& other) = default;


    };
}
