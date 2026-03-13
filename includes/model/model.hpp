#ifndef MODEL_HPP
#define MODEL_HPP

#include <filesystem>
#include <vector>

#include "math/aabb.hpp"
#include "math/triangle.hpp"

namespace model {

struct vertex_t {
  math::vec3 position{};
  math::vec3 normal{};
  math::vec2 uv{};
  math::vec3 tangent{};
  math::vec3 bi_tangent{};
};

enum texture_type_t {
  e_diffuse_map,
  e_specular_map,
  e_normal_map,
  e_emissive_map,
  e_diffuse_color,
  e_emissive_color,
};

struct texture_info_t {
  texture_type_t texture_type{};
  std::filesystem::path file_path{};
  math::vec4 diffuse_color{};
  math::vec3 emissive_color{};
};

struct material_description_t {
  std::vector<texture_info_t> texture_infos{};
};

struct raw_mesh_t {
  std::vector<vertex_t> vertices{};
  std::vector<uint32_t> indices{};
  material_description_t material_description{};
  math::aabb_t aabb{};
  std::string name{};
};

struct camera_t {
  std::string name{};
  math::vec3 position{};
  math::vec3 target{};
  math::vec3 up{};
  float fov{};
  float aspect{};
  float near_clip{};
  float far_clip{};
};

struct raw_model_t {
  std::vector<raw_mesh_t> meshes;
  std::vector<camera_t> cameras;
};

struct model_loading_info_t {
  std::filesystem::path file_path;
  raw_model_t model;
};

raw_model_t load_model_from_path(const std::filesystem::path &file_path);

// Note: discards all materials
raw_model_t merge_meshes(const raw_model_t &raw_model);

std::vector<math::triangle_t>
create_triangles_from_mesh(const raw_mesh_t &raw_mesh);

} // namespace model

#endif
