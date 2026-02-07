#pragma once
#include "ecs_pch.h"
#include "ds/hash_map.h"


template<typename T>
struct ComponentName;

#define ECS_COMPONENT(T) \
        template<> \
        struct ComponentName<T> \
        { \
            static constexpr const char* name = #T; \
        }; 


namespace ECS
{
    using EntityId = uint64_t;
    using LoEntityId = uint32_t;
    using HiEntityId = uint32_t;
    using GenCount = uint16_t;
    using ComponentId = EntityId;

    inline EntityId MakePair(EntityId first, EntityId sec)
    {
        constexpr uint32_t mask = 0xFFFFFFFFULL;

        LoEntityId f = first & mask;
        LoEntityId s = (sec >> 32) & mask;

        EntityId pair = (EntityId) f | ((EntityId)s << 32);

        return pair;
    }

    //inline ComponentId GetNextComponentId()
    //{
    //    static ComponentId id = 0;
    //    return id++;
    //}


    //template<typename T>
    //ComponentId GetComponentId()
    //{
    //    static ComponentId id = GetNextComponentId();
    //    return id;
    //}

    class World;
    template<typename T>
    struct ComponentTypeId
    {
        static ComponentId id;

        static void Id(ComponentId cid)
        {
            id = cid;
        }
    };

    template<typename T>
    ComponentId ComponentTypeId<T>::id = 0;

    template<typename T>
    struct Store
    {
        T* store;
        uint32_t count;
        uint32_t capacity;

        void Init(WorldAllocator& wAllocator)
        {
            uint32_t storeCapacity = 8;
            count = 0;
            store =
                PTR_CAST(wAllocator.AllocN(sizeof(T), storeCapacity, storeCapacity), T);
            capacity = storeCapacity;
        }

        void Grow(WorldAllocator& wAllocator)
        {
            uint32_t newStoreCapacity = capacity * 2;
            T* newStore =
                PTR_CAST(wAllocator.AllocN(sizeof(T), newStoreCapacity, newStoreCapacity), T);

            std::memcpy(newStore, store, sizeof(T) * capacity);

            wAllocator.Free(sizeof(T) * capacity, store);

            store = newStore;
            capacity = newStoreCapacity;
        }

        void Add(T id)
        {
            store[count] = id;
            ++count;
        }

        void Destroy(WorldAllocator& wAllocator)
        {
            wAllocator.Free(sizeof(T) * capacity, store);
        }
    };

    struct ComponentSet
    {
        ComponentId* idArr;
        uint32_t count;

        ComponentSet() = default;

        ~ComponentSet() = default;

        ComponentSet(ComponentSet&& other) noexcept
        {
            idArr = other.idArr;
            count = other.count;

            other.idArr = nullptr;
            other.count = 0;            
        }

        ComponentSet(const ComponentSet& other)
        {
            idArr = other.idArr;
            count = other.count;
        }

        ComponentSet& operator=(ComponentSet&& other) noexcept
        {
            idArr = other.idArr;
            count = other.count;

            other.idArr = nullptr;
            other.count = 0;

            return *this;
        }

        ComponentSet& operator=(const ComponentSet& other)
        {
            idArr = other.idArr;
            count = other.count;

            return *this;
        }

        bool operator==(const ComponentSet& other)
        {
            if(count != other.count)
            {
                return false;
            }

            for(uint32_t i = 0; i < count; i++)
            {
                if(idArr[i] != other.idArr[i])
                {
                    return false;
                }
            }

            return true;
        }

        bool operator!=(const ComponentSet& other)
        {
            if(count == other.count)
            {
                return true;
            }

            for(uint32_t i = 0; i < count; i++)
            {
                if(idArr[i] != other.idArr[i])
                {
                    return true;
                }
            }

            return false;
        }

        uint64_t Hash() const
        {
            uint64_t h = 0;
            for(uint32_t i = 0; i < count; i++)
            {
                h += HashU64(idArr[i]);
            }

            h /= count;

            return h;
        }

        void Sort(){
            std::sort(idArr, (idArr + count));
        }

