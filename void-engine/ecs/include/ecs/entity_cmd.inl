
namespace ECS
{
    /*
        Definition for entity command base interface's public funcs
    */

    template<typename Component>
    EntityCommandBase& EntityCommandBase::AddComponent()
    {
        AddComponentImpl(ComponentTypeId<Component>::id);

        return *this;
    }

    template<typename Component>
    EntityCommandBase& EntityCommandBase::AddTag()
    {
        AddTagImpl(ComponentTypeId<Component>::id);

        return *this;    
    }

    template<typename First>
    EntityCommandBase& EntityCommandBase::AddPair(EntityId second)
    {
        AddPairImpl(ComponentTypeId<First>::id, second);

        return *this;       
    }

    template<typename Component>
    EntityCommandBase& EntityCommandBase::RemoveComponent()
    {
        RemoveComponentImpl(ComponentTypeId<Component>::id);

        return *this;   
    }

    template<typename Component>
    EntityCommandBase& EntityCommandBase::Set(Component&& c)
    {
        SetImpl(ComponentTypeId<Component>::id, &c);

        return *this;
    }

    template<typename Component>
    Component& EntityCommandBase::Get()
    {
        return *PTR_CAST(GetImpl(ComponentTypeId<Component>::id), Component);
    }
}