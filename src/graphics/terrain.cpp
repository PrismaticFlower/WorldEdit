#include "terrain.hpp"
#include "gpu/barrier_helpers.hpp"
#include "gpu/command_list.hpp"
#include "gpu/device.hpp"
#include "math/align.hpp"

#include <limits>

#include <glm/glm.hpp>

namespace sk::graphics {

namespace {

constexpr uint32 patch_grid_count = 16;
constexpr uint32 patch_point_count = 17;

constexpr auto y = 45 / patch_grid_count;
constexpr auto x = 45 % patch_grid_count;

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

terrain::terrain(gpu::device& device) : _gpu_device{&device}
{
   _index_buffer =
      _gpu_device->create_buffer({.size = sizeof(terrain_patch_indices)},
                                 D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);
}

void terrain::init(const world::terrain& terrain, gpu::command_list& command_list,
                   gpu::dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   _terrain_length = terrain.active_right_offset - terrain.active_left_offset;
   _patch_count = _terrain_length / patch_grid_count;

   _terrain_half_world_size = float2{(_terrain_length / 2.0f) * terrain.grid_scale};
   _terrain_grid_size = terrain.grid_scale;
   _terrain_height_scale = std::numeric_limits<int16>::max() * terrain.height_scale;

   _height_map =
      _gpu_device->create_texture({.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                                   .format = DXGI_FORMAT_R16_SNORM,
                                   .width = _terrain_length,
                                   .height = _terrain_length},
                                  D3D12_RESOURCE_STATE_COPY_DEST);

   uint32 row_pitch = math::align_up(_terrain_length * uint32{sizeof(uint16)},
                                     D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
   uint32 texture_data_size = row_pitch * _terrain_length;

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

   uint32 input_length_offset = (terrain.length - _terrain_length) / 2;
   uint32 input_row_pitch = terrain.length * sizeof(int16);
   const int16* input_data =
      &terrain.heightmap[{input_length_offset, input_length_offset}];

   for (uint32 y = 0; y < _terrain_length; ++y) {
      std::memcpy(upload_buffer_ptr + (y * row_pitch),
                  input_data + (y * input_row_pitch), sizeof(int16) * _terrain_length);
   }

   const D3D12_RANGE write_range{0, texture_data_size};
   height_map_upload_buffer.resource()->Unmap(0, &write_range);

   auto indices_allocation =
      dynamic_buffer_allocator.allocate(sizeof(terrain_patch_indices));

   std::memcpy(indices_allocation.cpu_address, &terrain_patch_indices,
               sizeof(terrain_patch_indices));

   command_list.copy_buffer_region(*_index_buffer.resource(), 0,
                                   *dynamic_buffer_allocator.resource(),
                                   indices_allocation.gpu_address -
                                      dynamic_buffer_allocator.gpu_base_address(),
                                   sizeof(terrain_patch_indices));

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
      gpu::transition_barrier(*_index_buffer.resource(), D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_INDEX_BUFFER));
   command_list.deferred_resource_barrier(
      gpu::transition_barrier(*_height_map.resource(), D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE |
                                 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

   const std::array resource_view_descriptions{
      gpu::resource_view_desc{.resource = *_height_map.resource(),
                              .view_desc = gpu::shader_resource_view_desc{
                                 .format = _height_map.format(),
                                 .type_description = gpu::texture2d_srv{}}}};

   _resource_views = _gpu_device->create_resource_view_set(resource_view_descriptions);
}

void terrain::draw(const frustrum& view_frustrum,
                   gpu::descriptor_range camera_constant_buffer_view,
                   gpu::command_list& command_list,
                   gpu::dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   (void)view_frustrum; // TODO: Frustrum culling.

   // TODO: Clean this nonsense up.
   auto patches_srv_allocation = dynamic_buffer_allocator.allocate(
      _patch_count * _patch_count * sizeof(std::array<uint32, 2>));

   std::array<uint32, 2>* patches =
      reinterpret_cast<std::array<uint32, 2>*>(patches_srv_allocation.cpu_address);

   for (uint32 y = 0; y < _patch_count; ++y) {
      for (uint32 x = 0; x < _patch_count; ++x) {
         const std::array<uint32, 2> patch_index{x * patch_grid_count,
                                                 y * patch_grid_count};

         std::memcpy(patches, &patch_index, sizeof(patch_index));

         ++patches;
      }
   }

   std::array<float, 4> root_constants{_terrain_half_world_size.x,
                                       _terrain_half_world_size.y,
                                       _terrain_grid_size, _terrain_height_scale};

   command_list.set_pipeline_state(*_gpu_device->pipelines.terrain_basic);

   command_list.set_graphics_root_signature(*_gpu_device->root_signatures.terrain);
   command_list.set_graphics_root_descriptor_table(0, camera_constant_buffer_view);
   command_list.set_graphics_root_32bit_constants(1, std::as_bytes(std::span{root_constants}),
                                                  0);
   command_list.set_graphics_root_descriptor_table(2, _resource_views.descriptors());
   command_list.set_graphics_root_shader_resource_view(3, patches_srv_allocation.gpu_address);

   command_list.ia_set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
   command_list.ia_set_index_buffer(
      {.BufferLocation = _index_buffer.resource()->GetGPUVirtualAddress(),
       .SizeInBytes = _index_buffer.size(),
       .Format = DXGI_FORMAT_R16_UINT});

   command_list.draw_indexed_instanced(static_cast<uint32>(
                                          terrain_patch_indices.size() * 2 * 3),
                                       _patch_count * _patch_count, 0, 0, 0);
}

}
