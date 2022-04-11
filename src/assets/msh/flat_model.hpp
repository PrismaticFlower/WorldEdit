#pragma once

#include "flat_model_bvh.hpp"
#include "math/bounding_box.hpp"
#include "scene.hpp"

#include <memory>
#include <optional>

#include <boost/variant2/variant.hpp>

namespace we::assets::msh {

struct mesh {
   math::bounding_box bounding_box;

   material material;

   std::vector<float3> positions;
   std::vector<float3> normals;
   std::vector<float3> tangents;
   std::vector<float3> bitangents;
   std::vector<float4> colors;
   std::vector<float2> texcoords;

   std::vector<std::array<uint16, 3>> triangles;

   void regenerate_bounding_box() noexcept;
};

struct flat_model_node {
   std::string name;
   transform transform;
   node_type type = node_type::null;
   bool hidden = false;

   std::vector<flat_model_node> children;
};

struct flat_model_collision {
   math::bounding_box bounding_box;

   struct mesh {
      std::vector<float3> positions;
      std::vector<std::array<uint16, 3>> triangles;
   };

   struct primitive {
      transform transform;

      collision_primitive_shape shape = collision_primitive_shape::sphere;
      float radius = 0.0f;
      float height = 0.0f;
      float length = 0.0f;
   };

   boost::variant2::variant<primitive, mesh> geometry;

   void regenerate_bounding_box() noexcept;
};

struct flat_model {
   explicit flat_model(const scene& scene) noexcept;

   math::bounding_box bounding_box;

   std::vector<mesh> meshes;
   std::vector<flat_model_collision> collision;
   std::vector<flat_model_node> node_hierarchy;

   flat_model_bvh bvh;

   void regenerate_bounding_boxes() noexcept;

private:
   void flatten_segments_to_meshes(const std::vector<geometry_segment>& segments,
                                   const float4x4& node_to_object,
                                   const std::vector<material>& scene_materials);

   void flatten_node_to_collision(const node& node, const float4x4& node_to_object);

   auto select_mesh_for_segment(const geometry_segment& segment,
                                const material& material) -> mesh&;

   void generate_tangents_for_meshes();
};

}
