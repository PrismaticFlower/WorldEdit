#pragma once

#include "types.hpp"

#include "math/bounding_box.hpp"

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace we::munge {

struct skeleton_bone {
   std::string name;
   std::string parent;

   // Vertex Space - The space of an untransformed vertex.
   // Bone Space - This bone's space.
   // Parent Space - The bone space of the parent.

   float4x4 bone_from_parent;
   float4x4 bone_from_vertex;
   float4x4 vertex_from_bone;
};

struct skeleton {
   std::vector<skeleton_bone> bones;

   /// @brief Map node index to bone index.
   std::vector<uint8> bone_remap;

   /// @brief Map node index to parent bone index.
   std::vector<uint32> node_parent_remap;

   /// @brief Map node index to bone (or parent bone) index.
   std::vector<uint32> node_remap;

   /// @brief Transforms from the space of an untransformed vertex in a node to the space of the node.
   std::vector<float4x4> node_from_vertex;

   /// @brief Transforms from the space of an untransformed vertex in a node to local space.
   std::vector<float4x4> local_from_vertex;

   /// @brief Transforms from the space of a node to local space.
   std::vector<float4x4> local_from_node;

   /// @brief Same as skeleton_bone::bone_from_parent but for all nodes.
   std::vector<float4x4> node_from_parent;
};

enum class material_flags : uint32 {
   none = 0x0,

   lit = 0x1,

   alpha_cutout = 0x2,
   transparent = 0x4,
   gloss_map = 0x8,

   glow = 0x10,
   perpixel = 0x20,
   additive = 0x40,
   specular = 0x80,

   envmapped = 0x100,
   vertex_lit = 0x200,
   normal_map_tile = 0x800,

   reflective = 0x1000,
   reflected = 0x2000,

   doublesided = 0x10000,

   scroll = 0x1000000,
   blink = 0x2000000,
   animated = 0x4000000,
   attached_light = 0x8000000,
};

constexpr bool marked_as_enum_bitflag(material_flags)
{
   return true;
}

struct material {
   material_flags flags = material_flags::lit;
   uint32 diffuse_color = 0xff'ff'ff'ffu;
   uint32 specular_color = 0xff'ff'ff'ffu;
   uint32 specular_exponent = 50;
   int32 param0 = 0;
   int32 param1 = 0;

   std::string attached_light;
   std::string render_type;
   std::string name;
   std::array<std::string, 4> textures;
};

enum class model_lod { lod0, lod1, lod2, lowd };

enum class model_shadow_vertex_type { unskinned, hard_skinned, soft_skinned };

struct model_shadow_unskinned_vertex {
   float3 positionSS;
   std::array<uint8, 3> normalSS;

   float3 positionLS;
};

struct model_shadow_hard_skinned_vertex {
   std::array<float3, 3> positionSS;
   std::array<uint8, 3> bone_indices;

   float3 positionLS;
};

struct model_shadow_soft_skinned_vertex {
   std::array<float3, 3> positionSS;
   std::array<std::array<uint8, 3>, 3> bone_weights = {};
   std::array<std::array<uint8, 3>, 3> bone_indices = {};

   float3 positionLS;
};

struct model_shadow_vertices {
   model_shadow_vertex_type type = {};

   std::vector<model_shadow_unskinned_vertex> unskinned;
   std::vector<model_shadow_hard_skinned_vertex> hard_skinned;
   std::vector<model_shadow_soft_skinned_vertex> soft_skinned;
};

struct model_shadow {
   math::bounding_box bboxSS;
   math::bounding_box bboxLS;

   std::vector<std::array<uint16, 3>> index_buffer;

   model_shadow_vertices vertices;

   std::string bone_name;
   std::vector<uint8> bone_map;
};

struct model_segment_vertex_tangents {
   float3 tangentSS;
   float3 bitangentSS;
};

struct model_segment_vertices {
   std::size_t vertex_count = 0;

   std::unique_ptr<float3[]> positionSS;
   std::unique_ptr<float3[]> positionLS;
   std::unique_ptr<float3[]> bone_weights;
   std::unique_ptr<std::array<uint8, 3>[]> bone_indices;
   std::unique_ptr<float3[]> normalSS;
   std::unique_ptr<model_segment_vertex_tangents[]> tangents;
   std::unique_ptr<uint32[]> color;
   std::unique_ptr<float2[]> texcoords;
};

struct model_segment {
   material material;

   math::bounding_box bboxSS;
   math::bounding_box bboxLS;

   std::vector<std::array<uint16, 3>> index_buffer;

   model_segment_vertices vertices;

   std::string bone_name;
   std::vector<uint8> bone_map;
};

struct model_bounding_sphere {
   float3 position;
   float radius = 0.0f;
};

struct model {
   model_lod lod = model_lod::lod0;
   std::string node_name;

   bool pre_inverse_transformed_vertices = false;
   bool no_projection_lights = false;
   bool vertex_lighting = false;
   bool large_texcoords = false;

   math::bounding_box vertex_box;
   math::bounding_box visibility_box;

   uint32 total_triangle_count = 0;

   std::vector<model_shadow> shadows;
   std::vector<model_segment> segments;

   model_bounding_sphere bounding_sphere;
};

struct game_model {
   bool no_game_model = false;

   uint32 lod_group = 0;
   float lod_bias = 1.0f;
};

enum class collision_flags : uint8 {
   all = 0x0,
   soldier = 0x1,
   vehicle = 0x2,
   building = 0x4,
   terrain = 0x8,
   ordnance = 0x10,
   unknown = 0x20,
};

constexpr bool marked_as_enum_bitflag(collision_flags)
{
   return true;
}

struct collision_mesh_bbox_unorm {
   uint8 min_x;
   uint8 min_y;
   uint8 min_z;

   uint8 max_x;
   uint8 max_y;
   uint8 max_z;
};

enum class collision_mesh_node_type : uint8 {
   branch,
   leaf,
};

struct collision_mesh_node {
   collision_mesh_node_type type = collision_mesh_node_type::branch;

   collision_mesh_bbox_unorm bbox;

   uint8 face_index_count = 0;
   uint32 face_index_begin = 0;
};

struct collision_mesh {
   std::string node_name;

   collision_flags mask = collision_flags::all;

   std::vector<float3> vertices;

   math::bounding_box bbox;

   std::vector<uint16> face_indices;
   std::vector<collision_mesh_node> tree;
};

struct collision_primitive {
   std::string name;
   std::string parent;

   collision_flags mask = collision_flags::all;

   float4x4 transform;

   math::bounding_box bboxLS;

   uint32 shape;
   float3 size;
};

struct cloth_vertices {
   std::size_t vertex_count = 0;

   std::unique_ptr<float3[]> position;
   std::unique_ptr<float2[]> texcoords;
};

struct cloth_collision_primitive {
   std::string parent;
   uint32 shape = 0;
   float4x4 transform;
   float3 size;
};

struct cloth {
   std::string name;
   std::string parent;

   float4x4 transform;

   std::string texture_name;

   cloth_vertices vertices;

   std::vector<std::array<uint32, 3>> triangles;

   uint32 fixed_point_count = 0;

   std::vector<std::string> fixed_weights;

   std::vector<std::array<uint32, 2>> stretch_constraints;
   std::vector<std::array<uint32, 2>> bend_constraints;
   std::vector<std::array<uint32, 2>> cross_constraints;

   std::vector<cloth_collision_primitive> collision;
};

struct model_container {
   std::string name;

   skeleton skeleton;

   std::vector<model> models;

   game_model game_model;

   std::optional<collision_mesh> collision_mesh;
   std::vector<collision_primitive> collision_primitives;

   std::vector<cloth> cloths;
};

}