#include "validate_scene.hpp"
#include "error.hpp"

#include <algorithm>
#include <array>
#include <iterator>
#include <ranges>
#include <string_view>

#include <absl/container/inlined_vector.h>
#include <fmt/core.h>

using namespace std::literals;

namespace we::assets::msh {

namespace {

void check_not_empty(const scene& scene)
{
   if (scene.nodes.empty()) {
      throw read_error{".msh file validation failure! No nodes in .msh file.",
                       read_ec::validation_fail_not_empty};
   }
}

void check_node_name_uniqueness(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      if (std::ranges::any_of(scene.nodes, [&](const msh::node& other) {
             if (&other == &node) return false;

             return node.name == other.name;
          })) {
         throw read_error{
            fmt::format(".msh file validation failure! Two or more nodes have "
                        "the same name '{}'.",
                        node.name),
            read_ec::validation_fail_node_name_unique};
      }
   }
}

void check_node_type_validity(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      switch (node.type) {
      case node_type::null:
      case node_type::skinned_mesh:
      case node_type::cloth:
      case node_type::bone:
      case node_type::static_mesh:
      case node_type::shadow_volume:
         continue;
      default:
         throw read_error{
            fmt::format(
               ".msh file validation failure! Node '{}' has unknown node type "
               "'{}'.",
               node.name, static_cast<int32>(node.type)),
            read_ec::validation_fail_node_type_valid};
      }
   }
}

void check_node_parents_validity(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      if (not node.parent) continue;

      if (std::ranges::none_of(scene.nodes, [&](const msh::node& other) {
             return *node.parent == other.name;
          })) {
         throw read_error{
            fmt::format(".msh file validation failure! Node '{}' references "
                        "missing parent '{}'.",
                        node.name, *node.parent),
            read_ec::validation_fail_node_parents_valid,
         };
      }
   }
}

void check_node_parents_noncircular(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      if (not node.parent) continue;

      absl::InlinedVector<const msh::node*, 64> traversed_nodes = {&node};

      for (auto it = std::ranges::find_if(scene.nodes,
                                          [&](const msh::node& other) {
                                             return node.parent == other.name;
                                          });
           it != scene.nodes.cend();
           it = std::ranges::find_if(scene.nodes, [&](const msh::node& other) {
              return it->parent == other.name;
           })) {
         if (std::ranges::contains(traversed_nodes, &(*it))) {
            throw read_error{
               fmt::format(".msh file validation failure! Node '{}' has "
                           "circular relationship with ancestor/parent '{}'.",
                           node.name, it->name),
               read_ec::validation_fail_node_parents_noncircular};
         }

         traversed_nodes.push_back(&(*it));
      }
   }
}

void check_node_mesh_data(const scene& scene)
{
   for (const auto& node : scene.nodes | std::ranges::views::filter([](const auto& node) {
                              return node.type == node_type::null or
                                     node.type == node_type::skinned_mesh or
                                     node.type == node_type::cloth or
                                     node.type == node_type::static_mesh;
                           })) {
      for (const auto& segment : node.segments) {
         if (not segment.triangles.empty()) return;
      }

      if (node.type == node_type::cloth and node.cloth) {
         if (not node.cloth->triangles.empty()) return;
      }

      if (node.collision_primitive and node.name.starts_with("p_")) return;
   }

   throw read_error{
      fmt::format(".msh file validation failure! No node contains mesh data."),
      read_ec::validation_fail_node_mesh_data};
}

void check_geometry_segment_material_index_validity(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      for (std::size_t index = 0; index < node.segments.size(); ++index) {
         const auto& segment = node.segments[index];

         if (segment.material_index < 0 or
             segment.material_index >= std::ssize(scene.materials)) {
            throw read_error{
               fmt::format(".msh file validation failure! The material index "
                           "'{}' in geometry segment #{} in node '{}'  "
                           " is out of range. Max material index is '{}'.",
                           segment.material_index, index, node.name,
                           std::ssize(scene.materials)),
               read_ec::validation_fail_geometry_segment_material_index};
         }
      }
   }
}

