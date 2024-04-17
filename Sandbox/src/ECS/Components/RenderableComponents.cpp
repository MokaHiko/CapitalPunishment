#include "RenderableComponents.h"

#include <Resource/ResourceManager.h>
#include <Renderer/Animation.h>
#include <Renderer/Particles/ParticleSystem.h>

MeshRendererComponent::MeshRendererComponent()
{
}

MeshRendererComponent::~MeshRendererComponent()
{
}

const Ref<yoyo::Material>& MeshRendererComponent::GetMaterial() const
{
	return mesh_object->material;
}

void MeshRendererComponent::SetMaterial(Ref<yoyo::Material> new_material) 
{
	// Append mesh pass object
	mesh_object->material = new_material;
}

const Ref<yoyo::IMesh>& MeshRendererComponent::GetMesh() const
{
	return mesh_object->mesh;
}

void MeshRendererComponent::SetMesh(Ref<yoyo::IMesh> mesh) 
{
	mesh_object->mesh = mesh;
}

AnimatorComponent::AnimatorComponent() 
{
	animator = CreateRef<yoyo::Animator>();
}
