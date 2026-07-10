#include "terrain.hpp"
#include "math/align.hpp"
#include "math/vector_funcs.hpp"
#include "settings/graphics.hpp"
#include "utility/string_icompare.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace we::graphics {

namespace {

#pragma warning(disable : 4324) // structure was padded due to alignment specifier

struct alignas(16) terrain_constants {
   float2 half_world_size;
   float grid_size;
   float height_scale;

   uint32 terrain_max_index;
   float inv_terrain_length;
   uint32 foliage_map;
   float inv_grid_size;

   float3 grid_line_color;
   float grid_line_width;

   std::array<std::array<uint32, 4>, terrain::texture_count / 4> diffuse_maps;

   std::array<float4, terrain::texture_count> texture_transform_x;
   std::array<float4, terrain::texture_count> texture_transform_y;

   bool has_detail_map;
   uint32 detail_map_index;
   std::array<uint32, 2> pad;

   float3 foliage_color_0;
   float foliage_transparency;
   float3 foliage_color_1;
   uint32 foliage_color_1_pad;
   float3 foliage_color_2;
   uint32 foliage_color_2_pad;
   float3 foliage_color_3;
   uint32 foliage_color_3_pad;
};

static_assert(sizeof(terrain_constants) == 704);

constexpr auto generate_patch_indices()
{
   using tri = std::array<uint16, 3>;

   std::array<std::array<tri, 2>, terrain::patch_length_grids * terrain::patch_length_grids> triangles{};

   static_assert(sizeof(triangles) == sizeof(tri) * terrain::patch_length_grids *
                                         terrain::patch_length_grids * 2);

   for (uint16 y = 0; y < terrain::patch_length_grids; ++y) {
      for (uint16 x = 0; x < terrain::patch_length_grids; ++x) {
         const auto index = [](uint16 x, uint16 y) -> uint16 {
            return y * terrain::patch_length_points + x;
         };

         if (y & 1) {
            triangles[y * terrain::patch_length_grids + x][0] = {index(x, y),
                                                                 index(x, y + 1),
                                                                 index(x + 1, y)};
            triangles[y * terrain::patch_length_grids + x][1] = {index(x, y + 1),
                                                                 index(x + 1, y + 1),
                                                                 index(x + 1, y)};
         }
         else {
            triangles[y * terrain::patch_length_grids + x][0] = {index(x, y),
                                                                 index(x + 1, y + 1),
                                                                 index(x + 1, y)};
            triangles[y * terrain::patch_length_grids + x][1] = {index(x, y),
                                                                 index(x, y + 1),
                                                                 index(x + 1, y + 1)};
         }
      }
   }

   return triangles;
}

constexpr auto terrain_patch_indices = generate_patch_indices();

auto select_pipeline(const terrain_draw draw, pipeline_library& pipelines)
   -> gpu::pipeline_handle
{
   switch (draw) {
   case terrain_draw::depth_prepass:
      return pipelines.terrain_depth_prepass.get();
   case terrain_draw::main:
      return pipelines.terrain_normal.get();
   case terrain_draw::grid:
      return pipelines.terrain_grid.get();
   case terrain_draw::foliage_map:
      return pipelines.terrain_foliage_map.get();
   }

   std::unreachable();
}

}

terrain::terrain(gpu::device& device, copy_command_list_pool& copy_command_list_pool,
                 dynamic_buffer_allocator& dynamic_buffer_allocator,
                 texture_manager& texture_manager)
   : _device{device}
{
   _terrain_constants_buffer =
      {_device.create_buffer({.size = sizeof(terrain_constants),
                              .debug_name = "Terrain Constants Buffer"},
                             gpu::heap_type::default_),
       _device};
   _terrain_cbv = _device.get_gpu_virtual_address(_terrain_constants_buffer.get());

   _index_buffer = {_device.create_buffer({.size = sizeof(terrain_patch_indices),
                                           .debug_name =
                                              "Terrain Patch Indices"},
                                          gpu::heap_type::default_),
                    device};

   pooled_copy_command_list command_list = copy_command_list_pool.aquire_and_reset();

   auto terrain_patch_indices_allocation =
      dynamic_buffer_allocator.allocate_and_copy(terrain_patch_indices);

   command_list->copy_buffer_region(_index_buffer.get(), 0,
                                    terrain_patch_indices_allocation.resource,
                                    terrain_patch_indices_allocation.offset,
                                    sizeof(terrain_patch_indices));

   command_list->close();

   device.background_copy_queue.execute_command_lists(command_list.get());

   for (auto& texture : _diffuse_maps) {
      texture = texture_manager.null_diffuse_map();
   }
}

