#include "terrain.hpp"
#include "math/align.hpp"
#include "utility/string_icompare.hpp"

#include <limits>
#include <stdexcept>

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

terrain::terrain(gpu::device& device, copy_command_list_pool& copy_command_list_pool,
                 dynamic_buffer_allocator& dynamic_buffer_allocator,
                 texture_manager& texture_manager)
   : _device{device}
{
   _terrain_constants_buffer =
      {_device.create_buffer({.size = sizeof(terrain_constants),
                              .debug_name = "Terrain Constants Buffer"},
                             gpu::heap_type::default_),
       _device.direct_queue};
   _terrain_cbv = _device.get_gpu_virtual_address(_terrain_constants_buffer.get());

   _index_buffer = {_device.create_buffer({.size = sizeof(terrain_patch_indices),
                                           .debug_name =
                                              "Terrain Patch Indices"},
                                          gpu::heap_type::default_),
                    device.direct_queue};

   pooled_copy_command_list command_list = copy_command_list_pool.aquire_and_reset();

   command_list->copy_buffer_region(
      _index_buffer.get(), 0, dynamic_buffer_allocator.resource(),
      dynamic_buffer_allocator.allocate_and_copy(terrain_patch_indices).offset,
      sizeof(terrain_patch_indices));

   command_list->close();

   device.background_copy_queue.execute_command_lists(command_list.get());

   for (auto& texture : _diffuse_maps) {
      texture = texture_manager.null_diffuse_map();
   }
}