void check_geometry_segment_attibutes_count_matches(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      for (std::size_t index = 0; index < node.segments.size(); ++index) {
         const auto& segment = node.segments[index];

         const std::size_t positions_count = segment.positions.size();
         const std::array attribute_counts{
            segment.weights ? segment.weights->size() : positions_count,
            segment.normals ? segment.normals->size() : positions_count,
            segment.texcoords ? segment.texcoords->size() : positions_count,
            segment.colors ? segment.colors->size() : positions_count,
         };

         if (not std::ranges::all_of(attribute_counts, [=](const std::size_t size) {
                return size == positions_count;
             })) {
            throw read_error{
               fmt::format(".msh file validation failure! Geometry segment "
                           "#{} in node '{}'  "
                           " has mismatched vertex attribute counts.\n"
                           "   position count: {}\n"
                           "   weights count: {}\n"
                           "   normals count: {}\n"
                           "   texcoords count: {}\n"
                           "   colors count: {}",
                           index, node.name, positions_count,
                           segment.weights ? std::to_string(segment.weights->size())
                                           : "nullopt"s,
                           segment.normals ? std::to_string(segment.normals->size())
                                           : "nullopt"s,
                           segment.texcoords ? std::to_string(segment.texcoords->size())
                                             : "nullopt"s,
                           segment.colors ? std::to_string(segment.colors->size())
                                          : "nullopt"s),
               read_ec::validation_fail_geometry_segment_attibutes_count_matches};
         }
      }
   }
}

void check_geometry_segment_vertex_count_limit(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      for (std::size_t index = 0; index < node.segments.size(); ++index) {
         const auto& segment = node.segments[index];

         if (segment.positions.size() > geometry_segment::max_vertex_count) {
            throw read_error{
               fmt::format(
                  ".msh file validation failure! Geometry segment "
                  "#{} in node '{}'  "
                  " has '{}' vertices. This is invalid as the max a geometry "
                  "segment can "
                  "index is '{}'.",
                  index, node.name, segment.positions.size(),
                  geometry_segment::max_vertex_count),
               read_ec::validation_fail_geometry_segment_vertex_count_limit};
         }
      }
   }
}

void check_geometry_segment_triangles_index_validity(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      for (std::size_t index = 0; index < node.segments.size(); ++index) {
         const auto& segment = node.segments[index];

         for (const auto& tri : segment.triangles) {
            for (const auto i : tri) {
               if (i >= segment.positions.size()) {
                  throw read_error{
                     fmt::format(".msh file validation failure! A triangle in "
                                 "geometry segment #{} in node '{}' contains a "
                                 "vertex index that is out of range! "
                                 "Vertex count '{}', out of range index '{}'.",
                                 index, node.name, segment.positions.size(), i),
                     read_ec::validation_fail_geometry_segment_triangles_index_valid};
               }
            }
         }
      }
   }
}

void check_geometry_segment_no_nans(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      for (std::size_t segment_index = 0; segment_index < node.segments.size();
           ++segment_index) {
         const auto& segment = node.segments[segment_index];

         for (std::size_t vertex_index = 0;
              vertex_index < segment.positions.size(); ++vertex_index) {
            const float3 position = segment.positions[vertex_index];

            if (position.x != position.x or position.y != position.y or
                position.z != position.z) {
               throw read_error{
                  fmt::format(".msh file validation failure! Geometry segment "
                              "#{} in node "
                              "'{}' has a NaN (Not a Number) at vertex #{}.",
                              segment_index, node.name, vertex_index),
                  read_ec::validation_fail_geometry_segment_no_nans};
            }
         }
      }
   }
}

