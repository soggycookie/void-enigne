
namespace ECS
{
    template<typename Component>
    EntityCommand& EntityCommand::AddComponent()
    {
        m_world->AddComponent<Component>(m_id);
        return *this;
    }

    template<typename Component>
    EntityCommand& EntityCommand::AddTag()
    {
        m_world->AddTag<Component>(m_id);
        return *this;
    }

    template<typename Component>
    EntityCommand& EntityCommand::AddPair(EntityId second)
    {
        m_world->AddPair<Component>(m_id, second);
        return *this;
    }

    template<typename Component>
    EntityCommand& EntityCommand::RemoveComponent()
    {
        m_world->RemoveComponent<Component>(m_id);
        return *this;
    }

    template<typename FirstComponent, typename... Components>
    EntityCommand& EntityCommand::AddComponents(const FirstComponent& f, const Components&... c)
    {

        return *this;
    }

    template<typename FirstComponent, typename... Components>
    EntityCommand& EntityCommand::RemoveComponents()
    {

        return *this;
    }

    template<typename Component>
    EntityCommand& EntityCommand::Set(Component&& c)
    {
        m_world->Set<Component>(m_id, std::forward<Component>(c));

        return *this;
    }

    template<typename Component>
    Component& EntityCommand::Get()
    {
        return m_world->Get<Component>(m_id);
    }
}