namespace ECS
{
    template<typename T>
    TypeInfoBuilder<T>& TypeInfoBuilder<T>::Ctor(CtorHook ctor)
    {
        ti.hook.ctor = ctor;

        return *this;
    }

    template<typename T>
    TypeInfoBuilder<T>& TypeInfoBuilder<T>::CopyCtor(CopyCtorHook cctor)
    {
        ti.hook.copyCtor = cctor;

        return *this;
    }

    template<typename T>
    TypeInfoBuilder<T>& TypeInfoBuilder<T>::MoveCtor(MoveCtorHook mctor)
    {
        ti.hook.moveCtor = mctor;

        return *this;
    }

    template<typename T>
    TypeInfoBuilder<T>& TypeInfoBuilder<T>::Dtor(DtorHook dtor)
    {
        ti.hook.dtor = dtor;

        return *this;
    }

    template<typename T>
    TypeInfoBuilder<T>& TypeInfoBuilder<T>::AddEvent(AddEventHook e)
    {
        ti.hook.onAdd = e;

        return *this;
    }

    template<typename T>
    TypeInfoBuilder<T>& TypeInfoBuilder<T>::RemoveEvent(RemoveEventHook e)
    {
        ti.hook.onRemove = e;

        return *this;
    }

    template<typename T>
    TypeInfoBuilder<T>& TypeInfoBuilder<T>::SetEvent(SetEventHook e)
    {
        ti.hook.onSet = e;

        return *this;
    }

    template<typename T>
    TypeInfoBuilder<T>& TypeInfoBuilder<T>::Id(EntityId id)
    {
        ti.id = id;

        return *this;
    }

    template<typename T>
    void TypeInfoBuilder<T>::Register()
    {
        if(ti.id == 0)
        {
            Entity e = world->CreateEntity(ComponentName<T>::name, 0);
            ti.id = e.GetFullId();
        }
        else
        {
            world->CreateEntity(ti.id, ComponentName<T>::name, 0);
        }

        if(world->m_componentStore.capacity == world->m_componentStore.count)
        {
            world->m_componentStore.Grow(world->m_wAllocator);
        }
        world->m_componentStore.Add(ti.id);

        ComponentRecord cr;
        cr.id = ti.id;
        cr.typeInfo = &ti;

#ifdef ECS_DEBUG

        if(ti.IsFullPair())
        {
            //char pName[30];
            //int32_t r = std::snprintf(cr.name, 30, ComponentName<T>::name);
            int32_t r = std::snprintf(cr.name, 16, ComponentName<T>::name);

            std::snprintf(&cr.name[r], 16 - r, " %u", HI_ENTITY_ID(ti.id));
        }
        else
        {
            std::snprintf(cr.name, 16, ComponentName<T>::name);
            ComponentTypeId<T>::Id(ti.id);
        }
#endif

        cr.archetypeStore.Init(world->m_wAllocator);

        assert(cr.archetypeStore.store);

        world->m_componentIndex.Insert(ti.id, std::move(cr));
        world->m_typeInfos.Insert(ti.id, &ti);

    }

}