#include "UnitController.h"

#include <Core/Assert.h>
#include <Math/Random.h>
#include <Math/MatrixTransform.h>

#include <Renderer/Animation.h>

#include "ECS/Components/RenderableComponents.h"
#include "Process.h"

#include "Unit.h"
#include "Projectile.h"
#include "Turret.h"
#include "Effect.h"

// TODO: Make projectile factories
#include <Resource/ResourceManager.h>
#include "ParticleSystem/Particles.h"

static yoyo::PRNGenerator<float> pos_generator(-100.0f, 100.0f);
UnitController::UnitController(Entity e)
	:ScriptableEntity(e) {}

UnitController::~UnitController() {}

void UnitController::OnStart()
{
	if (!HasComponent<Unit>())
	{
		YERROR("UnitController disabled : cannot be used without <Unit> script component!");
		ToggleActive(false);
		return;
	}

	if (!HasComponent<psx::RigidBodyComponent>())
	{
		YERROR("UnitController disabled : cannot be used without Rigidbody component!");
		ToggleActive(false);
		return;
	}

	TransformComponent& transform = GetComponent<TransformComponent>();
	for (int i = 0; i < GetComponent<TransformComponent>().children_count; i++)
	{
		if (transform.children[i].HasComponent<MeshRendererComponent>())
		{
			m_view = transform.children[i];

			AnimatorComponent* animator_component;
			if (m_view.TryGetComponent<AnimatorComponent>(&animator_component))
			{
				m_animator = animator_component->animator;
			}
			else
			{
				YWARN("UnitController: Animations disabled. No animator in view!");
			}
		}
	}

	TravelTo(transform.position);
}

void UnitController::TravelTo(const yoyo::Vec3& target_position)
{
	m_target_position = target_position;
}

void UnitController::OnUpdate(float dt)
{
	if (!HasComponent<Unit>())
	{
		YERROR("UnitController disabled : cannot be used without <Unit> script component!");
		ToggleActive(false);
		return;
	}

	auto& transform = GetComponent<TransformComponent>();

	// Movement
	yoyo::Vec3 diff = m_target_position - transform.position;
	float distance = yoyo::Length(diff);
	if (!yoyo::FloatCompare(distance, 0.0f, 1.1f))
	{
		const UnitMovementStats& movement_stats = GetComponent<Unit>().GetMovementStats();
		yoyo::Vec3 dir = yoyo::Normalize(diff);
		GetComponent<psx::RigidBodyComponent>().AddForce(dir * movement_stats.agility * dt, psx::ForceMode::Impulse);

		// Process View
		YASSERT(m_view, "View has is null entity!");
		TransformComponent& view_transform = m_view.GetComponent<TransformComponent>();

		// Transform Animations
		// TODO: Make member
		static float last_rot = 0;

		// Rotation
		yoyo::Vec3 normalized_delta = dir;
		float dot = yoyo::Dot({ 0.0f, 0.0f, 1.0f }, normalized_delta);
		float rot = yoyo::ACos(dot);

		// Correct direction
		if (dir.x < 0)
		{
			rot *= -1.0f;
		}

		// Check if new rotation
		if(rot != last_rot)
		{
			if (m_animate_transform_process && m_animate_transform_process->IsAlive())
			{
				m_animate_transform_process->Abort();
			}

			yoyo::Quat target_rotation = yoyo::QuatFromAxisAngle(yoyo::Vec3{ 0.0f, 1.0f, 0.0f }, rot, true);
			m_animate_transform_process = CreateRef<AnimateTransformProcess>(m_view, view_transform.position, target_rotation, view_transform.scale, 0.15f);
			StartProcess(m_animate_transform_process);

			last_rot = rot;
		}

		// Animations
		AnimatorComponent* animator_component;
		if (m_view.TryGetComponent<AnimatorComponent>(&animator_component))
		{
			animator_component->animator->Play(0);
		}
	}
	else
	{
		GetComponent<psx::RigidBodyComponent>().SetLinearVelocity({ 0.0f });
		m_target_position = transform.position;

		// View
		 YASSERT(m_view, "View has is null entity!");

		 //Animations
		 AnimatorComponent* animator_component;
		 if (m_view.TryGetComponent<AnimatorComponent>(&animator_component))
		 {
		 	animator_component->animator->Play(1);
		 }
	}
}