void terrain::update(const world::terrain& terrain, gpu::copy_command_list& command_list,
                     dynamic_buffer_allocator& dynamic_buffer_allocator,
                     texture_manager& texture_manager, const settings::graphics& settings)
{
   if (const uint32 length = static_cast<uint32>(terrain.length);
       _terrain_length != length) {
      _terrain_length = length;
      _patches_length = (_terrain_length + patch_length_grids - 1u) / patch_length_grids;

      create_patches();
      create_gpu_resources();
   }

   const int32 half_terrain_length = static_cast<int32>(_terrain_length / 2);

   for (const world::dirty_rect& dirty : terrain.height_map_dirty) {
      uint32 start_x = dirty.left;
      uint32 end_x = dirty.right;

      uint32 start_z = dirty.top;
      uint32 end_z = dirty.bottom;

      if (start_x > 0) start_x -= 1;
      if (end_x < _terrain_length) end_x += 1;

      if (start_z > 0) start_z -= 1;
      if (end_z < _terrain_length) end_z += 1;

      const uint32 start_patch_x = start_x / patch_length_grids;
      const uint32 end_patch_x = (end_x + patch_length_grids - 1u) / patch_length_grids;

      const uint32 start_patch_z = start_z / patch_length_grids;
      const uint32 end_patch_z = (end_z + patch_length_grids - 1u) / patch_length_grids;

      for (uint32 patch_z = start_patch_z; patch_z < end_patch_z; ++patch_z) {
         for (uint32 patch_x = start_patch_x; patch_x < end_patch_x; ++patch_x) {
            const uint32 patch_index = patch_z * _patches_length + patch_x;

            if (not _dirty_vertex_patches[patch_index]) {
               _vertex_patches_to_upload.push_back(patch_index);
               _dirty_vertex_patches[patch_index] = true;
            }

            int16 min_y = INT16_MAX;
            int16 max_y = INT16_MIN;

            std::array<terrain_vertex, patch_vertex_count>& vertices =
               _patch_vertices[patch_index];

            for (uint32 local_z = 0; local_z < patch_length_points; ++local_z) {
               for (uint32 local_x = 0; local_x < patch_length_points; ++local_x) {
                  const uint32 x = std::clamp(patch_x * patch_length_grids + local_x,
                                              0u, terrain.length - 1u);
                  const uint32 z = std::clamp(patch_z * patch_length_grids + local_z,
                                              0u, terrain.length - 1u);

                  const int16 height = terrain.height_map[{x, z}];

                  min_y = std::min(height, min_y);
                  max_y = std::max(height, max_y);

                  vertices[local_z * patch_length_points + local_x].positionCS =
                     {static_cast<int16>(
                         static_cast<int32>(patch_x * patch_length_grids + local_x) -
                         half_terrain_length),
                      height,
                      static_cast<int16>(
                         static_cast<int32>(patch_z * patch_length_grids + local_z) -
                         half_terrain_length)};
               }
            }

            _patches[patch_index].min_y = min_y;
            _patches[patch_index].max_y = max_y;
         }
      }

      const float normal_height_scale =
         terrain.height_scale / (terrain.grid_scale * 2.0f);

      for (uint32 patch_z = start_patch_z; patch_z < end_patch_z; ++patch_z) {
         for (uint32 patch_x = start_patch_x; patch_x < end_patch_x; ++patch_x) {
            const uint32 patch_index = patch_z * _patches_length + patch_x;

            if (not _dirty_vertex_attributes_patches[patch_index]) {
               _vertex_attributes_patches_to_upload.push_back(patch_index);
               _dirty_vertex_attributes_patches[patch_index] = true;
            }

            std::array<terrain_vertex_attributes, patch_vertex_count>& vertices =
               _patch_vertex_attributes[patch_index];

            for (uint32 local_z = 0; local_z < patch_length_points; ++local_z) {
               for (uint32 local_x = 0; local_x < patch_length_points; ++local_x) {
                  const uint32 x = std::clamp(patch_x * patch_length_grids + local_x,
                                              0u, terrain.length - 1u);
                  const uint32 z = std::clamp(patch_z * patch_length_grids + local_z,
                                              0u, terrain.length - 1u);

                  const uint32 x0 = x > 0 ? x - 1 : x;
                  const uint32 x1 = x < _terrain_length - 1 ? x + 1 : x;
                  const uint32 z0 = z > 0 ? z - 1 : z;
                  const uint32 z1 = z < _terrain_length - 1 ? z + 1 : z;

                  const int16 height0x = terrain.height_map[{x0, z}];
                  const int16 height1x = terrain.height_map[{x1, z}];
                  const int16 height0z = terrain.height_map[{x, z0}];
                  const int16 height1z = terrain.height_map[{x, z1}];

                  const float3 normalWS = normalize(
                     float3{(height0x - height1x) * normal_height_scale, 1.0f,
                            (height0z - height1z) * normal_height_scale});

                  float3 normal_snormWS = normalWS * 127.0f;

                  if (normal_snormWS.x >= 0.0f) {
                     normal_snormWS.x += 0.5f;
                  }
                  else {
                     normal_snormWS.x -= 0.5f;
                  }

                  if (normal_snormWS.y >= 0.0f) {
                     normal_snormWS.y += 0.5f;
                  }
                  else {
                     normal_snormWS.y -= 0.5f;
                  }

                  if (normal_snormWS.z >= 0.0f) {
                     normal_snormWS.z += 0.5f;
                  }
                  else {
                     normal_snormWS.z -= 0.5f;
                  }

                  vertices[local_z * patch_length_points + local_x]
                     .normalWS = {static_cast<int8>(normal_snormWS.x),
                                  static_cast<int8>(normal_snormWS.y),
                                  static_cast<int8>(normal_snormWS.z)};
               }
            }
         }
      }
   }

   for (const world::dirty_rect& dirty : terrain.texture_weight_maps_dirty) {
      uint32 start_x = dirty.left;
      uint32 end_x = dirty.right;

      uint32 start_z = dirty.top;
      uint32 end_z = dirty.bottom;

      if (start_x > 0) start_x -= 1;
      if (end_x < _terrain_length) end_x += 1;

      if (start_z > 0) start_z -= 1;
      if (end_z < _terrain_length) end_z += 1;

      const uint32 start_patch_x = start_x / patch_length_grids;
      const uint32 end_patch_x = (end_x + patch_length_grids - 1u) / patch_length_grids;

      const uint32 start_patch_z = start_z / patch_length_grids;
      const uint32 end_patch_z = (end_z + patch_length_grids - 1u) / patch_length_grids;

      for (uint32 patch_z = start_patch_z; patch_z < end_patch_z; ++patch_z) {
         for (uint32 patch_x = start_patch_x; patch_x < end_patch_x; ++patch_x) {
            const uint32 patch_index = patch_z * _patches_length + patch_x;

            if (not _dirty_vertex_attributes_patches[patch_index]) {
               _vertex_attributes_patches_to_upload.push_back(patch_index);
               _dirty_vertex_attributes_patches[patch_index] = true;
            }

            std::array<terrain_vertex_attributes, patch_vertex_count>& vertices =
               _patch_vertex_attributes[patch_index];

            for (uint32 local_z = 0; local_z < patch_length_points; ++local_z) {
               for (uint32 local_x = 0; local_x < patch_length_points; ++local_x) {
                  const uint32 x = std::clamp(patch_x * patch_length_grids + local_x,
                                              0u, terrain.length - 1u);
                  const uint32 z = std::clamp(patch_z * patch_length_grids + local_z,
                                              0u, terrain.length - 1u);

                  std::array<uint8, texture_count> weights = {255};

                  for (std::size_t i = 1; i < weights.size(); ++i) {
                     weights[i] = terrain.texture_weight_maps[i][{x, z}];
                  }

                  // Zero out weights for fully obsecured textures.

                  std::ptrdiff_t highest_opaque_weight = texture_count - 1;

                  for (; highest_opaque_weight >= 0; --highest_opaque_weight) {
                     if (weights[highest_opaque_weight] == 255) {
                        break;
                     }
                  }

                  for (std::ptrdiff_t i = highest_opaque_weight - 1; i >= 0; --i) {
                     weights[i] = 0;
                  }

                  vertices[local_z * patch_length_points + local_x].weights = weights;
               }
            }
         }
      }
   }

   for (const world::dirty_rect& dirty : terrain.color_or_light_map_dirty) {
      uint32 start_x = dirty.left;
      uint32 end_x = dirty.right;

      uint32 start_z = dirty.top;
      uint32 end_z = dirty.bottom;

      if (start_x > 0) start_x -= 1;
      if (end_x < _terrain_length) end_x += 1;

      if (start_z > 0) start_z -= 1;
      if (end_z < _terrain_length) end_z += 1;

      const uint32 start_patch_x = start_x / patch_length_grids;
      const uint32 end_patch_x = (end_x + patch_length_grids - 1u) / patch_length_grids;

      const uint32 start_patch_z = start_z / patch_length_grids;
      const uint32 end_patch_z = (end_z + patch_length_grids - 1u) / patch_length_grids;

      for (uint32 patch_z = start_patch_z; patch_z < end_patch_z; ++patch_z) {
         for (uint32 patch_x = start_patch_x; patch_x < end_patch_x; ++patch_x) {
            const uint32 patch_index = patch_z * _patches_length + patch_x;

            if (not _dirty_vertex_attributes_patches[patch_index]) {
               _vertex_attributes_patches_to_upload.push_back(patch_index);
               _dirty_vertex_attributes_patches[patch_index] = true;
            }

            std::array<terrain_vertex_attributes, patch_vertex_count>& vertices =
               _patch_vertex_attributes[patch_index];

            for (uint32 local_z = 0; local_z < patch_length_points; ++local_z) {
               for (uint32 local_x = 0; local_x < patch_length_points; ++local_x) {
                  const uint32 x = std::clamp(patch_x * patch_length_grids + local_x,
                                              0u, terrain.length - 1u);
                  const uint32 z = std::clamp(patch_z * patch_length_grids + local_z,
                                              0u, terrain.length - 1u);

                  const uint32 u32_base_color = terrain.color_map[{x, z}];
                  const uint32 u32_light = terrain.light_map[{x, z}];

                  // We can just ignore BGR order here.

                  const float3 base_color = {
                     ((u32_base_color >> 0u) & 0xffu) * (1.0f / 255.0f),
                     ((u32_base_color >> 8u) & 0xffu) * (1.0f / 255.0f),
                     ((u32_base_color >> 16u) & 0xffu) * (1.0f / 255.0f),
                  };
                  const float3 light = {
                     ((u32_light >> 0u) & 0xffu) * (1.0f / 255.0f),
                     ((u32_light >> 8u) & 0xffu) * (1.0f / 255.0f),
                     ((u32_light >> 16u) & 0xffu) * (1.0f / 255.0f),
                  };

                  const float3 color = base_color * light;

                  uint32 u32_color = 0xff'00'00'00u;

                  u32_color |= static_cast<uint8>(color.x * 255.0f + 0.5f) << 0u;
                  u32_color |= static_cast<uint8>(color.y * 255.0f + 0.5f) << 8u;
                  u32_color |= static_cast<uint8>(color.z * 255.0f + 0.5f) << 16u;

                  vertices[local_z * patch_length_points + local_x].static_light =
                     u32_color;
               }
            }
         }
      }
   }

   for (uint32 i = 0; i < texture_count; ++i) {
      if (not string::iequals(_diffuse_maps_names[i], terrain.texture_names[i])) {
         _diffuse_maps_names[i] = lowercase_string{terrain.texture_names[i]};
         _diffuse_maps[i] =
            texture_manager.at_or(_diffuse_maps_names[i], world_texture_dimension::_2d,
                                  texture_manager.null_diffuse_map());

         if (not _diffuse_maps_names[i].empty() and
             _diffuse_maps[i] == texture_manager.null_diffuse_map()) {
            _diffuse_map_load_tokens[i] =
               texture_manager.acquire_load_token(_diffuse_maps_names[i]);
         }
      }
   }

   if (not string::iequals(_detail_map_name, terrain.detail_texture_name)) {
      _detail_map_name = lowercase_string{terrain.detail_texture_name};
      _detail_map =
         texture_manager.at_or(_detail_map_name, world_texture_dimension::_2d,
                               texture_manager.null_detail_map());

      if (not _detail_map_name.empty() and
          _detail_map == texture_manager.null_detail_map()) {
         _detail_map_load_token = texture_manager.acquire_load_token(_detail_map_name);
      }
   }

   for (const world::dirty_rect& dirty : terrain.foliage_map_dirty) {
      std::byte* const upload_ptr = _foliage_map_upload_ptr[_device.frame_index()];
      const uint32 row_pitch = _foliage_map_upload_row_pitch;

      for (uint32 y = dirty.top; y < dirty.bottom; ++y) {
         volatile uint8* out_row =
            reinterpret_cast<uint8*>(upload_ptr + (y * row_pitch) + dirty.left);

         for (uint32 x = dirty.left; x < dirty.right; ++x) {
            world::foliage_patch x0 = terrain.foliage_map[{x, y}];

            uint8 packed = 0;

            packed |= (x0.layer0 << 0);
            packed |= (x0.layer1 << 1);
            packed |= (x0.layer2 << 2);
            packed |= (x0.layer3 << 3);

            *out_row = packed;
            out_row += 1;
         }
      }

      gpu::box src_box{.left = dirty.left,
                       .top = dirty.top,
                       .front = 0,
                       .right = dirty.right,
                       .bottom = dirty.bottom,
                       .back = 1};

      command_list.copy_buffer_to_texture(
         _foliage_map.get(), 0, dirty.left, dirty.top, 0, _upload_buffer.get(),
         {.offset = _foliage_map_upload_offset[_device.frame_index()],
          .format = DXGI_FORMAT_R8_UINT,
          .width = _terrain_length / 2,
          .height = _terrain_length / 2,
          .depth = 1,
          .row_pitch = _foliage_map_upload_row_pitch},
         &src_box);
   }

   terrain_constants constants{
      .half_world_size = _terrain_half_world_size,
      .grid_size = _terrain_grid_size,
      .height_scale = terrain.height_scale,

      .terrain_max_index = _terrain_length - 1u,
      .inv_terrain_length = 1.0f / _terrain_length,

      .foliage_map = _foliage_map_srv.get().index,

      .inv_grid_size = 1.0f / _terrain_grid_size,
      .grid_line_color = settings.terrain_grid_color,
      .grid_line_width = settings.terrain_grid_line_width * (4.0f / _terrain_grid_size),

      .foliage_color_0 = settings.foliage_overlay_layer0_color,
      .foliage_transparency = settings.foliage_overlay_transparency,
      .foliage_color_1 = settings.foliage_overlay_layer1_color,
      .foliage_color_2 = settings.foliage_overlay_layer2_color,
      .foliage_color_3 = settings.foliage_overlay_layer3_color,
   };

   for (uint32 y = 0; y < texture_count / 4; ++y) {
      for (uint32 x = 0; x < texture_count / 4; ++x) {
         constants.diffuse_maps[y][x] = _diffuse_maps[y * 4 + x]->srv_srgb.index;
      }
   }

   for (uint32 i = 0; i < texture_count; ++i) {
      const float scale = terrain.texture_scales[i];
      float4& transform_x = constants.texture_transform_x[i];
      float4& transform_y = constants.texture_transform_y[i];

      switch (terrain.texture_axes[i]) {
      case world::texture_axis::xz:
         transform_x = {scale, 0.0f, 0.0f, 0.0f};
         transform_y = {0.0f, 0.0f, scale, 0.0f};
         break;
      case world::texture_axis::xy:
         transform_x = {scale, 0.0f, 0.0f, 0.0f};
         transform_y = {0.0f, scale, 0.0f, 0.0f};
         break;
      case world::texture_axis::yz:
         transform_x = {0.0f, scale, 0.0f, 0.0f};
         transform_y = {0.0f, 0.0f, scale, 0.0f};
         break;
      case world::texture_axis::zx:
         transform_x = {0.0f, 0.0f, scale, 0.0f};
         transform_y = {scale, 0.0f, 0.0f, 0.0f};
         break;
      case world::texture_axis::yx:
         transform_x = {0.0f, scale, 0.0f, 0.0f};
         transform_y = {scale, 0.0f, 0.0f, 0.0f};
         break;
      case world::texture_axis::zy:
         transform_x = {0.0f, 0.0f, scale, 0.0f};
         transform_y = {0.0f, scale, 0.0f, 0.0f};
         break;
      case world::texture_axis::negative_xz:
         transform_x = {-scale, 0.0f, 0.0f, 0.0f};
         transform_y = {0.0f, 0.0f, -scale, 0.0f};
         break;
      case world::texture_axis::negative_xy:
         transform_x = {-scale, 0.0f, 0.0f, 0.0f};
         transform_y = {0.0f, -scale, 0.0f, 0.0f};
         break;
      case world::texture_axis::negative_yz:
         transform_x = {0.0f, -scale, 0.0f, 0.0f};
         transform_y = {0.0f, 0.0f, -scale, 0.0f};
         break;
      case world::texture_axis::negative_zx:
         transform_x = {0.0f, 0.0f, -scale, 0.0f};
         transform_y = {-scale, 0.0f, 0.0f, 0.0f};
         break;
      case world::texture_axis::negative_yx:
         transform_x = {0.0f, -scale, 0.0f, 0.0f};
         transform_y = {-scale, 0.0f, 0.0f, 0.0f};
         break;
      case world::texture_axis::negative_zy:
         transform_x = {0.0f, 0.0f, -scale, 0.0f};
         transform_y = {0.0f, -scale, 0.0f, 0.0f};
         break;
      }
   }

   if (not _detail_map_name.empty()) {
      constants.has_detail_map = true;
      constants.detail_map_index = _detail_map->srv.index;
   }
   else {
      constants.has_detail_map = false;
      constants.detail_map_index = texture_manager.null_detail_map()->srv.index;
   }

   auto constants_allocation = dynamic_buffer_allocator.allocate_and_copy(constants);

   command_list.copy_buffer_region(_terrain_constants_buffer.get(), 0,
                                   constants_allocation.resource,
                                   constants_allocation.offset, sizeof(constants));

   const float terrain_half_world_size = (_terrain_length / 2.0f) * terrain.grid_scale;

   _active = terrain.active_flags.terrain;

   _terrain_half_world_size = float2{terrain_half_world_size, terrain_half_world_size};
   _terrain_grid_size = terrain.grid_scale;
   _terrain_height_scale = terrain.height_scale;

   for (uint32 patch_index : _vertex_patches_to_upload) {
      const std::array<terrain_vertex, patch_vertex_count>& vertices =
         _patch_vertices[patch_index];

      const uint64 patch_offset = patch_index * sizeof(vertices);

      std::byte* const upload_ptr = _vertices_upload_ptr[_device.frame_index()];

      std::memcpy(upload_ptr + patch_offset, &vertices, sizeof(vertices));

      command_list.copy_buffer_region(_vertex_buffer.get(), patch_offset,
                                      _upload_buffer.get(),
                                      _vertices_upload_offset[_device.frame_index()] +
                                         patch_offset,
                                      sizeof(vertices));

      _dirty_vertex_patches[patch_index] = false;
   }

   for (uint32 patch_index : _vertex_attributes_patches_to_upload) {
      const std::array<terrain_vertex_attributes, patch_vertex_count>& vertices =
         _patch_vertex_attributes[patch_index];

      const uint64 patch_offset = patch_index * sizeof(vertices);

      std::byte* const upload_ptr =
         _vertex_attributes_upload_ptr[_device.frame_index()];

      std::memcpy(upload_ptr + patch_offset, &vertices, sizeof(vertices));

      command_list.copy_buffer_region(_vertex_buffer.get(),
                                      _vertex_attributes_offset + patch_offset,
                                      _upload_buffer.get(),
                                      _vertex_attributes_upload_offset[_device.frame_index()] +
                                         patch_offset,
                                      sizeof(vertices));

      _dirty_vertex_attributes_patches[patch_index] = false;
   }

   _vertex_patches_to_upload.clear();
   _vertex_attributes_patches_to_upload.clear();
}

