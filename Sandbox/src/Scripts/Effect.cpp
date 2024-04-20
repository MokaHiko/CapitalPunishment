#include "Effect.h"

Effect::Effect(Entity e) 
	:ScriptableEntity(e){}

Effect::~Effect() {}

void Effect::OnStart() 
{

}

void Effect::OnUpdate(float dt) 
{
	time_alive += dt;
	if (time_alive >= life_span)
	{
		QueueDestroy();
	}
}
