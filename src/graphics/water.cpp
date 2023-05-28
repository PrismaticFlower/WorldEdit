#include "water.hpp"
#include "math/vector_funcs.hpp"

namespace we::graphics {

namespace {

struct patch {
   float2 min;
   float2 max;
};

constexpr uint32 default_patch_count = (256 * 256);

struct alignas(16) water_constants {
   float4 color;
   float height;
   uint32 color_map_index = 0;
};

static_assert(sizeof(water_constants) == 32);

}

water::water(gpu::device& device, texture_manager& texture_manager)
   : _device{device},
     _texture_manager{texture_manager},
     _water_constants_buffer{_device.create_buffer({.size = sizeof(water_constants),
                                                    .debug_name =
                                                       "Water Constants"},
                                                   gpu::heap_type::default_),
                             _device.direct_queue},
     _patch_buffer{_device.create_buffer({.size = default_patch_count * sizeof(patch),
                                          .debug_name = "Water Patches"},
                                         gpu::heap_type::default_),
                   _device.direct_queue},
     _water_constants_cbv{
        _device.get_gpu_virtual_address(_water_constants_buffer.get())}
{
}

void water::init(const world::terrain& terrain, gpu::copy_command_list& command_list,
                 dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   const float water_grid_scale = terrain.grid_scale * 4.0f;
   const float half_water_map_length =
      (static_cast<float>(terrain.water_map.sshape()[0]) / 2.0f) * water_grid_scale;

   std::vector<patch> patches;

   for (std::ptrdiff_t y = 0; y < terrain.water_map.sshape()[0]; ++y) {
      for (std::ptrdiff_t x = 0; x < terrain.water_map.sshape()[0]; ++x) {
         if (not terrain.water_map[{x, y}]) continue;

         const float2 point = {(static_cast<float>(x) * water_grid_scale) -
                                  half_water_map_length,
                               (static_cast<float>(y) * water_grid_scale) -
                                  half_water_map_length};

         patches.emplace_back(point, point + water_grid_scale);
      }
   }

   _patch_count = static_cast<uint32>(patches.size());
   _active = _patch_count != 0 and terrain.active_flags.water;

   if (_patch_count > default_patch_count) {
      _patch_buffer = {_device.create_buffer({.size = _patch_count * sizeof(patch),
                                              .debug_name = "Water Patches"},
                                             gpu::heap_type::default_),
                       _device.direct_queue};
   }

   {
      auto copy_allocation =
         dynamic_buffer_allocator.allocate(_patch_count * sizeof(patch));

      std::memcpy(copy_allocation.cpu_address, patches.data(),
                  _patch_count * sizeof(patch));

      command_list.copy_buffer_region(_patch_buffer.get(), 0,
                                      dynamic_buffer_allocator.resource(),
                                      copy_allocation.offset,
                                      _patch_count * sizeof(patch));
   }

   _color_map_name = lowercase_string{terrain.water_settings.texture};
   _color_map = _texture_manager.at_or(_color_map_name, world_texture_dimension::_2d,
                                       _texture_manager.null_color_map());

   {
      auto copy_allocation = dynamic_buffer_allocator.allocate_and_copy(
         water_constants{.color = terrain.water_settings.color,
                         .height = terrain.water_settings.height,
                         .color_map_index = _color_map->srv_srgb.index});

      command_list.copy_buffer_region(_water_constants_buffer.get(), 0,
                                      dynamic_buffer_allocator.resource(),
                                      copy_allocation.offset,
                                      sizeof(water_constants));
   }

   _patch_buffer_srv = _device.get_gpu_virtual_address(_patch_buffer.get());
}

void water::draw(gpu_virtual_address camera_constant_buffer_view,
                 gpu::graphics_command_list& command_list,
                 root_signature_library& root_signatures, pipeline_library& pipelines)
{
   if (not _active) return;

   command_list.set_pipeline_state(pipelines.water.get());

   command_list.set_graphics_root_signature(root_signatures.water.get());
   command_list.set_graphics_cbv(rs::water::frame_cbv, camera_constant_buffer_view);
   command_list.set_graphics_cbv(rs::water::water_cbv, _water_constants_cbv);
   command_list.set_graphics_srv(rs::water::water_patches_srv, _patch_buffer_srv);

   command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

   command_list.draw_instanced(_patch_count * 6, 1, 0, 0);
}

void water::process_updated_texture(gpu::copy_command_list& command_list,
                                    const updated_textures& updated)
{
   for (auto& [name, texture] : updated) {
      if (name == _color_map_name) {
         _color_map = texture;

         command_list.write_buffer_immediate(_water_constants_cbv +
                                                offsetof(water_constants, color_map_index),
                                             _color_map->srv_srgb.index);
      }
   }
}

}