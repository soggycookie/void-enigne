
namespace ECS
{
    template<typename Component>
    TypeInfoBuilder<Component> World::RegisterComponent()
    {
        TypeInfo* ti = new (m_wAllocator.Alloc(sizeof(TypeInfo))) TypeInfo();
        ti->size = sizeof(Component);
        ti->alignment = alignof(Component);
        ti->flags = (COMPONENT_TYPE | TYPE_HAS_DATA);
        ti->id = 0;
        //Entity e = CreateEntity(ComponentName<Component>::name, 0);
        //ti->id = e.GetFullId();

        assert(std::is_destructible_v<Component>);
        assert(std::is_trivially_constructible_v<Component>);

        TypeInfoBuilder<Component> tiBuilder{*ti, this};

        tiBuilder.Ctor(
            [](void* dest)
            {
                new (dest) Component();
            }
        );

        if constexpr(std::is_move_constructible_v<Component>)
        {
            tiBuilder.MoveCtor(
                [](void* dest, void* src)
                {
                    new (dest) Component(std::move(*PTR_CAST(src, Component)));
                }
            );
        }
        else if constexpr(!std::is_trivially_constructible_v<Component>)
        {
            tiBuilder.CopyCtor(
                [](void* dest, const void* src)
                {
                    new (dest) Component(*PTR_CAST(src, Component));
                }
            );
        }

        if constexpr(!std::is_trivially_destructible_v<Component>)
        {
            tiBuilder.Dtor(
                [](void* src)
                {
                    Component* c = PTR_CAST(src, Component);
                    c->~Component();
                }
            );
        }


        tiBuilder.AddEvent(
            []()
            {
                std::cout << "Add component " << ComponentName<Component>::name << std::endl;
            }
        );

        tiBuilder.RemoveEvent(
            []()
            {
                std::cout << "Remove component" << ComponentName<Component>::name << std::endl;
            }
        );

        tiBuilder.SetEvent(
            [](void* dest)
            {
                std::cout << "Set component" << ComponentName<Component>::name << std::endl;
            }
        );

        //if(m_componentStore.capacity == m_componentStore.count)
        //{
        //    m_componentStore.Grow(m_wAllocator);
        //}
        //m_componentStore.Add(e.GetFullId());

        //ComponentRecord cr;
        //cr.id = ti->id;
        //cr.typeInfo = ti;

        //cr.archetypeStore.Init(m_wAllocator);

        //assert(cr.archetypeStore.store);

        //m_componentIndex.Insert(ti->id, std::move(cr));
        //m_typeInfos.Insert(ti->id, ti);
        return tiBuilder;
    }

    template<typename Tag>
    TypeInfoBuilder<Tag> World::RegisterTag()
    {
        TypeInfo* ti = new (m_wAllocator.Alloc(sizeof(TypeInfo))) TypeInfo();
        ti->size = sizeof(Tag);
        ti->alignment = alignof(Tag);
        ti->flags = (TAG_TYPE);
        //Entity e = CreateEntity(ComponentName<Tag>::name, 0);
        ti->id = 0;

        assert(std::is_destructible_v<Tag>);
        assert(std::is_trivially_constructible_v<Tag>);
        assert(ti->size == 1 && "Tag can not have data");

        TypeInfoBuilder<Tag> tiBuilder{*ti, this};


        tiBuilder.AddEvent(
            []()
            {
                std::cout << "Add tag " << ComponentName<Tag>::name << std::endl;
            }
        );

        tiBuilder.RemoveEvent(
            []()
            {
                std::cout << "Remove tag" << ComponentName<Tag>::name << std::endl;
            }
        );

        tiBuilder.SetEvent(
            [](void* dest)
            {
                std::cout << "Set tag" << ComponentName<Tag>::name << std::endl;
            }
        );

        //if(m_componentStore.capacity == m_componentStore.count)
        //{
        //    m_componentStore.Grow(m_wAllocator);
        //}
        //m_componentStore.Add(ti->id);

        //ComponentRecord cr;
        //cr.id = ti->id;
        //cr.typeInfo = ti;
        //cr.archetypeStore.Init(m_wAllocator);

        //assert(cr.archetypeStore.store);

        //m_componentIndex.Insert(ti->id, std::move(cr));
        //m_typeInfos.Insert(ti->id, ti);
        return tiBuilder;
    }

