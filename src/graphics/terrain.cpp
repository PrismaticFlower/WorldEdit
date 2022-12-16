#include "terrain.hpp"
#include "gpu/barrier.hpp"
#include "math/align.hpp"

#include <limits>
#include <stdexcept>

#include <glm/glm.hpp>

#include <range/v3/view.hpp>

namespace we::graphics {

namespace {

#pragma warning(disable : 4324) // structure was padded due to alignment specifier

struct alignas(16) terrain_constants {
   float2 half_world_size;
   float grid_size;
   float height_scale;

   uint32 height_map;
   uint32 texture_weight_maps;
   std::array<uint32, 2> padding;

   std::array<std::array<uint32, 4>, terrain::texture_count> diffuse_maps;

   std::array<float4, terrain::texture_count> texture_transform_x;
   std::array<float4, terrain::texture_count> texture_transform_y;
};

static_assert(sizeof(terrain_constants) == 800);

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

auto select_pipeline(const terrain_draw draw, pipeline_library& pipelines)
   -> gpu::pipeline_handle
{
   switch (draw) {
   case terrain_draw::depth_prepass:
      return pipelines.terrain_depth_prepass.get();
   case terrain_draw::main:
      return pipelines.terrain_normal.get();
   }

   std::unreachable();
}

}

terrain::terrain(gpu::device& device, texture_manager& texture_manager)
   : _device{device}, _texture_manager{texture_manager}
{
   _index_buffer = {_device.create_buffer({.size = sizeof(terrain_patch_indices),
                                           .debug_name =
                                              "Terrain Patch Indices"},
                                          gpu::heap_type::default_),
                    device.direct_queue};
}

void terrain::init(const world::terrain& terrain,
                   gpu::graphics_command_list& command_list,
                   dynamic_buffer_allocator& dynamic_buffer_allocator)
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

void terrain::draw(const terrain_draw draw, const frustrum& view_frustrum,
                   gpu_virtual_address camera_constant_buffer_view,
                   gpu_virtual_address lights_constant_buffer_view,
                   gpu::graphics_command_list& command_list,
                   root_signature_library& root_signatures, pipeline_library& pipelines,
                   dynamic_buffer_allocator& dynamic_buffer_allocator)
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

   command_list.set_pipeline_state(select_pipeline(draw, pipelines));

   command_list.set_graphics_root_signature(root_signatures.terrain.get());
   command_list.set_graphics_cbv(rs::terrain::frame_cbv, camera_constant_buffer_view);
   command_list.set_graphics_cbv(rs::terrain::lights_cbv, lights_constant_buffer_view);
   command_list.set_graphics_cbv(rs::terrain::terrain_cbv, _terrain_cbv);
   command_list.set_graphics_srv(rs::terrain::terrain_patch_data_srv,
                                 patches_srv_allocation.gpu_address);

   command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);
   command_list.ia_set_index_buffer(
      {.buffer_location = _device.get_gpu_virtual_address(_index_buffer.get()),
       .size_in_bytes = sizeof(terrain_patch_indices)});

   command_list.draw_indexed_instanced(static_cast<uint32>(
                                          terrain_patch_indices.size() * 2 * 3),
                                       visible_patch_count, 0, 0, 0);
}

void terrain::process_updated_texture(gpu::graphics_command_list& command_list,
                                      const updated_textures& updated)
{
   using namespace ranges::views;

   const gpu_virtual_address constant_buffer_address =
      _device.get_gpu_virtual_address(_terrain_constants_buffer.get());
   bool need_barrier = false;

   for (auto [i, name, texture] : zip(iota(std::size_t{0}, texture_count),
                                      _diffuse_maps_names, _diffuse_maps)) {
      if (auto new_texture = updated.find(name); new_texture != updated.end()) {
         texture = new_texture->second;

         command_list.write_buffer_immediate(constant_buffer_address +
                                                offsetof(terrain_constants, diffuse_maps) +
                                                i * sizeof(std::array<uint32, 4>),
                                             texture->srv_srgb.index);

         need_barrier = true;
      }
   }

   if (need_barrier) {
      command_list.deferred_resource_barrier(
         gpu::transition_barrier(_terrain_constants_buffer.get(),
                                 gpu::resource_state::copy_dest,
                                 gpu::resource_state::vertex_and_constant_buffer));
   }
}

void terrain::init_gpu_resources(const world::terrain& terrain,
                                 gpu::graphics_command_list& command_list,
                                 dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   init_textures(terrain);
   init_gpu_height_map(terrain, command_list);
   init_gpu_texture_weight_map(terrain, command_list);
   init_gpu_terrain_constants_buffer(terrain, command_list, dynamic_buffer_allocator);

   _terrain_cbv = _device.get_gpu_virtual_address(_terrain_constants_buffer.get());

   auto indices_allocation =
      dynamic_buffer_allocator.allocate(sizeof(terrain_patch_indices));

   std::memcpy(indices_allocation.cpu_address, &terrain_patch_indices,
               sizeof(terrain_patch_indices));

   command_list.copy_buffer_region(_index_buffer.get(), 0,
                                   dynamic_buffer_allocator.resource(),
                                   indices_allocation.offset,
                                   sizeof(terrain_patch_indices));

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(_index_buffer.get(), gpu::resource_state::copy_dest,
                              gpu::resource_state::index_buffer));
}

