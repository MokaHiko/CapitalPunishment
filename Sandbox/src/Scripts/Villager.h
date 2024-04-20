#pragma once

#include "NativeScript.h"

class VillagerComponent : public ScriptableEntity
{
public:
    VillagerComponent(Entity e);
    virtual ~VillagerComponent();

    virtual void OnCreate() override;
    virtual void OnStart() override;

    virtual void OnUpdate(float dt) override;
private:
    Entity m_game_camera;
private:
    float attack_speed = 5.0f;
    float m_time = 0.0f;
};

