#include "model/model.hpp"
#include "assimp/config.h"
#include "assimp/mesh.h"
#include "math/triangle.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <optional>
#include <vector>

namespace model {

std::optional<texture_info_t>
process_texture(model_loading_info_t &model_loading_info, aiMaterial *material,
                aiTextureType type, texture_type_t texture_type) {
  if (material->GetTextureCount(type) == 0) {
    return std::nullopt;
  }

  for (uint32_t i = 0; i < material->GetTextureCount(type); i++) {
    aiString name;
    material->GetTexture(type, i, &name);

    texture_info_t texture_info{};
    texture_info.file_path = model_loading_info.file_path.parent_path();
    texture_info.file_path /= name.C_Str();
    texture_info.texture_type = texture_type;
    return {texture_info};
  }
  assert(false);
  return std::nullopt;
}

material_description_t
process_material(model_loading_info_t &model_loading_info,
                 aiMaterial *material) {
  material_description_t loaded_material_description;

  if (auto texture_info =
          process_texture(model_loading_info, material, aiTextureType_DIFFUSE,
                          texture_type_t::e_diffuse_map)) {
    loaded_material_description.texture_infos.push_back(*texture_info);
  }

  if (auto texture_info =
          process_texture(model_loading_info, material, aiTextureType_NORMALS,
                          texture_type_t::e_normal_map)) {
    loaded_material_description.texture_infos.push_back(*texture_info);
  }

  if (auto texture_info =
          process_texture(model_loading_info, material, aiTextureType_SPECULAR,
                          texture_type_t::e_specular_map)) {
    loaded_material_description.texture_infos.push_back(*texture_info);
  }

  aiColor3D diffuse_color;
  material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color);

  texture_info_t diffuse_texture_info{};
  diffuse_texture_info.texture_type = texture_type_t::e_diffuse_color;
  diffuse_texture_info.diffuse_color =
      math::vec4(diffuse_color.r, diffuse_color.g, diffuse_color.b, 1);

  loaded_material_description.texture_infos.push_back(diffuse_texture_info);

  return loaded_material_description;
}

raw_mesh_t process_mesh(model_loading_info_t &model_loading_info, aiMesh *mesh,
                        const aiScene *scene) {
  raw_mesh_t loaded_mesh;
  loaded_mesh.name = mesh->mName.C_Str();
  loaded_mesh.vertices.reserve(mesh->mNumVertices);
  for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
    vertex_t vertex{};

    vertex.position = {mesh->mVertices[i].x, mesh->mVertices[i].y,
                       mesh->mVertices[i].z};
    vertex.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y,
                     mesh->mNormals[i].z};

    if (mesh->mTextureCoords[0]) {
      vertex.uv = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
    } else {
      vertex.uv = {0, 0};
    }

    if (mesh->HasTangentsAndBitangents()) {
      vertex.tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y,
                        mesh->mTangents[i].z};
      vertex.bi_tangent = {mesh->mBitangents[i].x, mesh->mBitangents[i].y,
                           mesh->mBitangents[i].z};
    } else {
    }

    loaded_mesh.aabb.min.x =
        std::min(loaded_mesh.aabb.min.x, vertex.position.x);
    loaded_mesh.aabb.min.y =
        std::min(loaded_mesh.aabb.min.y, vertex.position.y);
    loaded_mesh.aabb.min.z =
        std::min(loaded_mesh.aabb.min.z, vertex.position.z);

    loaded_mesh.aabb.max.x =
        std::max(loaded_mesh.aabb.max.x, vertex.position.x);
    loaded_mesh.aabb.max.y =
        std::max(loaded_mesh.aabb.max.y, vertex.position.y);
    loaded_mesh.aabb.max.z =
        std::max(loaded_mesh.aabb.max.z, vertex.position.z);

    loaded_mesh.vertices.push_back(vertex);
  }

  for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
    aiFace &face = mesh->mFaces[i];
    // model_loading_info.model.primitive_count += face.mNumIndices;
    for (uint32_t j = 0; j < face.mNumIndices; j++) {
      loaded_mesh.indices.push_back(face.mIndices[j]);
    }
  }

  loaded_mesh.material_description = process_material(
      model_loading_info, scene->mMaterials[mesh->mMaterialIndex]);
  return loaded_mesh;
}

void process_node(model_loading_info_t &model_loading_info, aiNode *node,
                  const aiScene *scene) {
  for (uint32_t i = 0; i < node->mNumChildren; i++) {
    process_node(model_loading_info, node->mChildren[i], scene);
  }
  for (uint32_t i = 0; i < node->mNumMeshes; i++) {
    model_loading_info.model.meshes.push_back(process_mesh(
        model_loading_info, scene->mMeshes[node->mMeshes[i]], scene));
  }
}

raw_model_t load_model_from_path(const std::filesystem::path &file_path) {
  Assimp::Importer importer{};
  const aiScene *scene = importer.ReadFile(file_path.string(), 0);
  importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
                              aiPrimitiveType_POINT | aiPrimitiveType_LINE);
  importer.ApplyPostProcessing(aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_PreTransformVertices);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    throw std::runtime_error(importer.GetErrorString());
  }

  model_loading_info_t model_loading_info{};
  model_loading_info.file_path = file_path;
  process_node(model_loading_info, scene->mRootNode, scene);
  return model_loading_info.model;
}

raw_model_t merge_meshes(const raw_model_t &raw_model) {
  raw_model_t model{};
  model.meshes.push_back(raw_mesh_t{});
  uint32_t offset = 0;
  for (const auto raw_mesh : raw_model.meshes) {
    for (const auto raw_vertex : raw_mesh.vertices)
      model.meshes[0].vertices.push_back(raw_vertex);
    for (const auto raw_index : raw_mesh.indices)
      model.meshes[0].indices.push_back(raw_index + offset);
    offset += raw_mesh.vertices.size();
  }
  for (const auto vertex : model.meshes[0].vertices)
    model.meshes[0].aabb.grow(vertex.position);
  return model;
}

std::vector<math::triangle_t>
create_triangles_from_mesh(const raw_mesh_t &raw_mesh) {
  std::vector<math::triangle_t> triangles;
  for (uint32_t i = 0; i < raw_mesh.indices.size(); i += 3) {
    math::triangle_t triangle{
        raw_mesh.vertices[raw_mesh.indices[i + 0]].position,
        raw_mesh.vertices[raw_mesh.indices[i + 1]].position,
        raw_mesh.vertices[raw_mesh.indices[i + 2]].position,
    };
    triangles.push_back(triangle);
  }
  return triangles;
}

} // namespace model
