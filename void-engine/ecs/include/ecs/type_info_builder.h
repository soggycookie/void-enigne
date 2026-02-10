#pragma once
#include "ecs_type.h"

namespace ECS
{
    class World;

    template<typename T = void>
    struct TypeInfoBuilder
    {
        TypeInfo& ti;
        World* world;

        TypeInfoBuilder<T>& Ctor(CtorHook ctor);

        TypeInfoBuilder<T>& CopyCtor(CopyCtorHook cctor);

        TypeInfoBuilder<T>& MoveCtor(MoveCtorHook mctor);

        TypeInfoBuilder<T>& Dtor(DtorHook dtor);

        TypeInfoBuilder<T>& AddEvent(AddEventHook e);

        TypeInfoBuilder<T>& RemoveEvent(RemoveEventHook e);

        TypeInfoBuilder<T>& SetEvent(SetEventHook e);

        TypeInfoBuilder<T>& Id(EntityId id);

        void Register(const char* name = nullptr);
    };


}