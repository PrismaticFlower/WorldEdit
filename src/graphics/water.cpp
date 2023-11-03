#include "water.hpp"
#include "math/vector_funcs.hpp"
#include "utility/string_icompare.hpp"

#include <bit>

namespace we::graphics {

namespace {

struct alignas(16) water_constants {
   float4 color;
   float height;
   uint32 color_map_index = 0;
};

static_assert(sizeof(water_constants) == 32);

}

water::water(gpu::device& device, texture_manager& texture_manager)
   : _device{device},
     _water_constants_buffer{_device.create_buffer({.size = sizeof(water_constants),
                                                    .debug_name =
                                                       "Water Constants"},
                                                   gpu::heap_type::default_),
                             _device.direct_queue},
     _water_constants_cbv{
        _device.get_gpu_virtual_address(_water_constants_buffer.get())},
     _color_map{texture_manager.null_color_map()}
{
}

void water::update(const world::terrain& terrain, gpu::copy_command_list& command_list,
                   dynamic_buffer_allocator& dynamic_buffer_allocator,
                   texture_manager& texture_manager)
{
   if (terrain.water_map.shape()[0] != _water_map_length) {
      _water_map_length = static_cast<uint32>(terrain.water_map.shape()[0]);
      _water_map_row_words = (_water_map_length + 63u) / 64u;

      _water_map.clear();
      _water_map.resize(_water_map_row_words * _water_map_length);

      _patches.clear();
      _patches.reserve(_water_map_length * _water_map_length);
   }

   for (const world::dirty_rect& dirty : terrain.water_map_dirty) {
      for (uint32 y = dirty.y; y < dirty.height; ++y) {
         for (uint32 x = dirty.x; x < dirty.width; ++x) {
            uint64& word = _water_map[_water_map_row_words * y + (x / 64ull)];
            const uint64 bit = (1ull << (x % 64ull));

            if (terrain.water_map[{x, y}]) {
               word |= bit;
            }
            else {
               word &= ~bit;
            }
         }
      }
   }

   if (not string::iequals(_color_map_name, terrain.water_settings.texture)) {
      _color_map_name = lowercase_string{terrain.water_settings.texture};
      _color_map = texture_manager.at_or(_color_map_name, world_texture_dimension::_2d,
                                         texture_manager.null_color_map());
   }

   _active = terrain.active_flags.water;
   _water_grid_scale = terrain.grid_scale * 4.0f;
   _half_water_map_length = (_water_map_length / 2.0f) * _water_grid_scale;
   _water_height = terrain.water_settings.height;

   water_constants constants{.color = terrain.water_settings.color,
                             .height = terrain.water_settings.height,
                             .color_map_index = _color_map->srv_srgb.index};

   command_list
      .copy_buffer_region(_water_constants_buffer.get(), 0,
                          dynamic_buffer_allocator.resource(),
                          dynamic_buffer_allocator.allocate_and_copy(constants).offset,
                          sizeof(constants));
}

void water::draw(const frustum& view_frustum,
                 gpu_virtual_address camera_constant_buffer_view,
                 gpu::graphics_command_list& command_list,
                 dynamic_buffer_allocator& dynamic_buffer_allocator,
                 root_signature_library& root_signatures, pipeline_library& pipelines)
{
   if (not _active) return;

   _patches.clear();

   for (uint32 y = 0; y < _water_map_length; ++y) {
      for (uint32 x = 0; x < (_water_map_row_words * 64u); x += 64u) {
         uint64 word = _water_map[_water_map_row_words * y + (x / 64ull)];
         uint64 current_bit = 0;

         while (word) {
            const uint64 next_bit = std::countr_zero(word);

            word >>= (next_bit + 1ull);

            const float2 point = {(static_cast<float>(x + current_bit + next_bit) *
                                   _water_grid_scale) -
                                     _half_water_map_length,
                                  (static_cast<float>(y) * _water_grid_scale) -
                                     _half_water_map_length};

            const patch patch{point, point + _water_grid_scale};

            if (intersects(view_frustum,
                           math::bounding_box{.min = {patch.min.x, _water_height,
                                                      patch.min.y},
                                              .max = {patch.max.x, _water_height,
                                                      patch.max.y}})) {
               _patches.emplace_back(patch);
            }

            current_bit += (next_bit + 1ull);
         }
      }
   }

   if (_patches.empty()) return;

   auto allocation =
      dynamic_buffer_allocator.allocate(_patches.size() * sizeof(patch));

   std::memcpy(allocation.cpu_address, _patches.data(),
               _patches.size() * sizeof(patch));

   command_list.set_pipeline_state(pipelines.water.get());

   command_list.set_graphics_root_signature(root_signatures.water.get());
   command_list.set_graphics_cbv(rs::water::frame_cbv, camera_constant_buffer_view);
   command_list.set_graphics_cbv(rs::water::water_cbv, _water_constants_cbv);
   command_list.set_graphics_srv(rs::water::water_patches_srv, allocation.gpu_address);

   command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

   command_list.draw_instanced(static_cast<uint32>(_patches.size() * 6), 1, 0, 0);
}

void water::process_updated_texture(const updated_textures& updated)
{
   for (auto& [name, texture] : updated) {
      if (name == _color_map_name) _color_map = texture;
   }
}

}