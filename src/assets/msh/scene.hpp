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

enum class cloth_collision_primitive_shape : uint32 {
   sphere = 0,
   cylinder = 1,
   box = 2,
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

struct vertex_weight {
   uint32 bone_index = 0;
   float weight = 0.0f;
};

struct geometry_segment {
   constexpr static std::size_t max_vertex_count = 0x8000;

   int32 material_index = 0;

   std::vector<float3> positions;
   std::optional<std::vector<float3>> normals;
   std::optional<std::vector<uint32>> colors;
   std::optional<std::vector<float2>> texcoords;
   std::optional<std::vector<std::array<vertex_weight, 4>>> weights;

   std::vector<std::array<uint16, 3>> triangles;
};

struct shadow_volume_half_edge {
   uint16 vertex = 0;
   uint16 next = 0;
   uint16 twin = 0;
};

struct shadow_volume {
   std::vector<float3> positions;
   std::vector<shadow_volume_half_edge> edges;
};

struct collision_primitive {
   collision_primitive_shape shape = collision_primitive_shape::sphere;
   float radius = 0.0f;
   float height = 0.0f;
   float length = 0.0f;
};

struct cloth_collision_primitive {
   std::string name;
   std::string parent;
   cloth_collision_primitive_shape shape = cloth_collision_primitive_shape::sphere;
   float3 size;
};

struct cloth {
   std::string texture_name;
   std::vector<float3> positions;
   std::vector<float2> texcoords;

   std::vector<uint32> fixed_indices;
   std::vector<std::string> fixed_weights;

   std::vector<std::array<uint32, 3>> triangles;

   std::vector<std::array<uint16, 2>> stretch_constraints;
   std::vector<std::array<uint16, 2>> cross_constraints;
   std::vector<std::array<uint16, 2>> bend_constraints;

   std::vector<cloth_collision_primitive> collision;
};

struct node {
   std::string name;
   std::optional<std::string> parent;
   transform transform;
   node_type type = node_type::null;
   bool hidden = false;

   std::vector<geometry_segment> segments;
   std::vector<uint32> bone_map;
   std::vector<shadow_volume> shadow_volumes;
   std::optional<collision_primitive> collision_primitive;
   std::optional<cloth> cloth;
};

struct scene_option_attach_light {
   std::string node_name;
   std::string light_name;
};

struct scene_options {
   std::vector<std::string> keep_nodes;
   std::vector<std::string> keep_materials;
   std::vector<std::string> normal_maps;
   std::vector<scene_option_attach_light> attach_lights;

   bool keep_all = false;
   bool left_handed = false;
   bool no_collision = false;
   bool no_game_model = false;
   bool high_res_shadow = false;
   uint8 high_res_shadow_lod = 0;
   bool soft_skin_shadow = false;
   bool soft_skin = false;
   bool vertex_lighting = false;
   bool additive_emissive = false;
   bool do_not_merge_collision = false;
   bool no_projection_lights = false;
   bool large_texcoords = false;

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
   std::vector<uint32> blend_bone_list;
   scene_options options;
};
}