void terrain::draw(const terrain_draw draw, const frustum& view_frustum,
                   std::span<const terrain_cut> terrain_cuts,
                   gpu_virtual_address camera_constant_buffer_view,
                   gpu_virtual_address lights_constant_buffer_view,
                   gpu::graphics_command_list& command_list,
                   root_signature_library& root_signatures, pipeline_library& pipelines)
{
   if (not _active) return;

   command_list.set_pipeline_state(select_pipeline(draw, pipelines));

   command_list.set_graphics_root_signature(root_signatures.terrain.get());
   command_list.set_graphics_cbv(rs::terrain::frame_cbv, camera_constant_buffer_view);
   command_list.set_graphics_cbv(rs::terrain::lights_cbv, lights_constant_buffer_view);
   command_list.set_graphics_cbv(rs::terrain::terrain_cbv, _terrain_cbv);

   command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);
   command_list.ia_set_index_buffer(
      {.buffer_location = _device.get_gpu_virtual_address(_index_buffer.get()),
       .size_in_bytes = sizeof(terrain_patch_indices)});
   command_list.ia_set_vertex_buffers(0, _vertex_buffer_views);

   for (uint32 patch_z = 0; patch_z < _patches_length; ++patch_z) {
      for (uint32 patch_x = 0; patch_x < _patches_length; ++patch_x) {
         const uint32 patch_index = patch_z * _patches_length + patch_x;

         const terrain_patch& patch = _patches[patch_index];

         const float min_x = (patch_x * patch_length_grids * _terrain_grid_size) -
                             _terrain_half_world_size.x;
         const float max_x = min_x + (patch_length_grids * _terrain_grid_size);

         const float min_y = patch.min_y * _terrain_height_scale;
         const float max_y = patch.max_y * _terrain_height_scale;

         const float min_z = (patch_z * patch_length_grids * _terrain_grid_size) -
                             _terrain_half_world_size.y + _terrain_grid_size;
         const float max_z = min_z + (patch_length_grids * _terrain_grid_size);

         const math::bounding_box bbox{.min = {min_x, min_y, min_z},
                                       .max = {max_x, max_y, max_z}};

         if (not intersects(view_frustum, bbox)) continue;

         command_list.draw_indexed_instanced(static_cast<uint32>(
                                                terrain_patch_indices.size() * 2 * 3),
                                             1, 0,
                                             patch_index * patch_vertex_count, 0);
      }
   }

   if (draw == terrain_draw::depth_prepass) {
      draw_cuts(view_frustum, terrain_cuts, camera_constant_buffer_view,
                command_list, root_signatures, pipelines);
   }
}

