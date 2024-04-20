#include "RenderScene.h"

#include <Renderer/Camera.h>
#include <Renderer/Light.h>

#include <Math/MatrixTransform.h>
#include "ECS/Components/Components.h"

MeshSubsystem::MeshSubsystem(Scene* scene, Ref<yoyo::RenderPacket> rp)
    :System(scene), m_rp_ref(rp) {}

void MeshSubsystem::OnComponentCreated(Entity entity, MeshRendererComponent* component) {
    component->mesh_object = CreateRef<yoyo::MeshPassObject>();
    component->mesh_object->model_matrix = component->mesh_object->model_matrix;

    auto rp = m_rp_ref.lock();
    rp->new_objects.push_back(component->mesh_object);
}

void MeshSubsystem::OnComponentDestroyed(Entity entity, MeshRendererComponent* component)
{
    auto rp = m_rp_ref.lock();
    rp->deleted_objects.push_back(component->mesh_object);
}

CameraSubsystem::CameraSubsystem(Scene * scene, Ref<yoyo::RenderPacket> rp)
    :System(scene), m_rp_ref(rp) {}

void CameraSubsystem::OnComponentCreated(Entity entity,
    CameraComponent* component) {
    auto cam = component->camera = CreateRef<yoyo::Camera>();

    auto rp = m_rp_ref.lock();
    rp->new_camera = cam;
}

void CameraSubsystem::OnComponentDestroyed(Entity e, CameraComponent* component)
{
}

void CameraSubsystem::OnUpdate(float dt) 
{
    // Update camera matrices
    for (auto& id : GetScene()->Registry().view<TransformComponent, CameraComponent>())
    {
        Entity e{ id, GetScene()};

        Ref<yoyo::Camera> cam = e.GetComponent<CameraComponent>().camera;
        cam->position = e.GetComponent<TransformComponent>().position;
        cam->UpdateCameraVectors();
    }
}

DirectionalLightSubsystem::DirectionalLightSubsystem(Scene* scene, Ref<yoyo::RenderPacket> rp)
    :System(scene), m_rp_ref(rp){}

void DirectionalLightSubsystem::OnComponentCreated(Entity e, DirectionalLightComponent* component)
{
    auto dir_light = component->dir_light = CreateRef<yoyo::DirectionalLight>();

    auto rp = m_rp_ref.lock();
    rp->new_dir_lights.emplace_back(dir_light);
}

void DirectionalLightSubsystem::OnComponentDestroyed(Entity e, DirectionalLightComponent* component)
{
}

void DirectionalLightSubsystem::OnUpdate(float dt) 
{
    // Lights
    for (auto& id : GetScene()->Registry().view<TransformComponent, DirectionalLightComponent>())
    {
        Entity e{ id,  GetScene()};
        Ref<yoyo::DirectionalLight> dir_light = e.GetComponent<DirectionalLightComponent>().dir_light;

        // TODO: calculate base of transform
        // yoyo::Vec3 front = Normalize(dir_light.direction);
        // yoyo::Vec3 right = Normalize(Cross(front, { 0.0f, 1.0f, 0.0f }));
        // yoyo::Vec3 up = Normalize(Cross(right, front));

        //const yoyo::ApplicationSettings& settings = m_app->Settings();
        float width = 16 * 6.0f;
        float height = 9 * 6.0f;
        float half_width = static_cast<float>(width);
        float half_height = static_cast<float>(height);

        yoyo::Mat4x4 proj = yoyo::OrthographicProjectionMat4x4(-half_width, half_width, -half_height, half_height, -1000, 1000);
        proj[5] *= -1.0f;
        dir_light->view_proj = proj * yoyo::LookAtMat4x4(e.GetComponent<TransformComponent>().position, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    }
}

RenderSceneSystem::RenderSceneSystem(Scene* scene, yoyo::RendererLayer* renderer_layer)
    :System(scene), m_renderer_layer(renderer_layer)
{
    m_render_packet = CreateRef<yoyo::RenderPacket>();
    m_render_packet->ToggleAutoReset(true);

    AddSubsystem(Ref<DirectionalLightSubsystem>(YNEW DirectionalLightSubsystem(scene, m_render_packet)));
    AddSubsystem(Ref<CameraSubsystem>(YNEW CameraSubsystem(scene, m_render_packet)));
    AddSubsystem(Ref<MeshSubsystem>(YNEW MeshSubsystem(scene, m_render_packet)));
}

RenderSceneSystem::~RenderSceneSystem() {}

void RenderSceneSystem::OnInit()
{
}

void RenderSceneSystem::OnShutdown()
{
}

void RenderSceneSystem::OnUpdate(float dt)
{
    m_renderer_layer->SendRenderPacket(m_render_packet.get());
}
