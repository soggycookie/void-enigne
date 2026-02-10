#pragma once
#include "id.h"

namespace ECS
{
    class World;

    class EntityCommand : public Id
    {
    protected:
        EntityCommand(EntityId id, World* world)
            : m_world(world), Id(id)
        {
        }

        EntityCommand(LoEntityId lowId, HiEntityId highId, World* world)
            : m_world(world), Id(lowId, highId)
        {
        }

        virtual ~EntityCommand() = default;

        EntityCommand(EntityCommand&& other) = default;
        EntityCommand(const EntityCommand& other) = default;

        EntityCommand& operator=(EntityCommand&& other) = default;
        EntityCommand& operator=(const EntityCommand& other) = default;

    public:
        template<typename Component>
        EntityCommand& AddComponent();

        template<typename Component>
        EntityCommand& AddTag();

        template<typename Component>
        EntityCommand& AddPair(EntityId second);

        template<typename Component>
        EntityCommand& RemoveComponent();

        template<typename FirstComponent, typename... Components>
        EntityCommand& AddComponents(const FirstComponent& f, const Components&... c);

        template<typename FirstComponent, typename... Components>
        EntityCommand& RemoveComponents();

        template<typename Component>
        EntityCommand& Set(Component&& c);

        template<typename Component>
        Component& Get();
    protected:
        World* m_world;
    };

}