void terrain::draw_cuts(const frustum& view_frustum,
                        std::span<const terrain_cut> terrain_cuts,
                        gpu_virtual_address camera_constant_buffer_view,
                        gpu::graphics_command_list& command_list,
                        root_signature_library& root_signatures,
                        pipeline_library& pipelines)
{
   if (terrain_cuts.empty()) return;

   command_list.om_set_stencil_ref(0x0);
   command_list.set_graphics_root_signature(root_signatures.terrain_cut_mesh.get());
   command_list.set_graphics_cbv(rs::terrain_cut_mesh::frame_cbv,
                                 camera_constant_buffer_view);

   command_list.set_pipeline_state(pipelines.terrain_cut_mesh_mark.get());

   for (const auto& cut : terrain_cuts) {
      if (not intersects(view_frustum, cut.bbox)) continue;

      command_list.set_graphics_cbv(rs::terrain_cut_mesh::object_cbv,
                                    cut.constant_buffer);

      command_list.ia_set_index_buffer(cut.index_buffer_view);
      command_list.ia_set_vertex_buffers(0, cut.position_vertex_buffer_view);

      command_list.draw_indexed_instanced(cut.index_count, 1, cut.start_index,
                                          cut.start_vertex, 0);
   }

   command_list.set_pipeline_state(pipelines.terrain_cut_mesh_clear.get());

   for (const auto& cut : terrain_cuts) {
      if (not intersects(view_frustum, cut.bbox)) continue;

      command_list.set_graphics_cbv(rs::terrain_cut_mesh::object_cbv,
                                    cut.constant_buffer);

      command_list.ia_set_index_buffer(cut.index_buffer_view);
      command_list.ia_set_vertex_buffers(0, cut.position_vertex_buffer_view);

      command_list.draw_indexed_instanced(cut.index_count, 1, cut.start_index,
                                          cut.start_vertex, 0);
   }
}

