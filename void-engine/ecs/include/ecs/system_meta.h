#pragma once
#include "ecs_pch.h"

namespace ECS
{
    class World;

    struct Iterator
    {
        World* world;
        double deltaTime;
    };

    template<typename T>
    constexpr bool is_iterator_ptr_v = std::is_same_v<T, Iterator*>;

    template<typename T>
    using decay_t = typename std::remove_const<typename std::remove_reference<T>::type>::type;

    template<typename T, typename... Components>
    struct is_in_component_list;

    template<typename T>
    struct is_in_component_list<T> : std::false_type {};

    template<typename T, typename First, typename... Rest>
    struct is_in_component_list<T, First, Rest...> 
        : std::conditional_t<
            std::is_same_v<decay_t<T>, decay_t<First>>,
            std::true_type,
            is_in_component_list<T, Rest...>
        > {};

    template<typename T, typename... Components>
    struct index_of;
    
    template<typename T>
    struct index_of<T>
    {
        static constexpr uint32_t value = 0;
    };

    template<typename T, typename First, typename... Rest>
    struct index_of<T, First, Rest...> 
    {
        static constexpr uint32_t value = 
            std::is_same_v<decay_t<T>, decay_t<First>> ? 0 : 1 + index_of<T, Rest...>::value;
    };

    template<typename T, typename... Components>
    constexpr uint32_t index_of_v = index_of<T, Components...>::value;

    struct SystemCallback
    {
        void* funcPtr;
        void (*invoker)(void*, Iterator*, void**);
    
        void Execute(Iterator* it, void** components)
        {
            invoker(funcPtr, it, components);
        }
    };

    template<typename FuncArgs, typename... Components>
    FuncArgs GetArgs(Iterator* it, void** components)
    {
        if constexpr(is_iterator_ptr_v<FuncArgs>)
        {
            return it;
        }
        else
        {
            constexpr uint32_t idx = index_of_v<FuncArgs, Components...>;
            using ComponentType = decay_t<FuncArgs>;

            if constexpr (std::is_const_v<std::remove_reference_t<FuncArgs>>)
            {
                return *static_cast<const ComponentType*>(components[idx]);
            }
            else
            {
                return *static_cast<ComponentType*>(components[idx]);
            }
        }
    }

    template<typename... Components, typename... FuncArgs>
    SystemCallback CreateSystemCallback(void (*func)(FuncArgs...))
    {
        static_assert((... && 
                      (is_iterator_ptr_v<FuncArgs> || 
                      (is_in_component_list<FuncArgs, Components...>::value) &&
                      std::is_reference_v<FuncArgs>
                      )), "Invalid system parameters!");

        SystemCallback cb;
        cb.funcPtr = CAST(func, void*);
        cb.invoker = [](void* fn, Iterator* it, void** components)
            {
                auto actualFunc = CAST(fn, void (*)(FuncArgs...));
                actualFunc(GetArgs<FuncArgs, Components...>(it, components)...);
            };

        return cb;
    }
}