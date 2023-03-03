#include "sector_fill.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <limits>

namespace we::world {

namespace {

bool intersection(const float2 a_start, const float2 a_end,
                  const float2 b_start, const float2 b_end) noexcept
{
   const float denominator = (b_end.y - b_start.y) * (a_end.x - a_start.x) -
                             (b_end.x - b_start.x) * (a_end.y - a_start.y);

   const float a_u = (b_end.x - b_start.x) * (a_start.y - b_start.y) -
                     (b_end.y - b_start.y) * (a_start.x - b_start.x);
   const float b_u = (a_end.x - a_start.x) * (a_start.y - b_start.y) -
                     (a_end.y - a_start.y) * (a_start.x - b_start.x);

   const float a_t = a_u / denominator;
   const float b_t = b_u / denominator;

   return (a_t >= 0.0f) and (a_t <= 1.0f) and (b_t >= 0.0f) and (b_t <= 1.0f);
}

bool inside_sector(const std::span<const float2> sector_points,
                   const float2 sector_min, const float2 test_point) noexcept
{
   int intersections = 0;

   const float2 point_start = sector_min - 0.001f;

   for (std::size_t i = 0; i < sector_points.size(); ++i) {
      const float2 sector_start = sector_points[i];
      const float2 sector_end = sector_points[(i + 1) % sector_points.size()];

      if (intersection(sector_start, sector_end, point_start, test_point)) {
         intersections += 1;
      }
   }

   return (intersections % 2) == 1;
}

bool inside_sector(const std::span<const float2> sector_points,
                   const math::bounding_box bbox) noexcept
{
   const std::array bbox_corners{float2{bbox.min.x, bbox.min.z},
                                 float2{bbox.min.x, bbox.max.z},
                                 float2{bbox.max.x, bbox.max.z},
                                 float2{bbox.max.x, bbox.min.z}};

   struct line {
      float2 v0;
      float2 v1;
   };

   const std::array bbox_lines{line{bbox_corners[0], bbox_corners[1]},
                               line{bbox_corners[1], bbox_corners[2]},
                               line{bbox_corners[2], bbox_corners[3]},
                               line{bbox_corners[3], bbox_corners[0]}};

   for (std::size_t i = 0; i < sector_points.size(); ++i) {
      const float2 sector_start = sector_points[i];
      const float2 sector_end = sector_points[(i + 1) % sector_points.size()];

      int intersections = 0;

      for (auto [v0, v1] : bbox_lines) {
         if (intersection(sector_start, sector_end, v0, v1)) {
            intersections += 1;
         }
      }

      if ((intersections % 2) == 1) return true;
   }

   return false;
}

}

auto sector_fill(const sector& sector, const std::span<const object> world_objects,
                 const absl::flat_hash_map<lowercase_string, object_class>& object_classes)
   -> std::vector<std::string>
{
   if (sector.points.empty() or sector.points.size() < 3) return {};

   float3 sector_min{std::numeric_limits<float>::max(), 0.0f,
                     std::numeric_limits<float>::max()};
   float3 sector_max = -sector_min;

   for (const auto& point : sector.points) {
      sector_min = min(sector_min, {point.x, 0.0f, point.y});
      sector_max = max(sector_max, {point.x, 0.0f, point.y});
   }

   sector_min.y = sector.base;
   sector_max.y = sector.base + sector.height;

   std::vector<std::string> sector_objects;
   sector_objects.reserve(32);

   for (auto& object : world_objects) {
      if (object.name.empty()) continue;

      const auto object_class_it = object_classes.find(object.class_name);

      if (object_class_it == object_classes.end()) continue;

      const math::bounding_box model_bbox = object_class_it->second.model->bounding_box;
      const math::bounding_box bbox = object.rotation * model_bbox + object.position;

      if (bbox.min.x > sector_max.x or bbox.max.x < sector_min.x or
          bbox.min.y > sector_max.y or bbox.max.y < sector_min.y or
          bbox.min.z > sector_max.z or bbox.max.z < sector_min.z) {
         continue;
      }

      const float3 object_centre = (bbox.min + bbox.max) / 2.0f;

      if (inside_sector(sector.points, {sector_min.x, sector_min.z},
                        {object_centre.x, object_centre.z}) or
          inside_sector(sector.points, bbox)) {
         sector_objects.push_back(object.name);
      }
   }

   return sector_objects;
}
}