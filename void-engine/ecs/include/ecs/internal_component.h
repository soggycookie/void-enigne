#pragma once
#include "ecs_type.h"
#include "system_meta.h"

namespace ECS
{
    //component entity id
    constexpr EntityId EcsNameId = 1;
    constexpr EntityId EcsSystemId = 2;
    constexpr EntityId EcsPhaseId = 3;
    constexpr EntityId EcsArchetypeId = 4;
    constexpr EntityId ChildOfId = 5;
    constexpr EntityId DependOnId = 6;
    constexpr EntityId EcsPipelineId = 7;
    constexpr EntityId EcsComponentId = 8; 
    constexpr EntityId EcsQueryId = 9; 
    constexpr EntityId ToggleId = 10;


    //internal components
    struct EcsName
    {
        EcsName() = default;

        char name[16];
    };
    ECS_COMPONENT(EcsName);

    struct EcsSystem
    {
        EcsSystem() = default;

        SystemCallback sc;            
    };
    ECS_COMPONENT(EcsSystem);

    //internal tag

    struct EcsPhase
    {
    };
    ECS_COMPONENT(EcsPhase);

    struct EcsArchetype
    {
    };
    ECS_COMPONENT(EcsArchetype);

    struct EcsPipeline
    {
    };
    ECS_COMPONENT(EcsPipeline);

    struct EcsComponent
    {
    };
    ECS_COMPONENT(EcsComponent);
    
    struct EcsQuery
    {
    };
    ECS_COMPONENT(EcsQuery);

    //internal pair
    struct ChildOf
    {
    };
    ECS_COMPONENT(ChildOf);

    struct DependOn
    {
    };
    ECS_COMPONENT(DependOn);

    struct Toggle
    {
    };
    ECS_COMPONENT(Toggle);

}