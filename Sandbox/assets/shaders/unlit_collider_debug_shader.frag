#version 460

layout(location = 0) out vec4 frag_color;

layout(location = 0) in vec3 v_position_world_space;
layout(location = 1) in vec3 v_color;
layout(location = 2) in vec3 v_normal_world_space;
layout(location = 3) in vec2 v_uv;

layout(location = 4) in vec4 v_position_light_space;

struct DirectionalLight {
  mat4 view_proj;

  vec4 color;
  vec4 direction;
};

layout(set = 0, binding = 0) uniform SceneData {
  mat4 view;
  mat4 proj;

  uint dir_light_count;
  uint point_light_count;
  uint spot_light_count;
  uint area_light_count;
};

layout(std140, set = 0, binding = 1) readonly buffer DirectionalLights {
  DirectionalLight dir_lights[];
};

layout(set = 0, binding = 4) uniform sampler2D shadow_map;

// Descriptor set 1 is reserved for texture information
layout(set = 1, binding = 0) uniform sampler2D main_texture;
layout(set = 1, binding = 1) uniform sampler2D specular_texture;

// Descriptor set 2 is reserved for public material properties
layout(set = 2, binding = 0) uniform Material {
  vec4 diffuse_color;
  vec4 specular_color;
};

void main() {
  // vec3 normal = normalize(v_normal_world_space);
  // vec3 view_dir = normalize(v_position_world_space);

  // vec3 ambient = vec3(0.15f);
  // vec4 final_color = vec4(0.0f);

  frag_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
