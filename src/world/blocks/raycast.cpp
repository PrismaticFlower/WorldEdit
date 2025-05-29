#include "raycast.hpp"
#include "mesh_geometry.hpp"

#include "math/intersectors.hpp"
#include "math/iq_intersectors.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

namespace we::world {

namespace {

struct raycast_block_result_local {
   float distance = 0.0f;
   uint32 index = 0;
   uint32 surface_index = 0;
};

auto raycast(const float3 ray_originWS, const float3 ray_directionWS,
             const active_layers active_layers, const blocks_boxes& boxes,
             const float max_distance,
             function_ptr<bool(const block_id id) noexcept> filter) noexcept
   -> std::optional<raycast_block_result_local>
{
   float closest = max_distance;
   uint32 closest_index = UINT32_MAX;

   for (uint32 box_index = 0; box_index < boxes.size(); ++box_index) {
      if (not active_layers[boxes.layer[box_index]]) continue;
      if (boxes.hidden[box_index]) continue;

      const block_description_box& box = boxes.description[box_index];

      const quaternion local_from_world = conjugate(box.rotation);

      const float3 ray_originLS = local_from_world * (ray_originWS - box.position);
      const float3 ray_directionLS = normalize(local_from_world * ray_directionWS);

      if (float hit; intersect_aabb(ray_originLS, 1.0f / ray_directionLS,
                                    {-box.size, box.size}, closest, hit) and
                     hit >= 0.0f) {
         if (filter and not filter(boxes.ids[box_index])) continue;

         closest = hit;
         closest_index = box_index;
      }
   }

   if (closest_index != UINT32_MAX) {
      const block_description_box& box = boxes.description[closest_index];

      const float4x4 scale = {
         {box.size.x, 0.0f, 0.0f, 0.0f},
         {0.0f, box.size.y, 0.0f, 0.0f},
         {0.0f, 0.0f, box.size.z, 0.0f},
         {0.0f, 0.0f, 0.0f, 1.0f},
      };
      const float4x4 rotation = to_matrix(box.rotation);

      float4x4 world_from_object = rotation * scale;
      world_from_object[3] = {box.position, 1.0f};

      uint32 surface_index = UINT32_MAX;
      float closest_surface = FLT_MAX;

      for (const std::array<uint16, 3>& tri : block_cube_triangles) {
         const float3 pos0WS = world_from_object * block_cube_vertices[tri[0]].position;
         const float3 pos1WS = world_from_object * block_cube_vertices[tri[1]].position;
         const float3 pos2WS = world_from_object * block_cube_vertices[tri[2]].position;

         if (float hit; intersect_tri(ray_originWS, ray_directionWS, pos0WS,
                                      pos1WS, pos2WS, hit) and
                        hit < closest_surface) {
            surface_index = block_cube_vertices[tri[0]].surface_index;
            closest_surface = hit;
         }
      }

      if (surface_index != UINT32_MAX) {
         return raycast_block_result_local{.distance = closest,
                                           .index = closest_index,
                                           .surface_index = surface_index};
      }
   }

   return std::nullopt;
}

auto raycast(const float3 ray_originWS, const float3 ray_directionWS,
             const active_layers active_layers, const blocks_ramps& ramps,
             const float max_distance,
             function_ptr<bool(const block_id id) noexcept> filter) noexcept
   -> std::optional<raycast_block_result_local>
{
   float closest = max_distance;
   uint32 closest_index = UINT32_MAX;

   for (uint32 box_index = 0; box_index < ramps.size(); ++box_index) {
      if (not active_layers[ramps.layer[box_index]]) continue;
      if (ramps.hidden[box_index]) continue;

      const block_description_ramp& ramp = ramps.description[box_index];

      const quaternion local_from_world = conjugate(ramp.rotation);

      const float3 ray_originLS = local_from_world * (ray_originWS - ramp.position);
      const float3 ray_directionLS = normalize(local_from_world * ray_directionWS);

      if (float hit =
             iWedge({-ray_originLS.z, ray_originLS.y, ray_originLS.x},
                    {-ray_directionLS.z, ray_directionLS.y, ray_directionLS.x},
                    float3{ramp.size.z, ramp.size.y, ramp.size.x})
                .x;
          hit >= 0.0f and hit < closest) {
         if (filter and not filter(ramps.ids[box_index])) continue;

         math::bounding_box bbox = {-ramp.size, ramp.size};
         std::array<float3, 8> corners = to_corners(bbox);

         for (float3& v : corners) {
            v = ramp.rotation * v + ramp.position;
         }

         closest = hit;
         closest_index = box_index;
      }
   }

   if (closest_index != UINT32_MAX) {
      const block_description_ramp& ramp = ramps.description[closest_index];

      const float4x4 scale = {
         {ramp.size.x, 0.0f, 0.0f, 0.0f},
         {0.0f, ramp.size.y, 0.0f, 0.0f},
         {0.0f, 0.0f, ramp.size.z, 0.0f},
         {0.0f, 0.0f, 0.0f, 1.0f},
      };
      const float4x4 rotation = to_matrix(ramp.rotation);

      float4x4 world_from_object = rotation * scale;
      world_from_object[3] = {ramp.position, 1.0f};

      uint32 surface_index = UINT32_MAX;
      float closest_surface = FLT_MAX;

      for (const std::array<uint16, 3>& tri : block_ramp_triangles) {
         const float3 pos0WS = world_from_object * block_ramp_vertices[tri[0]].position;
         const float3 pos1WS = world_from_object * block_ramp_vertices[tri[1]].position;
         const float3 pos2WS = world_from_object * block_ramp_vertices[tri[2]].position;

         if (float hit; intersect_tri(ray_originWS, ray_directionWS, pos0WS,
                                      pos1WS, pos2WS, hit) and
                        hit < closest_surface) {
            surface_index = block_ramp_vertices[tri[0]].surface_index;
            closest_surface = hit;
         }
      }

      if (surface_index != UINT32_MAX) {
         return raycast_block_result_local{.distance = closest,
                                           .index = closest_index,
                                           .surface_index = surface_index};
      }
   }

   return std::nullopt;
}

auto raycast(const float3 ray_originWS, const float3 ray_directionWS,
             const active_layers active_layers, const blocks_quads& quads,
             const float max_distance,
             function_ptr<bool(const block_id id) noexcept> filter) noexcept
   -> std::optional<raycast_block_result_local>
{
   float closest = max_distance;
   uint32 closest_index = UINT32_MAX;

   for (uint32 box_index = 0; box_index < quads.size(); ++box_index) {
      if (not active_layers[quads.layer[box_index]]) continue;
      if (quads.hidden[box_index]) continue;

      const block_description_quad& quad = quads.description[box_index];

      for (const std::array<uint16, 3>& tri : quad.quad_split == block_quad_split::regular
                                                 ? block_quad_triangles
                                                 : block_quad_alternate_triangles) {
         if (float hit;
             intersect_tri(ray_originWS, ray_directionWS, quad.vertices[tri[0]],
                           quad.vertices[tri[1]], quad.vertices[tri[2]], hit) and
             hit < closest) {
            if (filter and not filter(quads.ids[box_index])) continue;

            closest = hit;
            closest_index = box_index;
         }
      }
   }

   if (closest_index != UINT32_MAX) {
      return raycast_block_result_local{.distance = closest,
                                        .index = closest_index,
                                        .surface_index = 0};
   }

   return std::nullopt;
}

auto raycast(const float3 ray_originWS, const float3 ray_directionWS,
             const active_layers active_layers,
             const blocks_cylinders& cylinders, const float max_distance,
             function_ptr<bool(const block_id id) noexcept> filter) noexcept
   -> std::optional<raycast_block_result_local>
{
   float closest = max_distance;
   uint32 closest_index = UINT32_MAX;
   uint32 surface_index = UINT32_MAX;

   for (uint32 cylinder_index = 0; cylinder_index < cylinders.size(); ++cylinder_index) {
      if (not active_layers[cylinders.layer[cylinder_index]]) continue;
      if (cylinders.hidden[cylinder_index]) continue;

      const block_description_cylinder& cylinder =
         cylinders.description[cylinder_index];

      const quaternion local_from_world = conjugate(cylinder.rotation);

      const float3 ray_originLS =
         local_from_world * (ray_originWS - cylinder.position);
      const float3 ray_directionLS = normalize(local_from_world * ray_directionWS);

      if (float hit; intersect_aabb(ray_originLS, 1.0f / ray_directionLS,
                                    {-cylinder.size, cylinder.size}, closest, hit) and
                     hit >= 0.0f) {
         if (filter and not filter(cylinders.ids[cylinder_index])) continue;

         const float4x4 scale = {
            {cylinder.size.x, 0.0f, 0.0f, 0.0f},
            {0.0f, cylinder.size.y, 0.0f, 0.0f},
            {0.0f, 0.0f, cylinder.size.z, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
         };
         const float4x4 rotation = to_matrix(cylinder.rotation);

         float4x4 world_from_object = rotation * scale;
         world_from_object[3] = {cylinder.position, 1.0f};

         for (const std::array<uint16, 3>& tri : block_cylinder_triangles) {
            const float3 pos0WS =
               world_from_object * block_cylinder_vertices[tri[0]].position;
            const float3 pos1WS =
               world_from_object * block_cylinder_vertices[tri[1]].position;
            const float3 pos2WS =
               world_from_object * block_cylinder_vertices[tri[2]].position;

            if (intersect_tri(ray_originWS, ray_directionWS, pos0WS, pos1WS, pos2WS, hit) and
                hit < closest) {
               closest = hit;
               closest_index = cylinder_index;
               surface_index = block_cylinder_vertices[tri[0]].surface_index;
            }
         }
      }
   }

   if (closest_index != UINT32_MAX) {
      return raycast_block_result_local{.distance = closest,
                                        .index = closest_index,
                                        .surface_index = surface_index};
   }

   return std::nullopt;
}

auto raycast(const float3 ray_originWS, const float3 ray_directionWS,
             const active_layers active_layers, const blocks_stairways& stairways,
             const blocks_custom_mesh_library& custom_meshes, const float max_distance,
             function_ptr<bool(const block_id id) noexcept> filter) noexcept
   -> std::optional<raycast_block_result_local>
{
   float closest = max_distance;
   uint32 closest_index = UINT32_MAX;
   uint32 surface_index = UINT32_MAX;

   for (uint32 stairway_index = 0; stairway_index < stairways.size(); ++stairway_index) {
      if (not active_layers[stairways.layer[stairway_index]]) continue;
      if (stairways.hidden[stairway_index]) continue;

      const block_description_stairway& stairway =
         stairways.description[stairway_index];

      const quaternion local_from_world = conjugate(stairway.rotation);

      const float3 ray_originLS =
         local_from_world * (ray_originWS - stairway.position);
      const float3 ray_directionLS = normalize(local_from_world * ray_directionWS);
      const float3 inv_ray_directionLS = 1.0f / ray_directionLS;

      const float half_width = stairway.size.x / 2.0f;
      const float half_length = stairway.size.z / 2.0f;

      if (float aabb_hit;
          not intersect_aabb(ray_originLS, inv_ray_directionLS,
                             {.min = {-half_width, 0.0f, -half_length},
                              .max = {half_width, stairway.size.y + stairway.first_step_offset,
                                      half_length}},
                             closest, aabb_hit)) {
         continue;
      }

      const int steps =
         static_cast<int>(ceilf(stairway.size.y / stairway.step_height));
      const float adjusted_step_height = stairway.size.y / steps;
      const float step_length = stairway.size.z / steps;

      bool step_hit = false;

      for (int i = 0; i < steps; ++i) {
         float step_base = i * adjusted_step_height;
         float step_top = (i + 1) * adjusted_step_height;

         step_top += stairway.first_step_offset;
         if (i != 0) step_base += stairway.first_step_offset;

         if (i + 1 == steps) {
            step_top = stairway.size.y + stairway.first_step_offset;
         }

         const float step_back = i * step_length - half_length;
         const float step_front = half_length;

         const math::bounding_box bboxLS{.min = {-half_width, step_base, step_back},
                                         .max = {half_width, step_top, step_front}};

         math::bounding_box bboxWS = stairway.rotation * bboxLS + stairway.position;

         float4x4 transform = {{(bboxWS.max - bboxWS.min).x / 2.0f, 0.0f, 0.0f, 0.0f},
                               {0.0f, (bboxWS.max - bboxWS.min).y / 2.0f, 0.0f, 0.0f},
                               {0.0f, 0.0f, (bboxWS.max - bboxWS.min).z / 2.0f, 0.0f},
                               {(bboxWS.min + bboxWS.max) / 2.0f, 1.0f}

         };

         if (float aabb_hit; intersect_aabb(ray_originLS, inv_ray_directionLS,
                                            bboxLS, closest, aabb_hit) and
                             aabb_hit >= 0.0f) {
            step_hit = true;
            break;
         }
      }

      if (not step_hit) continue;

      if (filter and not filter(stairways.ids[stairway_index])) continue;

      const block_custom_mesh& mesh = custom_meshes[stairways.mesh[stairway_index]];

      for (const std::array<uint16, 3>& tri : mesh.triangles) {
         if (float hit; intersect_tri(ray_originLS, ray_directionLS,
                                      mesh.vertices[tri[0]].position,
                                      mesh.vertices[tri[1]].position,
                                      mesh.vertices[tri[2]].position, hit) and
                        hit < closest) {
            closest = hit;
            closest_index = stairway_index;
            surface_index = mesh.vertices[tri[0]].surface_index;
         }
      }
   }

   if (closest_index != UINT32_MAX) {
      return raycast_block_result_local{.distance = closest,
                                        .index = closest_index,
                                        .surface_index = surface_index};
   }

   return std::nullopt;
}

}

auto raycast(const float3 ray_originWS, const float3 ray_directionWS,
             const active_layers active_layers, const blocks& blocks,
             function_ptr<bool(const block_id id) noexcept> filter) noexcept
   -> std::optional<raycast_block_result>
{
   float closest = FLT_MAX;
   block_id closest_id = {};
   uint32 closest_index = UINT32_MAX;
   uint32 closest_surface_index = UINT32_MAX;

   if (std::optional<raycast_block_result_local> hit =
          raycast(ray_originWS, ray_directionWS, active_layers, blocks.boxes,
                  closest, filter);
       hit) {
      closest = hit->distance;
      closest_id = blocks.boxes.ids[hit->index];
      closest_index = hit->index;
      closest_surface_index = hit->surface_index;
   }

   if (std::optional<raycast_block_result_local> hit =
          raycast(ray_originWS, ray_directionWS, active_layers, blocks.ramps,
                  closest, filter);
       hit) {
      closest = hit->distance;
      closest_id = blocks.ramps.ids[hit->index];
      closest_index = hit->index;
      closest_surface_index = hit->surface_index;
   }

   if (std::optional<raycast_block_result_local> hit =
          raycast(ray_originWS, ray_directionWS, active_layers, blocks.quads,
                  closest, filter);
       hit) {
      closest = hit->distance;
      closest_id = blocks.quads.ids[hit->index];
      closest_index = hit->index;
      closest_surface_index = hit->surface_index;
   }

   if (std::optional<raycast_block_result_local> hit =
          raycast(ray_originWS, ray_directionWS, active_layers,
                  blocks.cylinders, closest, filter);
       hit) {
      closest = hit->distance;
      closest_id = blocks.cylinders.ids[hit->index];
      closest_index = hit->index;
      closest_surface_index = hit->surface_index;
   }

   if (std::optional<raycast_block_result_local> hit =
          raycast(ray_originWS, ray_directionWS, active_layers,
                  blocks.stairways, blocks.custom_meshes, closest, filter);
       hit) {
      closest = hit->distance;
      closest_id = blocks.stairways.ids[hit->index];
      closest_index = hit->index;
      closest_surface_index = hit->surface_index;
   }

   if (closest_index != UINT32_MAX) {
      return raycast_block_result{
         .distance = closest,
         .id = closest_id,
         .index = closest_index,
         .surface_index = closest_surface_index,
      };
   }

   return std::nullopt;
}

}
