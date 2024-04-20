#include "Unit.h"

#include <Renderer/Material.h>
#include <Resource/ResourceManager.h>

#include "ECS/Components/RenderableComponents.h"
#include "ParticleSystem/Particles.h"

#include "Effect.h"

AnimateTransformProcess::AnimateTransformProcess(Entity e, const yoyo::Vec3& t_pos, const yoyo::Quat& t_rot, const yoyo::Vec3& t_scale, float duration)
{
	if (!e.IsValid())
	{
		YWARN("AnimateTransformProcess: Entity handle is null. Failing!");
		Fail();
		return;
	}

	m_entity = e;
	m_duration = duration;

	start_position = e.GetComponent<TransformComponent>().position;
	target_position = t_pos;

	start_rotation = e.GetComponent<TransformComponent>().quat_rotation;
	target_rotation = t_rot;

	start_scale = e.GetComponent<TransformComponent>().scale;
	target_scale = t_scale;
}

void AnimateTransformProcess::OnUpdate(float dt)
{
	if (!m_entity.IsValid())
	{
		YWARN("AnimateTransformProcess: Entity handle is null. Failing!");
		Fail();
		return;
	}

	m_time_elapsed += dt;
	if (m_time_elapsed >= m_duration)
	{
		Succeed();
	}
	TransformComponent& transform = m_entity.GetComponent<TransformComponent>();
	transform.position = yoyo::Lerp(start_position, target_position, m_time_elapsed / m_duration);
	transform.quat_rotation = yoyo::Slerp(start_rotation, target_rotation, m_time_elapsed / m_duration);
	transform.scale = yoyo::Lerp(start_scale, target_scale, m_time_elapsed / m_duration);
};

Unit::Unit(Entity e)
	:ScriptableEntity(e) {}

Unit::~Unit() {}

void Unit::TakeDamage(float damage, DamageType type)
{
	// TODO: reference to unit's materials material
	static const Ref<yoyo::Material> material = m_view.GetComponent<MeshRendererComponent>().GetMaterial();
	static const Ref<yoyo::Material> damaged_material = yoyo::ResourceManager::Instance().Load<yoyo::Material>("skinned_damaged_material");

	switch (type)
	{
	case(DamageType::Pure):
	{
		// Pure damage effect
		m_health -= damage;

		// Pure damage view effect
		MeshRendererComponent* mesh_renderer;
		if (m_view.TryGetComponent<MeshRendererComponent>(&mesh_renderer))
		{
			mesh_renderer->SetMaterial(damaged_material);
		}

		// Queue reset 
		if (!reset_pure_damage_effect_process)
		{
			reset_pure_damage_effect_process = CreateRef<DelayProcess>(0.15f, [&]() {
				MeshRendererComponent* mesh_renderer;
				if (m_view.TryGetComponent<MeshRendererComponent>(&mesh_renderer))
				{
					mesh_renderer->SetMaterial(material);
				}

				reset_pure_damage_effect_process.reset();
				reset_pure_damage_effect_process = nullptr;
				});

			StartProcess(reset_pure_damage_effect_process);
		}
	}break;
	default:
		YASSERT(0, "Uknown Damage Type!");
		break;
	}

	if (m_health <= 0.0f)
	{
		Die();
	}
}

void Unit::OnStart()
{
	YASSERT(HasComponent<psx::RigidBodyComponent>(), "Unit as no rigidbody!");
	GetComponent<psx::RigidBodyComponent>().LockRotationAxis({ 1.0f, 1.0f, 1.0f });
	GetComponent<psx::RigidBodyComponent>().SetMaxLinearVelocity(GetComponent<Unit>().GetMovementStats().ms);

	TransformComponent& transform = GetComponent<TransformComponent>();
	for (int i = 0; i < GetComponent<TransformComponent>().children_count; i++)
	{
		if (transform.children[i].HasComponent<MeshRendererComponent>())
		{
			m_view = transform.children[i];
		}
	}
	YASSERT(m_view, "Unit as not view [MeshRendere]!");

	// if(true){
	// 	auto& particles = villager.AddComponent<ParticleSystemComponent>();
	// 	particles.SetLifeTimeRange(0.4, 0.8f);
	// 	particles.SetEmissionRate(128);
	// 	particles.SetScaleRange(0.15f, 1.0f);

	// 	const yoyo::Vec3 angular_velocity = yoyo::Vec3{0.0f, 0.0f, 1.0f} * 2.0f;
	// 	particles.SetAngularVelocityRange(angular_velocity * -1.0f, angular_velocity);

	// 	particles.SetMaxParticles(64);
	// }
}

void Unit::OnUpdate(float dt)
{
	if (IsStatusEffect(StatusEffect::Dead))
	{
		return;
	}
}

const bool Unit::IsStatusEffect(StatusEffect effect_flags) const
{
	return (m_status_effect & effect_flags) == effect_flags;
}

void Unit::Die()
{
	if (IsStatusEffect(StatusEffect::Dead))
	{
		return;
	}
	m_status_effect |= StatusEffect::Dead;

	YINFO("Unit just died!");
	YASSERT(!death_process, "Cannot start death process more than once!");

	// TODO: Set dying
	//death_process = CreateRef<DelayProcess>(1.0f, [&]() {
	//	});
	//StartProcess(death_process);

	static auto death_particles_material = yoyo::Material::Create(yoyo::ResourceManager::Instance().Load<yoyo::Material>("default_particle_material"), "death_particles_material");
	death_particles_material->SetTexture(yoyo::MaterialTextureType::MainTexture, yoyo::ResourceManager::Instance().Load<yoyo::Texture>("white.yo"));

	auto explosion = Instantiate("Death Effect", GetComponent<TransformComponent>().position);
	explosion.AddComponent<Effect>(explosion);
	auto& particles = explosion.AddComponent<ParticleSystemComponent>();
	particles.AddMaterial(death_particles_material);
	particles.SetLifeTimeRange(3.0f, 8.0f);
	particles.SetScaleRange(0.1f, 0.8f);
	particles.SetAngularVelocityRange(yoyo::Vec3{ 0.0f, 0.0f, 2.0f } *-1.0f, yoyo::Vec3{ 0.0f, 0.0f, 2.0f });
	particles.SetLinearVelocityRange(yoyo::Vec3{ 1.0f, 1.0f, 1.0f } *-5.0f, yoyo::Vec3{ 1.0f, 1.0f, 1.0f } *5.0f);
	particles.SetGravityScale({ 0.0f, 0.15f, 0.0f });
	particles.SetExplosiveness(1.0f);
	particles.SetMaxParticles(32.0f);

	// Despawn
	QueueDestroy();
}