void UnitController::BasicAttack()
{
	//Projectile
	const TransformComponent& view_transform = m_view.GetComponent<TransformComponent>();
	yoyo::Vec3 fire_point = GetComponent<TransformComponent>().position + (view_transform.Forward() * 3.0f);
	fire_point.y += 1.0f;

	float bullet_speed = 30.0f;
	yoyo::Mat4x4 t_matrix = yoyo::TranslationMat4x4(fire_point) * 
							yoyo::ScaleMat4x4({0.5f, 0.5f, 0.5f});

	Entity b = Instantiate("Bullet", t_matrix);
	TransformComponent& b_t = b.GetComponent<TransformComponent>();
	b_t.quat_rotation = view_transform.quat_rotation;
	b_t.UpdateModelMatrix();

	MeshRendererComponent& mesh_renderer = b.AddComponent<MeshRendererComponent>();
	mesh_renderer.SetMesh(yoyo::ResourceManager::Instance().Load<yoyo::StaticMesh>("Cube"));
	mesh_renderer.SetMaterial(yoyo::ResourceManager::Instance().Load<yoyo::Material>("grenade_instanced_material"));

	psx::RigidBodyComponent& rb = b.AddComponent<psx::RigidBodyComponent>();
	rb.LockRotationAxis({1,1,1});
	rb.SetUseGravity(false);

	psx::BoxColliderComponent& col = b.AddComponent<psx::BoxColliderComponent>(yoyo::Vec3{0.5f, 0.5f, 0.5f});
	b.AddComponent<Projectile>(b);
	yoyo::Vec3 impulse = b.GetComponent<TransformComponent>().Forward() * bullet_speed;
	rb.AddForce(impulse, psx::ForceMode::Impulse);

	// Muzzle flare
	if(true){
		auto explosion = Instantiate("explosion");
		explosion.AddComponent<Effect>(b);
		auto& particles = explosion.AddComponent<ParticleSystemComponent>();
		yoyo::Vec3 v = {-10.0f, 0.0f, -10.0f};
		
		particles.SetMaxParticles(128);
		particles.SetExplosiveness(1.0f);
		particles.SetLifeTimeRange(1.0f, 3.0f);
		particles.SetLinearVelocityRange(v , v );
		particles.SetScaleRange(0.15, 1.5f);
		particles.SetMaxParticles(128);
		
		b.GetComponent<TransformComponent>().AddChild(explosion);
	}
}

void UnitController::AltAttack()
{
	//Projectile
	const auto& transform = GetComponent<TransformComponent>();
	static const auto& view_transform = m_view.GetComponent<TransformComponent>();
	const yoyo::Vec3& fire_point = transform.position + (view_transform.Forward() * 3.0f);

	float bullet_speed = 20.0f;
	// yoyo::Mat4x4 transform_matrix = yoyo::TranslationMat4x4(transform.position + view_transform.Forward() * 2.0f) * yoyo::ScaleMat4x4({0.25f, 0.25f, 0.25f});
	yoyo::Mat4x4 transform_matrix = yoyo::TranslationMat4x4(fire_point) * yoyo::ScaleMat4x4({1.0f, 1.0f, 1.0f});

	Entity bullet = Instantiate("Turret", transform_matrix);
	MeshRendererComponent& mesh_renderer = bullet.AddComponent<MeshRendererComponent>();
	mesh_renderer.SetMesh(yoyo::ResourceManager::Instance().Load<yoyo::StaticMesh>("Cube"));
	mesh_renderer.SetMaterial(yoyo::ResourceManager::Instance().Load<yoyo::Material>("grenade_instanced_material"));

	psx::RigidBodyComponent& rb = bullet.AddComponent<psx::RigidBodyComponent>();
	//rb.SetUseGravity(false);
	rb.LockRotationAxis({0,1,0});

	//psx::BoxColliderComponent& col = bullet.AddComponent<psx::BoxColliderComponent>(yoyo::Vec3{0.25f, 0.25f, 0.25f});
	psx::BoxColliderComponent& col = bullet.AddComponent<psx::BoxColliderComponent>();

	bullet.AddComponent<Turret>(bullet);
	yoyo::Vec3 impulse = view_transform.Forward() * bullet_speed;
	impulse.y = 20.0f;
	rb.AddForce(impulse, psx::ForceMode::Impulse);
}