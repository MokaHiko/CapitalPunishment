#include "Particles.h"
#include "ECS/Components/Components.h"

#include <Renderer/Shader.h>
#include <Renderer/Material.h>

#include <Resource/ResourceManager.h>

#include <Math/Random.h>
#include <Math/MatrixTransform.h>

#include <Renderer/RendererLayer.h>
#include <Renderer/Camera.h>

#include <Core/Time.h>

ParticleSystemComponent::ParticleSystemComponent()
{
	if (!m_particle_system)
	{
		m_particle_system = yoyo::ParticleSystem::Create("");
	}
}

ParticleSystemComponent::~ParticleSystemComponent() {}

const std::vector<yoyo::Particle>& ParticleSystemComponent::GetParticles() const { return m_particle_system->GetParticles(); }

const uint32_t ParticleSystemComponent::GetMaxParticles() const { return m_particle_system->GetMaxParticles(); }

const uint32_t ParticleSystemComponent::GetParticlesAlive() const { return m_particle_system->GetParticlesAlive(); }

void ParticleSystemComponent::SetMaxParticles(uint32_t size)
{
	m_particle_system->SetMaxParticles(size);
}

const yoyo::Vec3& ParticleSystemComponent::GetGravityScale() const
{
	return m_particle_system->GetGravityScale();
}

void ParticleSystemComponent::SetGravityScale(const yoyo::Vec3& gravity_scale)
{
	m_particle_system->SetGravityScale(gravity_scale);
}

const float ParticleSystemComponent::GetEmissionRate() const
{
	return m_particle_system->GetEmissionRate();
}

void ParticleSystemComponent::SetEmissionRate(float emission_rate)
{
	m_particle_system->SetEmissionRate(emission_rate);
}

const yoyo::ParticleSystemType ParticleSystemComponent::GetType() const
{
	return m_particle_system->GetType();
}

void ParticleSystemComponent::SetType(yoyo::ParticleSystemType type)
{
	return m_particle_system->SetType(type);
}

const yoyo::ParticleSystemSpace ParticleSystemComponent::GetSimulationSpace() const
{
	return m_particle_system->GetSimulationSpace();
}

void ParticleSystemComponent::SetSimulationSpace(yoyo::ParticleSystemSpace simulation_space)
{
	return m_particle_system->SetSimulationSpace(simulation_space);
}

void ParticleSystemComponent::ToggleBillBoard(bool is_billboard)
{
	// TODO: set dirty flag
	m_is_billboard = is_billboard;
}

const std::pair<float, float>& ParticleSystemComponent::GetLifeTimeRange() const
{
	return m_particle_system->life_span_range;
}

void ParticleSystemComponent::SetLifeTimeRange(float min, float max)
{
	m_particle_system->life_span_range = { min, max };
}

const std::pair<yoyo::Vec3, yoyo::Vec3>& ParticleSystemComponent::GetPositionOffsetRange() const
{
	return m_particle_system->position_offset_range;
}

void ParticleSystemComponent::SetPositionOffsetRange(const yoyo::Vec3& min, const yoyo::Vec3& max)
{
	m_particle_system->position_offset_range = { min, max };
}

const std::pair<yoyo::Vec3, yoyo::Vec3>& ParticleSystemComponent::GetLinearVelocityRange() const
{
	return m_particle_system->linear_velocity_range;
}

void ParticleSystemComponent::SetLinearVelocityRange(const yoyo::Vec3& min, const yoyo::Vec3& max)
{
	m_particle_system->linear_velocity_range = { min, max };
}

const std::pair<yoyo::Vec3, yoyo::Vec3>& ParticleSystemComponent::GetAngularVelocityRange() const
{
	return m_particle_system->angular_velocity_range;
}

void ParticleSystemComponent::SetAngularVelocityRange(const yoyo::Vec3& min, const yoyo::Vec3& max)
{
	m_particle_system->angular_velocity_range = { min, max };
}

