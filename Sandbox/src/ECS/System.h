#pragma once

#include "Scene.h"
#include <Core/Memory.h>
#include <Core/Assert.h>

class ISystem
{
public:
    virtual void OnInit() {};
    virtual void OnUpdate(float dt) {};
    virtual void OnShutdown() {};
};

// Systems operate on a scene of component type T
template<typename T = int>
class System : public ISystem
{
public:
    System(Scene* scene)
        :m_scene(scene) 
    {
        // TODO: Move to Init
        YASSERT(m_scene, "System creation failure. Null Scene!");
        m_scene->Registry().on_construct<T>().connect<&System::TCreated>(this);
        m_scene->Registry().on_destroy<T>().connect<&System::TDestroyed>(this);
    }

    void Init()
    {
        OnInit();
        for(auto& system : m_subsystems)
        {
            system->OnInit();
        }
    }

    void Update(float dt)
    {
        for(auto& system : m_subsystems)
        {
            system->OnUpdate(dt);
        }
        OnUpdate(dt);
    }

    void Shutdown()
    {
        for(auto& system : m_subsystems)
        {
            system->OnShutdown();
        }
        OnShutdown();

        YASSERT(m_scene, "System shutdown failure. Null Scene!");
        m_scene->Registry().on_construct<T>().disconnect<&System::TCreated>(this);
        m_scene->Registry().on_destroy<T>().disconnect<&System::TDestroyed>(this);
    };
protected:
    Scene* GetScene() { YASSERT(m_scene, "System has invalid scene!"); return m_scene; } // Get the scene this sytem operates on
    void AddSubsystem(Ref<ISystem> system) 
    {
        m_subsystems.push_back(system);
    };
protected:
    virtual void OnComponentCreated(Entity e, T* component) {};
    virtual void OnComponentDestroyed(Entity e, T* component) {};

    // Used in the editor layer to draw the componet in the inspector.
    virtual void InspectorPanelDraw(T* component) {};
private:
    void TCreated(entt::basic_registry<entt::entity>&, entt::entity entity)
    {
        Entity e(entity, m_scene);

        T& component = e.GetComponent<T>();
        OnComponentCreated(e, &component);
    }

    void TDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity)
    {
        Entity e(entity, m_scene);
        T& component = e.GetComponent<T>();
        OnComponentDestroyed(e, &component);
    }
private:
    std::vector<Ref<ISystem>> m_subsystems;
    Scene* m_scene;
};
