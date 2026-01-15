#pragma once
#include "void/pch.h"
#include "ecs_type.h"

namespace VoidEngine
{
    namespace ECS
    {
        
        class World;

        class Entity
        {
        public:
            Entity(EntityId id, World* scene)
                :id(id), scene(scene)
            {
            }

            EntityId id;
            World* scene;

            void Destroy();
            bool IsAlive();

        };

        class World
        {
        public:
            World();
            ~World();

            Entity CreateEntity();
            void DestroyEntity(EntityId id);

#define ADDED_GROW_RATIO 0.5f
            void EnsureCapacity(Archetype& archetype, uint32_t addedCount)
            {
                uint32_t totalCount = archetype.count + addedCount;

                if(archetype.capacity >= totalCount)
                {
                    return;
                }

                //can be overflow 
                uint32_t capacity = archetype.capacity == 0 ? 1 : archetype.capacity;

                float ratio = static_cast<float>(totalCount) / static_cast<float>(capacity);
                ratio += ADDED_GROW_RATIO;

                uint32_t newCapacity = std::round(capacity * ratio);

                AllocateArchetypeData(archetype, newCapacity);
            }

            void AllocateArchetypeData(Archetype& archetype, uint32_t capacity)
            {
                if(archetype.capacity >= capacity)
                {
                    return;
                }
                
                std::vector<uint8_t*> reallocAddrs(archetype.componentSet.GetCount(), nullptr);
                uint32_t dataExistCount = 0;                
                for(uint32_t i = 0; i < archetype.componentSet.GetCount(); i++)
                {
                    uint8_t* data = nullptr;
                    ComponentColumn& col = archetype.data.columns[i];
                    size_t totalSize = capacity * col.typeInfo.size;
#if defined(_MSC_VER)
                    data = static_cast<uint8_t*>(_aligned_malloc(totalSize, col.typeInfo.alignment));
#else
                    data = static_cast<uint8_t*>(std::aligned_alloc(col.typeInfo.alignment, totalSize));
#endif       
                    if(!data)
                    {
                        assert(0 && "Fail to allocate column data!");
                    }

                    if(col.colData)
                    {
                        ++dataExistCount;
                        reallocAddrs[i] = data;
                    }
                    else
                    {
#if defined(_MSC_VER)
                        _aligned_free(col.colData);
#else
                        std::free(col.colData);
#endif
                        col.colData = data;
                    }
                }


                if(dataExistCount == archetype.componentSet.GetCount())
                {
                    for(uint32_t i = 0; i < archetype.componentSet.GetCount(); i++)
                    {
                        ComponentColumn& col = archetype.data.columns[i];
                        uint8_t* srcAddr = col.colData;
                        uint8_t* destAddr = reallocAddrs[i];

                        if(col.typeInfo.isTriviallyCopyable)
                        {
                            std::memcpy(destAddr, srcAddr, archetype.count * col.typeInfo.size);
                            continue;
                        }
                        else if(col.typeInfo.isMoveContructible)
                        {
                            if(col.hook.move)
                            {
                                for(uint32_t element = 0; element < archetype.count; element++)
                                {
                                    uint8_t* elementSrcAddr = srcAddr + element * col.typeInfo.size;
                                    uint8_t* elementDestAddr = destAddr + element * col.typeInfo.size;
                                    col.hook.move(elementDestAddr , elementSrcAddr);

                                    if(!col.typeInfo.isTriviallyDestructible && col.hook.dtor)
                                    {
                                        col.hook.dtor(elementSrcAddr);
                                    }
                                }
                            }
                            else
                            {
                                assert(0 && "Move hook missing!");
                            }
                        }
                        else
                        {
                            if(col.hook.copy)
                            {
                                for(uint32_t element = 0; element < archetype.count; element++)
                                {
                                    uint8_t* elementSrcAddr = srcAddr + element * col.typeInfo.size;
                                    uint8_t* elementDestAddr = destAddr + element * col.typeInfo.size;
                                    col.hook.copy(elementDestAddr , elementSrcAddr);

                                    if(!col.typeInfo.isTriviallyDestructible && col.hook.dtor)
                                    {
                                        col.hook.dtor(elementSrcAddr);
                                    }
                                }
                            }
                            else
                            {
                                assert(0 && "Copy hook missing!");
                            }
                        }

                        #if defined(_MSC_VER)
                                    _aligned_free(col.colData);
                        #else
                                    std::free(col.colData);
                        #endif

                        col.colData = reallocAddrs[i];
                    }
                }
                else
                {
                    assert(0 && "Archetype corrupted!");
                }
                archetype.capacity = capacity;
            }