void check_geometry_segment_weights_bone_indices_validity(const scene& scene)
{
   for (const node& node : scene.nodes) {
      for (std::size_t index = 0; index < node.segments.size(); ++index) {
         const geometry_segment& segment = node.segments[index];

         if (not segment.weights) continue;

         for (const std::array<vertex_weight, 4>& weights : *segment.weights) {
            for (const vertex_weight& weight : weights) {
               if (weight.bone_index >= node.bone_map.size()) {
                  throw read_error{
                     fmt::format(
                        ".msh file validation failure! A vertex weight in "
                        "geometry segment #{} in node '{}' contains a "
                        "bone index that is out of range! "
                        "Bone map (ENVL) size '{}', out of range index '{}'.",
                        index, node.name, node.bone_map.size(), weight.bone_index),
                     read_ec::validation_fail_geometry_segment_weights_bone_indices_valid};
               }
            }
         }
      }
   }
}

void check_shadow_volume_edges_validity(const scene& scene)
{
   // We validate 3 things in order here.
   // - That vertex, next edge and twin edge indices are valid. (We can use them without going out of bounds.)
   // - That twins are valid.
   // - That every edge is part of a valid loop.

   std::vector<bool> visited;

   for (const node& node : scene.nodes) {
      for (std::size_t index = 0; index < node.shadow_volumes.size(); ++index) {
         const shadow_volume& shadow_volume = node.shadow_volumes[index];

         for (std::size_t edge_index = 0;
              edge_index < shadow_volume.edges.size(); ++edge_index) {
            const shadow_volume_half_edge& edge = shadow_volume.edges[edge_index];

            if (edge.vertex >= shadow_volume.positions.size()) {
               throw read_error{
                  fmt::format(".msh file validation failure! Shadow volume #{} "
                              "in node '{}' is invalid! "
                              "Vertex for edge #{} is out of range (Vertex "
                              "'{}' Vertex Count '{}'.",
                              index, node.name, edge_index, edge.vertex,
                              shadow_volume.positions.size()),
                  read_ec::validation_fail_shadow_volume_edges_valid};
            }

            if (edge.next >= shadow_volume.edges.size()) {
               throw read_error{
                  fmt::format(
                     ".msh file validation failure! Shadow volume #{} "
                     "in node '{}' is invalid! "
                     "Next edge for edge #{} is out of range (Next Edge "
                     "'{}' Edge Count '{}'.",
                     index, node.name, edge_index, edge.next,
                     shadow_volume.edges.size()),
                  read_ec::validation_fail_shadow_volume_edges_valid};
            }

            if (edge.twin >= shadow_volume.edges.size()) {
               throw read_error{
                  fmt::format(
                     ".msh file validation failure! Shadow volume #{} "
                     "in node '{}' is invalid! "
                     "Twin edge for edge #{} is out of range (Next Edge "
                     "'{}' Edge Count '{}'.",
                     index, node.name, edge_index, edge.twin,
                     shadow_volume.edges.size()),
                  read_ec::validation_fail_shadow_volume_edges_valid};
            }
         }

         for (std::size_t edge_index = 0;
              edge_index < shadow_volume.edges.size(); ++edge_index) {
            const shadow_volume_half_edge& edge = shadow_volume.edges[edge_index];
            const shadow_volume_half_edge& twin_edge = shadow_volume.edges[edge.twin];

            if (edge.vertex != shadow_volume.edges[twin_edge.next].vertex) {
               throw read_error{
                  fmt::format(".msh file validation failure! Shadow volume #{} "
                              "in node '{}' is invalid! "
                              "Twin edge for edge #{} is invalid. Vertices do "
                              "not match! Edge Vertex "
                              "'{}' Twin Next Vertex '{}'.",
                              index, node.name, edge_index, edge.vertex,
                              shadow_volume.edges[twin_edge.next].vertex),
                  read_ec::validation_fail_shadow_volume_edges_valid};
            }

            if (twin_edge.vertex != shadow_volume.edges[edge.next].vertex) {
               throw read_error{
                  fmt::format(".msh file validation failure! Shadow volume #{} "
                              "in node '{}' is invalid! "
                              "Twin edge for edge #{} is invalid. Vertices do "
                              "not match! Twin Vertex "
                              "'{}' Edge Next Vertex '{}'.",
                              index, node.name, edge_index, twin_edge.vertex,
                              shadow_volume.edges[edge.next].vertex),
                  read_ec::validation_fail_shadow_volume_edges_valid};
            }
         }

         visited.clear();
         visited.resize(shadow_volume.edges.size(), false);

         for (std::size_t edge_index = 0;
              edge_index < shadow_volume.edges.size(); ++edge_index) {
            if (visited[edge_index]) continue;

            visited[edge_index] = true;

            for (std::size_t current_edge_index =
                    shadow_volume.edges[edge_index].next;
                 current_edge_index != edge_index;
                 current_edge_index = shadow_volume.edges[current_edge_index].next) {
               if (visited[current_edge_index]) {
                  throw read_error{
                     fmt::format(
                        ".msh file validation failure! Shadow volume #{} "
                        "in node '{}' is invalid! "
                        "Edge #{} does not form a valid, unqiue loop. Double "
                        "Visited Edge Index '{}'",
                        index, node.name, edge_index, current_edge_index),
                     read_ec::validation_fail_shadow_volume_edges_valid};
               }

               visited[current_edge_index] = true;
            }
         }
      }
   }
}

