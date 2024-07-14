#include "bake_terrain_light_map.hpp"

#include "../active_elements.hpp"
#include "../object_class.hpp"
#include "../object_class_library.hpp"
#include "../world.hpp"

#include "assets/asset_ref.hpp"
#include "assets/msh/flat_model.hpp"
#include "math/bvh.hpp"
#include "math/intersectors.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/sampling.hpp"
#include "math/vector_funcs.hpp"
#include "utility/srgb_conversion.hpp"

#include <algorithm>
#include <numbers>

#define SUPERSAMPLE 1

namespace we::world {

namespace {

struct terrain_mesh_chunk {
   std::vector<float3> positions;
   std::vector<std::array<uint16, 3>> triangles;
   bvh bvh;
};

struct scene {
   scene(const world& world, const object_class_library& object_class_library,
         const active_layers active_layers)
   {
      build_terrain_mesh(world.terrain);

      _models.reserve(world.objects.size());
      _bvh_instances.reserve(world.objects.size() * 8 + _terrain_mesh.size());

      for (const object& object : world.objects) {
         if (not active_layers[object.layer]) continue;
         if (object.hidden) continue;

         asset_data<assets::msh::flat_model> model =
            object_class_library[object.class_name].model;

         quaternion inverse_rotation = conjugate(object.rotation);
         float3 inverse_position = inverse_rotation * -object.position;

         for (const bvh& bvh : model->bvh.get_child_bvhs()) {
            _bvh_instances.push_back(
               top_level_bvh::instance{.inverse_rotation = inverse_rotation,
                                       .inverse_position = inverse_position,
                                       .bvh = &bvh,
                                       .rotation = object.rotation,
                                       .position = object.position});
         }

         _models.emplace_back(std::move(model));
      }

      for (const terrain_mesh_chunk& mesh : _terrain_mesh) {
         _bvh_instances.push_back(
            top_level_bvh::instance{.inverse_rotation = quaternion{},
                                    .inverse_position = float3{},
                                    .bvh = &mesh.bvh,
                                    .rotation = quaternion{},
                                    .position = float3{}});
      }

      _bvh = top_level_bvh{_bvh_instances};
   }

   bool raycast_shadow(const float3& ray_directionWS, const float3& ray_originWS) const noexcept
   {
      return _bvh
         .raycast(ray_directionWS, ray_originWS, FLT_MAX,
                  {.allow_backface_cull = false, .accept_first_hit = true})
         .has_value();
   }

private:
   std::vector<asset_data<assets::msh::flat_model>> _models;
   std::vector<top_level_bvh::instance> _bvh_instances;
   std::vector<terrain_mesh_chunk> _terrain_mesh;
   top_level_bvh _bvh;

