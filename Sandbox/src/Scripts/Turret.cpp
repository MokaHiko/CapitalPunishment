#include "Turret.h"

#include <Math/Math.h>
#include <Math/MatrixTransform.h>

#include "ECS/Components/RenderableComponents.h"

#include "Projectile.h"
#include "Process.h"

#include <Resource/ResourceManager.h>
#include <ParticleSystem/Particles.h>

#include <Physics/PhysicsTypes.h>

Turret::Turret(Entity e)
	:ScriptableEntity(e) {}

Turret::~Turret() {}

void Turret::OnStart()
{
	float theta = 0.0f;
	float radians = 0.0f;

	positions.resize(12);
	quaternions.resize(12);
	for (int i = 0; i < 12; i++)
	{
		positions[i] = yoyo::Vec3{yoyo::Cos(radians), 0.0f, yoyo::Sin(radians)};
		quaternions[i] = yoyo::QuatFromAxisAngle({0.0f, 1.0f, 0.0f}, yoyo::RadToDeg(radians));
		radians += yoyo::DegToRad(30);
	}

	Ref<DelayProcess> life_time_delay_process = CreateRef<DelayProcess>(5.0f, [&]() {QueueDestroy();});
	StartProcess(life_time_delay_process);
}

void Turret::OnUpdate(float dt) {
	m_time_elapsed += dt;

	GetComponent<psx::RigidBodyComponent>().SetAngularVelocity({0.0f, Y_PI * 360.0f, 0});
	if (m_time_elapsed > (1.0f / m_attack_rate))
	{
		for (int i = 0; i < 12; i++)
		{
			const yoyo::Vec3& position = positions[i];
			const yoyo::Quat& rot = quaternions[i];

			const auto& transform = GetComponent<TransformComponent>();
			yoyo::Vec3 fire_point = transform.position + (position * 2.0f);
			fire_point.y = 1.0f;

			float bullet_speed = 20.0f;
			yoyo::Mat4x4 transform_matrix = yoyo::TranslationMat4x4(fire_point) * yoyo::ScaleMat4x4({ 0.5f, 0.5f, 0.5f });

			Entity bullet = Instantiate("bullet", transform_matrix);

			MeshRendererComponent& mesh_renderer = bullet.AddComponent<MeshRendererComponent>();
			mesh_renderer.SetMesh(yoyo::ResourceManager::Instance().Load<yoyo::StaticMesh>("Cube"));
			mesh_renderer.SetMaterial(yoyo::ResourceManager::Instance().Load<yoyo::Material>("grenade_instanced_material"));

			psx::RigidBodyComponent& rb = bullet.AddComponent<psx::RigidBodyComponent>();
			rb.LockRotationAxis({ 1, 1, 1 });
			rb.SetUseGravity(false);

			psx::BoxColliderComponent& col = bullet.AddComponent<psx::BoxColliderComponent>(yoyo::Vec3{ 0.5f, 0.5f, 0.5f });

			{
				auto& particles = bullet.AddComponent<ParticleSystemComponent>();
				particles.SetLifeTimeRange(0.2, 0.8f);
				particles.SetEmissionRate(42);

				yoyo::Vec3 p_v = { Normalize(position) * bullet_speed };
				p_v.y = 0.0f;
				particles.SetGravityScale({ 0.0f, 9.8f, 0.0f }); // Smoke rises
				particles.SetLinearVelocityRange(p_v, p_v);
				particles.SetScaleRange(0.15f, 1.5f);

				const yoyo::Vec3 angular_velocity = yoyo::Vec3{ 0.0f, 0.0f, 1.0f } *2.0f;
				particles.SetAngularVelocityRange(angular_velocity * -1.0f, angular_velocity);

				particles.SetMaxParticles(32);
			}

			bullet.AddComponent<Projectile>(bullet);
			yoyo::Vec3 impulse = position * bullet_speed;
			//yoyo::Vec3 impulse = bullet.GetComponent<TransformComponent>().Forward() * bullet_speed;
			rb.AddForce(impulse, psx::ForceMode::Impulse);
		}

		m_time_elapsed = 0.0f;
	}
}