void check_cloth_attibutes_count_matches(const scene& scene)
{
   for (const node& node : scene.nodes) {
      if (not node.cloth) continue;

      const cloth& cloth = *node.cloth;

      if (cloth.positions.size() != cloth.texcoords.size()) {
         throw read_error{
            fmt::format(".msh file validation failure! Cloth in node '{}' has "
                        "mismatched vertex attribute counts.\n"
                        "   position count: {}\n"
                        "   texcoords count: {}",
                        node.name, cloth.positions.size(), cloth.texcoords.size()),
            read_ec::validation_fail_cloth_attibutes_count_matches};
      }
   }
}

void check_cloth_fixed_index_validity(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      if (not node.cloth) continue;

      const cloth& cloth = *node.cloth;

      for (uint32 fixed_index : cloth.fixed_indices) {
         if (fixed_index > cloth.positions.size()) {
            throw read_error{
               fmt::format(".msh file validation failure! Fixed weight index "
                           "'{}' in cloth in node '{}'"
                           " is out of range. Vertex count is '{}'.",
                           fixed_index, node.name, cloth.positions.size()),
               read_ec::validation_fail_cloth_fixed_index_valid};
         }
      }
   }
}

void check_cloth_fixed_weights_validity(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      if (not node.cloth) continue;

      const cloth& cloth = *node.cloth;

      for (std::string_view fixed_weight : cloth.fixed_weights) {
         bool found = false;
         bool is_bone = false;

         for (const auto& other_node : scene.nodes) {
            if (other_node.name == fixed_weight) {
               found = true;
               is_bone = other_node.type == node_type::bone;

               break;
            }
         }

         if (not found) {
            throw read_error{
               fmt::format(
                  ".msh file validation failure! Node for fixed weight "
                  "'{}' in cloth in node '{}' does not exist.",
                  fixed_weight, node.name),
               read_ec::validation_fail_cloth_fixed_weight_valid};
         }
         else if (not is_bone) {
            throw read_error{
               fmt::format(
                  ".msh file validation failure! Node for fixed weight "
                  "'{}' in cloth in node '{}' exists but is not a bone.",
                  fixed_weight, node.name),
               read_ec::validation_fail_cloth_fixed_weight_valid};
         }
      }
   }
}

void check_cloth_triangles_index_validity(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      if (not node.cloth) continue;

      const cloth& cloth = *node.cloth;

      for (const std::array<uint32, 3>& tri : cloth.triangles) {
         for (uint32 index : tri) {
            if (index > cloth.positions.size()) {
               throw read_error{
                  fmt::format(".msh file validation failure! A triangle in "
                              "cloth in node '{}' contains a "
                              "vertex index that is out of range! "
                              "Vertex count '{}', out of range index '{}'.",
                              node.name, cloth.positions.size(), index),
                  read_ec::validation_fail_cloth_triangles_index_valid};
            }
         }
      }
   }
}

