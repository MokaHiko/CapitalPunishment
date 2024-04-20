#pragma once

#include "ECS/Components/RenderableComponents.h"
#include "ECS/System.h"

#include <Renderer/RendererLayer.h>

class MeshSubsystem : public System<MeshRendererComponent>
{
public:
    ~MeshSubsystem() = default;
protected:
    virtual void OnComponentCreated(Entity e, MeshRendererComponent* component) override;
    virtual void OnComponentDestroyed(Entity e, MeshRendererComponent* component)  override;
private:
    friend class RenderSceneSystem;
    MeshSubsystem(Scene* scene, Ref<yoyo::RenderPacket> rp);
    WeakRef<yoyo::RenderPacket> m_rp_ref;
};

class CameraSubsystem : public System<CameraComponent>
{
public:
    ~CameraSubsystem() = default;
protected:
    virtual void OnComponentCreated(Entity e, CameraComponent* component) override;
    virtual void OnComponentDestroyed(Entity e, CameraComponent* component) override;
    virtual void OnUpdate(float dt) override;
private:
    friend class RenderSceneSystem;
    CameraSubsystem(Scene* scene, Ref<yoyo::RenderPacket> rp);
    WeakRef<yoyo::RenderPacket> m_rp_ref;
};

class DirectionalLightSubsystem : public System<DirectionalLightComponent>
{
public:
    ~DirectionalLightSubsystem() = default;
protected:
    virtual void OnComponentCreated(Entity e, DirectionalLightComponent* component) override;
    virtual void OnComponentDestroyed(Entity e, DirectionalLightComponent* component) override;
    virtual void OnUpdate(float dt) override;
private:
    friend class RenderSceneSystem;
    DirectionalLightSubsystem(Scene* scene, Ref<yoyo::RenderPacket> rp);
    WeakRef<yoyo::RenderPacket> m_rp_ref;
};

class RenderSceneSystem : public System<>
{
public:
    RenderSceneSystem(Scene* scene, yoyo::RendererLayer* renderer_layer);
    virtual ~RenderSceneSystem();

    virtual void OnInit() override;
    virtual void OnShutdown() override;
    virtual void OnUpdate(float dt) override;
private:
    Ref<yoyo::RenderPacket> m_render_packet;
    yoyo::RendererLayer* m_renderer_layer;
};
