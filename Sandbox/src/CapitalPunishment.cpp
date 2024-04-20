#include "CapitalPunishment.h"

#include <Yoyo.h>

#include <Core/Log.h>

#include <Math/Math.h>
#include <Math/MatrixTransform.h>
#include <Math/Quaternion.h>
#include <Math/Random.h>

#include <Input/Input.h>

#include <Core/Memory.h>
#include <Core/Time.h>
#include <Resource/ResourceManager.h>

#include <Renderer/RendererLayer.h>
#include <Renderer/Texture.h>
#include <Renderer/Shader.h>
#include <Renderer/Material.h>
#include <Renderer/Camera.h>
#include <Renderer/Light.h>

#include "Scripts/CameraController.h"
#include "Scripts/Sun.h"
#include "Scripts/VillageManager.h"

#include "SceneGraph/SceneGraph.h"
#include "Physics/Physics3D.h"
#include "ParticleSystem/Particles.h"
#include "RenderScene/RenderScene.h"

#include "Editor/EditorLayer.h"

// Move to animation system
#include "Renderer/Animation.h"

struct ProfileMetrics
{
    float frame_time = 0.0f;
    float average_time = 0.0f;
    float min = 0.0f;
    float max = 0.0f;
} profile_metrics;

GameLayer::GameLayer(yoyo::Application* app)
    :m_app(app) {}

GameLayer::~GameLayer() {}

void GameLayer::OnAttach()
{
    // Init scene
    m_scene = YNEW Scene();

    // Init systems
    m_scene_graph = CreateRef<SceneGraph>(m_scene);
    m_physics_world = CreateRef<psx::PhysicsWorld>(m_scene);
    m_scripting = CreateRef<ScriptingSystem>(m_scene, m_physics_world.get());
}

void GameLayer::OnDetatch()
{
    // Clean up handles
    YDELETE m_scene;
}

