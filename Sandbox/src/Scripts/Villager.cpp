#include "Villager.h"

#include <Math/Quaternion.h>
#include <Math/MatrixTransform.h>

#include <Input/Input.h>
#include <Renderer/Animation.h>
#include <Renderer/Camera.h>

#include <ECS/Components/RenderableComponents.h>
#include "Process.h"

#include "CameraController.h"

#include "Unit.h"
#include "UnitController.h"

VillagerComponent::VillagerComponent(Entity e)
	:ScriptableEntity(e) {}

VillagerComponent::~VillagerComponent() {}

void VillagerComponent::OnCreate() {}

void VillagerComponent::OnStart()
{
	m_game_camera = FindEntityWithComponent<CameraControllerComponent>();

	if (!m_game_camera.IsValid())
	{
		YERROR("Failed to find game camera");
		ToggleActive(false);
		return;
	}

	m_game_camera.GetComponent<CameraControllerComponent>().follow_target = GameObject();
}

void VillagerComponent::OnUpdate(float dt)
{
	auto& transform = GetComponent<TransformComponent>();

	// Combat
	m_time += dt;
	if(yoyo::Input::GetMouseButton(1))
	{
		if (m_time > (1.0f / attack_speed))
		{
			GetComponent<UnitController>().BasicAttack();
			m_time = 0.0f;
		}
	}

	if (yoyo::Input::GetMouseButton(3))
	{
		{
			yoyo::IVec2 mouse_pos = yoyo::Input::GetMousePosition();
			const Ref<yoyo::Camera>& camera = m_game_camera.GetComponent<CameraComponent>().camera;

			float x = (2.0f * mouse_pos.x) / 1920.0f - 1.0f;
			float y = 1.0f - (2.0f * mouse_pos.y) / 1080.0f;
			float z = 1.0f;
			yoyo::Vec3 ray_nds{x, y, z};

			yoyo::Vec4 ray_clip{ray_nds.x, ray_nds.y, -1.0f, 1.0f};

			yoyo::Vec4 ray_eye = yoyo::InverseMat4x4(camera->Projection()) * ray_clip;
			ray_eye.z = -1.0f;
			ray_eye.w = 0.0f;

			yoyo::Vec4 ray_world = yoyo::InverseMat4x4(camera->View()) * ray_eye;
			yoyo::Vec3 dir = Normalize(yoyo::Vec3{ ray_world.x, ray_world.y, ray_world.z } - m_game_camera.GetComponent<TransformComponent>().position);

			const auto& transform = GetComponent<TransformComponent>();
			const yoyo::Vec3& fire_point = m_game_camera.GetComponent<TransformComponent>().position;

			psx::RaycastHit hit;
			if(Raycast(fire_point, dir, 1000.0f, hit))
			{
				if(hit.entity_id == 0);
				Entity e(hit.entity_id, GetScene());
				GetComponent<UnitController>().TravelTo(hit.point);
			}
		}

	}

	// Skill
	if (yoyo::Input::GetKeyDown(yoyo::KeyCode::Key_q))
	{
		if (m_time > (1.0f / attack_speed))
		{
			GetComponent<UnitController>().AltAttack();
			m_time = 0.0f;
		}
	}

	// Movement
	yoyo::Vec3 input = { 0.0f, 0.0f, 0.0f };
	if (yoyo::Input::GetKey(yoyo::KeyCode::Key_w))
	{
		input += yoyo::Vec3{ 0.0f, 0.0f, 1.0f };
	}

	if (yoyo::Input::GetKey(yoyo::KeyCode::Key_s))
	{
		input += yoyo::Vec3{ 0.0f, 0.0f, -1.0f };
	}

	if (yoyo::Input::GetKey(yoyo::KeyCode::Key_d))
	{
		input += yoyo::Vec3{ -1, 0.0f, 0.0f };
	}

	if (yoyo::Input::GetKey(yoyo::KeyCode::Key_a))
	{
		input += yoyo::Vec3{ 1, 0.0f, 0.0f };
	}

	if (Length(input) > 0)
	{
		GetComponent<UnitController>().TravelTo(transform.position + Normalize(input));
	}
}


