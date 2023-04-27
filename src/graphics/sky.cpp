
#include "sky.hpp"
#include "assets/sky/sky.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "utility/string_icompare.hpp"

namespace we::graphics {

sky::sky(gpu::device& device, model_manager& model_manager,
         assets::libraries_manager& assets)
   : _device{device}, _model_manager{model_manager}, _assets{assets}
{
}

void sky::update(const std::string_view world_name) noexcept
{
   if (_world_name == world_name) return;

   _world_name = world_name;

   {
      std::scoped_lock lock{_sky_asset_mutex};

      _sky_asset = _assets.skies[lowercase_string{_world_name}];
   }

   asset_data<assets::sky::config> config = _sky_asset.get_if();

   if (config) sky_loaded(lowercase_string{_world_name}, _sky_asset, config);
}

void sky::draw(gpu_virtual_address frame_constant_buffer_view,
               gpu::graphics_command_list& command_list,
               root_signature_library& root_signatures, pipeline_library& pipelines,
               dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   std::shared_lock lock{_dome_models_mutex};

   if (_dome_models.empty()) return;

   command_list.set_graphics_root_signature(root_signatures.sky_mesh.get());
   command_list.set_graphics_cbv(rs::sky_mesh::frame_cbv, frame_constant_buffer_view);

   command_list.set_pipeline_state(pipelines.sky_mesh.get());

   for (auto& dome_model : _dome_models) {
      auto& model = _model_manager[dome_model.geometry];

      const std::array vertex_buffers{model.gpu_buffer.position_vertex_buffer_view,
                                      model.gpu_buffer.attributes_vertex_buffer_view};

      command_list.ia_set_index_buffer(model.gpu_buffer.index_buffer_view);
      command_list.ia_set_vertex_buffers(0, vertex_buffers);

      for (auto& mesh : model.parts) {
         (void)dynamic_buffer_allocator;

         command_list.set_graphics_32bit_constant(rs::sky_mesh::sky_mesh_cbv,
                                                  (mesh.material.flags &
                                                   material_pipeline_flags::alpha_cutout) ==
                                                     material_pipeline_flags::alpha_cutout,
                                                  0);
         command_list.set_graphics_cbv(rs::sky_mesh::material_cbv,
                                       mesh.material.constant_buffer_view);

         command_list.draw_indexed_instanced(mesh.index_count, 1, mesh.start_index,
                                             mesh.start_vertex, 0);
      }
   }
}

void sky::sky_loaded([[maybe_unused]] const lowercase_string& name,
                     asset_ref<assets::sky::config> asset,
                     asset_data<assets::sky::config> data) noexcept
{
   std::scoped_lock lock{_sky_asset_mutex, _dome_models_mutex};

   if (asset != _sky_asset) return;

   _dome_models.clear();

   _dome_models.reserve(data->dome_models.size());

   for (auto& dome_model : data->dome_models) {
      const lowercase_string geometry{dome_model.geometry};

      _dome_models.push_back({.geometry = geometry,

                              .movement_scale = dome_model.movement_scale,
                              .rotation_speed = dome_model.rotation.speed,
                              .rotation_direction = dome_model.rotation.direction,

                              .asset = _assets.models[geometry]});
   }
}

}