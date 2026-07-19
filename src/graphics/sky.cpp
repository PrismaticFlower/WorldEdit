
#include "sky.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"

#include "assets/sky/sky.hpp"

#include "utility/string_icompare.hpp"

namespace we::graphics {

namespace {

struct sky_mesh_constants {
   float4x4 rotation;

   float movement_scale = 1.0f;
   float offset = 0.0f;
   uint32 alpha_cutout = 0;
   uint32 padding = 0;
};

static_assert(sizeof(sky_mesh_constants) == 80);

}

sky::sky(gpu::device& device, model_manager& model_manager,
         assets::libraries_manager& assets)
   : _device{device}, _model_manager{model_manager}, _assets{assets}
{
}

void sky::update(const std::string_view world_name) noexcept
{
   if (_world_name != world_name) {
      _world_name = world_name;
      _sky_state = {};

      asset_data<assets::sky::config> config;

      if (std::scoped_lock lock{_load_mutex}; _world_name.empty()) {
         _sky_asset = asset_ref<assets::sky::config>{};
      }
      else {
         _sky_asset = _assets.skies[lowercase_string{_world_name}];
         _sky_data = _sky_asset.get_if();
      }
   }

   {
      std::scoped_lock lock{_load_mutex};

      if (_sky_data) {
         _sky_state = {};

         _sky_state.dome_models.reserve(_sky_data->dome_models.size());

         for (auto& dome_model : _sky_data->dome_models) {
            const lowercase_string geometry{dome_model.geometry};

            _sky_state.dome_models.push_back(
               {.geometry = geometry,

                .movement_scale = dome_model.movement_scale,
                .offset = dome_model.offset,
                .rotation_speed = dome_model.rotation.speed,
                .rotation_direction = dome_model.rotation.direction,

                .asset = _assets.models[geometry]});
         }

         _sky_state.fog_color = float3{_sky_data->fog_color[0] / 255.0f,
                                       _sky_data->fog_color[1] / 255.0f,
                                       _sky_data->fog_color[2] / 255.0f};

         const float fog_range_start = _sky_data->fog_range_start;
         const float fog_range_end = _sky_data->fog_range_end;

         if (fog_range_start < fog_range_end) {
            const float fog_mul = 1.0f / (fog_range_end - fog_range_start);
            const float fog_add = fog_range_end * fog_mul;

            _sky_state.fog_mul_add = {-fog_mul, fog_add};
         }

         const float world_fog_range_start = _sky_data->world_fog_range_start;
         const float world_fog_range_end = _sky_data->world_fog_range_end;

         if (world_fog_range_start > world_fog_range_end) {
            const float fog_mul = 1.0f / (world_fog_range_end - world_fog_range_start);
            const float fog_add = world_fog_range_end * fog_mul;

            _sky_state.world_fog_mul_add = {-fog_mul, fog_add};
         }

         _sky_state.terrain_normal_map = _sky_data->terrain_normal_map;
         _sky_state.terrain_normal_map_scale = _sky_data->terrain_normal_map_scale;

         _sky_data = nullptr;
      }
   }
}

void sky::draw(gpu_virtual_address frame_constant_buffer_view,
               gpu::graphics_command_list& command_list,
               root_signature_library& root_signatures, pipeline_library& pipelines,
               dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   if (_sky_state.dome_models.empty()) return;

   command_list.set_graphics_root_signature(root_signatures.sky_mesh.get());
   command_list.set_graphics_cbv(rs::sky_mesh::frame_cbv, frame_constant_buffer_view);

   command_list.set_pipeline_state(pipelines.sky_mesh.get());

   for (auto& dome_model : _sky_state.dome_models) {
      auto& model = _model_manager[dome_model.geometry];

      if (_model_manager.is_placeholder(model)) continue;

      const std::array vertex_buffers{model.gpu_buffer.position_vertex_buffer_view,
                                      model.gpu_buffer.attributes_vertex_buffer_view};

      command_list.ia_set_index_buffer(model.gpu_buffer.index_buffer_view);
      command_list.ia_set_vertex_buffers(0, vertex_buffers);

      for (auto& mesh : model.parts) {
         sky_mesh_constants constants{.rotation = {},

                                      .movement_scale = dome_model.movement_scale,
                                      .offset = dome_model.offset,
                                      .alpha_cutout =
                                         (mesh.material.depth_prepass_flags &
                                          depth_prepass_pipeline_flags::alpha_cutout) ==
                                         depth_prepass_pipeline_flags::alpha_cutout};

         command_list.set_graphics_cbv(
            rs::sky_mesh::sky_mesh_cbv,
            dynamic_buffer_allocator.allocate_and_copy(constants).gpu_address);
         command_list.set_graphics_cbv(rs::sky_mesh::material_cbv,
                                       mesh.material.constant_buffer_view);

         command_list.draw_indexed_instanced(mesh.index_count, 1, mesh.start_index,
                                             mesh.start_vertex, 0);
      }
   }
}

auto sky::fog_mul_add() const noexcept -> const float2&
{
   return _sky_state.fog_mul_add;
}

auto sky::world_fog_mul_add() const noexcept -> const float2&
{
   return _sky_state.world_fog_mul_add;
}

auto sky::fog_color() const noexcept -> const float3&
{
   return _sky_state.fog_color;
}

auto sky::terrain_normal_map() const noexcept -> std::string_view
{
   return _sky_state.terrain_normal_map;
}

auto sky::terrain_normal_map_scale() const noexcept -> float
{
   return _sky_state.terrain_normal_map_scale;
}

void sky::sky_loaded([[maybe_unused]] const lowercase_string& name,
                     asset_ref<assets::sky::config> asset,
                     asset_data<assets::sky::config> data) noexcept
{
   std::scoped_lock lock{_load_mutex};

   if (asset != _sky_asset) return;

   _sky_data = data;
}

}