void check_cloth_constraints_validity(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      if (not node.cloth) continue;

      const cloth& cloth = *node.cloth;

      for (const std::array<uint16, 2>& constraint : cloth.stretch_constraints) {
         for (uint16 index : constraint) {
            if (index > cloth.positions.size()) {
               throw read_error{
                  fmt::format(".msh file validation failure! A stretch "
                              "constraint pair in "
                              "cloth in node '{}' contains a "
                              "vertex index that is out of range! "
                              "Vertex count '{}', out of range index '{}'.",
                              node.name, cloth.positions.size(), index),
                  read_ec::validation_fail_cloth_constraints_valid};
            }
         }
      }

      for (const std::array<uint16, 2>& constraint : cloth.cross_constraints) {
         for (uint16 index : constraint) {
            if (index > cloth.positions.size()) {
               throw read_error{
                  fmt::format(".msh file validation failure! A cross "
                              "constraint pair in "
                              "cloth in node '{}' contains a "
                              "vertex index that is out of range! "
                              "Vertex count '{}', out of range index '{}'.",
                              node.name, cloth.positions.size(), index),
                  read_ec::validation_fail_cloth_constraints_valid};
            }
         }
      }

      for (const std::array<uint16, 2>& constraint : cloth.bend_constraints) {
         for (uint16 index : constraint) {
            if (index > cloth.positions.size()) {
               throw read_error{
                  fmt::format(".msh file validation failure! A bend "
                              "constraint pair in "
                              "cloth in node '{}' contains a "
                              "vertex index that is out of range! "
                              "Vertex count '{}', out of range index '{}'.",
                              node.name, cloth.positions.size(), index),
                  read_ec::validation_fail_cloth_constraints_valid};
            }
         }
      }
   }
}

void check_cloth_collision_parent_validity(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      if (not node.cloth) continue;

      const cloth& cloth = *node.cloth;

      for (const cloth_collision_primitive& collision : cloth.collision) {
         bool found = false;

         for (const auto& other_node : scene.nodes) {
            if (other_node.name == collision.parent) {
               found = true;

               break;
            }
         }

         if (not found) {
            throw read_error{
               fmt::format(
                  ".msh file validation failure! Parent for collision "
                  "primitive '{}' in cloth in node '{}' does not exist.",
                  collision.name, collision.parent, node.name),
               read_ec::validation_fail_cloth_collision_parent_valid};
         }
      }
   }
}

void check_cloth_collision_shape_validity(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      if (not node.cloth) continue;

      const cloth& cloth = *node.cloth;

      for (const cloth_collision_primitive& collision : cloth.collision) {
         switch (collision.shape) {
         case cloth_collision_primitive_shape::sphere:
         case cloth_collision_primitive_shape::cylinder:
         case cloth_collision_primitive_shape::box:
            break;
         default:
            throw read_error{
               fmt::format(".msh file validation failure! Collision "
                           "primitive '{}' in cloth in node '{}' has invalid "
                           "shape '{}'.",
                           collision.name, node.name,
                           static_cast<uint32>(collision.shape)),
               read_ec::validation_fail_cloth_collision_shape_valid};
         }
      }
   }
}

}

void validate_scene(const scene& scene)
{
   constexpr std::array validation_checks{check_not_empty,
                                          check_node_name_uniqueness,
                                          check_node_type_validity,
                                          check_node_parents_validity,
                                          check_node_parents_noncircular,
                                          check_node_mesh_data,
                                          check_geometry_segment_material_index_validity,
                                          check_geometry_segment_attibutes_count_matches,
                                          check_geometry_segment_vertex_count_limit,
                                          check_geometry_segment_triangles_index_validity,
                                          check_geometry_segment_no_nans,
                                          check_geometry_segment_weights_bone_indices_validity,
                                          check_shadow_volume_edges_validity,
                                          check_cloth_attibutes_count_matches,
                                          check_cloth_fixed_index_validity,
                                          check_cloth_fixed_weights_validity,
                                          check_cloth_triangles_index_validity,
                                          check_cloth_constraints_validity,
                                          check_cloth_collision_parent_validity,
                                          check_cloth_collision_shape_validity};

   for (auto check : validation_checks) {
      check(scene);
   }
}
}
