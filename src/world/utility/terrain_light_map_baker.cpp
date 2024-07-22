#include "terrain_light_map_baker.hpp"

#include "../active_elements.hpp"
#include "../object_class.hpp"
#include "../object_class_library.hpp"
#include "../world.hpp"

#include "assets/asset_ref.hpp"
#include "assets/msh/flat_model.hpp"
#include "async/thread_pool.hpp"
#include "math/bvh.hpp"
#include "math/intersectors.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/sampling.hpp"
#include "math/vector_funcs.hpp"
#include "utility/srgb_conversion.hpp"
#include "utility/string_icompare.hpp"

#include <algorithm>
#include <bit>
#include <new>

namespace we::world {

namespace {

constexpr uint32 directional_light_patch_size = 8;

struct terrain_mesh_chunk {
   std::vector<float3> positions;
   std::vector<std::array<uint16, 3>> triangles;
   bvh bvh;
};

struct directional_light {
   float3 directionWS;
   float3 color;
};

struct point_light {
   float3 positionWS;
   float range_sq;
   float inv_range_sq;
   float3 color;
};

struct spot_light {
   float3 positionWS;
   float range_sq;
   float inv_range_sq;
   float3 directionWS;
   float outer_param;
   float inner_param;
   float3 color;
};

struct scene {
   scene(const world& world, const object_class_library& object_class_library,
         const active_layers active_layers, const terrain_light_map_baker_config& config)
   {
      build_terrain_mesh(world.terrain);
      gather_lights(world);

      _models.reserve(world.objects.size());
      _bvh_instances.reserve(world.objects.size() * 8 + _terrain_mesh.size());

      if (config.include_object_shadows) {
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

   auto raycast(const float3& ray_directionWS, const float3& ray_originWS,
                float max_distance) const noexcept -> std::optional<float>
   {
      return _bvh.raycast(ray_directionWS, ray_originWS, max_distance);
   }

   auto static_directional_lights() const noexcept -> std::span<const directional_light>
   {
      return std::span{_static_directional_lights.lights.data(),
                       _static_directional_lights.count};
   }

   auto dynamic_directional_lights() const noexcept -> std::span<const directional_light>
   {
      return std::span{_dynamic_directional_lights.lights.data(),
                       _dynamic_directional_lights.count};
   }

   auto static_point_lights() const noexcept -> std::span<const point_light>
   {
      return _static_point_lights;
   }

   auto dynamic_point_lights() const noexcept -> std::span<const point_light>
   {
      return _dynamic_point_lights;
   }

   auto static_spot_lights() const noexcept -> std::span<const spot_light>
   {
      return _static_spot_lights;
   }

   auto dynamic_spot_lights() const noexcept -> std::span<const spot_light>
   {
      return _dynamic_spot_lights;
   }

private:
   struct directional_lights {
      uint32 count = 0;
      std::array<directional_light, 2> lights;
   };

   std::vector<asset_data<assets::msh::flat_model>> _models;
   std::vector<top_level_bvh::instance> _bvh_instances;
   std::vector<terrain_mesh_chunk> _terrain_mesh;
   top_level_bvh _bvh;

   directional_lights _static_directional_lights;
   directional_lights _dynamic_directional_lights;

   std::vector<point_light> _static_point_lights;
   std::vector<point_light> _dynamic_point_lights;

   std::vector<spot_light> _static_spot_lights;
   std::vector<spot_light> _dynamic_spot_lights;

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

   void gather_lights(const world& world) noexcept
   {
      _static_point_lights.reserve(world.lights.size());
      _dynamic_point_lights.reserve(world.lights.size());

      _static_spot_lights.reserve(world.lights.size());
      _dynamic_spot_lights.reserve(world.lights.size());

      for (const light& light : world.lights) {
         switch (light.light_type) {
         case light_type::directional: {
            directional_lights& out_lights = light.static_
                                                ? _static_directional_lights
                                                : _dynamic_directional_lights;

            if (out_lights.count == 2) continue;

            if (string::iequals(light.name, world.global_lights.global_light_1) or
                string::iequals(light.name, world.global_lights.global_light_2)) {
               out_lights.lights[out_lights.count] = {.directionWS = normalize(
                                                         light.rotation *
                                                         float3{0.0f, 0.0f, -1.0f}),
                                                      .color = light.color};

               out_lights.count += 1;
            }
         } break;
         case light_type::point: {
            point_light& point_light = light.static_
                                          ? _static_point_lights.emplace_back()
                                          : _dynamic_point_lights.emplace_back();

            point_light.positionWS = light.position;
            point_light.range_sq = light.range * light.range;
            point_light.inv_range_sq = 1.0f / point_light.range_sq;
            point_light.color = light.color;
         } break;
         case light_type::spot: {
            spot_light& spot_light = light.static_
                                        ? _static_spot_lights.emplace_back()
                                        : _dynamic_spot_lights.emplace_back();

            spot_light.positionWS = light.position;
            spot_light.range_sq = light.range * light.range;
            spot_light.inv_range_sq = 1.0f / spot_light.range_sq;
            spot_light.directionWS =
               normalize(light.rotation * float3{0.0f, 0.0f, -1.0f});
            spot_light.outer_param = std::cos(light.outer_cone_angle * 0.5f);
            spot_light.inner_param =
               1.0f / (std::cos(light.inner_cone_angle * 0.5f) - spot_light.outer_param);
            spot_light.color = light.color;
         } break;
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

auto sample_terrain(const container::dynamic_array_2d<int16>& height_map,
                    int32 terrain_max_index, float terrain_half_length,
                    float terrain_grid_scale, float terrain_height_scale,
                    float x, float z) noexcept -> float3
{
   const int32 ix = static_cast<int32>(x);
   const int32 iz = static_cast<int32>(z);

   const auto get_position = [&](const int32 x, const int32 z) {
      return float3{(x - terrain_half_length) * terrain_grid_scale,
                    height_map[{std::clamp(x, 0, terrain_max_index), std::clamp(z, 0, terrain_max_index)}] *
                       terrain_height_scale,
                    (z - terrain_half_length + 1) * terrain_grid_scale};
   };

   const float3 position_00WS = get_position(ix, iz);
   const float3 position_01WS = get_position(ix, iz + 1);
   const float3 position_10WS = get_position(ix + 1, iz);
   const float3 position_11WS = get_position(ix + 1, iz + 1);

   const float3 ray_originWS =
      float3{(x - terrain_half_length) * terrain_grid_scale,
             INT16_MAX * terrain_height_scale,
             (z - terrain_half_length + 1) * terrain_grid_scale};
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

auto sample_terrain_normal(const container::dynamic_array_2d<float3>& normal_map,
                           const int32 terrain_max_index, float x,
                           float z) noexcept -> float3
{
   const int32 ix = static_cast<int32>(x);
   const int32 iz = static_cast<int32>(z);

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

constexpr static std::array<float2, 16> super_sample_grid_offsets = {{
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

constexpr static std::array<float2, 16> super_sample_offsets = {{
   super_sample_grid_offsets[0] / 8.0f,
   super_sample_grid_offsets[1] / 8.0f,
   super_sample_grid_offsets[2] / 8.0f,
   super_sample_grid_offsets[3] / 8.0f,
   super_sample_grid_offsets[4] / 8.0f,
   super_sample_grid_offsets[5] / 8.0f,
   super_sample_grid_offsets[6] / 8.0f,
   super_sample_grid_offsets[7] / 8.0f,
   super_sample_grid_offsets[8] / 8.0f,
   super_sample_grid_offsets[9] / 8.0f,
   super_sample_grid_offsets[10] / 8.0f,
   super_sample_grid_offsets[11] / 8.0f,
   super_sample_grid_offsets[12] / 8.0f,
   super_sample_grid_offsets[13] / 8.0f,
   super_sample_grid_offsets[14] / 8.0f,
   super_sample_grid_offsets[15] / 8.0f,
}};

constexpr static std::array<float2, 1> single_sample_offsets = {{
   {0.0f, 0.0f},
}};

}

struct detail::terrain_light_map_baker_impl {
   terrain_light_map_baker_impl(const world& world, const object_class_library& library,
                                const active_layers active_layers,
                                async::thread_pool& thread_pool,
                                const terrain_light_map_baker_config& config) noexcept
      : _total_points{static_cast<float>(world.terrain.length * world.terrain.length)},
        _scene{world, library, active_layers, config}
   {
      if (not std::has_single_bit(static_cast<uint32>(world.terrain.length))) {
         std::terminate();
      }

      _ambient_ground_color =
         utility::decompress_srgb(world.global_lights.ambient_ground_color);
      _ambient_sky_color =
         utility::decompress_srgb(world.global_lights.ambient_sky_color);

      _terrain_length = world.terrain.length;
      _terrain_max_index = _terrain_length - 1;
      _terrain_half_length = world.terrain.length / 2.0f;
      _terrain_grid_scale = world.terrain.grid_scale;
      _terrain_height_scale = world.terrain.height_scale;

      _height_map = world.terrain.height_map;
      _normal_map = build_normal_map(world.terrain);
      _light_map = {_terrain_length, _terrain_length};

      if (config.supersample) {
         _sample_offsets = super_sample_offsets;
      }
      else {
         _sample_offsets = single_sample_offsets;
      }

      _inv_sample_count = 1.0f / _sample_offsets.size();

      if (config.ambient_occlusion) {
         _ao_sample_count = std::max(config.ambient_occlusion_samples, 1);
         _ao_sample_count_flt = static_cast<float>(_ao_sample_count);
         _inv_ao_sample_count_flt = 1.0f / _ao_sample_count_flt;

         _ao_sample_directions.resize(_sample_offsets.size() * _ao_sample_count);

         for (int32 i = 0; i < std::ssize(_ao_sample_directions); ++i) {
            _ao_sample_directions[i] = cosine_sample_hemisphere(R2(i));
         }
      }
      else {
         _ao_sample_count = 0;
         _ao_sample_count_flt = 0.0f;
         _inv_ao_sample_count_flt = 0.0f;
      }

      _task = thread_pool.exec(async::task_priority::low,
                               [&] { start_bake(thread_pool); });
   }

   bool ready() const noexcept
   {
      return _task.ready();
   }

   auto progress() const noexcept -> float
   {
      return _points_baked.load(std::memory_order_relaxed) / _total_points;
   }

   auto light_map() noexcept -> container::dynamic_array_2d<uint32>
   {
      return _task.ready() ? std::move(_light_map)
                           : container::dynamic_array_2d<uint32>{};
   }

private:
   std::atomic_int32_t _points_baked = 0;
   float _total_points = 0.0f;

   constexpr static int32 _bake_patch_length = 8;

   int32 _terrain_length = 0;
   int32 _terrain_max_index = 0;
   float _terrain_half_length = 0.0f;
   float _terrain_grid_scale = 0.0f;
   float _terrain_height_scale = 0.0f;

   scene _scene;
   container::dynamic_array_2d<int16> _height_map;
   container::dynamic_array_2d<float3> _normal_map;
   container::dynamic_array_2d<uint32> _light_map;

   std::span<const float2> _sample_offsets;
   float _inv_sample_count = 0.0f;

   int32 _ao_sample_count = 128;
   float _ao_sample_count_flt = 128.0f;
   float _inv_ao_sample_count_flt = 1.0f / 128.0f;
   std::vector<float3> _ao_sample_directions;

   float3 _ambient_ground_color;
   float3 _ambient_sky_color;

   async::task<void> _task;

   void start_bake(async::thread_pool& thread_pool)
   {
      std::vector<async::task<void>> tasks;
      tasks.reserve(thread_pool.thread_count(async::task_priority::low) - 1);

      std::atomic_int32_t z = 0;

      for (std::size_t i = 0;
           i < thread_pool.thread_count(async::task_priority::low) - 1; ++i) {
         tasks.emplace_back(thread_pool.exec([&]() noexcept {
            while (true) {
               const int32 my_z = z.fetch_add(1, std::memory_order_relaxed);

               if (my_z >= _terrain_length) return;

               bake_row(my_z);
            }
         }));
      }

      while (true) {
         const int32 my_z = z.fetch_add(1, std::memory_order_relaxed);

         if (my_z >= _terrain_length) return;

         bake_row(my_z);
      }

      tasks.clear();
   }

   void bake_row(int32 zi) noexcept
   {
      for (int32 xi = 0; xi < _terrain_length; ++xi) {
         float3 light_color = {};

         for (int32 sample_index = 0;
              sample_index < std::ssize(_sample_offsets); ++sample_index) {
            const float2& sample = _sample_offsets[sample_index];

            float x = static_cast<float>(xi) + sample.x * 0.5f;
            float z = static_cast<float>(zi) + sample.y * 0.5f;

            const float3 positionWS =
               sample_terrain(_height_map, _terrain_max_index, _terrain_half_length,
                              _terrain_grid_scale, _terrain_height_scale, x, z);
            const float3 normalWS =
               sample_terrain_normal(_normal_map, _terrain_max_index, x, z);

            const float3x3 world_from_basis = get_orthonormal_basis(normalWS);

            float ambient_visibility = 1.0f;

            if (_ao_sample_count != 0) {
               ambient_visibility = static_cast<float>(_ao_sample_count_flt);

               for (int32 i = 0; i < _ao_sample_count; ++i) {
                  const float3 directionWS =
                     world_from_basis *
                     _ao_sample_directions[_ao_sample_count * sample_index + i];

                  if (_scene.raycast_shadow(positionWS + directionWS * 0.001f,
                                            directionWS)) {
                     ambient_visibility -= 1.0f;
                  }
               }

               ambient_visibility *= _inv_ao_sample_count_flt;
            }

            light_color += calculate_ambient_light(normalWS, _ambient_ground_color,
                                                   _ambient_sky_color) *
                           ambient_visibility;

            for (auto& light : _scene.static_directional_lights()) {
               const float NdotL = dot(normalWS, light.directionWS);

               if (NdotL < 0.0f) continue;

               if (_scene.raycast_shadow(positionWS + light.directionWS * 0.001f,
                                         light.directionWS)) {
                  continue;
               }

               light_color += std::clamp(NdotL, 0.0f, 1.0f) * light.color;
            }

            for (auto& light : _scene.static_point_lights()) {
               const float3 light_vectorWS = light.positionWS - positionWS;
               const float light_distance_sq = dot(light_vectorWS, light_vectorWS);

               if (light_distance_sq > light.range_sq) continue;

               const float3 light_directionWS = normalize(light_vectorWS);

               const float NdotL = dot(normalWS, light_directionWS);

               if (NdotL < 0.0f) continue;

               if (_scene.raycast(positionWS + light_directionWS * 0.001f,
                                  light_directionWS, sqrt(light_distance_sq))) {
                  continue;
               }

               const float attenuation =
                  std::clamp(1.0f - light_distance_sq * light.inv_range_sq, 0.0f, 1.0f);

               light_color +=
                  std::clamp(NdotL, 0.0f, 1.0f) * attenuation * light.color;
            }

            for (auto& light : _scene.static_spot_lights()) {
               const float3 light_vectorWS = light.positionWS - positionWS;
               const float light_distance_sq = dot(light_vectorWS, light_vectorWS);

               if (light_distance_sq > light.range_sq) continue;

               const float3 light_directionWS = normalize(light_vectorWS);

               const float NdotL = dot(normalWS, light_directionWS);

               if (NdotL < 0.0f) continue;

               const float LdotL = dot(light_directionWS, light.directionWS);

               if (LdotL < 0.0f) continue;

               const float theta = std::clamp(LdotL, 0.0f, 1.0f);
               const float cone_falloff =
                  std::clamp((theta - light.outer_param) * light.inner_param,
                             0.0f, 1.0f);

               if (_scene.raycast(positionWS + light_directionWS * 0.001f,
                                  light_directionWS, sqrt(light_distance_sq))) {
                  continue;
               }

               const float attenuation =
                  std::clamp(1.0f - light_distance_sq * light.inv_range_sq, 0.0f, 1.0f);

               light_color += std::clamp(NdotL, 0.0f, 1.0f) * attenuation *
                              cone_falloff * light.color;
            }
         }

         _light_map[{xi, zi}] =
            utility::pack_srgb_bgra({light_color * _inv_sample_count, 1.0f});

         _points_baked.fetch_add(1, std::memory_order_relaxed);
      }
   }
};

terrain_light_map_baker::terrain_light_map_baker(
   const world& world, const object_class_library& library,
   const active_layers active_layers, async::thread_pool& thread_pool,
   const terrain_light_map_baker_config& config) noexcept
   : _impl{std::make_unique<detail::terrain_light_map_baker_impl>(world, library,
                                                                  active_layers,
                                                                  thread_pool, config)}
{
}

terrain_light_map_baker::~terrain_light_map_baker() = default;

bool terrain_light_map_baker::ready() const noexcept
{
   return _impl->ready();
}

auto terrain_light_map_baker::progress() const noexcept -> float
{
   return _impl->progress();
}

auto terrain_light_map_baker::light_map() noexcept
   -> container::dynamic_array_2d<uint32>
{
   return _impl->light_map();
}
}
