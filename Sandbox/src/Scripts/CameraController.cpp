#include "CameraController.h"

#include <Renderer/Camera.h>
#include <ECS/Components/RenderableComponents.h>
#include <ECS/Components/Components.h>

#include <Input/Input.h>

#include <Math/Quaternion.h>

#include "Scripts/Unit.h"

CameraControllerComponent::CameraControllerComponent(Entity e)
	:ScriptableEntity(e) {}

CameraControllerComponent::~CameraControllerComponent() {}

void CameraControllerComponent::OnCreate() {}

void CameraControllerComponent::OnStart()
{
}

void CameraControllerComponent::OnUpdate(float dt)
{
	Ref<yoyo::Camera> camera = GetComponent<CameraComponent>().camera;
	if (follow && follow_target)
	{
		yoyo::Vec3 new_position = follow_target.GetComponent<TransformComponent>().position + follow_offset;
		if(m_target_position != new_position)
		{
			if (m_animate_follow && m_animate_follow ->IsAlive())
			{
				m_animate_follow->Abort();
			}
			m_target_position = new_position;

			const auto& transform = GetComponent<TransformComponent>();
			m_animate_follow = CreateRef<AnimateTransformProcess>(GameObject(), m_target_position, transform.quat_rotation, transform.scale, follow_delay);
			StartProcess(m_animate_follow);
		}

		// TODO: start follow then reset
	}
	else
	{
		FreeControlls(dt);
	}

	camera->pitch = pitch;
	camera->yaw = yaw;
}

void CameraControllerComponent::FreeControlls(float dt) 
{
	auto& transform = GetComponent<TransformComponent>();
	Ref<yoyo::Camera>& camera = GetComponent<CameraComponent>().camera;

	if (yoyo::Input::GetKey(yoyo::KeyCode::Key_w))
	{
		transform.position += camera->Front() * m_movement_speed * dt;
	}

	if (yoyo::Input::GetKey(yoyo::KeyCode::Key_s))
	{
		transform.position -= camera->Front() * m_movement_speed * dt;
	}

	if (yoyo::Input::GetKey(yoyo::KeyCode::Key_d))
	{
		transform.position += camera->Right() * m_movement_speed * dt;
	}

	if (yoyo::Input::GetKey(yoyo::KeyCode::Key_a))
	{
		transform.position -= camera->Right() * m_movement_speed * dt;
	}

	if (yoyo::Input::GetKey(yoyo::KeyCode::Key_Space))
	{
		transform.position += yoyo::Vec3{ 0.0f, 1.0f, 0.0f } *m_movement_speed * dt;
	}

	if (yoyo::Input::GetKey(yoyo::KeyCode::Key_q))
	{
		transform.position -= yoyo::Vec3{ 0.0f, 1.0f, 0.0f } *m_movement_speed * dt;
	}
}