    template<typename Pair>
    TypeInfoBuilder<Pair> World::RegisterPair(bool isExclusive, bool isToggle)
    {
        TypeInfo* ti = new (m_wAllocator.Alloc(sizeof(TypeInfo))) TypeInfo();
        ti->size = sizeof(Pair);
        ti->alignment = alignof(Pair);

        ti->flags = PAIR_TYPE;

        if(ti->size > 1 || isToggle)
        {
            ti->flags |= TYPE_HAS_DATA;
        }

        if(isExclusive)
        {
            ti->flags |= EXCLUSIVE_PAIR;
        }

        //Entity e = CreateEntity(ComponentName<Pair>::name, 0);
        ti->id = 0;

        assert(std::is_destructible_v<Pair>);
        assert(std::is_trivially_constructible_v<Pair>);

        TypeInfoBuilder<Pair> tiBuilder{*ti, this};


        if(ti->size > 1)
        {
            tiBuilder.Ctor(
                [](void* dest)
                {
                    new (dest) Pair();
                }
            );

            if constexpr(std::is_move_constructible_v<Pair>)
            {
                tiBuilder.MoveCtor(
                    [](void* dest, void* src)
                    {
                        new (dest) Pair(std::move(*PTR_CAST(src, Pair)));
                    }
                );
            }
            else if constexpr(!std::is_trivially_constructible_v<Pair>)
            {
                tiBuilder.CopyCtor(
                    [](void* dest, const void* src)
                    {
                        new (dest) Pair(*PTR_CAST(src, Pair));
                    }
                );
            }

            if constexpr(!std::is_trivially_destructible_v<Pair>)
            {
                tiBuilder.Dtor(
                    [](void* src)
                    {
                        Pair* c = PTR_CAST(src, Pair);
                        c->~Pair();
                    }
                );
            }
        }

        tiBuilder.AddEvent(
            []()
            {
                std::cout << "Add pair " << ComponentName<Pair>::name << std::endl;
            }
        );

        tiBuilder.RemoveEvent(
            []()
            {
                std::cout << "Remove pair" << ComponentName<Pair>::name << std::endl;
            }
        );

        tiBuilder.SetEvent(
            [](void* dest)
            {
                std::cout << "Set pair" << ComponentName<Pair>::name << std::endl;
            }
        );

        //if(m_componentStore.capacity == m_componentStore.count)
        //{
        //    m_componentStore.Grow(m_wAllocator);
        //}
        //m_componentStore.Add(ti->id);

        //ComponentRecord cr;
        //cr.id = ti->id;
        //cr.typeInfo = ti;

        //cr.archetypeStore.Init(m_wAllocator);

        //assert(cr.archetypeStore.store);

        //m_componentIndex.Insert(ti->id, std::move(cr));
        //m_typeInfos.Insert(ti->id, ti);
        return tiBuilder;
    }

    template<typename Component>
    void World::AddComponent(EntityId id)
    {
        EntityRecord* r = m_entityIndex.GetPageData(id);

        assert(r);

        if(r->archetype)
        {
            int32_t s = r->archetype->components.Search(GetComponentId<Component>());

            assert(s == -1);
        }

        uint32_t count = 1;
        uint32_t* arr = PTR_CAST(m_wAllocator.Alloc(sizeof(ComponentId) * count), ComponentId);

        ComponentDiff cdiff;
        cdiff.idArr = arr;
        cdiff.count = count;
        cdiff.idArr[0] = GetComponentId<Component>();

        Archetype* destArchetype = GetOrCreateArchetype_Add(r->archetype, std::move(cdiff));

        if(destArchetype->count == destArchetype->capacity)
        {
            GrowArchetype(*destArchetype);
        }

        //empty entity
        if(!r->archetype)
        {
            for(uint32_t i = 0; i < destArchetype->components.count; i++)
            {
                TypeInfo& ti = *(destArchetype->columns[i].typeInfo);
                void* dataAddr = OFFSET(destArchetype->columns[i].data, destArchetype->count * ti.size);
                ti.hook.ctor(dataAddr);
            }
        }
        //at least 1 component
        else
        {
            //SWAP BACK IN SRC ARCHETYPE
            SwapBack(*r);

            for(uint32_t i = 0; i < destArchetype->components.count; i++)
            {
                Column& destCol = destArchetype->columns[i];
                TypeInfo& ti = *destCol.typeInfo;

                void* dest = OFFSET(destCol.data, ti.size * destArchetype->count);

                int32_t srcIndex = r->archetype->components.Search(destArchetype->components.idArr[i]);

                if(srcIndex == -1)
                {
                    ti.hook.ctor(dest);
                }
                else
                {
                    Column& srcCol = r->archetype->columns[srcIndex];
                    void* src = OFFSET(srcCol.data, ti.size * r->row);

                    if(ti.hook.moveCtor)
                    {
                        ti.hook.moveCtor(dest, src);
                    }
                    else if(ti.hook.copyCtor)
                    {
                        ti.hook.copyCtor(dest, src);
                    }
                    else
                    {
                        std::memcpy(dest, src, ti.size);
                    }

                    if(ti.hook.dtor)
                    {
                        ti.hook.dtor(src);
                    }
                }
            }

        }

        if(r->archetype)
        {
            --r->archetype->count;
        }

        destArchetype->entities[destArchetype->count] = id;
        r->archetype = destArchetype;
        r->row = destArchetype->count;
        ++destArchetype->count;

        TypeInfo& ti = *m_typeInfos[GetComponentId<Component>()];
        ti.hook.onAdd();
    }

