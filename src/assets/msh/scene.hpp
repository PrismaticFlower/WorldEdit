#pragma once

#include "material.hpp"
#include "types.hpp"

#include <array>
#include <optional>
#include <vector>

namespace we::assets::msh {

enum class node_type : int32 {
   null = 0,
   skinned_mesh = 1,
   cloth = 2,
   bone = 3,
   static_mesh = 4,
   shadow_volume = 6
};

enum class collision_primitive_shape : int32 {
   sphere = 0,
   cylinder = 2,
   box = 4
};

enum class lod_group : uint32 {
   model = 0,
   big_model = 1,
   soldier = 2,
   huge_model = 3,
};

struct transform {
   float3 translation = {0.0f, 0.0f, 0.0f};
   quaternion rotation = {1.0f, 0.0f, 0.0f, 0.0f};

   explicit operator float4x4() const noexcept;
};

struct geometry_segment {
   constexpr static std::size_t max_vertex_count = 0x8000;

   int32 material_index = 0;

   std::vector<float3> positions;
   std::optional<std::vector<float3>> normals;
   std::optional<std::vector<uint32>> colors;
   std::optional<std::vector<float2>> texcoords;

   std::vector<std::array<uint16, 3>> triangles;
};

struct collision_primitive {
   collision_primitive_shape shape = collision_primitive_shape::sphere;
   float radius = 0.0f;
   float height = 0.0f;
   float length = 0.0f;
};

struct node {
   std::string name;
   std::optional<std::string> parent;
   transform transform;
   node_type type = node_type::null;
   bool hidden = false;

   std::vector<geometry_segment> segments;
   std::vector<uint32> bone_map;
   std::optional<collision_primitive> collision_primitive;
};

struct scene_options {
   std::vector<std::string> keep_nodes;
   std::vector<std::string> keep_materials;
   std::vector<std::string> normal_maps;

   bool keep_all = false;
   bool left_handed = false;
   bool no_collision = false;
   bool no_game_model = false;
   bool high_res_shadow = false;
   uint8 high_res_shadow_lod = 0;
   bool shadow_on = false;
   bool soft_skin_shadow = false;
   bool hard_skin_only = false;
   bool soft_skin = false;
   bool do_not_merge_skins = false;
   bool vertex_lighting = false;
   bool additive_emissive = false;
   bool k_collision = false;
   bool do_not_merge_collision = false;
   bool remove_vertices_on_merge = false;

   float scale = 1.0f;
   uint32 max_bones = 0;
   lod_group lod_group = lod_group::model;
   float lod_bias = 1.0f;

   std::optional<float3> ambient_lighting;

   float bounding_box_scale = 1.0f;
   float3 bounding_box_offset = {0.0f, 0.0f, 0.0f};
};

struct scene {
   std::vector<material> materials;
   std::vector<node> nodes;
   scene_options options;
};
}
