#pragma once

#include "ECS/Components/Components.h"
#include "ECS/System.h"

// Updates transforms and relative transform components of the scene hierarchy
class SceneGraph : public System<TransformComponent>
{
public:
    SceneGraph(Scene* scene)
        :System<TransformComponent>(scene)
    {
    }

    virtual ~SceneGraph() = default;

    virtual void OnInit() override;
    virtual void OnShutdown() override;
    virtual void OnUpdate(float dt) override;

    virtual void OnComponentCreated(Entity e, TransformComponent* component) override;
    virtual void OnComponentDestroyed(Entity e, TransformComponent* component) override;
private:
    void RecursiveUpdate(TransformComponent& node);
    void RecursiveUpdate(Entity e);
};