            void FreeArchetypeData(Archetype& type){}
            void EraseArchetype(Archetype* type){}


            ////////////////////////////////////////////////////////////////

            //NOTE: WE HAVE TO REWRITE THEM MOSTLY TO DEFER THE ARCHETYPE CHANGE

            ////////////////////////////////////////////////////////////////

            template<typename T>
            void Add(EntityId id, const T& data)
            {
                if(m_componentIndex.find(GetComponentId<T>()) == m_componentIndex.end())
                {
                    return;
                }
                
                auto it = m_entityRecords.find(id);
                if(it == m_entityRecords.end())
                {
                    return;
                }

                EntityRecord record = it->second;
                Archetype* srcArchetype = record.archetype;
                Archetype* destArchetype = ResolveDestArchetype<T>(srcArchetype);


                if(!destArchetype)
                {
                    assert(0 && "destArchetype is nullptr!");
                }

                if(destArchetype->capacity == destArchetype->count)
                {
                    EnsureCapacity(*destArchetype, srcArchetype->count);
                }

                SwapBack(record);
                for(uint32_t i = 0; i < destArchetype->componentSet.GetCount(); i++)
                {
                    const ComponentColumn& destCol = destArchetype->data.columns[i];
                    uint8_t* destAddr = destArchetype->data.columns[i].colData + 
                                        destArchetype->count * destCol.size;

                    auto it = srcArchetype->componentSet.Search(destArchetype->componentSet[i]);

                    if(it == srcArchetype->componentSet.End())
                    {
                        //placeholder
                        if(GetComponentId<T>() == destArchetype->componentSet[i])
                        {
                            destCol.hook.copy(destAddr, &data);
                        }                       
                    }
                    else
                    {
                        const ComponentColumn& srcCol = srcArchetype->data.columns[*it];
                        //swap back
                        uint8_t* srcAddr = srcCol.colData + (srcArchetype->count - 1) * srcCol.typeInfo.size;

                        if(destCol.typeInfo.isTriviallyCopyable)
                        {
                            std::memcpy(destAddr, srcAddr, destCol.size);
                        }
                        else if(destCol.typeInfo.isMoveContructible)
                        {
                            destCol.hook.move(destAddr, srcAddr);
                        }
                        else
                        {
                            destCol.hook.copy(destAddr, srcAddr);
                        }

                        if(!destCol.isTriviallyDestructible)
                        {
                            destCol.dtor(srcAddr);
                        }                    
                    }
                }

                if(srcArchetype)
                {
                    srcArchetype->data.entities.pop_back();
                    srcArchetype->count--;
                }

                destArchetype->data.entities.push_back(id);
                destArchetype->count++;
            }
            
            template<typename T>
            Archetype* ResolveDestArchetype(Archetype* srcArchetype)
            {
                Archetype* destArchetype = nullptr;
                ComponentSet destComponentSet;
                if(srcArchetype)
                {
                    auto edgeIt = srcArchetype->edges.find(GetComponentId<T>());
                    //edge exist
                    if(edgeIt != srcArchetype->edges.end())
                    {
                        ArchetypeEdge& edge = edgeIt->second;

                        //added archetype not exist
                        if(!edge.added)
                        {
                            //lazy load
                            destComponentSet = srcArchetype->componentSet;
                            destComponentSet.
                                Add(GetComponentId<T>()).
                                Sort();

                            auto mappedArchetypeIt = m_mappedArchetype.find(destComponentSet);

                            if(mappedArchetypeIt != m_mappedArchetype.end())
                            {
                                edge.added = mappedArchetypeIt->second;
                            }
                            else
                            {
                                edge.added = CreateAndGetArchetype(destComponentSet);
                            }
                        }
                        //added archetype exist
                        else
                        {
                            //NOTE: there maybe invalid ptr
                            destComponentSet = edge.added->componentSet;
                        }

                        destArchetype = edge.added;
                    }
                    //edge does not exist yet
                    else
                    {
                        //lazy load
                        destComponentSet = srcArchetype->componentSet;
                        destComponentSet.
                            Add(GetComponentId<T>()).
                            Sort();

                        auto mappedArchetypeIt = m_mappedArchetype.find(destComponentSet);

                        if(mappedArchetypeIt != m_mappedArchetype.end())
                        {
                            destArchetype = mappedArchetypeIt->second;
                        }
                        else
                        {
                            //Create Archetype
                            destArchetype = CreateAndGetArchetype(destComponentSet);
                        }

                        ArchetypeEdge edge{destArchetype, nullptr};
                        srcArchetype->edges.insert(GetComponentId<T>(), edge);
                    }
                }
                //entity not contains any archetype
                else
                {
                    destComponentSet = std::move(ComponentSet({GetComponentId<T>()}));
                    auto mappedArchetypeIt = m_mappedArchetype.find(destComponentSet);

                    if(mappedArchetypeIt != m_mappedArchetype.end())
                    {
                        destArchetype = mappedArchetypeIt->second;
                    }
                    else
                    {
                        //Create Archetype
                        destArchetype = CreateAndGetArchetype(destComponentSet);
                    }
                }

                return destArchetype;
            }

