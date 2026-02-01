#pragma once
#include "ecs_pch.h"
#include "ds/hash_map.h"

namespace ECS
{
    using ComponentId = uint32_t;

    inline ComponentId GetNextComponentId()
    {
        static ComponentId id = 0;
        return id++;
    }

    template<typename T>
    ComponentId GetComponentId()
    {
        static ComponentId id = GetNextComponentId();
        return id;
    }

    struct ComponentSet
    {
        ComponentId* ids;
        uint32_t count;

        bool operator==(const ComponentSet& other)
        {
            if(count != other.count)
            {
                return false;
            }

            for(uint32_t i = 0; i < count; i++)
            {
                if(ids[i] != other.ids[i])
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
                if(ids[i] != other.ids[i])
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
                h += HashU64(ids[i]);
            }

            h /= count;

            return h;
        }

        void Sort(){
            std::sort(ids, (ids + count));
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

    using CopyCtorHook = void (*)(void* src, void* dest);
    using MoveCtorHook = void (*)(void* src, void* dest);
    using DtorHook     = void (*)(void* src);

    using AddEventHook = void (*)();
    using RemoveEventHook = void (*)();
    using SetEventHook = void (*)(void* dest);

    struct TypeHook
    {
        void (*copyCtor)(void* src, void* dest);
        void (*moveCtor)(void* src, void* dest);
        void (*dtor)(void* src);

        void (*onAdd)();
        void (*onRemove)();
        void (*onSet)(void* dest);
    };

    struct TypeInfo
    {
        ComponentId id;
        uint32_t alignment;
        uint64_t size;
        EntityId entityId;
        const char* name;
        TypeHook hook;

        void CopyCtor(CopyCtorHook cctor)
        {
            hook.copyCtor = cctor;
        }

        void MoveCtor(MoveCtorHook mctor)
        {
            hook.moveCtor = mctor;
        }

        void Dtor(DtorHook dtor)
        {
            hook.dtor = dtor;
        }

        void AddEvent(AddEventHook e)
        {
            hook.onAdd = e;
        }
        
        void RemoveEvent(RemoveEventHook e)
        {
            hook.onRemove = e;
        }
        
        void SetEvent(SetEventHook e)
        {
            hook.onSet = e;
        }
    };

    struct Column
    {
        void* data;
        TypeInfo* typeInfo;
    };

    struct Archetype
    {
        uint32_t id;
        uint32_t count;
        Column* columns;
        EntityId* entities;
        ComponentSet components;
    };

    struct ArchetypeCache
    {
        Archetype** archetypes;
        uint32_t count;
    };

    struct ComponentRecord
    {
        ComponentId id;
        ArchetypeCache archetypeCache;
        TypeInfo* typeInfo;
    };

    struct EntityRecord
    {
        Archetype* archetype;
        uint32_t row;
        uint32_t dense;
        char name[16];
    };


}