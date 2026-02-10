#pragma once
#include "ecs_type.h"

namespace ECS
{
    struct EntityDesc
    {
        EntityId id;
        EntityId parent;
        ComponentSet add;
        Store<void*> componentData;
        const char* name;
    };

    class World;

    class EntityCommandBase
    {
    public:
        template<typename Component>
        EntityCommandBase& AddComponent();

        template<typename Component>
        EntityCommandBase& AddTag();

        template<typename First>
        EntityCommandBase& AddPair(EntityId second);

        template<typename Component>
        EntityCommandBase& RemoveComponent();

        /*template<typename FirstComponent, typename... Components>
        EntityMutator& AddComponents(const FirstComponent& f, const Components&... c);

        template<typename FirstComponent, typename... Components>
        EntityMutator& RemoveComponents();*/

        template<typename Component>
        EntityCommandBase& Set(Component&& c);

        template<typename Component>
        Component& Get();

    protected:
        virtual void AddComponentImpl(EntityId id) = 0;
        virtual void AddTagImpl(EntityId id) = 0;
        virtual void AddPairImpl(EntityId first, EntityId second) = 0;
        virtual void RemoveComponentImpl(EntityId id) = 0;
        virtual void* GetImpl(EntityId id) = 0;
        virtual void SetImpl(EntityId id, void* data) = 0;
    };

    template<typename... Components>
    struct EntityCommand
    {
        EntityDesc desc;
        World* world;
        
        
    };


}