const std::pair<float, float>& ParticleSystemComponent::GetScaleRange() const
{
	return m_particle_system->scale_range;
}

void ParticleSystemComponent::SetScaleRange(float min, float max)
{
	m_particle_system->scale_range = { min, max };
}

void ParticleSystemComponent::AddMaterial(Ref<yoyo::Material> material) {
	YASSERT(material, "Cannot add null material!");
	m_materials.push_back(material);
}

void ParticleSystemManager::Init()
{
	Ref<yoyo::Shader> unlit_particle_shader = yoyo::ResourceManager::Instance().Load<yoyo::Shader>("unlit_particle_instanced_shader");
	Ref<yoyo::Material> particle_instanced_material = yoyo::Material::Create(unlit_particle_shader, "default_particle_material");
	particle_instanced_material->ToggleCastShadows(false);
	particle_instanced_material->ToggleReceiveShadows(false);

	particle_instanced_material->SetTexture(yoyo::MaterialTextureType::MainTexture, yoyo::ResourceManager::Instance().Load<yoyo::Texture>("assets/textures/white.yo"));
	particle_instanced_material->SetColor(yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
	particle_instanced_material->SetVec4("diffuse_color", yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
	particle_instanced_material->SetVec4("specular_color", yoyo::Vec4{ 1.0, 1.0f, 1.0f, 1.0f });

	Ref<yoyo::StaticMesh> quad = yoyo::StaticMesh::Create("particle_quad");
	quad->GetVertices() =
	{
		{{-1.00, -1.00,  0.00}, {0.00, 0.00, 0.00},  {0.00,  0.00,  1.00},  {0.00,  0.00}},
		{{ 1.00, -1.00,  0.00}, {0.00, 0.00, 0.00},  {0.00,  0.00,  1.00},  {1.00,  0.00}},
		{{ 1.00,  1.00,  0.00}, {0.00, 0.00, 0.00},  {0.00,  0.00,  1.00},  {1.00,  1.00}},
		{{-1.00, -1.00,  0.00}, {0.00, 0.00, 0.00},  {0.00,  0.00,  1.00},  {0.00,  0.00}},
		{{ 1.00,  1.00,  0.00}, {0.00, 0.00, 0.00},  {0.00,  0.00,  1.00},  {1.00,  1.00}},
		{{-1.00,  1.00,  0.00}, {0.00, 0.00, 0.00},  {0.00,  0.00,  1.00},  {0.00,  1.00}},
	};
}

void ParticleSystemManager::Shutdown()
{
}

void ParticleSystemManager::Update(float dt)
{
	// Remove camera rotation for billboard particles
	yoyo::Mat4x4  transpose_view = {};
	if (const auto camera = m_renderer_layer->GetScene()->camera)
	{
		transpose_view = yoyo::TransposeMat4x4(camera->View());

		// Remain Homogenous
		transpose_view[3] = 0;
		transpose_view[7] = 0;
		transpose_view[11] = 0;
		transpose_view[15] = 1;
	}

	for (auto entity : GetScene()->Registry().group<ParticleSystemComponent>())
	{
		Entity e(entity, GetScene());
		const TransformComponent& transform = e.GetComponent<TransformComponent>();

		ParticleSystemComponent& particle_system_component = e.GetComponent<ParticleSystemComponent>();
		const auto& particles = particle_system_component.GetParticles();

		uint32_t prev_particle_count = particle_system_component.GetParticlesAlive();
		particle_system_component.m_particle_system->Update(dt);
		uint32_t particle_count = particle_system_component.GetParticlesAlive();

		// Create renderables for new particles
		if(particle_count - prev_particle_count > 0)
		{
			//YINFO("%d new particles", particle_count - prev_particle_count);

			// TODO: Delete Render packet
			static yoyo::RenderPacket* packet = YNEW yoyo::RenderPacket;
			if (packet->IsProccessed())
			{
				packet->Reset();
			}

			// Add new particles renderables
			for (uint32_t i = prev_particle_count; i < particle_count; i++)
			{
				if (particle_system_component.IsBillBoard())
				{
					// Local
					transpose_view[12] = transform.model_matrix.data[12];
					transpose_view[13] = transform.model_matrix.data[13];
					transpose_view[14] = transform.model_matrix.data[14];

					auto& renderable_object = particle_system_component.m_particle_renderable_objects[i];
					renderable_object->model_matrix =
						transpose_view *
						yoyo::TranslationMat4x4(particles[i].position) *
						yoyo::RotateEulerMat4x4(particles[i].rotation) *
						yoyo::ScaleMat4x4(particles[i].scale);
					renderable_object->color = particles[i].color;
				}
				else 
				{
					auto& renderable_object = particle_system_component.m_particle_renderable_objects[i];
					renderable_object->model_matrix = transform.model_matrix * yoyo::TranslationMat4x4(particles[i].position);
				}

				packet->new_objects.push_back(particle_system_component.m_particle_renderable_objects[i]);
			}

			m_renderer_layer->SendRenderPacket(packet);
		}

		// Update particle renderable properties
		for (int i = 0; i < particle_system_component.GetParticlesAlive(); i++)
		{
			if (particle_system_component.IsBillBoard())
			{
				// Local
				transpose_view[12] = transform.model_matrix.data[12];
				transpose_view[13] = transform.model_matrix.data[13];
				transpose_view[14] = transform.model_matrix.data[14];

				auto& renderable_object = particle_system_component.m_particle_renderable_objects[i];
				renderable_object->model_matrix =
					transpose_view *
					yoyo::TranslationMat4x4(particles[i].position) *
					yoyo::RotateEulerMat4x4(particles[i].rotation) *
					yoyo::ScaleMat4x4(particles[i].scale);
				renderable_object->color = particles[i].color;
			}
			else
			{
				auto& renderable_object = particle_system_component.m_particle_renderable_objects[i];
				renderable_object->model_matrix = transform.model_matrix * yoyo::TranslationMat4x4(particles[i].position);
			}
		}
	}
}

void ParticleSystemManager::OnComponentCreated(Entity entity, ParticleSystemComponent& particle_system_component)
{
	Entity e(entity, GetScene());

	if (!e.IsValid())
	{
		YERROR("Invalid entity handle holding particle system!");
		return;
	}

	const TransformComponent& transform = e.GetComponent<TransformComponent>();

	// Allocate particle system
	particle_system_component.m_particle_system = yoyo::ParticleSystem::Create("");
	particle_system_component.SetMaxParticles(128);

	Ref<yoyo::StaticMesh> quad = yoyo::ResourceManager::Instance().Load<yoyo::StaticMesh>("particle_quad");
	Ref<yoyo::Material> material = nullptr;
	if (particle_system_component.m_materials.empty())
	{
		particle_system_component.m_materials.push_back(yoyo::ResourceManager::Instance().Load<yoyo::Material>("default_particle_material"));
	}
	material = particle_system_component.m_materials[0];

	// Instanced
	{
		// Pre create all renderable objects
		particle_system_component.m_particle_renderable_objects.resize(particle_system_component.GetMaxParticles());
		for (int i = 0; i < particle_system_component.m_particle_renderable_objects.size(); i++)
		{
			particle_system_component.m_particle_renderable_objects[i] = CreateRef<yoyo::MeshPassObject>();
			particle_system_component.m_particle_renderable_objects[i]->mesh = quad;
			particle_system_component.m_particle_renderable_objects[i]->material = material;
		}
	}

	// Batched
}

void ParticleSystemManager::OnComponentDestroyed(Entity e, ParticleSystemComponent& particle_system_component)
{
	// TODO: Delete render packet
	yoyo::RenderPacket* packet = new yoyo::RenderPacket;
	for (auto renderable : particle_system_component.m_particle_renderable_objects)
	{
		if(renderable->Valid())
		{
			packet->deleted_objects.push_back(renderable);
		}
	}

	m_renderer_layer->SendRenderPacket(packet);
}
