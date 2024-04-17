#include "NativeScript.h"

#include "ScriptEvents.h"
#include "Process.h"
#include "Physics/PhysicsEvents.h"

#include "Scripts/CameraController.h"
#include "Scripts/Enemy.h"
#include "Scripts/Projectile.h"
#include "Scripts/Sun.h"
#include "Scripts/Turret.h"
#include "Scripts/Unit.h"
#include "Scripts/UnitController.h"
#include "Scripts/VillageManager.h"
#include "Scripts/Villager.h"

void ScriptingSystem::Init()
{
	yoyo::EventManager::Instance().Subscribe(ScriptCreatedEvent::s_event_type, [&](Ref<yoyo::Event> event) {
		const Ref<ScriptCreatedEvent>& script_event = std::static_pointer_cast<ScriptCreatedEvent>(event);
		OnScriptCreatedCallback(script_event->script);
		return false;
	});

	yoyo::EventManager::Instance().Subscribe(ScriptDestroyedEvent::s_event_type, [&](Ref<yoyo::Event> event) {
		const Ref<ScriptDestroyedEvent>& script_event = std::static_pointer_cast<ScriptDestroyedEvent>(event);
		OnScriptDestroyedCallback(script_event->script);
		return false;
	});

	yoyo::EventManager::Instance().Subscribe(psx::CollisionEvent::s_event_type, [&](Ref<yoyo::Event> event) {
		const Ref<psx::CollisionEvent>& col_event = std::static_pointer_cast<psx::CollisionEvent>(event);
		OnCollisionCallback(col_event->collision);
		return false;
	});
}
void ScriptingSystem::Shutdown() {}

template<typename T>
void UpdateScript(Scene* scene, float dt)
{
	for (auto entity : scene->Registry().group<T>())
	{
		Entity e(entity, scene);
		T& script = e.GetComponent<T>();

		if (!e.GetComponent<T>().IsActive())
		{
			continue;
		}

		if (!script.started)
		{
			e.GetComponent<T>().OnStart();
			e.GetComponent<T>().started = true;
		}

		e.GetComponent<T>().OnUpdate(dt);
	}
}

void ScriptingSystem::Update(float dt)
{
	const auto& scene = GetScene();

	UpdateScript<CameraControllerComponent>(scene, dt);
	UpdateScript<Enemy>(scene, dt);
	UpdateScript<Projectile>(scene, dt);
	UpdateScript<SunComponent>(scene, dt);
	UpdateScript<Turret>(scene, dt);
	UpdateScript<Unit>(scene, dt);
	UpdateScript<UnitController>(scene, dt);
	UpdateScript<VillageManagerComponent>(scene, dt);
	UpdateScript<VillagerComponent>(scene, dt);

	// Process are updated after all scripts
	UpdateProcesses(dt);
}

void ScriptingSystem::OnComponentCreated(Entity e, NativeScriptComponent& native_script)
{
}

void ScriptingSystem::OnComponentDestroyed(Entity e, NativeScriptComponent& native_script)
{
}

void ScriptingSystem::OnScriptCreatedCallback(ScriptableEntity* script)
{
	script->ToggleActive(true);
	script->m_scripting_system = this;
	script->m_physics_world = m_physics_world;
}

void ScriptingSystem::OnScriptDestroyedCallback(ScriptableEntity* script)
{
}

template<typename T>
void ProcessScriptCollision(psx::Collision& col, Entity& e1, Entity& e2)
{
	if (e1.HasComponent<T>())
	{
		e1.GetComponent<T>().OnCollisionEnter(col);
	}

	if (e2.HasComponent<T>())
	{
		std::swap(col.a, col.b);
		e2.GetComponent<T>().OnCollisionEnter(col);
	}
}

void ScriptingSystem::OnCollisionCallback(psx::Collision& col)
{
	Entity e1(col.a, GetScene());
	Entity e2(col.b, GetScene());

	ProcessScriptCollision<CameraControllerComponent>(col, e1, e2);
	ProcessScriptCollision<Enemy>(col, e1, e2);
	ProcessScriptCollision<Projectile>(col, e1, e2);
	ProcessScriptCollision<SunComponent>(col, e1, e2);
	ProcessScriptCollision<Turret>(col, e1, e2);
	ProcessScriptCollision<Unit>(col, e1, e2);
	ProcessScriptCollision<UnitController>(col, e1, e2);
	ProcessScriptCollision<VillageManagerComponent>(col, e1, e2);
	ProcessScriptCollision<VillagerComponent>(col, e1, e2);
}

void ScriptingSystem::AttachProcess(Ref<Process> process)
{
	YASSERT(process != nullptr, "Process is null");
	m_processes.push_back(process);
}

void ScriptingSystem::AbortAllProcesses(bool immediate)
{
	// TODO: Abort all processes
}

void ScriptingSystem::UpdateProcesses(float dt)
{
	auto it = m_processes.begin();

	uint8_t success_count = 0;
	uint8_t fail_count = 0;

	while (it != m_processes.end()) {
		Ref<Process> process = (*it);

		std::list<Ref<Process>>::iterator this_it = it;
		it++;

		if (process->State() == Process::Uninitialized)
		{
			process->OnInit();
		}

		if (process->State() == Process::Running)
		{
			process->OnUpdate(dt);
		}

		if (process->IsDead()) {
			switch (process->State())
			{
			case Process::ProcessState::Succeeded:
			{
				process->OnSuccess();
				if (process->Child())
				{
					AttachProcess(process->Child());
				}
				else
				{
					success_count++;
				}
			} break;
			case Process::ProcessState::Failed:
			{
				process->OnFail();
				fail_count++;
			} break;
			case Process::ProcessState::Aborted:
			{
				process->OnAbort();
				fail_count++;
			} break;
			}

			m_processes.erase(this_it);
		}
	}
}

NativeScriptComponent::NativeScriptComponent() {}

void NativeScriptComponent::AddScript(ScriptableEntity* script)
{
	YASSERT(script != nullptr, "Cannot add invalid script!");
	auto it = std::find(m_scripts.begin(), m_scripts.end(), script);

	if (it != m_scripts.end())
	{
		YERROR("Entity already has script!");
	}
	else
	{
		YASSERT(m_insert_index < MAX_SCRIPTS, "Maximum scripts on entity reached!");
		script->ToggleActive(true);

		m_scripts[m_insert_index++] = script;
		m_scripts_count++;
	}
}

void NativeScriptComponent::RemoveScript(ScriptableEntity* script)
{
	//TODO: Add remove script flag
	auto it = std::find(m_scripts.begin(), m_scripts.end(), script);

	int index = -1;
	if (it != m_scripts.end())
	{
		int index = std::distance(m_scripts.begin(), it);
		m_scripts[index] = nullptr;
	}

	// Return if no script found
	if (index < 0)
	{
		return;
	}

	// Make sure scripts array is contiguos
	if (index != m_scripts.size() - 1)
	{
		m_scripts[index] = m_scripts.back();
		m_scripts[m_scripts.size() - 1] = nullptr;
		--m_scripts_count;
	}
}
