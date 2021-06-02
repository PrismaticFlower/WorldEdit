#include "terrain.hpp"
#include "gpu/barrier_helpers.hpp"
#include "gpu/command_list.hpp"
#include "gpu/device.hpp"
#include "math/align.hpp"

#include <limits>
#include <stdexcept>

#include <glm/glm.hpp>

#include <range/v3/view.hpp>

namespace we::graphics {

namespace {

#pragma warning(disable : 4324) // structure was padded due to alignment specifier

struct alignas(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT) terrain_constants {
   float2 half_world_size;
   float grid_size;
   float height_scale;

   std::array<float4, terrain::texture_count> texture_transform_x;
   std::array<float4, terrain::texture_count> texture_transform_y;
};

static_assert(sizeof(terrain_constants) == 768);

struct patch_info_shader {
   uint32 x;
   uint32 y;
   uint32 active_textures;
   uint32 padding;
};

static_assert(sizeof(patch_info_shader) == 16);

constexpr uint32 patch_grid_count = 16;
constexpr uint32 patch_point_count = 17;

constexpr auto generate_patch_indices()
{
   using tri = std::array<uint16, 3>;

   std::array<std::array<tri, 2>, patch_grid_count * patch_grid_count> triangles{};

   static_assert(sizeof(triangles) ==
                 sizeof(tri) * patch_grid_count * patch_grid_count * 2);

   for (uint16 y = 0; y < patch_grid_count; ++y) {
      for (uint16 x = 0; x < patch_grid_count; ++x) {
         const auto index = [](uint16 x, uint16 y) -> uint16 {
            return y * patch_point_count + x;
         };

         triangles[y * patch_grid_count + x][0] = {index(x, y), index(x + 1, y + 1),
                                                   index(x + 1, y)};
         triangles[y * patch_grid_count + x][1] = {index(x, y), index(x, y + 1),
                                                   index(x + 1, y + 1)};
      }
   }

   return triangles;
}

constexpr auto terrain_patch_indices = generate_patch_indices();

}

terrain::terrain(gpu::device& device, texture_manager& texture_manager)
   : _gpu_device{&device}, _texture_manager{texture_manager}
{
   _index_buffer =
      _gpu_device->create_buffer({.size = sizeof(terrain_patch_indices)},
                                 D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);
}

void terrain::init(const world::terrain& terrain, gpu::command_list& command_list,
                   gpu::dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   _terrain_length = terrain.length;
   _patch_count = _terrain_length / patch_grid_count;

   _terrain_half_world_size = float2{(_terrain_length / 2.0f) * terrain.grid_scale};
   _terrain_grid_size = terrain.grid_scale;
   _terrain_height_scale = std::numeric_limits<int16>::max() * terrain.height_scale;

   _active = terrain.active_flags.terrain;

   init_gpu_resources(terrain, command_list, dynamic_buffer_allocator);
   init_patches_info(terrain);
}

void terrain::draw(const frustrum& view_frustrum,
                   gpu::descriptor_range camera_constant_buffer_view,
                   gpu::descriptor_range light_descriptors,
                   gpu::command_list& command_list,
                   gpu::dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   if (not _active) return;

   auto patches_srv_allocation = dynamic_buffer_allocator.allocate(
      _patch_count * _patch_count * sizeof(patch_info_shader));

   uint32 visible_patch_count = 0;

   for (const auto& patch : _patches) {
      if (not intersects(view_frustrum, patch.bbox)) continue;

      patch_info_shader info{.x = patch.x * patch_grid_count,
                             .y = patch.y * patch_grid_count,
                             .active_textures = patch.active_textures.to_ulong()};

      std::memcpy(patches_srv_allocation.cpu_address +
                     (sizeof(patch_info_shader) * visible_patch_count),
                  &info, sizeof(patch_info_shader));

      ++visible_patch_count;
   }

   command_list.set_pipeline_state(*_gpu_device->pipelines.terrain_normal);

   command_list.set_graphics_root_signature(*_gpu_device->root_signatures.terrain);
   command_list.set_graphics_root_descriptor_table(0, camera_constant_buffer_view);
   command_list.set_graphics_root_descriptor_table(1, light_descriptors);
   command_list.set_graphics_root_descriptor_table(2, _resource_views.descriptors());
   command_list.set_graphics_root_shader_resource_view(3, patches_srv_allocation.gpu_address);
   command_list.set_graphics_root_descriptor_table(4, _texture_resource_views.descriptors());

   command_list.ia_set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
   command_list.ia_set_index_buffer(
      {.BufferLocation = _index_buffer.resource()->GetGPUVirtualAddress(),
       .SizeInBytes = _index_buffer.size(),
       .Format = DXGI_FORMAT_R16_UINT});

   command_list.draw_indexed_instanced(static_cast<uint32>(
                                          terrain_patch_indices.size() * 2 * 3),
                                       visible_patch_count, 0, 0, 0);
}

void terrain::process_updated_texture(updated_texture updated)
{
   using namespace ranges::views;

   bool reinit_views = false;

   for (auto [name, texture] : zip(_diffuse_maps_names, _diffuse_maps)) {
      if (name != updated.name) continue;

      texture = updated.texture;
      reinit_views = true;
   }

   if (reinit_views) init_textures_resource_views();
}

void terrain::init_gpu_resources(const world::terrain& terrain,
                                 gpu::command_list& command_list,
                                 gpu::dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   init_textures(terrain);
   init_textures_resource_views();
   init_gpu_height_map(terrain, command_list);
   init_gpu_texture_weight_map(terrain, command_list);
   init_gpu_terrain_constants_buffer(terrain, command_list, dynamic_buffer_allocator);

   // _resource_views = _gpu_device->create_resource_view_set(std::array{
   //    gpu::resource_view_desc{
   //       .resource = *_terrain_constants_buffer.resource(),
   //       .view_desc =
   //          gpu::constant_buffer_view{.buffer_location =
   //                                       _terrain_constants_buffer.resource()->GetGPUVirtualAddress(),
   //                                    .size = _terrain_constants_buffer.size()}},
   //
   //    gpu::resource_view_desc{.resource = *_height_map.resource(),
   //                            .view_desc =
   //                               gpu::shader_resource_view_desc{
   //                                  .format = _height_map.format(),
   //                                  .type_description = gpu::texture2d_srv{}}},
   //
   //    gpu::resource_view_desc{.resource = *_texture_weight_maps.resource(),
   //                            .view_desc = gpu::shader_resource_view_desc{
   //                               .format = _texture_weight_maps.format(),
   //                               .type_description = gpu::texture2d_array_srv{
   //                                  .array_size = _texture_weight_maps.array_size()}}}});

   // Above ICEs, workaround below...

   std::array resource_view_descs{
      gpu::resource_view_desc{.resource = *_terrain_constants_buffer.resource()},

      gpu::resource_view_desc{.resource = *_height_map.resource()},

      gpu::resource_view_desc{.resource = *_texture_weight_maps.resource()}};

   resource_view_descs[0].view_desc =
      gpu::constant_buffer_view{.buffer_location =
                                   _terrain_constants_buffer.resource()->GetGPUVirtualAddress(),
                                .size = _terrain_constants_buffer.size()};

   resource_view_descs[1].view_desc =
      gpu::shader_resource_view_desc{.format = _height_map.format(),
                                     .type_description = gpu::texture2d_srv{}};

   resource_view_descs[2].view_desc =
      gpu::shader_resource_view_desc{.format = _texture_weight_maps.format(),
                                     .type_description = gpu::texture2d_array_srv{
                                        .array_size =
                                           _texture_weight_maps.array_size()}};

   _resource_views = _gpu_device->create_resource_view_set(resource_view_descs);

   auto indices_allocation =
      dynamic_buffer_allocator.allocate(sizeof(terrain_patch_indices));

   std::memcpy(indices_allocation.cpu_address, &terrain_patch_indices,
               sizeof(terrain_patch_indices));

   command_list.copy_buffer_region(*_index_buffer.resource(), 0,
                                   *dynamic_buffer_allocator.resource(),
                                   indices_allocation.gpu_address -
                                      dynamic_buffer_allocator.gpu_base_address(),
                                   sizeof(terrain_patch_indices));

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(*_index_buffer.resource(), D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_INDEX_BUFFER));
}