   void build_terrain_mesh(const terrain& terrain) noexcept
   {
      const float terrain_half_length = terrain.length / 2.0f;

      const int32 chunk_length = std::min(terrain.length, 256);
      const int32 length_in_chunks = terrain.length / chunk_length;

      _terrain_mesh.reserve(length_in_chunks * length_in_chunks);

      for (int32 z_chunk = 0; z_chunk < length_in_chunks; ++z_chunk) {
         for (int32 x_chunk = 0; x_chunk < length_in_chunks; ++x_chunk) {
            terrain_mesh_chunk& mesh = _terrain_mesh.emplace_back();

            mesh.positions.resize(chunk_length * chunk_length);

            for (int32 z = 0; z < chunk_length; ++z) {
               for (int32 x = 0; x < chunk_length; ++x) {
                  const int32 x_terrain = x_chunk * chunk_length + x;
                  const int32 z_terrain = z_chunk * chunk_length + z;

                  const float3 positionWS =
                     {(x_terrain - terrain_half_length) * terrain.grid_scale,
                      terrain.height_map[{x_terrain, z_terrain}] * terrain.height_scale,
                      (z_terrain - terrain_half_length + 1) * terrain.grid_scale};

                  mesh.positions[z * chunk_length + x] = positionWS;
               }
            }

            const int32 chunk_length_quads = chunk_length - 1;

            mesh.triangles.resize(chunk_length_quads * chunk_length_quads * 2);

            for (int32 z = 0; z < chunk_length_quads; ++z) {
               for (int32 x = 0; x < chunk_length_quads; ++x) {
                  const int32 tri_index = z * (chunk_length_quads * 2) + x * 2;

                  const auto index = [&](int32 x, int32 z) noexcept -> uint16 {
                     return static_cast<uint16>(z * chunk_length + x);
                  };

                  if (z & 1) {
                     mesh.triangles[tri_index] = {index(x, z), index(x, z + 1),
                                                  index(x + 1, z)};
                     mesh.triangles[tri_index + 1] = {index(x, z + 1),
                                                      index(x + 1, z + 1),
                                                      index(x + 1, z)};
                  }
                  else {
                     mesh.triangles[tri_index] = {index(x, z), index(x + 1, z + 1),
                                                  index(x + 1, z)};
                     mesh.triangles[tri_index + 1] = {index(x, z), index(x, z + 1),
                                                      index(x + 1, z + 1)};
                  }
               }
            }

            mesh.bvh = bvh{mesh.triangles, mesh.positions, {.backface_cull = false}};
         }
      }
   }
};

auto build_normal_map(const terrain& terrain) noexcept
   -> container::dynamic_array_2d<float3>
{
   const float terrain_half_length = terrain.length / 2.0f;

   container::dynamic_array_2d<float3> normal_map{terrain.length, terrain.length};

   for (int32 z = 0; z < terrain.length; ++z) {
      for (int32 x = 0; x < terrain.length; ++x) {
         const auto position = [&](int32 x, int32 z) noexcept {
            return float3{(x - terrain_half_length) * terrain.grid_scale,
                          terrain.height_map[{x, z}] * terrain.height_scale,
                          (z - terrain_half_length + 1) * terrain.grid_scale};
         };

         const int32 x0 = x;
         const int32 z0 = z;
         const int32 x1 = std::clamp(x + 1, 0, terrain.length - 1);
         const int32 z1 = std::clamp(z + 1, 0, terrain.length - 1);

         if (z & 1) {
            const std::array tri0 = {position(x0, z0), position(x0, z1),
                                     position(x1, z0)};
            const std::array tri1 = {position(x0, z1), position(x1, z1),
                                     position(x1, z0)};

            const float3 normal0 = cross(tri0[1] - tri0[0], tri0[2] - tri0[0]);
            const float3 normal1 = cross(tri1[1] - tri1[0], tri1[2] - tri1[0]);

            normal_map[{x0, z0}] += normal0;
            normal_map[{x0, z1}] += normal0;
            normal_map[{x1, z0}] += normal0;

            normal_map[{x0, z1}] += normal1;
            normal_map[{x1, z1}] += normal1;
            normal_map[{x1, z0}] += normal1;
         }
         else {
            const std::array tri0 = {position(x0, z0), position(x1, z1),
                                     position(x1, z0)};
            const std::array tri1 = {position(x0, z0), position(x0, z1),
                                     position(x1, z1)};

            const float3 normal0 = cross(tri0[1] - tri0[0], tri0[2] - tri0[0]);
            const float3 normal1 = cross(tri1[1] - tri1[0], tri1[2] - tri1[0]);

            normal_map[{x0, z0}] += normal0;
            normal_map[{x1, z1}] += normal0;
            normal_map[{x1, z0}] += normal0;

            normal_map[{x0, z0}] += normal1;
            normal_map[{x0, z1}] += normal1;
            normal_map[{x1, z1}] += normal1;
         }
      }
   }

   for (float3& normal : normal_map) normal = normalize(normal);

   return normal_map;
}

auto calculate_ambient_light(const float3& normalWS, const float3& ambient_ground_color,
                             const float3& ambient_sky_color) noexcept -> float3
{
   const float factor = normalWS.y * 2.0f - 1.0f;

   return ambient_ground_color * (1.0f - factor) + ambient_sky_color * factor;
}

auto sample_terrain(const terrain& terrain, float x, float z) noexcept -> float3
{
   const int32 ix = static_cast<int32>(x);
   const int32 iz = static_cast<int32>(z);
   const int32 terrain_max_index = terrain.length - 1;
   const float terrain_half_length = terrain.length / 2.0f;

   const auto get_position = [&](const int32 x, const int32 z) {
      return float3{(x - terrain_half_length) * terrain.grid_scale,
                    terrain.height_map[{std::clamp(x, 0, terrain_max_index),
                                        std::clamp(z, 0, terrain_max_index)}] *
                       terrain.height_scale,
                    (z - terrain_half_length + 1) * terrain.grid_scale};
   };

   const float3 position_00WS = get_position(ix, iz);
   const float3 position_01WS = get_position(ix, iz + 1);
   const float3 position_10WS = get_position(ix + 1, iz);
   const float3 position_11WS = get_position(ix + 1, iz + 1);

   const float3 ray_originWS =
      float3{(x - terrain_half_length) * terrain.grid_scale,
             INT16_MAX * terrain.height_scale,
             (z - terrain_half_length + 1) * terrain.grid_scale};
   const float3 ray_directionWS = {0.0f, -1.0f, 0.0f};

   if (iz & 1) {
      const std::array tri0 = {position_00WS, position_01WS, position_10WS};

      if (float distance = 0.0f; intersect_tri(ray_originWS, ray_directionWS,
                                               tri0[0], tri0[1], tri0[2], distance)) {
         return ray_originWS + ray_directionWS * distance;
      }

      const std::array tri1 = {position_01WS, position_11WS, position_10WS};

      if (float distance = 0.0f; intersect_tri(ray_originWS, ray_directionWS,
                                               tri1[0], tri1[1], tri1[2], distance)) {
         return ray_originWS + ray_directionWS * distance;
      }
   }
   else {
      const std::array tri0 = {position_00WS, position_11WS, position_10WS};

      if (float distance = 0.0f; intersect_tri(ray_originWS, ray_directionWS,
                                               tri0[0], tri0[1], tri0[2], distance)) {
         return ray_originWS + ray_directionWS * distance;
      }

      const std::array tri1 = {position_00WS, position_01WS, position_11WS};

      if (float distance = 0.0f; intersect_tri(ray_originWS, ray_directionWS,
                                               tri1[0], tri1[1], tri1[2], distance)) {
         return ray_originWS + ray_directionWS * distance;
      }
   }

   return ray_originWS;
}

auto sample_terrain_normal(const world& world,
                           const container::dynamic_array_2d<float3>& normal_map,
                           float x, float z) noexcept -> float3
{
   const int32 ix = static_cast<int32>(x);
   const int32 iz = static_cast<int32>(z);
   const int32 terrain_max_index = world.terrain.length - 1;

   const float3 normal00 =
      normal_map[{std::clamp(ix, 0, terrain_max_index), std::clamp(iz, 0, terrain_max_index)}];
   const float3 normal10 =
      normal_map[{std::clamp(ix + 1, 0, terrain_max_index), std::clamp(iz, 0, terrain_max_index)}];
   const float3 normal01 =
      normal_map[{std::clamp(ix, 0, terrain_max_index), std::clamp(iz + 1, 0, terrain_max_index)}];
   const float3 normal11 =
      normal_map[{std::clamp(ix + 1, 0, terrain_max_index), std::clamp(iz + 1, 0, terrain_max_index)}];

   const float x_factor = x - floorf(x);
   const float z_factor = z - floorf(z);

   const float3 x_normal0 = normal00 * (1.0f - x_factor) + normal10 * x_factor;
   const float3 x_normal1 = normal01 * (1.0f - x_factor) + normal11 * x_factor;

   return normalize(x_normal0 * (1.0f - z_factor) + x_normal1 * z_factor);
}

auto get_orthonormal_basis(float3 normal) noexcept -> float3x3
{
   float sign = copysign(1.0f, normal.z);
   float a = -1.0f / (sign + normal.z);
   float b = normal.x * normal.y * a;

   float3 b1 = {1.0f + sign * normal.x * normal.x * a, sign * b, -sign * normal.x};
   float3 b2 = {b, sign + normal.y * normal.y * a, -normal.y};

   return float3x3{b1, b2, normal};
};

#if SUPERSAMPLE
constexpr static std::array<float2, 16> sample_grid_offsets = {{
   {1.0f, 1.0f},
   {-1.0f, -3.0f},
   {-3.0f, 2.0f},
   {4.0f, -1.0f},
   {-5.0f, -2.0f},
   {2.0f, 5.0f},
   {5.0f, 3.0f},
   {3.0f, -5.0f},
   {-2.0f, 6.0f},
   {0.0f, -7.0f},
   {-4.0f, -6.0f},
   {-6.0f, 4.0f},
   {-8.0f, 0.0f},
   {7.0f, -4.0f},
   {6.0f, 7.0f},
   {-7.0f, -8.0f},
}};

constexpr static std::array<float2, 16> sample_offsets = {{
   sample_grid_offsets[0] / 8.0f,
   sample_grid_offsets[1] / 8.0f,
   sample_grid_offsets[2] / 8.0f,
   sample_grid_offsets[3] / 8.0f,
   sample_grid_offsets[4] / 8.0f,
   sample_grid_offsets[5] / 8.0f,
   sample_grid_offsets[6] / 8.0f,
   sample_grid_offsets[7] / 8.0f,
   sample_grid_offsets[8] / 8.0f,
   sample_grid_offsets[9] / 8.0f,
   sample_grid_offsets[10] / 8.0f,
   sample_grid_offsets[11] / 8.0f,
   sample_grid_offsets[12] / 8.0f,
   sample_grid_offsets[13] / 8.0f,
   sample_grid_offsets[14] / 8.0f,
   sample_grid_offsets[15] / 8.0f,
}};
#else
constexpr static std::array<float2, 1> sample_grid_offsets = {{
   {0.0f, 0.0f},
}};

constexpr static std::array<float2, 1> sample_offsets = {{
   {0.0f, 0.0f},
}};
#endif

const int32 ao_sample_count = 128;

}

auto bake_terrain_light_map(const world& world,
                            const object_class_library& object_class_library,
                            const active_layers active_layers) noexcept
   -> container::dynamic_array_2d<uint32>
{
   scene scene{world, object_class_library, active_layers};

   container::dynamic_array_2d<float3> normal_map = build_normal_map(world.terrain);
   container::dynamic_array_2d<uint32> light_map{world.terrain.length,
                                                 world.terrain.length};

   const float3 ambient_ground_color =
      utility::decompress_srgb(world.global_lights.ambient_ground_color);
   const float3 ambient_sky_color =
      utility::decompress_srgb(world.global_lights.ambient_sky_color);

   std::vector<float3> ao_sample_directions;
   ao_sample_directions.resize(sample_offsets.size() * ao_sample_count);

   for (int32 i = 0; i < std::ssize(ao_sample_directions); ++i) {
      ao_sample_directions[i] = cosine_sample_hemisphere(R2(i));
   }

   for (int32 zi = 0; zi < world.terrain.length; ++zi) {
      for (int32 xi = 0; xi < world.terrain.length; ++xi) {
         float3 light_color = {};

         int32 sample_index = 0;

         for (const float2& sample : sample_offsets) {
            float x = static_cast<float>(xi) + sample.x * 0.5f;
            float z = static_cast<float>(zi) + sample.y * 0.5f;

            const float3 positionWS = sample_terrain(world.terrain, x, z);
            const float3 normalWS = sample_terrain_normal(world, normal_map, x, z);

            const float3x3 world_from_basis = get_orthonormal_basis(normalWS);

            float ambient_visibility = static_cast<float>(ao_sample_count);

            for (int i = 0; i < ao_sample_count; ++i) {
               const float3 directionWS =
                  world_from_basis *
                  ao_sample_directions[ao_sample_count * sample_index + i];

               if (scene.raycast_shadow(positionWS + directionWS * 0.001f, directionWS)) {
                  ambient_visibility -= 1.0f;
               }
            }

            sample_index += 1;

            ambient_visibility /= static_cast<float>(ao_sample_count);

            light_color += calculate_ambient_light(normalWS, ambient_ground_color,
                                                   ambient_sky_color) *
                           ambient_visibility;

#if 0
            for (auto& light : world.lights) {
               if (not light.static_) continue;

               switch (light.light_type) {
               case light_type::directional: {
                  const float3 light_directionWS =
                     normalize(light.rotation * float3{0.0f, 0.0f, -1.0f});

                  if (scene.raycast_shadow(positionWS - light_directionWS * 0.00001f,
                                           light_directionWS)) {
                     continue;
                  }

                  light_color +=
                     std::clamp(dot(normalWS, light_directionWS), 0.0f, 1.0f) *
                     light.color;
               } break;
               case light_type::point: {
                  const float3 light_directionWS =
                     normalize(light.position - positionWS);
                  const float light_distance = distance(light.position, positionWS);

                  if (light_distance > light.range) continue;

                  if (scene.raycast_shadow(positionWS - light_directionWS * 0.00001f,
                                           light_directionWS)) {
                     continue;
                  }

                  const float attenuation =
                     std::clamp(1.0f - ((light_distance * light_distance) /
                                        (light.range * light.range)),
                                0.0f, 1.0f);

                  light_color +=
                     std::clamp(dot(normalWS, light_directionWS), 0.0f, 1.0f) *
                     attenuation * light.color;

               } break;
               }
            }
#endif
         }

         light_map[{xi, zi}] = utility::pack_srgb_bgra(
            {light_color / static_cast<float>(sample_offsets.size()), 1.0f});
      }
   }

   return light_map;
}

}