    template<typename Component>
    void World::RemoveComponent(EntityId id)
    {
        EntityRecord* r = m_entityIndex.GetPageData(id);

        assert(r);
        assert(r->archetype);
        Archetype* srcArchetype = r->archetype;
        int32_t s = srcArchetype->components.Search(GetComponentId<Component>());

        assert(s != -1);

        uint32_t count = 1;
        uint32_t* arr = PTR_CAST(m_wAllocator.Alloc(sizeof(ComponentId) * count), ComponentId);

        ComponentDiff cdiff;
        cdiff.idArr = arr;
        cdiff.count = count;
        cdiff.idArr[0] = GetComponentId<Component>();

        Archetype* destArchetype = GetOrCreateArchetype_Remove(srcArchetype, std::move(cdiff));

        if(destArchetype->count == destArchetype->capacity)
        {
            GrowArchetype(*destArchetype);
        }

        //SWAP BACK IN SRC ARCHETYPE
        SwapBack(*r);

        for(uint32_t i = 0; i < srcArchetype->components.count; i++)
        {
            Column& srcCol = srcArchetype->columns[i];
            TypeInfo& ti = *srcCol.typeInfo;
            void* src = OFFSET(srcCol.data, ti.size * r->row);

            int32_t destIndex = destArchetype->components.Search(srcArchetype->components.idArr[i]);

            if(destIndex != -1)
            {
                Column& destCol = destArchetype->columns[destIndex];
                void* dest = OFFSET(destCol.data, ti.size * destArchetype->count);

                if(ti.hook.moveCtor)
                {
                    ti.hook.moveCtor(dest, src);
                }
                else if(ti.hook.copyCtor)
                {
                    ti.hook.copyCtor(dest, src);
                }
                else
                {
                    std::memcpy(dest, src, ti.size);
                }
            }

            if(ti.hook.dtor)
            {
                ti.hook.dtor(src);
            }
        }

        destArchetype->entities[destArchetype->count] = id;
        --r->archetype->count;
        r->archetype = destArchetype;
        r->row = destArchetype->count;
        ++destArchetype->count;

        TypeInfo& ti = *m_typeInfos[GetComponentId<Component>()];
        ti.hook.onRemove();
    }

    template<typename Component>
    void World::Set(EntityId id, Component&& c)
    {
        EntityRecord* r = m_entityIndex.GetPageData(id);
        assert(r);
        assert(r->archetype);
        assert(r->dense);

        int32_t idx = r->archetype->components.Search(ComponentTypeId<Component>::id);

        assert(idx != -1);

        Component& component = *CAST_OFFSET_ELEMENT(r->archetype->columns[idx].data, Component, sizeof(Component), r->row);

        component = std::move(c);
    }

    template<typename Component>
    Component& World::Get(EntityId id)
    {
        EntityRecord* r = m_entityIndex.GetPageData(id);
        assert(r);
        assert(r->archetype);
        assert(r->dense);

        int32_t idx = r->archetype->components.Search(ComponentTypeId<Component>::id);

        assert(idx != -1);

        Component& component = *CAST_OFFSET_ELEMENT(r->archetype->columns[idx].data, Component, sizeof(Component), r->row);

        return component;
    }

