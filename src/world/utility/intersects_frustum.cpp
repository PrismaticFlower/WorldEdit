#include "intersects_frustum.hpp"

#include "../object_class.hpp"
#include "../object_class_library.hpp"

#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

namespace we::world {

bool intersects(const frustum& frustumWS, const object& object,
                const object_class_library& object_classes) noexcept
{
   const quaternion inverse_rotation = conjugate(object.rotation);
   const float3 inverse_position = inverse_rotation * -object.position;

   const frustum frustumOS = transform(frustumWS, inverse_rotation, inverse_position);

   return object_classes[object.class_handle].model->bvh.intersects(frustumOS);
}

bool intersects(const frustum& frustumWS, const light& light) noexcept
{
   switch (light.light_type) {
   case light_type::directional:
      return intersects(frustumWS, light.position, 0.0f);
   case light_type::point:
      return intersects(frustumWS, light.position, light.range);
   case light_type::spot: {
      const float outer_cone_radius =
         light.range * std::tan(light.outer_cone_angle * 0.5f);
      const float inner_cone_radius =
         light.range * std::tan(light.inner_cone_angle * 0.5f);
      const float radius = std::min(outer_cone_radius, inner_cone_radius);

      math::bounding_box bbox{.min = {-radius, -radius, 0.0f},
                              .max = {radius, radius, light.range}};

      bbox = light.rotation * bbox + light.position;

      return intersects(frustumWS, bbox);
   }
   case light_type::directional_region_box: {
      math::bounding_box bbox{.min = {-light.region_size}, .max = {light.region_size}};

      bbox = light.region_rotation * bbox + light.position;

      return intersects(frustumWS, bbox);
   }
   case light_type::directional_region_sphere:
      return intersects(frustumWS, light.position, length(light.region_size));
   case light_type::directional_region_cylinder: {
      const float cylinder_length =
         length(float2{light.region_size.x, light.region_size.z});

      math::bounding_box bbox{.min = {-cylinder_length, -light.region_size.y,
                                      -cylinder_length},
                              .max = {cylinder_length, light.region_size.y,
                                      cylinder_length}};

      bbox = light.region_rotation * bbox + light.position;

      return intersects(frustumWS, bbox);
   }
   default:
      return false;
   }
}

bool intersects(const frustum& frustumWS, const region& region) noexcept
{
   switch (region.shape) {
   case region_shape::box: {
      math::bounding_box bbox{.min = {-region.size}, .max = {region.size}};

      bbox = region.rotation * bbox + region.position;

      return intersects(frustumWS, bbox);
   }
   case region_shape::sphere:
      return intersects(frustumWS, region.position, length(region.size));
   case region_shape::cylinder: {
      const float cylinder_length = length(float2{region.size.x, region.size.z});

      math::bounding_box bbox{.min = {-cylinder_length, -region.size.y, -cylinder_length},
                              .max = {cylinder_length, region.size.y, cylinder_length}};

      bbox = region.rotation * bbox + region.position;

      return intersects(frustumWS, bbox);
   }
   default:
      return false;
   }
}

bool intersects(const frustum& frustumWS, const sector& sector) noexcept
{
   float2 point_min{FLT_MAX, FLT_MAX};
   float2 point_max{-FLT_MAX, -FLT_MAX};

   for (auto& point : sector.points) {
      point_min = min(point, point_min);
      point_max = max(point, point_max);
   }

   const math::bounding_box bboxWS{{point_min.x, sector.base, point_min.y},
                                   {point_max.x, sector.base + sector.height,
                                    point_max.y}};

   return intersects(frustumWS, bboxWS);
}

bool intersects(const frustum& frustumWS, const portal& portal) noexcept
{
   return intersects(frustumWS, portal.position, std::max(portal.height, portal.width));
}

bool intersects(const frustum& frustumWS, const barrier& barrier,
                const float visualizer_height) noexcept
{
   math::bounding_box bboxWS{{-barrier.size.x, -visualizer_height, -barrier.size.y},
                             {barrier.size.x, visualizer_height, barrier.size.y}};
   bboxWS = make_quat_from_euler({0.0f, barrier.rotation_angle, 0.0f}) * bboxWS +
            barrier.position;

   return intersects(frustumWS, bboxWS);
}

bool intersects(const frustum& frustumWS, const hintnode& hintnode) noexcept
{
   return intersects(frustumWS, hintnode.position, 2.0f);
}

bool intersects(const frustum& frustumWS, const planning_hub& hub,
                const float visualizer_height) noexcept
{

   math::bounding_box bboxWS{{-hub.radius, -visualizer_height, -hub.radius},
                             {hub.radius, visualizer_height, hub.radius}};
   bboxWS = bboxWS + hub.position;

   return intersects(frustumWS, bboxWS);
}

bool intersects(const frustum& frustumWS, const planning_connection& connection,
                std::span<const planning_hub> hubs, const float visualizer_height) noexcept
{
   const planning_hub& start = hubs[connection.start_hub_index];
   const planning_hub& end = hubs[connection.end_hub_index];

   const math::bounding_box start_bboxWS{
      .min = float3{-start.radius, -visualizer_height, -start.radius} + start.position,
      .max = float3{start.radius, visualizer_height, start.radius} + start.position};
   const math::bounding_box end_bboxWS{
      .min = float3{-end.radius, -visualizer_height, -end.radius} + end.position,
      .max = float3{end.radius, visualizer_height, end.radius} + end.position};

   const math::bounding_box bboxWS = math::combine(start_bboxWS, end_bboxWS);

   if (not intersects(frustumWS, bboxWS)) return false;

   const float3 normal = normalize(float3{-(start.position.z - end.position.z), 0.0f,
                                          start.position.x - end.position.x});

   const std::array<float3, 4> quadWS = {start.position + normal * start.radius,
                                         start.position - normal * start.radius,
                                         end.position + normal * end.radius,
                                         end.position - normal * end.radius};

   const float3 height_offset = {0.0f, visualizer_height, 0.0f};

   const std::array<float3, 8> cornersWS = {quadWS[0] + height_offset,
                                            quadWS[1] + height_offset,
                                            quadWS[2] + height_offset,
                                            quadWS[3] + height_offset,
                                            quadWS[0] - height_offset,
                                            quadWS[1] - height_offset,
                                            quadWS[2] - height_offset,
                                            quadWS[3] - height_offset};

   constexpr static std::array<std::array<int8, 3>, 12> tris = {
      // Top
      3, 2, 0, //
      3, 0, 1, //

      // Bottom
      4, 6, 7, //
      5, 4, 7, //

      // Side 0
      0, 6, 4, //
      0, 2, 6, //

      // Side 1
      1, 5, 7, //
      7, 3, 1, //

      // Back
      4, 1, 0, //
      5, 1, 4, //

      // Front
      2, 3, 6, //
      6, 3, 7  //
   };

   for (const std::array<int8, 3>& tri : tris) {
      if (intersects(frustumWS, cornersWS[tri[0]], cornersWS[tri[1]],
                     cornersWS[tri[2]])) {
         return true;
      }
   }

   return false;
}

bool intersects(const frustum& frustumWS, const boundary& boundary,
                const float visualizer_height) noexcept
{
   const std::span<const float3> nodes = boundary.points;

   for (std::size_t i = 0; i < nodes.size(); ++i) {
      const float3 a = nodes[i];
      const float3 b = nodes[(i + 1) % nodes.size()];

      const std::array quadWS = {
         float3{b.x, b.y + visualizer_height, b.z},
         float3{a.x, a.y + visualizer_height, a.z},
         float3{a.x, a.y - visualizer_height, a.z},
         float3{b.x, b.y - visualizer_height, b.z},
      };

      if (intersects(frustumWS, quadWS[0], quadWS[1], quadWS[2]) or
          intersects(frustumWS, quadWS[0], quadWS[2], quadWS[3])) {
         return true;
      }
   }

   return false;
}

bool intersects(const frustum& frustumWS, const measurement& measurement) noexcept
{
   math::bounding_box bboxWS{min(measurement.start, measurement.end),
                             max(measurement.start, measurement.end)};

   return intersects(frustumWS, bboxWS);
}

}