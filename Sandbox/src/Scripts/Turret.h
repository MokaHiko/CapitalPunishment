#pragma once

#include "NativeScript.h"

class Turret : public ScriptableEntity
{
public:
    Turret(Entity e);
    virtual ~Turret();

    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;

	std::vector<yoyo::Vec3> positions = {};
	std::vector<yoyo::Quat> quaternions = {};
private:
    float m_time_elapsed = 0.0f;
    float m_attack_rate = 1.0f;
};