            void SwapBack(EntityRecord record)
            {
                if(!record.archetype || record.row == 0)
                {
                    return;
                }

                Archetype& archetype = *(record.archetype);
                uint32_t swappingIndex = record.row - 1;
                uint32_t backIndex = archetype.count - 1;

                EntityId swappingId = archetype.data.entities[swappingIndex];
                EntityId backId = archetype.data.entities[backIndex];

                auto it = m_entityRecords.find(backId);

                if(it != m_entityRecords.end())
                {
                    EntityRecord& swappedRecord = it->second;
                    swappedRecord.row = record.row;
                    
                    archetype.data.entities[swappingIndex] = backId;
                    archetype.data.entities[backIndex] = swappingId;
                }
                else
                {
                    return;
                }

                for(uint32_t i = 0; i < archetype.componentSet.GetCount(); i++)
                {
                    const ComponentColumn& col = archetype.data.columns[i];
                    uint8_t* swappingAddr = col.colData + swappingIndex * col.typeInfo.size;
                    uint8_t* backAddr = col.colData + backIndex * col.typeInfo.size;

                    //fix this later
                    uint8_t temp[256];

                    if(col.typeInfo.isTriviallyCopyable)
                    {
                        std::memcpy(temp, swappingAddr, col.typeInfo.size);
                        std::memcpy(swappingAddr, backAddr, col.typeInfo.size);
                        std::memcpy(backAddr, temp, col.typeInfo.size);
                    }
                    else if(col.typeInfo.isMoveContructible)
                    {
                        col.hook.move(temp, swappingAddr);
                        col.hook.move(swappingAddr, backAddr);
                        col.hook.move(backAddr, temp);
                    }
                    else
                    {
                        col.hook.copy(temp, swappingAddr);
                        col.hook.copy(swappingAddr, backAddr);
                        col.hook.copy(backAddr, temp);                    
                    }
                }
            }

#define DEFAULT_COMPONENT_CAPACITY 4

            template<typename T>
            Archetype* CreateAndGetArchetype(const ComponentSet& components)
            {
                auto mappedArchetypeIt = m_mappedArchetype.find(components);
                
                if(mappedArchetypeIt != m_mappedArchetype.end())
                {
                    return mappedArchetypeIt->second;
                }

                auto componentIndexIt = m_componentIndex.find(GetComponentId<T>());

                if(componentIndexIt == m_componentIndex.end())
                {
                    Register<T>();
                }

                Archetype* newArchetype = new Archetype();
                newArchetype->hashId = components.Hash();
                newArchetype->count = 0;
                newArchetype->capacity = 0;
                newArchetype->storeIndex = 0;
                newArchetype->componentSet = components;
                newArchetype->edges = {};
                newArchetype->data.entities = {};
                newArchetype->data.columns = {};

                m_store.push_back(newArchetype);
                newArchetype->storeIndex = m_store.size() - 1;
                m_mappedArchetype[components] = newArchetype;

                for (ComponentId id : components.m_components)
                {
                    auto searchIt =  components.Search(id);
                    
                    m_componentIndex[id].push_back({
                        newArchetype,
                        searchIt - components.Begin()
                    });
                }


                newArchetype->data.columns.resize(components.GetCount());
                
                AllocateArchetypeData(*newArchetype, DEFAULT_COMPONENT_CAPACITY);
                
            }
            
            template<typename T>
            void Remove(EntityId id)
            {
                
            }

            template<typename T>
            ComponentTypeMetaData<T>& Register()
            {
                m_componentIndex.insert({GetComponentId<T>(), {}});

                static_assert(std::is_default_constructible_v<T>,
                              "Component must be default constructible");

                static_assert(std::is_copy_constructible_v<T>,
                              "Component must be copyable");

                auto& meta = ComponentTypeMetaData<T>::Get();

                if constexpr(!std::is_trivially_default_constructible_v<T>)
                    meta.SetCtorHook([](void* ptr) { new (ptr) T(); });

                if constexpr(!std::is_trivially_destructible_v<T>)
                    meta.SetDtorHook([](void* ptr)
                                     {
                                         static_cast<T*>(ptr)->~T();
                                     });

                if constexpr(!std::is_trivially_copyable_v<T>)
                    meta.SetCopyHook([](void* dst, const void* src)
                                     {
                                         new (dst) T(*static_cast<const T*>(src));
                                     });

                if constexpr(std::is_move_constructible_v<T> &&
                             !std::is_trivially_move_constructible_v<T>)
                    meta.SetMoveHook([](void* dst, void* src)
                                     {
                                         new (dst) T(std::move(*static_cast<T*>(src)));
                                     });

                return meta;
            }

