#include "validate_scene.hpp"

#include <array>
#include <iterator>
#include <string_view>

#include <boost/container/small_vector.hpp>
#include <fmt/format.h>
#include <range/v3/algorithm.hpp>
#include <range/v3/view.hpp>

using namespace std::literals;

namespace we::assets::msh {

namespace {

void check_node_name_uniqueness(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      if (ranges::any_of(scene.nodes, [&](const msh::node& other) {
             if (&other == &node) return false;

             return node.name == other.name;
          })) {
         throw std::runtime_error{
            fmt::format(".msh file validation failure! Two or more nodes have "
                        "the same name '{}'."sv,
                        node.name)};
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
         throw std::runtime_error{fmt::format(
            ".msh file validation failure! Node '{}' has unknown node type "
            "'{}'."sv,
            node.name, static_cast<int32>(node.type))};
      }
   }
}

void check_node_parents_validity(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      if (not node.parent) continue;

      if (ranges::none_of(scene.nodes, [&](const msh::node& other) {
             return *node.parent == other.name;
          })) {
         throw std::runtime_error{
            fmt::format(".msh file validation failure! Node '{}' references missing parent '{}'."sv,
                        node.name, *node.parent)};
      }
   }
}

void check_node_parents_noncircular(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      if (not node.parent) continue;

      boost::container::small_vector<const msh::node*, 64> traversed_nodes = {&node};

      for (auto it = ranges::find_if(scene.nodes,
                                     [&](const msh::node& other) {
                                        return node.parent == other.name;
                                     });
           it != scene.nodes.cend();
           it = ranges::find_if(scene.nodes, [&](const msh::node& other) {
              return it->parent == other.name;
           })) {
         if (ranges::contains(traversed_nodes, &(*it))) {
            throw std::runtime_error{
               fmt::format(".msh file validation failure! Node '{}' has circular relationship with ancestor/parent '{}'."sv,
                           node.name, it->name)};
         }

         traversed_nodes.push_back(&(*it));
      }
   }
}

void check_geometry_segment_material_index_validity(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      for (const auto& [index, segment] : ranges::views::enumerate(node.segments)) {
         if (segment.material_index < 0 or
             segment.material_index >= std::ssize(scene.materials)) {
            throw std::runtime_error{
               fmt::format(".msh file validation failure! The material index "
                           "'{}' in geometry segment #{} in node '{}'  "
                           " is out of range. Max material index is '{}'."sv,
                           segment.material_index, index, node.name,
                           std::ssize(scene.materials))};
         }
      }
   }
}

void check_geometry_segment_attibutes_count_matches(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      for (const auto& [index, segment] : ranges::views::enumerate(node.segments)) {
         const std::size_t positions_count = segment.positions.size();
         const std::array attribute_counts{segment.normals ? segment.normals->size()
                                                           : positions_count,
                                           segment.texcoords
                                              ? segment.texcoords->size()
                                              : positions_count,
                                           segment.colors ? segment.colors->size()
                                                          : positions_count};

         if (not ranges::all_of(attribute_counts, [=](const std::size_t size) {
                return size == positions_count;
             })) {
            throw std::runtime_error{fmt::format(
               ".msh file validation failure! Geometry segment "
               "#{} in node '{}'  "
               " has mismatched vertex attribute counts.\n"
               "   position count: {}\n"
               "   normals count: {}\n"
               "   texcoords count: {}\n"
               "   colors count: {}"sv,
               index, node.name, positions_count,
               segment.normals ? std::to_string(segment.normals->size()) : "nullopt"sv,
               segment.texcoords ? std::to_string(segment.texcoords->size()) : "nullopt"sv,
               segment.colors ? std::to_string(segment.colors->size()) : "nullopt"sv)};
         }
      }
   }
}

void check_geometry_segment_vertex_count_limit(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      for (const auto& [index, segment] : ranges::views::enumerate(node.segments)) {
         if (segment.positions.size() > geometry_segment::max_vertex_count) {
            throw std::runtime_error{fmt::format(
               ".msh file validation failure! Geometry segment "
               "#{} in node '{}'  "
               " has '{}' vertices. This is invalid as the max a geometry "
               "segment can "
               "index is '{}'."sv,
               index, node.name, segment.positions.size(),
               geometry_segment::max_vertex_count)};
         }
      }
   }
}

void check_geometry_segment_triangles_index_validity(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      for (const auto& [index, segment] : ranges::views::enumerate(node.segments)) {
         for (const auto& tri : segment.triangles) {
            for (const auto i : tri) {
               if (i >= segment.positions.size()) {
                  throw std::runtime_error{
                     fmt::format(".msh file validation failure! A triangle in "
                                 "geometry segment #{} in node '{}' contains a "
                                 "vertex index that is out of range! "
                                 "Vertex count '{}', out of range index '{}'."sv,
                                 index, node.name, segment.positions.size(), i)};
               }
            }
         }
      }
   }
}

void check_collision_primitive_shape_validity(const scene& scene)
{
   for (const auto& node : scene.nodes) {
      if (not node.collision_primitive) continue;

      switch (node.collision_primitive->shape) {
      case collision_primitive_shape::sphere:
      case collision_primitive_shape::cylinder:
      case collision_primitive_shape::box:
         continue;
      default:
         throw std::runtime_error{
            fmt::format(".msh file validation failure! The collision primitive "
                        "for node '{}' has unknown shape "
                        "'{}'."sv,
                        node.name,
                        static_cast<int32>(node.collision_primitive->shape))};
      }
   }
}

}

void validate_scene(const scene& scene)
{
   constexpr std::array validation_checks{check_node_name_uniqueness,
                                          check_node_type_validity,
                                          check_node_parents_validity,
                                          check_node_parents_noncircular,
                                          check_geometry_segment_material_index_validity,
                                          check_geometry_segment_attibutes_count_matches,
                                          check_geometry_segment_vertex_count_limit,
                                          check_geometry_segment_triangles_index_validity,
                                          check_collision_primitive_shape_validity};

   for (auto check : validation_checks) {
      check(scene);
   }
}

}
