#include "SceneGraph.h"

#include <imgui.h>
#include "SceneGraph.h"

#include "Core/Application.h"
#include "ECS/Entity.h "

void SceneGraph::Init()
{
}

void SceneGraph::Shutdown()
{
	// Unsubscribe
}

void SceneGraph::Update(float dt)
{
	Entity e{ 0, GetScene() };
	TransformComponent& transform = e.GetComponent<TransformComponent>();
	RecursiveUpdate(e);
	//RecursiveUpdate(GetScene()->Root());
}

void SceneGraph::OnComponentCreated(Entity e, TransformComponent& transform)
{
	// Define self
	e.GetComponent<TransformComponent>().self = e;
}

void SceneGraph::OnComponentDestroyed(Entity e, TransformComponent& transform)
{
	if (auto parent = e.GetComponent<TransformComponent>().parent)
	{
		parent.GetComponent<TransformComponent>().RemoveChild(e);
	}

	// Destroy children immediately
	for (uint32_t i = 0; i < e.GetComponent<TransformComponent>().children_count; i++)
	{
		GetScene()->Destroy(e.GetComponent<TransformComponent>().children[i]);
	}

	e.GetComponent<TransformComponent>().self = {};
}

void SceneGraph::RecursiveUpdate(TransformComponent& node)
{
	for (uint32_t i = 0; i < node.children_count; i++)
	{
		TransformComponent& transform = node.children[i].GetComponent<TransformComponent>();

		// TODO: Dirty Check
		transform.UpdateModelMatrix();
		
		RecursiveUpdate(transform);
	}
}

void SceneGraph::RecursiveUpdate(Entity e)
{
	for (uint32_t i = 0; i < e.GetComponent<TransformComponent>().children_count; i++)
	{
		TransformComponent& transform = e.GetComponent<TransformComponent>().children[i].GetComponent<TransformComponent>();

		// TODO: Dirty Check
		transform.UpdateModelMatrix();
		RecursiveUpdate(e.GetComponent<TransformComponent>().children[i]);
	}
}
