#pragma once

#include "material.hpp"
#include "types.hpp"

#include <array>
#include <optional>
#include <vector>

namespace sk::assets::msh {

enum class node_type : int32 {
   null = 0,
   skinned_mesh = 1,
   cloth = 2,
   bone = 3,
   static_mesh = 4
};

enum class collision_primitive_shape : int32 {
   sphere = 0,
   cylinder = 2,
   box = 4
};

struct transform {
   glm::vec3 translation = {0.0f, 0.0f, 0.0f};
   glm::quat rotation = {1.0f, 0.0f, 0.0f, 0.0f};
};

struct geometry_segment {
   int32 material_index = 0;

   std::vector<vec3> positions;
   std::vector<vec3> normals;
   std::optional<std::vector<vec4>> colors;
   std::vector<vec2> texcoords;

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
   std::optional<collision_primitive> collision_primitive;
};

struct scene {
   std::vector<material> materials;
   std::vector<node> nodes;
};

}
