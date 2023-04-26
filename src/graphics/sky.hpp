#pragma once

#include "assets/asset_libraries.hpp"
#include "assets/asset_ref.hpp"
#include "camera.hpp"
#include "dynamic_buffer_allocator.hpp"
#include "model_manager.hpp"

#include <shared_mutex>
#include <vector>

namespace we::graphics {

struct sky {
   sky(gpu::device& device, model_manager& model_manager,
       assets::libraries_manager& assets);

   void update(const std::string_view world_name) noexcept;

   void draw(gpu_virtual_address frame_constant_buffer_view,
             gpu::graphics_command_list& command_list,
             root_signature_library& root_signatures, pipeline_library& pipelines,
             dynamic_buffer_allocator& dynamic_buffer_allocator);

private:
   void sky_loaded(const lowercase_string& name, asset_ref<assets::sky::config> asset,
                   asset_data<assets::sky::config> data) noexcept;

   gpu::device& _device;
   model_manager& _model_manager;
   assets::libraries_manager& _assets;

   struct dome_model {
      lowercase_string geometry;

      float movement_scale = 1.0f;
      float rotation_speed = 0.0f;
      float3 rotation_direction = {0.0f, 1.0f, 0.0f};

      assets::asset_ref<assets::msh::flat_model> asset;
   };

   std::string _world_name;

   std::shared_mutex _dome_models_mutex;
   std::vector<dome_model> _dome_models;

   std::shared_mutex _sky_asset_mutex;
   assets::asset_ref<assets::sky::config> _sky_asset;

   event_listener<void(const lowercase_string&, asset_ref<assets::sky::config>,
                       asset_data<assets::sky::config>)>
      _sky_load_listener = _assets.skies.listen_for_loads(
         [this](const lowercase_string& name, asset_ref<assets::sky::config> asset,
                asset_data<assets::sky::config> data) {
            sky_loaded(name, asset, data);
         });
};

}