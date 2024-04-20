#pragma once

#include "ScriptableEntity.h"

class AnimateTransformProcess;
class CameraControllerComponent : public ScriptableEntity
{
public:
    CameraControllerComponent(Entity e);
    virtual ~CameraControllerComponent();

    virtual void OnCreate() override;
    virtual void OnStart() override;

    virtual void OnUpdate(float dt) override;

    bool follow = true;
    Entity follow_target = {};
    yoyo::Vec3 follow_offset = {-10, 25, -15};

    float pitch = -65;
    float yaw = 90.0f;

    float follow_delay = 0.25f;
private:
    void FreeControlls(float dt);
private:
    float m_movement_speed = 10.0f;
    yoyo::Vec3 m_target_position = {0.0f, 0.0f, 0.0f};
    Ref<AnimateTransformProcess> m_animate_follow;
    Ref<AnimateTransformProcess> m_animate_reset;
};