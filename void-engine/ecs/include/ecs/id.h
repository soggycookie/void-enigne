#pragma once
#include "ecs_pch.h"

namespace ECS
{
    using EntityId = uint64_t;
    using LoEntityId = uint32_t;
    using HiEntityId = uint32_t;
    using GenCount = uint16_t;

#define ENTITY_ID_MASK      0xFFFFFFFFULL
#define ENTITY_GEN_MASK     0xFFFFULL

#define LO_ENTITY_ID(x) \
    ((uint32_t)((x) & ENTITY_ID_MASK))

#define HI_ENTITY_ID(x) \
    ((uint32_t)(((x) >> 32) & ENTITY_ID_MASK))

#define ENTITY_GEN_COUNT(x) \
    ((uint16_t)(((x) >> 32) & ENTITY_GEN_MASK))

#define MAKE_ENTITY_ID(lo, hi) \
    ((((uint64_t)(hi) & ENTITY_ID_MASK) << 32) | \
     ((uint64_t)(lo)  & ENTITY_ID_MASK))

#define INCRE_GEN_COUNT(x) \
        MAKE_ENTITY_ID( \
            LO_ENTITY_ID(x), \
            (uint16_t)(ENTITY_GEN_COUNT(x) + 1) \
        )

    class Id
    {
    protected:

        Id(EntityId id)
            : m_id(id)
        {
        }
        
        Id(LoEntityId lowId, HiEntityId highId)
            : m_id(MAKE_ENTITY_ID(lowId, highId))
        {
        }

        virtual ~Id() = default;

        Id(Id&& other) = default;
        Id(const Id& other) = default;

        Id& operator=(const Id& other) = default;
        Id& operator=(Id&& other) = default;
    
    public:
        EntityId GetFullId() const
        {
            return m_id;
        }

        LoEntityId GetLowId() const
        {
            return LO_ENTITY_ID(m_id);
        }
        
        HiEntityId GetHighId() const
        {
            return HI_ENTITY_ID(m_id);
        }

        GenCount GetGenCount() const
        {
            return ENTITY_GEN_COUNT(m_id);
        }

        void IncreGenCount()
        {
            if(isValid())
            {
                m_id = INCRE_GEN_COUNT(m_id);
            }
        }

        bool isValid() const
        {
            return LO_ENTITY_ID(m_id) != 0;
        }

    protected:
        EntityId m_id;
    };


}