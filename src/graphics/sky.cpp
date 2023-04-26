
#include "sky.hpp"
#include "assets/sky/sky.hpp"
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

   for (auto& dome_model : _dome_models) {
      auto& model = _model_manager[dome_model.geometry];

      (void)model;
   }

   (void)frame_constant_buffer_view, command_list, root_signatures, pipelines,
      dynamic_buffer_allocator;
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