void GameLayer::OnEnable()
{
    YASSERT(m_app != nullptr, "Invalid application handle!");
    yoyo::RendererLayer* renderer_layer = m_app->FindLayer<yoyo::RendererLayer>();

    m_render_scene = CreateRef<RenderSceneSystem>(m_scene, renderer_layer);
    m_render_scene->Init();

    m_particles = CreateRef<ParticleSystemManager>(m_scene, renderer_layer);
    m_particles->Init();

    m_scene_graph->Init();
    m_physics_world->Init();
    m_scripting->Init();

    // Load assets
    Ref<yoyo::Shader> default_lit = yoyo::ResourceManager::Instance().Load<yoyo::Shader>("lit_shader");
    Ref<yoyo::Shader> default_lit_instanced = yoyo::ResourceManager::Instance().Load<yoyo::Shader>("lit_instanced_shader");
    Ref<yoyo::Shader> skinned_lit = yoyo::ResourceManager::Instance().Load<yoyo::Shader>("skinned_lit_shader");

    Ref<yoyo::Material> default_material = yoyo::Material::Create(default_lit, "default_material");
    Ref<yoyo::Texture> default_texture = yoyo::ResourceManager::Instance().Load<yoyo::Texture>("assets/textures/prototype_512x512_white.yo");
    default_material->SetTexture(yoyo::MaterialTextureType::MainTexture, default_texture);
    default_material->SetVec4("diffuse_color", yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
    default_material->SetVec4("specular_color", yoyo::Vec4{ 1.0, 1.0f, 1.0f, 1.0f });
    default_material->SetColor(yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });

    Ref<yoyo::Material> default_instanced_material = yoyo::Material::Create(default_lit_instanced, "default_instanced_material");
    Ref<yoyo::Texture> default_instanced_texture = yoyo::ResourceManager::Instance().Load<yoyo::Texture>("assets/textures/prototype_512x512_white.yo");
    default_instanced_material->SetTexture(yoyo::MaterialTextureType::MainTexture, default_instanced_texture);
    default_instanced_material->SetVec4("diffuse_color", yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
    default_instanced_material->SetVec4("specular_color", yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
    default_instanced_material->SetColor(yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });

    yoyo::ResourceManager::Instance().Load<yoyo::Model>("assets/models/plane.yo");
    Ref<yoyo::Model> cube_model = yoyo::ResourceManager::Instance().Load<yoyo::Model>("assets/models/cube.yo");

    // Universal game material
    {
        Ref<yoyo::Material> people_material = yoyo::Material::Create(default_lit_instanced, "people_material");
        people_material->SetTexture(yoyo::MaterialTextureType::MainTexture, yoyo::ResourceManager::Instance().Load<yoyo::Texture>("assets/textures/people_texture_map.yo"));
        people_material->SetColor(yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
        people_material->SetVec4("diffuse_color", yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
        people_material->SetVec4("specular_color", yoyo::Vec4{ 1.0, 1.0f, 1.0f, 1.0f });

        Ref<yoyo::Material> colormap_material = yoyo::Material::Create(default_lit_instanced, "colormap_material");
        colormap_material->SetTexture(yoyo::MaterialTextureType::MainTexture, yoyo::ResourceManager::Instance().Load<yoyo::Texture>("assets/textures/colormap.yo"));
        colormap_material->SetColor(yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
        colormap_material->SetVec4("diffuse_color", yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
        colormap_material->SetVec4("specular_color", yoyo::Vec4{ 1.0, 1.0f, 1.0f, 1.0f });

        Ref<yoyo::Material> grenade_instanced_material = yoyo::Material::Create(default_lit_instanced, "grenade_instanced_material");
        Ref<yoyo::Texture> grenade_instanced_texture = yoyo::ResourceManager::Instance().Load<yoyo::Texture>("assets/textures/prototype_512x512_white.yo");
        grenade_instanced_material->SetTexture(yoyo::MaterialTextureType::MainTexture, grenade_instanced_texture);
        grenade_instanced_material->SetVec4("diffuse_color", yoyo::Vec4{ 0.0f, 1.0f, 0.0f, 1.0f });
        grenade_instanced_material->SetVec4("specular_color", yoyo::Vec4{ 0.25, 0.0f, 0.0f, 1.0f });
        grenade_instanced_material->SetColor(yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
    }

    // Animated Universal
    {
        Ref<yoyo::Material> skinned_default_material = yoyo::Material::Create(skinned_lit, "skinned_default_material");
        skinned_default_material->SetTexture(yoyo::MaterialTextureType::MainTexture, yoyo::ResourceManager::Instance().Load<yoyo::Texture>("assets/textures/prototype_512x512_white.yo"));
        skinned_default_material->SetColor(yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
        skinned_default_material->SetVec4("diffuse_color", yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
        skinned_default_material->SetVec4("specular_color", yoyo::Vec4{ 1.0, 1.0f, 1.0f, 1.0f });

        Ref<yoyo::Material> skinned_people_material = yoyo::Material::Create(skinned_lit, "skinned_people_material");
        skinned_people_material->SetTexture(yoyo::MaterialTextureType::MainTexture, yoyo::ResourceManager::Instance().Load<yoyo::Texture>("assets/textures/people_texture_map.yo"));
        skinned_people_material->SetColor(yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
        skinned_people_material->SetVec4("diffuse_color", yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
        skinned_people_material->SetVec4("specular_color", yoyo::Vec4{ 0.0f, 0.0f, 0.0f, 0.0f });

        Ref<yoyo::Material> skinned_damaged_material = yoyo::Material::Create(skinned_lit, "skinned_damaged_material");
        skinned_damaged_material->SetTexture(yoyo::MaterialTextureType::MainTexture, yoyo::ResourceManager::Instance().Load<yoyo::Texture>("assets/textures/people_texture_map.yo"));
        skinned_damaged_material->SetColor(yoyo::Vec4{ 1.0f, 0.0f, 0.0f, 1.0f });
        skinned_damaged_material->SetVec4("diffuse_color", yoyo::Vec4{ 1.0f, 0.0f, 0.0f, 1.0f });
        skinned_damaged_material->SetVec4("specular_color", yoyo::Vec4{ 0.0f, 0.0f, 0.0f, 0.0f });
#ifdef Y_DEBUG
        {
            Ref<yoyo::Shader> skinned_lit_debug = yoyo::ResourceManager::Instance().Load<yoyo::Shader>("skinned_lit_debug_shader");

            Ref<yoyo::Material> color_map_skinned_material = yoyo::Material::Create(skinned_lit_debug, "skinned_colormap_debug_material");
            color_map_skinned_material->SetTexture(yoyo::MaterialTextureType::MainTexture, yoyo::ResourceManager::Instance().Load<yoyo::Texture>("assets/textures/colormap.yo"));
            color_map_skinned_material->SetColor(yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
            color_map_skinned_material->SetVec4("diffuse_color", yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
            color_map_skinned_material->SetVec4("specular_color", yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });

            uint32_t index = 0;
            color_map_skinned_material->SetProperty("focused_bone_index", &index);
        }

        {
            Ref<yoyo::Shader> collider_debug = yoyo::ResourceManager::Instance().Load<yoyo::Shader>("unlit_collider_debug_shader");
            Ref<yoyo::Material> collider_debug_material = yoyo::Material::Create(collider_debug, "collider_debug_material");

            collider_debug_material->SetTexture(yoyo::MaterialTextureType::MainTexture, yoyo::ResourceManager::Instance().Load<yoyo::Texture>("assets/textures/prototype_512x512_white.yo"));
            collider_debug_material->SetColor(yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
            collider_debug_material->SetVec4("diffuse_color", yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
            collider_debug_material->SetVec4("specular_color", yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
        }
#endif
    }

    // Animated Mutant
    {
        Ref<yoyo::Material> skinned_mutant_material = yoyo::Material::Create(skinned_lit, "skinned_mutant_material");
        skinned_mutant_material->SetTexture(yoyo::MaterialTextureType::MainTexture, yoyo::ResourceManager::Instance().Load<yoyo::Texture>("assets/textures/Mutant_diffuse.yo"));
        skinned_mutant_material->SetColor(yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
        skinned_mutant_material->SetVec4("diffuse_color", yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
        skinned_mutant_material->SetVec4("specular_color", yoyo::Vec4{ 0.0, 0.0f, 0.0f, 0.0f });
    }

    // Lights
    {
        Ref<yoyo::Material> light_material = yoyo::Material::Create(default_lit, "light_material");
        //light_material->ToggleReceiveShadows(false);
        light_material->SetTexture(yoyo::MaterialTextureType::MainTexture, yoyo::ResourceManager::Instance().Load<yoyo::Texture>("assets/textures/prototype_512x512_white.yo"));

        // TODO: Apply material color
        light_material->SetColor(yoyo::Vec4{ 1.0f, 1.0f, 0.0f, 1.0f });
        light_material->SetVec4("diffuse_color", yoyo::Vec4{ 1.0f, 1.0f, 0.0f, 1.0f });
        light_material->SetVec4("specular_color", yoyo::Vec4{ 1.0, 1.0f, 1.0f, 1.0f });

        Entity light = m_scene->Instantiate("light", { 100.0f, 60.0f, 5.0f });
        Ref<yoyo::DirectionalLight> dir_light = light.AddComponent<DirectionalLightComponent>().dir_light;
        dir_light->color = { 1.0f, 1.0f, 1.0f, 1.0f };
        dir_light->direction = yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f } *-1.0f;

        auto& mesh_renderer = light.AddComponent<MeshRendererComponent>();
        mesh_renderer.SetMesh(yoyo::ResourceManager::Instance().Load<yoyo::StaticMesh>("Cube"));
        mesh_renderer.SetMaterial(light_material);

        light.AddComponent<SunComponent>(light);
    }

    // Set up scene
    auto camera = m_scene->Instantiate("camera", { 0.0f, 50.0f, 50.0f });
    // camera.AddComponent<CameraComponent>().camera->SetType(yoyo::CameraType::Orthographic);
    camera.AddComponent<CameraComponent>().camera->SetType(yoyo::CameraType::Perspective);
    camera.AddComponent<CameraControllerComponent>(camera);

    // Village Manager
    {
        auto village_manager = m_scene->Instantiate("village_manager", { 0.0f, 0.0f, 0.0f });

        auto& village_props = village_manager.AddComponent<VillageProps>();
        village_props.max_villagers = 1;
        village_props.spawn_rate = 1.0f;

        village_manager.AddComponent<VillageManagerComponent>(village_manager);
    }

    // Plane
    if(true){
        Ref<yoyo::Material> grid_material = yoyo::Material::Create(default_lit_instanced, "grid_material");
        Ref<yoyo::Texture> grid_texture = yoyo::ResourceManager::Instance().Load<yoyo::Texture>("assets/textures/prototype_512x512_white.yo");
        grid_texture->SetSamplerType(yoyo::TextureSamplerType::Linear);
        grid_material->SetTexture(yoyo::MaterialTextureType::MainTexture, grid_texture);
        grid_material->SetColor(yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
        grid_material->SetVec4("diffuse_color", yoyo::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
        grid_material->SetVec4("specular_color", yoyo::Vec4{ 0.0, 0.0f, 0.0f, 0.0f });

        Entity floors = m_scene->Instantiate("floors", { 0.0f, 0.0f, 0.0f });

        float root = 5;
        float dim = 16.0f;
        for (int j = -root; j < root; j++)
        {
            for (int i = -root; i < root; i++)
            {
                auto plane = m_scene->Instantiate("plane", { dim * 2.0f * i, 0.0f, dim * 2.0f * j });

                TransformComponent& transform = plane.GetComponent<TransformComponent>();
                transform.scale = { dim, dim, 1.0f };
                transform.quat_rotation = yoyo::QuatFromAxisAngle({ 1, 0, 0 }, yoyo::DegToRad(-90));

                MeshRendererComponent& mesh_renderer = plane.AddComponent<MeshRendererComponent>();
                mesh_renderer.SetMesh(yoyo::ResourceManager::Instance().Load<yoyo::StaticMesh>("Plane"));
                mesh_renderer.SetMaterial(grid_material);

                floors.GetComponent<TransformComponent>().AddChild(plane);
            }
        }
    }
}

void GameLayer::OnDisable()
{
    // Shutdown Systems
    m_particles->Shutdown();
    m_scripting->Shutdown();
    m_physics_world->Shutdown();
    m_scene_graph->Shutdown();
    m_render_scene->Shutdown();
};

void GameLayer::OnUpdate(float dt)
{
    // Physics System
    {
#ifdef Y_DEBUG
        yoyo::ScopedTimer timer([&](const yoyo::ScopedTimer& timer) {
            m_app->d_layer_profiles["Game [PhysicsWorld]"] = timer.delta;
            });
#endif
        m_physics_world->Update(dt);
    }


    // Scene Graph
    {
#ifdef Y_DEBUG
        yoyo::ScopedTimer timer([&](const yoyo::ScopedTimer& timer) {
            m_app->d_layer_profiles["Game [Scene Graph]"] = timer.delta;
            });
#endif
        m_scene_graph->Update(dt);
    }

    // Update Render Scene Mesh
    for (auto& id : m_scene->Registry().view<TransformComponent, MeshRendererComponent>())
    {
        Entity e{ id, m_scene };

        // TODO: Move to renderable 
        MeshRendererComponent& mesh_renderer = e.GetComponent<MeshRendererComponent>();
        mesh_renderer.mesh_object->model_matrix = e.GetComponent<TransformComponent>().model_matrix;
    }

    // Animation System
    for (auto& id : m_scene->Registry().view<TransformComponent, AnimatorComponent>())
    {
        Entity e{ id, m_scene };
        AnimatorComponent& animator = e.GetComponent<AnimatorComponent>();
        animator.animator->Update(dt);
    }

    // Scripting System
    {
#ifdef Y_DEBUG
        yoyo::ScopedTimer timer([&](const yoyo::ScopedTimer& timer) {
            m_app->d_layer_profiles["Game [Scripting]"] = timer.delta;
            });
#endif
        m_scripting->Update(dt);
        m_scene->FlushDestructionQueue();
    }


    {
#ifdef Y_DEBUG
        yoyo::ScopedTimer timer([&](const yoyo::ScopedTimer& timer) {
            m_app->d_layer_profiles["Game [Particles]"] = timer.delta;
        });
#endif
        m_particles->Update(dt);
    }

    m_render_scene->Update(dt);
};

static std::vector<yoyo::RenderPacket> render_packets;
static int packet_count = 0;

class CapitalPunishment : public yoyo::Application
{
public:
    CapitalPunishment()
        : yoyo::Application({ "CapitalPunishment", 0, 0, 1920, 1080 })
    {
        PushLayer(YNEW EditorLayer(this));
        PushLayer(YNEW GameLayer(this));
    }

    ~CapitalPunishment() {}
};

yoyo::Application* CreateApplication()
{
    return YNEW CapitalPunishment;
};