        int32_t Search(ComponentId id)
        {
            ComponentId* v = std::lower_bound(idArr, (idArr + count), id);
            
            if(v == (idArr + count) || *v != id)
            {
                return -1;
            }

            return static_cast<int32_t>(v - idArr);
        }

        void Alloc(WorldAllocator& wAllocator, uint32_t capacity)
        {
            idArr = PTR_CAST(wAllocator.Alloc(capacity * sizeof(ComponentId)), ComponentId);
        }

        void Free(WorldAllocator& wAllocator)
        {
            wAllocator.Free(sizeof(ComponentId) * count, idArr);
        }
    };

    template<>
    struct Hash<ComponentSet>
    {
        static uint64_t Value(const ComponentSet& v)
        {
            return v.Hash();
        }        
    };

    using CtorHook = void (*)(void* dest);
    using CopyCtorHook = void (*)(void* dest, const void* src);
    using MoveCtorHook = void (*)(void* dest, void* src);
    using DtorHook     = void (*)(void* src);

    using AddEventHook = void (*)();
    using RemoveEventHook = void (*)();
    using SetEventHook = void (*)(void* dest);

    struct TypeHook
    {
        void (*ctor)(void* dest);
        void (*copyCtor)(void* dest, const void* src);
        void (*moveCtor)(void* dest, void* src);
        void (*dtor)(void* src);

        void (*onAdd)();
        void (*onRemove)();
        void (*onSet)(void* dest);
    };

#define COMPONENT_TYPE      1 << 0
#define TAG_TYPE            1 << 1
#define PAIR_TYPE           1 << 2
#define TYPE_HAS_DATA       1 << 3
#define EXCLUSIVE_PAIR      1 << 4

    struct TypeInfo
    {
        ComponentId id;
        uint32_t alignment;
        uint32_t size;
        TypeHook hook;
        uint32_t flags;

        bool HasData()
        {
            return (flags & TYPE_HAS_DATA) > 0;
        }

    };

    struct Column
    {
        void* data;
        TypeInfo* typeInfo;
    };

    using ComponentDiff = ComponentSet;
    using ArchetypeId = uint32_t;

    constexpr uint32_t DefaultArchetypeCapacity = 4;

    struct Archetype
    {
        ArchetypeId id;
        uint32_t count;
        uint32_t capacity;
        uint32_t flags;
        Column* columns;
        EntityId* entities;
        ComponentSet components;
        uint32_t* componentMap;
        HashMap<ComponentId, Archetype*> addEdges;
        HashMap<ComponentId, Archetype*> removeEdges;

        Archetype()
            : id(0), count(0), capacity(0), flags(0),
            columns(nullptr), entities(nullptr), components(), addEdges(), removeEdges()
        {
        }

        Archetype(Archetype&& other) noexcept
        {
            id = other.id;
            count = other.count;
            capacity = other.capacity;
            flags = other.flags;
            columns = other.columns;
            entities = other.entities;
            components = std::move(other.components);
            addEdges = std::move(other.addEdges);
            removeEdges = std::move(other.removeEdges);

            other.columns = nullptr;
            other.entities = nullptr;
            other.components.idArr = nullptr;
            other.components.count = 0;
        }

        Archetype& operator=(Archetype&& other) noexcept
        {
            columns = other.columns;
            entities = other.entities;
            components = std::move(other.components);
            addEdges = std::move(other.addEdges);
            removeEdges = std::move(other.removeEdges);

            other.columns = nullptr;
            other.entities = nullptr;
            other.components.idArr = nullptr;
            other.components.count = 0;

            return *this;
        }

    };

    inline ArchetypeId GetArchetypeId()
    {
        static ArchetypeId id = 0;

        return ++id;
    }

    struct ComponentRecord
    {
        ComponentId id;
        Store<Archetype*> archetypeStore;
        TypeInfo* typeInfo;
    };

    struct EntityRecord
    {
        Archetype* archetype;
        uint32_t row;
        uint32_t dense;
        char name[16];
    };

    struct EntityDesc
    {
        EntityId id;
        EntityId parent;
        ComponentSet add;
        Store<void*> componentData;
        const char* name;
    };
}