void terrain::process_updated_texture(const updated_textures& updated)
{
   for (std::size_t i = 0; i < texture_count; ++i) {
      if (auto new_texture = updated.check(_diffuse_maps_names[i]); new_texture) {
         _diffuse_map_load_tokens[i] = nullptr;
         _diffuse_maps[i] = std::move(new_texture);
      }
   }

   if (auto new_texture = updated.check(_detail_map_name); new_texture) {
      _detail_map_load_token = nullptr;
      _detail_map = std::move(new_texture);
   }
}

void terrain::create_patches()
{
   const uint32 patch_count = _patches_length * _patches_length;

   _patches.clear();
   _patches.resize(patch_count);

   _patch_vertices.clear();
   _patch_vertices.resize(patch_count);

   _patch_vertex_attributes.clear();
   _patch_vertex_attributes.resize(patch_count);

   _dirty_vertex_patches.clear();
   _dirty_vertex_patches.resize(patch_count);

   _vertex_patches_to_upload.clear();
   _vertex_patches_to_upload.reserve(patch_count);

   _dirty_vertex_attributes_patches.clear();
   _dirty_vertex_attributes_patches.resize(patch_count);

   _vertex_attributes_patches_to_upload.clear();
   _vertex_attributes_patches_to_upload.reserve(patch_count);
}