            /// <summary>
            /// use in editor ?
            /// </summary>
            /// <typeparam name="T"></typeparam>

            template<typename T>
            void Unregister()
            {
                //NOTE: cascade erase archetype, component data

                auto it = m_componentIndex.find(GetComponentId<T>());
                if(it != m_componentIndex.end())
                {
                    auto& archetypeRecords = it->second;

                    for(ArchetypeRecord& record : archetypeRecords)
                    {
                        Archetype& removedArchetype = *(record.type);

                        //NOTE:: handle special type of Component that hold resource
                        //leave open for implementing type hook

                        if constexpr(!std::is_trivially_destructible_v<T>)
                        {
                            if(removedArchetype.data.columns.size() - 1 >= record.column)
                            {
                                ComponentColumn& col = removedArchetype.data.columns[record.column];
                                for(uint32_t i = 0; i < removedArchetype.count; i++)
                                {
                                    T* element = static_cast<T*>(col.colData + i * col.typeInfo.size);
                                    element->~T();
                                }
                            }
                        }

                        //TODO: Free component data array

                        //merge with already exist archetype or create a new one
                        auto edgeIter = removedArchetype.edges.find(GetComponentId<T>());
                        if(edgeIter != m_componentIndex.end())
                        {
                            ArchetypeEdge edge = edgeIter->second;

                            //must validate componentSet

                            if(!removedArchetype.componentSet.m_components.size() == 1)
                            {
                                if(!edge.removed)
                                {
                                    //how to handle this lazy loading case?

                                }

                                Archetype& movedArchetype = *(edge.removed);
                                uint32_t movedArchetypeCol = 0;
                                uint32_t remainingSlot = movedArchetype.capacity - movedArchetype.count;

                                if(remainingSlot < removedArchetype.count)
                                {
                                    //reallocate
                                    AllocateArchetypeData(movedArchetype, remainingSlot);
                                }

                                for(uint32_t i = 0; i < removedArchetype.data.columns.size(); i++)
                                {
                                    if(i == record.column)
                                    {
                                        continue;
                                    }

                                    ComponentColumn& destCol = movedArchetype.data.columns[movedArchetypeCol];

                                    uint8_t* destAddr = destCol.colData + movedArchetype.count * destCol.typeInfo.size;
                                    uint8_t* srcAddr = removedArchetype.data.columns[i].colData;
                                    if(destCol.typeInfo.isTriviallyCopyable)
                                    {
                                        std::memmove(destAddr,
                                                     srcAddr,
                                                     removedArchetype.count * destCol.typeInfo.size
                                        );
                                    }
                                    else if(destCol.typeInfo.isMoveContructible)
                                    {
                                        for(uint32_t j = 0; j < removedArchetype.count; j++)
                                        {
                                            destCol.hook.move(destAddr, srcAddr);
                                            destCol.hook.dtor(srcAddr);
                                        }
                                    }
                                    else
                                    {
                                        for(uint32_t j = 0; j < removedArchetype.count; j++)
                                        {
                                            destCol.hook.copy(destAddr, srcAddr);
                                            destCol.hook.dtor(srcAddr);
                                        }
                                    }

                                    movedArchetypeCol++;
                                }

                                movedArchetype.count += removedArchetype.count;
                            }


                            FreeArchetypeData(removedArchetype);

                        }
                        else
                        {
                            //SHOULD NEVER BE HERE
                            assert(0 && "Unregister Component does not exist in archetype!");
                        }

                        m_mappedArchetype.erase(removedArchetype.componentSet);
                        EraseArchetype(record.type);
                        //m_store.erase(removedArchetype);
                    }
                    m_componentIndex.erase(GetComponentId<T>());
                }
            }

            EntityId GenerateEntityId();

            std::vector<EntityId> m_freeEntityIndex;
            std::unordered_map<EntityId, EntityRecord> m_entityRecords;
            std::unordered_map<ComponentId, std::vector<ArchetypeRecord>> m_componentIndex;
            std::unordered_map<ComponentSet, Archetype*> m_mappedArchetype;
            std::vector<Archetype*> m_store;
            uint32_t m_generatedEntityId;
        };


    }
}