void terrain::update(const world::terrain& terrain, gpu::copy_command_list& command_list,
                     dynamic_buffer_allocator& dynamic_buffer_allocator,
                     texture_manager& texture_manager)
{
   if (std::exchange(_terrain_length, static_cast<uint32>(terrain.length)) !=
       _terrain_length) {
      _patch_count = _terrain_length / patch_grid_count;

      create_gpu_textures();
      create_unfilled_patch_info();
   }

   for (const world::dirty_rect& dirty : terrain.height_map_dirty) {
      std::byte* const upload_ptr = _height_map_upload_ptr[_device.frame_index()];
      const uint32 row_pitch = _height_map_upload_row_pitch;

      for (uint32 y = dirty.y; y < (_terrain_length + dirty.height); ++y) {
         std::memcpy(upload_ptr + (y * row_pitch) + dirty.x,
                     &terrain.height_map[{dirty.x, y}], sizeof(int16) * dirty.width);
      }

      gpu::box src_box{.left = dirty.x,
                       .top = dirty.y,
                       .front = 0,
                       .right = dirty.x + dirty.width,
                       .bottom = dirty.y + dirty.height,
                       .back = 1};

      command_list.copy_buffer_to_texture(
         _height_map.get(), 0, dirty.x, dirty.y, 0, _upload_buffer.get(),
         {.offset = _height_map_upload_offset[_device.frame_index()],
          .format = DXGI_FORMAT_R16_SINT,
          .width = _terrain_length,
          .height = _terrain_length,
          .depth = 1,
          .row_pitch = _height_map_upload_row_pitch},
         &src_box);
   }

   for (uint32 i = 0; i < texture_count; ++i) {
      for (const world::dirty_rect& dirty : terrain.texture_weight_maps_dirty[i]) {
         std::byte* const upload_ptr =
            _weight_map_upload_ptr[_device.frame_index()][i];
         const uint32 row_pitch = _weight_map_upload_row_pitch;

         for (uint32 y = dirty.y; y < (_terrain_length + dirty.height); ++y) {
            std::memcpy(upload_ptr + (y * row_pitch) + dirty.x,
                        &terrain.texture_weight_maps[i][{dirty.x, y}], dirty.width);
         }

         gpu::box src_box{.left = dirty.x,
                          .top = dirty.y,
                          .front = 0,
                          .right = dirty.x + dirty.width,
                          .bottom = dirty.y + dirty.height,
                          .back = 1};

         command_list.copy_buffer_to_texture(
            _texture_weight_maps.get(), i, dirty.x, dirty.y, 0, _upload_buffer.get(),
            {.offset = _weight_map_upload_offset[_device.frame_index()][i],
             .format = DXGI_FORMAT_R8_UNORM,
             .width = _terrain_length,
             .height = _terrain_length,
             .depth = 1,
             .row_pitch = _weight_map_upload_row_pitch},
            &src_box);
      }
   }

   for (uint32 i = 0; i < texture_count; ++i) {
      if (not string::iequals(_diffuse_maps_names[i], terrain.texture_names[i])) {
         _diffuse_maps_names[i] = lowercase_string{terrain.texture_names[i]};
         _diffuse_maps[i] =
            texture_manager.at_or(_diffuse_maps_names[i], world_texture_dimension::_2d,
                                  texture_manager.null_diffuse_map());
      }
   }

   terrain_constants constants{
      .half_world_size = _terrain_half_world_size,
      .grid_size = _terrain_grid_size,
      .height_scale = _terrain_height_scale,

      .height_map = _height_map_srv.get().index,
      .texture_weight_maps = _texture_weight_maps_srv.get().index,
   };

   for (uint32 i = 0; i < texture_count; ++i) {
      constants.diffuse_maps[i][0] = _diffuse_maps[i]->srv_srgb.index;
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

   command_list
      .copy_buffer_region(_terrain_constants_buffer.get(), 0,
                          dynamic_buffer_allocator.resource(),
                          dynamic_buffer_allocator.allocate_and_copy(constants).offset,
                          sizeof(constants));

   const float terrain_half_world_size = (_terrain_length / 2.0f) * terrain.grid_scale;

   _active = terrain.active_flags.terrain;

   _terrain_half_world_size = float2{terrain_half_world_size, terrain_half_world_size};
   _terrain_grid_size = terrain.grid_scale;
   _terrain_height_scale = std::numeric_limits<int16>::max() * terrain.height_scale;

   for (const world::dirty_rect& dirty : terrain.height_map_dirty) {
      const uint32 start_patch_x = dirty.x / patch_grid_count;
      const uint32 end_patch_x =
         start_patch_x + (dirty.width + patch_grid_count - 1) / patch_grid_count;

      const uint32 start_patch_y = dirty.y / patch_grid_count;
      const uint32 end_patch_y =
         start_patch_y + (dirty.height + patch_grid_count - 1) / patch_grid_count;

      for (uint32 patch_y = start_patch_y; patch_y < end_patch_y; ++patch_y) {
         for (uint32 patch_x = start_patch_x; patch_x < end_patch_x; ++patch_x) {
            const float min_x = (patch_x * patch_grid_count * _terrain_grid_size) -
                                _terrain_half_world_size.x;
            const float max_x = min_x + (patch_grid_count * _terrain_grid_size);

            const float min_z = (patch_y * patch_grid_count * _terrain_grid_size) -
                                _terrain_half_world_size.y + _terrain_grid_size;
            const float max_z = min_z + (patch_grid_count * _terrain_grid_size);

            float min_y = std::numeric_limits<float>::max();
            float max_y = std::numeric_limits<float>::lowest();

            for (uint32 local_y = 0; local_y < patch_point_count; ++local_y) {
               for (uint32 local_x = 0; local_x < patch_point_count; ++local_x) {
                  const auto x = std::clamp(patch_x * patch_grid_count + local_x,
                                            0u, terrain.length - 1u);
                  const auto y = std::clamp(patch_y * patch_grid_count + local_y,
                                            0u, terrain.length - 1u);

                  const float height = terrain.height_map[{x, y}] * terrain.height_scale;

                  min_y = std::min(min_y, height);
                  max_y = std::max(max_y, height);
               }
            }

            _patches[patch_y * _patch_count + patch_x].bbox =
               {.min = {min_x, min_y, min_z}, .max = {max_x, max_y, max_z}};
         }
      }
   }

   for (uint32 texture = 0; texture < texture_count; ++texture) {
      for (const world::dirty_rect& dirty : terrain.texture_weight_maps_dirty[texture]) {
         const uint32 start_patch_x = dirty.x / patch_grid_count;
         const uint32 end_patch_x =
            start_patch_x + (dirty.width + patch_grid_count - 1) / patch_grid_count;

         const uint32 start_patch_y = dirty.y / patch_grid_count;
         const uint32 end_patch_y =
            start_patch_y + (dirty.height + patch_grid_count - 1) / patch_grid_count;

         for (uint32 patch_y = start_patch_y; patch_y < end_patch_y; ++patch_y) {
            for (uint32 patch_x = start_patch_x; patch_x < end_patch_x; ++patch_x) {
               bool texture_active = false;

               for (uint32 local_y = 0; local_y < patch_point_count; ++local_y) {
                  for (uint32 local_x = 0; local_x < patch_point_count; ++local_x) {
                     const auto x = std::clamp(patch_x * patch_grid_count + local_x,
                                               0u, terrain.length - 1u);
                     const auto y = std::clamp(patch_y * patch_grid_count + local_y,
                                               0u, terrain.length - 1u);

                     texture_active |=
                        terrain.texture_weight_maps[texture][{x, y}] > 0;
                  }
               }

               _patches[patch_y * _patch_count + patch_x].active_textures[texture] =
                  texture_active;
            }
         }
      }
   }
}

void terrain::draw(const terrain_draw draw, const frustum& view_frustum,
                   std::span<const terrain_cut> terrain_cuts,
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
      if (not intersects(view_frustum, patch.bbox)) continue;

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
      if (auto new_texture = updated.find(_diffuse_maps_names[i]);
          new_texture != updated.end()) {
         _diffuse_maps[i] = new_texture->second;
      }
   }
}

