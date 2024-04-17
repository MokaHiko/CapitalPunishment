#pragma once

#include <Core/Memory.h>

#include <Renderer/Mesh.h>
#include <Renderer/SkinnedMesh.h>
#include <Renderer/Model.h>
#include <Renderer/RenderScene.h>

// Forward declarations
namespace yoyo
{
    class ParticleSystem;
    class DirectionalLight;
    class Camera;

    class Material;
}

struct DirectionalLightComponent
{
    Ref<yoyo::DirectionalLight> dir_light;
};

struct CameraComponent
{
    Ref<yoyo::Camera> camera;
    bool active;
};

struct MeshRendererComponent
{
    MeshRendererComponent();
    ~MeshRendererComponent();

    yoyo::MeshType type = yoyo::MeshType::Static;

    const Ref<yoyo::Material>& GetMaterial() const;
    void SetMaterial(Ref<yoyo::Material> material);

    const Ref<yoyo::IMesh>& GetMesh() const;
    void SetMesh(Ref<yoyo::IMesh> mesh);

    Ref<yoyo::MeshPassObject> mesh_object;
};

namespace yoyo{class Animator;}
struct AnimatorComponent
{
    AnimatorComponent();
    ~AnimatorComponent() = default;

    Ref<yoyo::Animator> animator;
};

struct DebugColliderRendererComponent
{
    Ref<yoyo::MeshPassObject> mesh_object;
};