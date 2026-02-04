
namespace ECS
{
    template<typename Component>
    TypeInfoBuilder<Component> World::RegisterComponent()
    {
        TypeInfo* ti = new (m_wAllocator.Alloc(sizeof(TypeInfo))) TypeInfo();
        ti->size = sizeof(Component);
        ti->alignment = alignof(Component);
        ti->id = GetComponentId<Component>();
        ti->entityId = 0;
        ti->name = typeid(Component).name();

        auto e = CreateEntity(ti->name);
        ti->entityId = e.GetFullId();

        assert(std::is_destructible_v<Component>);
        assert(std::is_trivially_constructible_v<Component>);

        TypeInfoBuilder<Component> tiBuilder{*ti};

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

        if(m_componentStore.capacity == m_componentStore.count)
        {
            m_componentStore.Grow(m_wAllocator);
        }
        m_componentStore.Add(ti->id);

        ComponentRecord cr;
        cr.id = ti->id;
        cr.typeInfo = ti;
        cr.archetypeCache.capacity = 4;
        cr.archetypeCache.archetypeArr = PTR_CAST(m_wAllocator.Alloc(sizeof(Archetype*) * cr.archetypeCache.capacity), Archetype*);
        cr.archetypeCache.count = 0;

        assert(cr.archetypeCache.archetypeArr);

        m_componentRecords.Insert(ti->id, std::move(cr));
        m_typeInfos.Insert(ti->id, ti);
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

        int32_t idx = r->archetype->components.Search(GetComponentId<Component>());

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

        int32_t idx = r->archetype->components.Search(GetComponentId<Component>());

        assert(idx != -1);

        Component& component = *CAST_OFFSET_ELEMENT(r->archetype->columns[idx].data, Component, sizeof(Component), r->row);

        return component;
    }

    template<typename... Components, typename... FuncArgs>
    void World::System(void (*func)(FuncArgs...))
    {
        ComponentId ids[] = {GetComponentId<Components>()...};
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
            ComponentRecord& cr = m_componentRecords.GetValue(ids[idx]);

            for(uint32_t aIdx = 0; aIdx < cr.archetypeCache.count; aIdx++)
            {
                Archetype* archetype = cr.archetypeCache.archetypeArr[aIdx];
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

    template<typename... FuncArgs>
    void World::Each(void (*func)(FuncArgs...))
    {
        ComponentId ids[] = {GetComponentId<std::remove_const_t<std::remove_reference_t<FuncArgs>>>()...};
        uint32_t count = sizeof...(FuncArgs);

        ArchetypeLinkedList* node = ArchetypeLinkedList::Alloc(m_wAllocator);

        ArchetypeLinkedList* head = node;

        SystemCallback sc = CreateSystemCallback<std::remove_const_t<std::remove_reference_t<FuncArgs>>..., FuncArgs...>(func);

        for(uint32_t idx = 0; idx < 1; idx++)
        {
            ComponentRecord& cr = m_componentRecords.GetValue(ids[idx]);

            for(uint32_t aIdx = 0; aIdx < cr.archetypeCache.count; aIdx++)
            {
                Archetype* archetype = cr.archetypeCache.archetypeArr[aIdx];
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

            Iterator* it = nullptr;

            for(uint32_t row = 0; row < archetype->count; row++)
            {
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
                sc.Execute(it, componentsData);
            }
            ArchetypeLinkedList* freeNode = head;
            head = head->next;

            ArchetypeLinkedList::Free(m_wAllocator, freeNode);
        }
    }
}