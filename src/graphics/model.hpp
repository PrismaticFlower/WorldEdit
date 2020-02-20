#pragma once

#include "assets/msh/scene.hpp"
#include "bounding_box.hpp"
#include "types.hpp"

#include <array>
#include <vector>

#include <boost/container/small_vector.hpp>

namespace sk::graphics {

struct mesh_vertex {
   vec3 position;
   vec2 texcoords;
   vec2 lightmap_texcoords;
   vec3 tangent;
   vec3 bitangent;
   vec3 normal;
   uint32 colour;
};

struct mesh_vertex_packed {
   i16vec4 position;
   i16vec4 qtangent;
   i16vec2 texcoords;
   i16vec2 lightmap_texcoords;
   uint32 colour;
};

struct model_part {
   int index_count;
   int start_index;
   int start_vertex;
};

struct model {
   explicit model(const assets::msh::scene& msh_scene);

   bounding_box bbox;

   std::vector<mesh_vertex_packed> vertices;
   std::vector<std::array<uint16, 3>> indices;

   boost::container::small_vector<model_part, 4> parts;
};

}