void terrain::create_gpu_resources()
{
   const uint32 vertices_size =
      _patches_length * _patches_length *
      sizeof(std::array<terrain_vertex, patch_vertex_count>);

   const uint32 aligned_vertices_size =
      math::align_up(vertices_size, gpu::raw_uav_srv_byte_alignment);

   const uint32 vertex_attributes_size =
      _patches_length * _patches_length *
      sizeof(std::array<terrain_vertex_attributes, patch_vertex_count>);

   _vertex_buffer = {_device.create_buffer({.size = aligned_vertices_size + vertex_attributes_size,
                                            .debug_name =
                                               "Terrain Vertex Buffer"},
                                           gpu::heap_type::default_),
                     _device};

   _vertex_attributes_offset = aligned_vertices_size;

   _vertex_buffer_views = {
      gpu::vertex_buffer_view{
         .buffer_location = _device.get_gpu_virtual_address(_vertex_buffer.get()),
         .size_in_bytes = vertices_size,
         .stride_in_bytes = sizeof(terrain_vertex),
      },
      gpu::vertex_buffer_view{
         .buffer_location = _device.get_gpu_virtual_address(_vertex_buffer.get()) +
                            _vertex_attributes_offset,
         .size_in_bytes = vertex_attributes_size,
         .stride_in_bytes = sizeof(terrain_vertex_attributes),
      },
   };

   _foliage_map = {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                                           .format = DXGI_FORMAT_R8_UINT,
                                           .width = _terrain_length / 2,
                                           .height = _terrain_length / 2,
                                           .debug_name = "Terrain Foliage Map"},
                                          gpu::barrier_layout::common,
                                          gpu::legacy_resource_state::common),
                   _device};

   _foliage_map_srv = {_device.create_shader_resource_view(_foliage_map.get(),
                                                           {.format = DXGI_FORMAT_R8_UINT}),
                       _device};

   _foliage_map_upload_row_pitch =
      math::align_up(_terrain_length / 2, gpu::texture_data_pitch_alignment);

   const uint32 vertex_upload_size =
      math::align_up(aligned_vertices_size + vertex_attributes_size,
                     gpu::texture_data_placement_alignment);

   const uint32 foliage_map_upload_item_size =
      math::align_up((_terrain_length / 2) * _foliage_map_upload_row_pitch,
                     gpu::texture_data_placement_alignment);

   const uint32 upload_buffer_frame_size =
      vertex_upload_size + foliage_map_upload_item_size;

   _upload_buffer = {_device.create_buffer({.size = upload_buffer_frame_size * 2,
                                            .debug_name =
                                               "Terrain Upload Buffer"},
                                           gpu::heap_type::upload),
                     _device};

   std::byte* const upload_buffer_ptr =
      static_cast<std::byte*>(_device.map(_upload_buffer.get(), 0, {}));

   for (uint32 i = 0; i < gpu::frame_pipeline_length; ++i) {
      const uint32 frame_offset = (upload_buffer_frame_size * i);

      _vertices_upload_offset[i] = frame_offset;
      _vertices_upload_ptr[i] = upload_buffer_ptr + _vertices_upload_offset[i];

      _vertex_attributes_upload_offset[i] = frame_offset + aligned_vertices_size;
      _vertex_attributes_upload_ptr[i] =
         upload_buffer_ptr + _vertex_attributes_upload_offset[i];

      _foliage_map_upload_offset[i] = frame_offset + vertex_upload_size;
      _foliage_map_upload_ptr[i] = upload_buffer_ptr + _foliage_map_upload_offset[i];
   }
}
}