void terrain::init_gpu_height_map(const world::terrain& terrain,
                                  gpu::command_list& command_list)
{
   _height_map =
      _gpu_device->create_texture({.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                                   .format = DXGI_FORMAT_R16_SNORM,
                                   .width = _terrain_length,
                                   .height = _terrain_length},
                                  D3D12_RESOURCE_STATE_COPY_DEST);

   const uint32 row_pitch = math::align_up(_terrain_length * uint32{sizeof(uint16)},
                                           D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
   const uint32 texture_data_size = row_pitch * _terrain_length;

   auto height_map_upload_buffer =
      _gpu_device->create_buffer({.size = texture_data_size}, D3D12_HEAP_TYPE_UPLOAD,
                                 D3D12_RESOURCE_STATE_GENERIC_READ);

   std::byte* const upload_buffer_ptr = [&] {
      const D3D12_RANGE read_range{};
      void* map_void_ptr = nullptr;

      throw_if_failed(height_map_upload_buffer.resource()->Map(0, &read_range,
                                                               &map_void_ptr));

      return static_cast<std::byte*>(map_void_ptr);
   }();

   for (uint32 y = 0; y < _terrain_length; ++y) {
      std::memcpy(upload_buffer_ptr + (y * row_pitch),
                  &terrain.height_map[{0, y}], sizeof(int16) * _terrain_length);
   }

   const D3D12_RANGE write_range{0, texture_data_size};
   height_map_upload_buffer.resource()->Unmap(0, &write_range);

   command_list.copy_texture_region(
      {.pResource = _height_map.resource(),
       .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
       .SubresourceIndex = 0},
      0, 0, 0,
      {.pResource = height_map_upload_buffer.resource(),
       .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
       .PlacedFootprint = {.Offset = 0,
                           .Footprint = {.Format = DXGI_FORMAT_R16_SNORM,
                                         .Width = _terrain_length,
                                         .Height = _terrain_length,
                                         .Depth = 1,
                                         .RowPitch = row_pitch}}});

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(*_height_map.resource(), D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE |
                                 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

void terrain::init_gpu_texture_weight_map(const world::terrain& terrain,
                                          gpu::command_list& command_list)
{
   _texture_weight_maps =
      _gpu_device->create_texture({.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                                   .format = DXGI_FORMAT_R8_UNORM,
                                   .width = _terrain_length,
                                   .height = _terrain_length,
                                   .array_size = texture_count},
                                  D3D12_RESOURCE_STATE_COPY_DEST);

   const uint32 row_pitch =
      math::align_up(_terrain_length, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
   const uint32 texture_item_size =
      math::align_up(row_pitch * _terrain_length, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
   const uint32 texture_data_size = texture_item_size * texture_count;

   auto upload_buffer =
      _gpu_device->create_buffer({.size = texture_data_size}, D3D12_HEAP_TYPE_UPLOAD,
                                 D3D12_RESOURCE_STATE_GENERIC_READ);

   std::byte* const upload_buffer_ptr = [&] {
      const D3D12_RANGE read_range{};
      void* map_void_ptr = nullptr;

      throw_if_failed(upload_buffer.resource()->Map(0, &read_range, &map_void_ptr));

      return static_cast<std::byte*>(map_void_ptr);
   }();

   for (uint32 item = 0; item < texture_count; ++item) {
      auto item_upload_ptr = upload_buffer_ptr + (texture_item_size * item);

      for (uint32 y = 0; y < _terrain_length; ++y) {
         std::memcpy(item_upload_ptr + (y * row_pitch),
                     &terrain.texture_weight_maps[item][{0, y}], _terrain_length);
      }
   }

   const D3D12_RANGE write_range{0, texture_data_size};
   upload_buffer.resource()->Unmap(0, &write_range);

   for (uint32 item = 0; item < texture_count; ++item) {
      command_list.copy_texture_region(
         {.pResource = _texture_weight_maps.resource(),
          .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
          .SubresourceIndex = item},
         0, 0, 0,
         {.pResource = upload_buffer.resource(),
          .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
          .PlacedFootprint = {.Offset = texture_item_size * item,
                              .Footprint = {.Format = DXGI_FORMAT_R8_UNORM,
                                            .Width = _terrain_length,
                                            .Height = _terrain_length,
                                            .Depth = 1,
                                            .RowPitch = row_pitch}}});
   }

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(*_texture_weight_maps.resource(), D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE |
                                 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

void terrain::init_gpu_terrain_constants_buffer(const world::terrain& terrain,
                                                gpu::command_list& command_list,
                                                gpu::dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   using namespace ranges::views;

   terrain_constants constants{.half_world_size = _terrain_half_world_size,
                               .grid_size = _terrain_grid_size,
                               .height_scale = _terrain_height_scale};

   for (auto [axis, scale, transform_x, transform_y] :
        zip(terrain.texture_axes, terrain.texture_scales,
            constants.texture_transform_x, constants.texture_transform_y)) {
      switch (axis) {
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

   _terrain_constants_buffer =
      _gpu_device->create_buffer({.size = sizeof(constants)}, D3D12_HEAP_TYPE_DEFAULT,
                                 D3D12_RESOURCE_STATE_COMMON);

   auto upload_allocation = dynamic_buffer_allocator.allocate(sizeof(constants));

   std::memcpy(upload_allocation.cpu_address, &constants, sizeof(constants));

   command_list.copy_buffer_region(*_terrain_constants_buffer.resource(), 0,
                                   *dynamic_buffer_allocator.resource(),
                                   upload_allocation.gpu_address -
                                      dynamic_buffer_allocator.gpu_base_address(),
                                   sizeof(constants));

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(*_terrain_constants_buffer.resource(),
                              D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}

void terrain::init_textures_resource_views()
{
   const auto diffuse_map_view_desc = [&](const int i) {
      return gpu::resource_view_desc{.resource = *_diffuse_maps[i]->resource(),
                                     .view_desc = gpu::shader_resource_view_desc{
                                        .format = _diffuse_maps[i]->format(),
                                        .type_description = gpu::texture2d_srv{}}};
   };

   const std::array resource_view_descriptions{
      // clang-format off
      diffuse_map_view_desc(0),
      diffuse_map_view_desc(1),
      diffuse_map_view_desc(2),
      diffuse_map_view_desc(3),
      diffuse_map_view_desc(4),
      diffuse_map_view_desc(5),
      diffuse_map_view_desc(6),
      diffuse_map_view_desc(7),
      diffuse_map_view_desc(8),
      diffuse_map_view_desc(9),
      diffuse_map_view_desc(10),
      diffuse_map_view_desc(11),
      diffuse_map_view_desc(12),
      diffuse_map_view_desc(13),
      diffuse_map_view_desc(14),
      diffuse_map_view_desc(15)
      // clang-format on
   };

   _texture_resource_views =
      _gpu_device->create_resource_view_set(resource_view_descriptions);
}

void terrain::init_textures(const world::terrain& terrain)
{
   using namespace ranges::views;

   for (auto i : iota(std::size_t{0}, _diffuse_maps_names.size())) {
      _diffuse_maps_names[i] = lowercase_string{terrain.texture_names[i]};
   }

   for (auto& name : _diffuse_maps_names) {
      if (const auto ext_offset = name.find_last_of('.');
          ext_offset != std::string::npos) {
         name.resize(ext_offset);
      }
   }

   for (auto [name, texture] : zip(_diffuse_maps_names, _diffuse_maps)) {
      if (name.empty()) {
         texture = _texture_manager.null_diffuse_map();
      }
      else {
         texture = _texture_manager.at_or(name, _texture_manager.null_diffuse_map());
      }
   }
}

void terrain::init_patches_info(const world::terrain& terrain)
{
   _patches.clear();
   _patches.reserve(_patch_count);

   for (uint32 patch_y = 0; patch_y < _patch_count; ++patch_y) {
      for (uint32 patch_x = 0; patch_x < _patch_count; ++patch_x) {

         const float min_x = (patch_x * patch_grid_count * _terrain_grid_size) -
                             _terrain_half_world_size.x;
         const float max_x = min_x + (patch_grid_count * _terrain_grid_size);

         const float min_z = (patch_y * patch_grid_count * _terrain_grid_size) -
                             _terrain_half_world_size.y + _terrain_grid_size;
         const float max_z = min_z + (patch_grid_count * _terrain_grid_size);

         float min_y = std::numeric_limits<float>::max();
         float max_y = std::numeric_limits<float>::lowest();

         std::bitset<16> active_textures{};

         for (uint32 local_y = 0; local_y < patch_point_count; ++local_y) {
            for (uint32 local_x = 0; local_x < patch_point_count; ++local_x) {
               const auto x = std::clamp(patch_x * patch_grid_count + local_x,
                                         0u, terrain.length - 1u);
               const auto y = std::clamp(patch_y * patch_grid_count + local_y,
                                         0u, terrain.length - 1u);

               const float height = terrain.height_map[{x, y}] * terrain.height_scale;

               min_y = std::min(min_y, height);
               max_y = std::max(max_y, height);

               for (uint32 texture = 0; texture < texture_count; ++texture) {
                  active_textures.set(texture,
                                      terrain.texture_weight_maps[texture][{x, y}] > 0);
               }
            }
         }

         _patches.push_back(
            {.bbox = {.min = {min_x, min_y, min_z}, .max = {max_x, max_y, max_z}},
             .x = patch_x,
             .y = patch_y,
             .active_textures = active_textures});
      }
   }
}
}