    template<typename... Components, typename... FuncArgs>
    void World::System(void (*func)(FuncArgs...))
    {
        ComponentId ids[] = {ComponentTypeId<Components>::id...};
        uint32_t count = sizeof...(Components);
        ComponentSet componentSet;
        componentSet.Alloc(m_wAllocator, count);
        componentSet.count = count;

        std::memcpy(componentSet.idArr, ids, count * sizeof(ComponentId));

        ArchetypeLinkedList* node = ArchetypeLinkedList::Alloc(m_wAllocator);

        ArchetypeLinkedList* head = node;

        SystemCallback sc = CreateSystemCallback<Components..., FuncArgs...>(func);

        for(uint32_t idx = 0; idx < 1; idx++)
        {
            ComponentRecord& cr = m_componentIndex.GetValue(ids[idx]);

            for(uint32_t aIdx = 0; aIdx < cr.archetypeStore.count; aIdx++)
            {
                Archetype* archetype = cr.archetypeStore.store[aIdx];
                bool skip = false;
                assert(archetype);

                for(uint32_t remainIdx = 1; remainIdx < count; remainIdx++)
                {
                    //Archetype does not contain the same set of components
                    if(archetype->components.Search(ids[remainIdx]) == -1)
                    {
                        skip = true;
                        break;
                    }
                }

                if(!skip)
                {
                    node->archetype = archetype;
                    ArchetypeLinkedList* newNode = ArchetypeLinkedList::Alloc(m_wAllocator);
                    node->next = newNode;
                    node = newNode;
                }
            }
        }

        sc.components = std::move(componentSet);
        sc.archetypeList = head;

        if(m_systemStore.capacity == m_systemStore.count)
        {
            m_systemStore.Grow(m_wAllocator);
        }

        m_systemStore.Add(sc);
    }

    template<typename... Components, typename... FuncArgs>
    void World::Each(void (*func)(FuncArgs...))
    {
        ComponentId ids[] = {ComponentTypeId<decay_t<Components>>::id...};
        uint32_t count = sizeof...(Components);

        ArchetypeLinkedList* node = ArchetypeLinkedList::Alloc(m_wAllocator);

        ArchetypeLinkedList* head = node;

        SystemCallback sc = CreateSystemCallback<Components..., FuncArgs...>(func);

        for(uint32_t idx = 0; idx < 1; idx++)
        {
            ComponentRecord& cr = m_componentIndex.GetValue(ids[idx]);

            for(uint32_t aIdx = 0; aIdx < cr.archetypeStore.count; aIdx++)
            {
                Archetype* archetype = cr.archetypeStore.store[aIdx];
                bool skip = false;
                assert(archetype);

                for(uint32_t remainIdx = 1; remainIdx < count; remainIdx++)
                {
                    //Archetype does not contain the same set of components
                    if(archetype->components.Search(ids[remainIdx]) == -1)
                    {
                        skip = true;
                        break;
                    }
                }

                if(!skip)
                {
                    node->archetype = archetype;
                    ArchetypeLinkedList* newNode = ArchetypeLinkedList::Alloc(m_wAllocator);
                    node->next = newNode;
                    node = newNode;
                }
            }
        }

        void* componentsData[sizeof...(FuncArgs)];

        while(head->archetype)
        {
            Archetype* archetype = head->archetype;

            for(uint32_t row = 0; row < archetype->count; row++)
            {
                QueryIterator it;
                it.archetype = archetype;
                it.world = this;
                it.row = row;

                for(uint32_t idx = 0; idx < count; idx++)
                {
                    uint32_t colIdx = ids[idx];

                    assert(colIdx != -1);

                    Column& col = archetype->columns[colIdx];
                    TypeInfo& ti = *col.typeInfo;
                    void* comData = OFFSET(col.data, ti.size * row);
                    componentsData[idx] = comData;
                }

                //EXECUTE
                sc.Execute(&it, componentsData);
            }
            ArchetypeLinkedList* freeNode = head;
            head = head->next;

            ArchetypeLinkedList::Free(m_wAllocator, freeNode);
        }
    }
}