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

   auto fog_mul_add() const noexcept -> const float2&;

   auto world_fog_mul_add() const noexcept -> const float2&;

   auto fog_color() const noexcept -> const float3&;

   auto terrain_normal_map() const noexcept -> std::string_view;

   auto terrain_normal_map_scale() const noexcept -> float;

private:
   void sky_loaded(const lowercase_string& name, asset_ref<assets::sky::config> asset,
                   asset_data<assets::sky::config> data) noexcept;

   gpu::device& _device;
   model_manager& _model_manager;
   assets::libraries_manager& _assets;

   struct dome_model {
      lowercase_string geometry;

      float movement_scale = 1.0f;
      float offset = 0.0f;
      float rotation_speed = 0.0f;
      float3 rotation_direction = {0.0f, 1.0f, 0.0f};

      assets::asset_ref<assets::msh::flat_model> asset;
   };

   struct sky_state {
      float2 fog_mul_add = {0.0f, 1.0f};
      float2 world_fog_mul_add = {0.0f, 1.0f};

      float3 fog_color = {};

      std::string terrain_normal_map;
      float terrain_normal_map_scale = 1.0f;

      std::vector<dome_model> dome_models;
   };

   std::string _world_name;

   sky_state _sky_state;

   std::shared_mutex _load_mutex;
   assets::asset_ref<assets::sky::config> _sky_asset;
   assets::asset_data<assets::sky::config> _sky_data;

   event_listener<void(const lowercase_string&, asset_ref<assets::sky::config>,
                       asset_data<assets::sky::config>)>
      _sky_load_listener = _assets.skies.listen_for_loads(
         [this](const lowercase_string& name, asset_ref<assets::sky::config> asset,
                asset_data<assets::sky::config> data) {
            sky_loaded(name, asset, data);
         });
};

}