void terrain::init_gpu_height_map(const world::terrain& terrain,
                                  gpu::graphics_command_list& command_list)
{
   _height_map = {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                                          .format = DXGI_FORMAT_R16_SNORM,
                                          .width = _terrain_length,
                                          .height = _terrain_length},
                                         gpu::resource_state::copy_dest),
                  _device.direct_queue};

   const uint32 row_pitch = math::align_up(_terrain_length * uint32{sizeof(uint16)},
                                           gpu::texture_data_pitch_alignment);
   const uint32 texture_data_size = row_pitch * _terrain_length;

   _height_map_upload_buffer =
      {_device.create_buffer({.size = texture_data_size,
                              .debug_name = "Height Map Upload Buffer"},
                             gpu::heap_type::upload),
       _device.direct_queue};

   std::byte* const upload_buffer_ptr =
      static_cast<std::byte*>(_device.map(_height_map_upload_buffer.get(), 0, {}));

   for (uint32 y = 0; y < _terrain_length; ++y) {
      std::memcpy(upload_buffer_ptr + (y * row_pitch),
                  &terrain.height_map[{0, y}], sizeof(int16) * _terrain_length);
   }

   _device.unmap(_height_map_upload_buffer.get(), 0, {0, texture_data_size});

   command_list.copy_buffer_to_texture(_height_map.get(), 0, 0, 0, 0,
                                       _height_map_upload_buffer.get(),
                                       {.format = DXGI_FORMAT_R16_SNORM,
                                        .width = _terrain_length,
                                        .height = _terrain_length,
                                        .depth = 1,
                                        .row_pitch = row_pitch});

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(_height_map.get(), gpu::resource_state::copy_dest,
                              gpu::resource_state::all_shader_resource));

   _height_map_srv = {_device.create_shader_resource_view(_height_map.get(),
                                                          {.format = DXGI_FORMAT_R16_SNORM}),
                      _device.direct_queue};
}

void terrain::init_gpu_texture_weight_map(const world::terrain& terrain,
                                          gpu::graphics_command_list& command_list)
{
   _texture_weight_maps = {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                                                   .format = DXGI_FORMAT_R8_UNORM,
                                                   .width = _terrain_length,
                                                   .height = _terrain_length,
                                                   .array_size = texture_count},
                                                  gpu::resource_state::copy_dest),
                           _device.direct_queue};

   const uint32 row_pitch =
      math::align_up(_terrain_length, gpu::texture_data_pitch_alignment);
   const uint32 texture_item_size =
      math::align_up(row_pitch * _terrain_length, gpu::texture_data_placement_alignment);
   const uint32 texture_data_size = texture_item_size * texture_count;

   _texture_weight_upload_buffer =
      {_device.create_buffer({.size = texture_data_size,
                              .debug_name = "Texture Weight Map Upload Buffer"},
                             gpu::heap_type::upload),
       _device.direct_queue};

   std::byte* const upload_buffer_ptr = static_cast<std::byte*>(
      _device.map(_texture_weight_upload_buffer.get(), 0, {}));

   for (uint32 item = 0; item < texture_count; ++item) {
      auto item_upload_ptr = upload_buffer_ptr + (texture_item_size * item);

      for (uint32 y = 0; y < _terrain_length; ++y) {
         std::memcpy(item_upload_ptr + (y * row_pitch),
                     &terrain.texture_weight_maps[item][{0, y}], _terrain_length);
      }
   }

   _device.unmap(_texture_weight_upload_buffer.get(), 0, {0, texture_data_size});

   for (uint32 item = 0; item < texture_count; ++item) {
      command_list.copy_buffer_to_texture(_texture_weight_maps.get(), item, 0, 0,
                                          0, _texture_weight_upload_buffer.get(),
                                          {.offset = texture_item_size * item,
                                           .format = DXGI_FORMAT_R8_UNORM,
                                           .width = _terrain_length,
                                           .height = _terrain_length,
                                           .depth = 1,
                                           .row_pitch = row_pitch});
   }

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(_texture_weight_maps.get(), gpu::resource_state::copy_dest,
                              gpu::resource_state::all_shader_resource));

   _texture_weight_maps_srv =
      {_device.create_shader_resource_view(_texture_weight_maps.get(),
                                           {.format = DXGI_FORMAT_R8_UNORM}),
       _device.direct_queue};
}

void terrain::init_gpu_terrain_constants_buffer(const world::terrain& terrain,
                                                gpu::graphics_command_list& command_list,
                                                dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   using namespace ranges::views;

   terrain_constants constants{
      .half_world_size = _terrain_half_world_size,
      .grid_size = _terrain_grid_size,
      .height_scale = _terrain_height_scale,

      .height_map = _height_map_srv.get().index,
      .texture_weight_maps = _texture_weight_maps_srv.get().index,
   };

   for (std::size_t i = 0; i < texture_count; ++i) {
      constants.diffuse_maps[i][0] = _diffuse_maps[i]->srv_srgb.index;
   }

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
      {_device.create_buffer({.size = sizeof(constants),
                              .debug_name = "Terrain Constants Buffer"},
                             gpu::heap_type::default_),
       _device.direct_queue};

   auto upload_allocation = dynamic_buffer_allocator.allocate(sizeof(constants));

   std::memcpy(upload_allocation.cpu_address, &constants, sizeof(constants));

   command_list.copy_buffer_region(_terrain_constants_buffer.get(), 0,
                                   dynamic_buffer_allocator.resource(),
                                   upload_allocation.offset, sizeof(constants));

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(_terrain_constants_buffer.get(),
                              gpu::resource_state::copy_dest,
                              gpu::resource_state::vertex_and_constant_buffer));
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