void terrain::create_gpu_textures()
{
   _height_map = {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                                          .format = DXGI_FORMAT_R16_SNORM,
                                          .width = _terrain_length,
                                          .height = _terrain_length,
                                          .debug_name = "Terrain Height Map"},
                                         gpu::barrier_layout::common,
                                         gpu::legacy_resource_state::common),
                  _device.direct_queue};

   _height_map_srv = {_device.create_shader_resource_view(_height_map.get(),
                                                          {.format = DXGI_FORMAT_R16_SNORM}),
                      _device.direct_queue};

   _texture_weight_maps =
      {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                               .format = DXGI_FORMAT_R8_UNORM,
                               .width = _terrain_length,
                               .height = _terrain_length,
                               .array_size = texture_count,
                               .debug_name = "Terrain Texture Weight Maps"},
                              gpu::barrier_layout::common,
                              gpu::legacy_resource_state::common),
       _device.direct_queue};

   _texture_weight_maps_srv =
      {_device.create_shader_resource_view(_texture_weight_maps.get(),
                                           {.format = DXGI_FORMAT_R8_UNORM}),
       _device.direct_queue};

   _height_map_upload_row_pitch =
      math::align_up(_terrain_length * uint32{sizeof(uint16)},
                     gpu::texture_data_pitch_alignment);

   _weight_map_upload_row_pitch =
      math::align_up(_terrain_length, gpu::texture_data_pitch_alignment);

   const uint32 height_map_upload_item_size =
      math::align_up(_terrain_length * _height_map_upload_row_pitch,
                     gpu::texture_data_placement_alignment);
   const uint32 weight_map_upload_item_size =
      math::align_up(_terrain_length * _weight_map_upload_row_pitch,
                     gpu::texture_data_placement_alignment);

   const uint32 upload_buffer_frame_size =
      height_map_upload_item_size + weight_map_upload_item_size * texture_count;

   _upload_buffer = {_device.create_buffer({.size = upload_buffer_frame_size * 2,
                                            .debug_name =
                                               "Terrain Upload Buffer"},
                                           gpu::heap_type::upload),
                     _device.direct_queue};

   std::byte* const upload_buffer_ptr =
      static_cast<std::byte*>(_device.map(_upload_buffer.get(), 0, {}));

   for (uint32 i = 0; i < gpu::frame_pipeline_length; ++i) {
      _height_map_upload_offset[i] = upload_buffer_frame_size * i;
      _height_map_upload_ptr[i] = upload_buffer_ptr + _height_map_upload_offset[i];

      for (uint32 texture = 0; texture < texture_count; ++texture) {
         _weight_map_upload_offset[i][texture] =
            (upload_buffer_frame_size * i) + height_map_upload_item_size +
            (texture * weight_map_upload_item_size);
         _weight_map_upload_ptr[i][texture] =
            upload_buffer_ptr + _weight_map_upload_offset[i][texture];
      }
   }
}

void terrain::create_unfilled_patch_info()
{
   _patches.clear();
   _patches.reserve(_patch_count * _patch_count);

   for (uint32 patch_y = 0; patch_y < _patch_count; ++patch_y) {
      for (uint32 patch_x = 0; patch_x < _patch_count; ++patch_x) {
         _patches.push_back({.x = patch_x, .y = patch_y});
      }
   }
}

}
