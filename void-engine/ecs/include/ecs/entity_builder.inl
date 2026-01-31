
namespace ECS
{
    template<typename Component>
    EntityBuilder& EntityBuilder::AddComponent(const Component& c)
    {

        return *this;
    }

    template<typename Component>
    EntityBuilder& EntityBuilder::RemoveComponent()
    {
        return *this;
    }

    template<typename FirstComponent, typename... Components>
    EntityBuilder& EntityBuilder::AddComponents(const FirstComponent& f, const Components&... c)
    {

        return *this;
    }

    template<typename FirstComponent, typename... Components>
    EntityBuilder& EntityBuilder::RemoveComponents()
    {

        return *this;
    }
}