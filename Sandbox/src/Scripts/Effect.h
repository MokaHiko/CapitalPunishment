#pragma once

#include "NativeScript.h"

class Effect : public ScriptableEntity
{
public:
    Effect(Entity e);
    virtual ~Effect();

    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;
private:
    float life_span = 3.0f;
    float time_alive = 0.0f;
};
