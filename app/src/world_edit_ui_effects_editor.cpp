#include "world_edit.hpp"

#include "edits/imgui_ext.hpp"
#include "edits/imgui_ext_platform_var.hpp"

#include <imgui.h>

namespace we {

namespace {

const world::precipitation rain_default_settings = {
   .enable = {true},
   .type = {world::precipitation_type::streaks},
   .range = {12.5f},
   .color = {{216.0f / 255.0f, 220.0f / 255.0f, 228.0f / 255.0f}},

   .velocity = {9.0f},
   .velocity_range = {1.0f},

   .particle_density = {256.0f},
   .particle_density_range = {0.0f},
   .particle_size = {0.02f},

   .streak_length = {1.0f},

   .camera_cross_velocity_scale = {0.2f},
   .camera_axial_velocity_scale = {1.0f},

   .ground_effect = {"com_sfx_rainsplash"},
   .ground_effects_per_sec = {15},
   .ground_effect_spread = {8},

   .alpha_min_max = {{0.3f, 0.45f}},
};

const world::precipitation snow_default_settings = {
   .enable = {true},
   .type = {world::precipitation_type::quads},
   .texture = {"fx_snow"},
   .range = {15.0f},
   .color = {{1.0f, 1.0f, 1.0f}},

   .velocity = {2.0f},
   .velocity_range = {0.8f},

   .particle_density = {100.0f},
   .particle_density_range = {0.0f},
   .particle_size = {0.015f},

   .camera_cross_velocity_scale = {1.0f},
   .camera_axial_velocity_scale = {1.0f},

   .ground_effects_per_sec = {0},
   .ground_effect_spread = {0},

   .alpha_min_max = {{0.3f, 0.45f}},
   .rotation_range = 2.0f,
};

bool is_enabled(const world::platform_var<bool>& enable) noexcept
{
   return enable.pc or (enable.per_platform and (enable.ps2 or enable.xbox));
}

}

void world_edit::ui_show_effects_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});

   ImGui::Begin("World Effects Editor", &_effects_editor_open);

   if (not _settings.ui.hide_extra_effects_properties and
       ImGui::TreeNodeEx("Color Control", ImGuiTreeNodeFlags_Framed)) {
      world::color_control& color_control = _world.effects.color_control;

      ImGui::Checkbox("Enabled", &color_control.enable, _edit_stack_world, _edit_context);

      ImGui::BeginDisabled(not is_enabled(color_control.enable));

      ImGui::Separator();

      ImGui::DragFloat("Gamma Brightness", &color_control.gamma_brightness,
                       _edit_stack_world, _edit_context, 0.01f, 0.0f, 1.0f);

      ImGui::DragFloat("Gamma Color Balance", &color_control.gamma_brightness,
                       _edit_stack_world, _edit_context, 0.01f, 0.0f, 1.0f);

      ImGui::DragFloat("Gamma Contrast", &color_control.gamma_contrast,
                       _edit_stack_world, _edit_context, 0.01f, 0.0f, 1.0f);

      ImGui::DragFloat("Gamma Correction", &color_control.gamma_correction,
                       _edit_stack_world, _edit_context, 0.01f, 0.0f, 1.0f);

      ImGui::DragFloat("Gamma Hue", &color_control.gamma_hue, _edit_stack_world,
                       _edit_context, 0.01f, 0.0f, 1.0f);

      ImGui::Separator();

      ImGui::DragFloat("World Brightness", &color_control.world_brightness,
                       _edit_stack_world, _edit_context, 0.01f, 0.0f, 1.0f);

      ImGui::DragFloat("World Contrast", &color_control.world_contrast,
                       _edit_stack_world, _edit_context, 0.01f, 0.0f, 1.0f);

      ImGui::DragFloat("World Saturation", &color_control.world_saturation,
                       _edit_stack_world, _edit_context, 0.01f, 0.0f, 1.0f);

      ImGui::EndDisabled();

      ImGui::TreePop();
   }

   if (ImGui::TreeNodeEx("Fog Cloud", ImGuiTreeNodeFlags_Framed)) {
      world::fog_cloud& fog_cloud = _world.effects.fog_cloud;

      ImGui::Checkbox("Enabled", &fog_cloud.enable, _edit_stack_world, _edit_context);

      ImGui::BeginDisabled(not is_enabled(fog_cloud.enable));

      ImGui::Separator();

      ImGui::CustomWidget("Texture", &fog_cloud.texture, _edit_stack_world, _edit_context,
                          [&](const char* label, std::string* texture) noexcept {
                             IM_ASSERT(texture);
                             IM_ASSERT(_edit_context.is_memory_valid(texture));

                             return ui_texture_pick_widget(label, texture);
                          });

      ImGui::DragFloat2("Range", &fog_cloud.range, _edit_stack_world, _edit_context);

      ImGui::ColorEdit4("Color", &fog_cloud.color, _edit_stack_world, _edit_context);

      ImGui::DragFloat2("Velocity", &fog_cloud.velocity, _edit_stack_world,
                        _edit_context);

      ImGui::DragFloat("Rotation", &fog_cloud.rotation, _edit_stack_world,
                       _edit_context, 0.1f);

      ImGui::DragFloat("Height", &fog_cloud.height, _edit_stack_world, _edit_context);

      ImGui::DragFloat("Particle Size", &fog_cloud.particle_size,
                       _edit_stack_world, _edit_context, 1.0f, 0.0f, 1e10f);

      ImGui::DragFloat("Particle Density", &fog_cloud.particle_density,
                       _edit_stack_world, _edit_context, 1.0f, 0.0f, 1e10f);

      ImGui::EndDisabled();

      ImGui::TreePop();
   }

   if (ImGui::TreeNodeEx("Wind", ImGuiTreeNodeFlags_Framed)) {
      world::wind& wind = _world.effects.wind;

      ImGui::Checkbox("Enabled", &wind.enable, _edit_stack_world, _edit_context);

      ImGui::BeginDisabled(not is_enabled(wind.enable));

      ImGui::Separator();

      ImGui::DragFloat2("Velocity", &wind.velocity, _edit_stack_world,
                        _edit_context, 0.1f);

      ImGui::DragFloat("Velocity Range", &wind.velocity_range,
                       _edit_stack_world, _edit_context, 0.1f, 0.0f, 1e10f);

      ImGui::DragFloat("Velocity Change Rate", &wind.velocity_change_rate,
                       _edit_stack_world, _edit_context, 0.1f, 0.0f, 1e10f);

      ImGui::EndDisabled();

      ImGui::TreePop();
   }

   if (ImGui::TreeNodeEx("Precipitation", ImGuiTreeNodeFlags_Framed)) {
      world::precipitation& precipitation = _world.effects.precipitation;

      ImGui::Checkbox("Enabled", &precipitation.enable, _edit_stack_world, _edit_context);

      ImGui::BeginDisabled(not is_enabled(precipitation.enable));

      ImGui::Separator();

      ImGui::EditPrecipitationType("Type", &precipitation.type,
                                   _edit_stack_world, _edit_context);

      ImGui::CustomWidget("Texture", &precipitation.texture, _edit_stack_world,
                          _edit_context,
                          [&](const char* label, std::string* texture) noexcept {
                             IM_ASSERT(texture);
                             IM_ASSERT(_edit_context.is_memory_valid(texture));

                             return ui_texture_pick_widget(label, texture);
                          });

      ImGui::Separator();

      ImGui::DragFloat("Range", &precipitation.range, _edit_stack_world,
                       _edit_context, 0.5f);

      ImGui::ColorEdit3("Color", &precipitation.color, _edit_stack_world, _edit_context);

      ImGui::Separator();

      ImGui::DragFloat("Velocity", &precipitation.velocity, _edit_stack_world,
                       _edit_context, 0.25f);

      ImGui::DragFloat("Velocity Range", &precipitation.velocity_range,
                       _edit_stack_world, _edit_context, 0.1f, 0.0f, 1e10f);

      ImGui::Separator();

      ImGui::DragFloat("Particle Density", &precipitation.particle_density,
                       _edit_stack_world, _edit_context);

      ImGui::DragFloat("Particle Density Range", &precipitation.particle_density_range,
                       _edit_stack_world, _edit_context, 0.1f, 0.0f, 1e10f);

      ImGui::DragFloat("Particle Size", &precipitation.particle_size,
                       _edit_stack_world, _edit_context, 0.01f, 0.0f, 1e10f);

      ImGui::Separator();

      ImGui::DragFloat("Streak Length", &precipitation.streak_length,
                       _edit_stack_world, _edit_context, 0.1f, 0.0f, 1e10f);

      ImGui::DragFloat("Camera Cross Velocity Scale",
                       &precipitation.camera_cross_velocity_scale,
                       _edit_stack_world, _edit_context, 0.1f, 0.0f, 1e10f);

      ImGui::DragFloat("Camera Axial Velocity Scale",
                       &precipitation.camera_axial_velocity_scale,
                       _edit_stack_world, _edit_context, 0.1f, 0.0f, 1e10f);

      ImGui::Separator();

      ImGui::InputText("Ground Effect", &precipitation.ground_effect,
                       _edit_stack_world, _edit_context);

      ImGui::DragInt("Ground Effects Per Second", &precipitation.ground_effects_per_sec,
                     _edit_stack_world, _edit_context, 1.0f, 1, 100000);

      ImGui::DragInt("Ground Effects Spread", &precipitation.ground_effect_spread,
                     _edit_stack_world, _edit_context, 1.0f, 1, 100000);

      ImGui::Separator();

      ImGui::DragFloat2("Alpha Min-Max", &precipitation.alpha_min_max,
                        _edit_stack_world, _edit_context, 0.05f, 0.0f, 1.0f);

      ImGui::DragFloat("Rotation Range", &precipitation.rotation_range,
                       _edit_stack_world, _edit_context);

      ImGui::EndDisabled();

      ImGui::Separator();

      if (ImGui::Button("Load Rain Defaults", {ImGui::CalcItemWidth(), 0.0f})) {
         _edit_stack_world.apply(edits::make_set_value(&_world.effects.precipitation,
                                                       rain_default_settings),
                                 _edit_context, {.closed = true});
      }

      if (ImGui::Button("Load Snow Defaults", {ImGui::CalcItemWidth(), 0.0f})) {
         _edit_stack_world.apply(edits::make_set_value(&_world.effects.precipitation,
                                                       snow_default_settings),
                                 _edit_context, {.closed = true});
      }

      ImGui::TreePop();
   }

   if (ImGui::TreeNodeEx("Lightning", ImGuiTreeNodeFlags_Framed)) {
      world::lightning& lightning = _world.effects.lightning;

      ImGui::Checkbox("Enabled", &lightning.enable, _edit_stack_world, _edit_context);

      ImGui::BeginDisabled(not is_enabled(lightning.enable));

      ImGui::Separator();

      ImGui::ColorEdit3("Color", &lightning.color, _edit_stack_world, _edit_context);

      ImGui::DragFloat("Sunlight Fade Factor", &lightning.sunlight_fade_factor,
                       _edit_stack_world, _edit_context, 0.05f, 0.0f, 1.0f);

      ImGui::DragFloat("Sky Dome Darken Factor", &lightning.sky_dome_darken_factor,
                       _edit_stack_world, _edit_context, 0.05f, 0.0f, 1.0f);

      ImGui::DragFloat("Brightness Min", &lightning.brightness_min,
                       _edit_stack_world, _edit_context, 0.05f, 0.0f, 1e10f);

      ImGui::DragFloat("Fade Time", &lightning.fade_time, _edit_stack_world,
                       _edit_context, 0.1f);

      ImGui::Separator();

      ImGui::DragFloat2("Time Between Flashes Min-Max",
                        &lightning.time_between_flashes_min_max,
                        _edit_stack_world, _edit_context, 0.1f, 0.0f, 1e10f);

      ImGui::DragFloat2("Time Between Sub-Flashes Min-Max",
                        &lightning.time_between_sub_flashes_min_max,
                        _edit_stack_world, _edit_context, 0.1f, 0.0f, 1e10f);

      ImGui::DragInt2("Sub-Flashes Min-Max Count", &lightning.num_sub_flashes_min_max,
                      _edit_stack_world, _edit_context, 1.0f, 0, 100000);

      ImGui::Separator();

      ImGui::DragInt2("Horizon Angle Min-Max", &lightning.horizon_angle_min_max,
                      _edit_stack_world, _edit_context, 1.0f);

      ImGui::Separator();

      ImGui::InputText("Sound Crack", &lightning.sound_crack, _edit_stack_world,
                       _edit_context);

      ImGui::InputText("Sound Sub-Crack", &lightning.sound_sub_crack,
                       _edit_stack_world, _edit_context);

      ImGui::SeparatorText("Lightning Bolt");

      world::lightning_bolt& bolt = _world.effects.lightning_bolt;

      ImGui::PushID("Bolt");

      ImGui::CustomWidget("Texture", &bolt.texture, _edit_stack_world, _edit_context,
                          [&](const char* label, std::string* texture) noexcept {
                             IM_ASSERT(texture);
                             IM_ASSERT(_edit_context.is_memory_valid(texture));

                             return ui_texture_pick_widget(label, texture);
                          });

      ImGui::DragFloat("Width", &bolt.width, _edit_stack_world, _edit_context);

      ImGui::DragFloat("Fade Time", &bolt.fade_time, _edit_stack_world,
                       _edit_context, 0.1f, 0.0f, 1e10f);

      ImGui::DragFloat("Break Distance", &bolt.break_distance,
                       _edit_stack_world, _edit_context);

      ImGui::DragFloat("Texture Size", &bolt.texture_size, _edit_stack_world,
                       _edit_context);

      ImGui::DragFloat("Spread Factor", &bolt.spread_factor, _edit_stack_world,
                       _edit_context);

      ImGui::Separator();

      ImGui::DragFloat("Max Branches", &bolt.max_branches, _edit_stack_world,
                       _edit_context, 1.0f, 0.0f, 1e10f);

      ImGui::DragFloat("Branch Factor", &bolt.branch_factor, _edit_stack_world,
                       _edit_context, 0.1f);

      ImGui::DragInt("Branch Spread Factor", &bolt.branch_spread_factor,
                     _edit_stack_world, _edit_context);

      ImGui::DragFloat("Branch Length", &bolt.branch_length, _edit_stack_world,
                       _edit_context, 1.0f, 0.0f, 1e10f);

      ImGui::Separator();

      ImGui::DragFloat("Interpolation Speed", &bolt.interpolation_speed,
                       _edit_stack_world, _edit_context, 0.05f);

      ImGui::Separator();

      ImGui::DragInt("Children Count", &bolt.num_children, _edit_stack_world,
                     _edit_context, 0.05f);

      ImGui::DragFloat("Child Break Distance", &bolt.child_break_distance,
                       _edit_stack_world, _edit_context);

      ImGui::DragFloat("Child Texture Size", &bolt.child_texture_size,
                       _edit_stack_world, _edit_context);

      ImGui::DragFloat("Child Width", &bolt.child_width, _edit_stack_world,
                       _edit_context);

      ImGui::DragFloat("Child Spread Factor", &bolt.child_spread_factor,
                       _edit_stack_world, _edit_context);

      ImGui::Separator();

      ImGui::ColorEdit4("Color", &bolt.color, _edit_stack_world, _edit_context);

      ImGui::ColorEdit4("Child Color", &bolt.color, _edit_stack_world, _edit_context);

      ImGui::PopID();

      ImGui::EndDisabled();

      ImGui::TreePop();
   }

   if (ImGui::TreeNodeEx("Godray", ImGuiTreeNodeFlags_Framed)) {
      world::godray& godray = _world.effects.godray;

      ImGui::Checkbox("Enabled", &godray.enable, _edit_stack_world, _edit_context);

      ImGui::BeginDisabled(not is_enabled(godray.enable));

      ImGui::Separator();

      ImGui::DragInt("Max Godrays in World", &godray.max_godrays_in_world,
                     _edit_stack_world, _edit_context, 1.0f, 1, 100000);

      ImGui::DragInt("Max Godrays on Screen", &godray.max_godrays_on_screen,
                     _edit_stack_world, _edit_context, 1.0f, 1, 100000);

      ImGui::Separator();

      ImGui::DragFloat("Max View Distance", &godray.max_view_distance,
                       _edit_stack_world, _edit_context, 1.0f, 1.0f, 1e10f);

      ImGui::DragFloat("Fade View Distance", &godray.fade_view_distance,
                       _edit_stack_world, _edit_context, 1.0f, 1.0f, 1e10f);

      ImGui::Separator();

      ImGui::DragFloat("Max Length", &godray.max_length, _edit_stack_world,
                       _edit_context, 1.0f, 1.0f, 1e10f);

      ImGui::DragFloat("Offset Angle", &godray.offset_angle, _edit_stack_world,
                       _edit_context, 1.0f);

      ImGui::Separator();

      ImGui::DragInt("Min Rays Per Godray", &godray.min_rays_per_godray,
                     _edit_stack_world, _edit_context, 1.0f, 1, 100000);

      ImGui::DragInt("Max Rays Per Godray", &godray.max_rays_per_godray,
                     _edit_stack_world, _edit_context, 1.0f, 1, 100000);

      ImGui::DragFloat("Radius for Max Rays", &godray.radius_for_max_rays,
                       _edit_stack_world, _edit_context);

      ImGui::DragFloat3("Dust Velocity", &godray.dust_velocity,
                        _edit_stack_world, _edit_context, 0.1f);

      ImGui::Separator();

      ImGui::CustomWidget("Texture", &godray.texture, _edit_stack_world, _edit_context,
                          [&](const char* label, std::string* texture) noexcept {
                             IM_ASSERT(texture);
                             IM_ASSERT(_edit_context.is_memory_valid(texture));

                             return ui_texture_pick_widget(label, texture);
                          });

      ImGui::DragFloat2("Texture Scale", &godray.texture_scale,
                        _edit_stack_world, _edit_context, 0.1f, 0.0f, 1e10f);

      ImGui::DragFloat3("Texture Velocity", &godray.texture_velocity,
                        _edit_stack_world, _edit_context, 0.1f);

      ImGui::DragFloat("Texture Jitter Speed", &godray.texture_jitter_speed,
                       _edit_stack_world, _edit_context, 0.01f);

      ImGui::EndDisabled();

      ImGui::TreePop();
   }

   if (ImGui::TreeNodeEx("Heat Shimmer", ImGuiTreeNodeFlags_Framed)) {
      world::heat_shimmer& heat_shimmer = _world.effects.heat_shimmer;

      ImGui::Checkbox("Enabled", &heat_shimmer.enable, _edit_stack_world, _edit_context);

      ImGui::BeginDisabled(not is_enabled(heat_shimmer.enable));

      ImGui::Separator();

      ImGui::DragFloat("World Height", &heat_shimmer.world_height,
                       _edit_stack_world, _edit_context);

      ImGui::DragFloat("Geometry Height", &heat_shimmer.geometry_height,
                       _edit_stack_world, _edit_context);

      ImGui::DragFloat("Scroll Speed", &heat_shimmer.scroll_speed,
                       _edit_stack_world, _edit_context, 0.01f);

      ImGui::EditBumpMap(&heat_shimmer.bump_map, _edit_stack_world, _edit_context,
                         [&](const char* label, std::string* texture) noexcept {
                            IM_ASSERT(texture);
                            IM_ASSERT(_edit_context.is_memory_valid(texture));

                            return ui_texture_pick_widget(label, texture);
                         });

      ImGui::DragFloat("Distortion Scale", &heat_shimmer.distortion_scale,
                       _edit_stack_world, _edit_context, 0.01f);

      ImGui::DragInt2("Tessellation", &heat_shimmer.tessellation,
                      _edit_stack_world, _edit_context, 1.0f, 1, 1000);

      ImGui::EndDisabled();

      ImGui::TreePop();
   }

   if (ImGui::TreeNodeEx("Space Dust", ImGuiTreeNodeFlags_Framed)) {
      world::space_dust& space_dust = _world.effects.space_dust;

      ImGui::Checkbox("Enabled", &space_dust.enable, _edit_stack_world, _edit_context);

      ImGui::BeginDisabled(not is_enabled(space_dust.enable));

      ImGui::CustomWidget("Texture", &space_dust.texture, _edit_stack_world,
                          _edit_context,
                          [&](const char* label, std::string* texture) noexcept {
                             IM_ASSERT(texture);
                             IM_ASSERT(_edit_context.is_memory_valid(texture));

                             return ui_texture_pick_widget(label, texture);
                          });

      ImGui::DragFloat("Spawn Distance", &space_dust.spawn_distance,
                       _edit_stack_world, _edit_context);

      ImGui::DragFloat("Max Random Side Offset", &space_dust.max_random_side_offset,
                       _edit_stack_world, _edit_context);

      ImGui::DragFloat("Center Dead Zone Radius", &space_dust.center_dead_zone_radius,
                       _edit_stack_world, _edit_context);

      ImGui::Separator();

      ImGui::DragFloat("Min Particle Scale", &space_dust.min_particle_scale,
                       _edit_stack_world, _edit_context, 0.1f, 0.0f, 1e10f);

      ImGui::DragFloat("Max Particle Scale", &space_dust.max_particle_scale,
                       _edit_stack_world, _edit_context, 0.1f, 0.0f, 1e10f);

      ImGui::Separator();

      ImGui::DragFloat("Spawn Delay", &space_dust.spawn_delay,
                       _edit_stack_world, _edit_context, 0.01f, 0.0f, 1e10f);

      ImGui::DragFloat("Reference Speed", &space_dust.reference_speed,
                       _edit_stack_world, _edit_context, 0.01f);

      ImGui::DragFloat("Dust particle Speed", &space_dust.dust_particle_speed,
                       _edit_stack_world, _edit_context);

      ImGui::Separator();

      ImGui::DragFloat("Speed Particle Min Length", &space_dust.speed_particle_min_length,
                       _edit_stack_world, _edit_context, 1.0f, 0.0f, 1e10f);

      ImGui::DragFloat("Speed Particle Max Length", &space_dust.speed_particle_max_length,
                       _edit_stack_world, _edit_context, 1.0f, 0.0f, 1e10f);

      ImGui::DragFloat("Particle Length Min Speed", &space_dust.particle_length_min_speed,
                       _edit_stack_world, _edit_context, 1.0f, 0.0f, 1e10f);

      ImGui::DragFloat("Particle Length Max Speed", &space_dust.particle_length_max_speed,
                       _edit_stack_world, _edit_context, 1.0f, 0.0f, 1e10f);

      ImGui::EndDisabled();

      ImGui::TreePop();
   }

   if (ImGui::TreeNodeEx("World Shadow Map", ImGuiTreeNodeFlags_Framed)) {
      world::world_shadow_map& world_shadow_map = _world.effects.world_shadow_map;

      ImGui::Checkbox("Enabled", &world_shadow_map.enable, _edit_stack_world,
                      _edit_context);

      ImGui::BeginDisabled(not is_enabled(world_shadow_map.enable));

      ImGui::Separator();

      ImGui::CustomWidget("Texture", &world_shadow_map.texture,
                          _edit_stack_world, _edit_context,
                          [&](const char* label, std::string* texture) noexcept {
                             IM_ASSERT(texture);
                             IM_ASSERT(_edit_context.is_memory_valid(texture));

                             return ui_texture_pick_widget(label, texture);
                          });

      ImGui::DragFloat("Texture Scale", &world_shadow_map.texture_scale,
                       _edit_stack_world, _edit_context);

      ImGui::Separator();

      ImGui::CustomWidget(
         "Light Name", &world_shadow_map.light_name, _edit_stack_world,
         _edit_context, [&](const char* label, std::string* light_name) noexcept {
            IM_ASSERT(light_name);
            IM_ASSERT(_edit_context.is_memory_valid(light_name));

            bool edited = false;

            if (ImGui::BeginCombo(label, light_name->c_str())) {
               for (const world::light& light : _world.lights) {
                  if (light.light_type == world::light_type::directional) {
                     if (ImGui::Selectable(light.name.c_str(),
                                           string::iequals(*light_name, light.name))) {
                        _edit_stack_world
                           .apply(edits::make_set_memory_value(light_name, light.name),
                                  _edit_context, {.closed = true});

                        edited = true;
                     }
                  }
               }

               if (ImGui::Selectable("<clear>")) {
                  _edit_stack_world.apply(edits::make_set_memory_value(light_name,
                                                                       std::string{""}),
                                          _edit_context, {.closed = true});

                  edited = true;
               }

               ImGui::EndCombo();
            }

            if (ImGui::IsItemHovered() and
                ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_MouseLeft)) {
               const world::light* light =
                  world::find_entity(_world.lights, *light_name);

               if (light) {
                  _interaction_targets.selection.add(light->id);

                  ImGui::SetWindowFocus("Selection");
               }
            }

            return edited;
         });

      ImGui::Separator();

      ImGui::DragFloat("Animation Frequency", &world_shadow_map.animation_frequency,
                       _edit_stack_world, _edit_context, 0.01f);

      ImGui::DragFloat2("Animation Amplitude 0", &world_shadow_map.animation_amplitude0,
                        _edit_stack_world, _edit_context, 0.05f);

      ImGui::DragFloat2("Animation Amplitude 1", &world_shadow_map.animation_amplitude1,
                        _edit_stack_world, _edit_context, 0.05f);

      ImGui::EndDisabled();
      ImGui::TreePop();
   }

   if (ImGui::TreeNodeEx("Blur", ImGuiTreeNodeFlags_Framed)) {
      world::blur& blur = _world.effects.blur;

      ImGui::Checkbox("Enabled", &blur.enable, _edit_stack_world, _edit_context);

      ImGui::BeginDisabled(not is_enabled(blur.enable));

      ImGui::DragFloat("Constant Blend", &blur.constant_blend,
                       _edit_stack_world, _edit_context, 0.125f, 0.125f, 1.0f);

      ImGui::DragFloat("Down Size Factor", &blur.down_size_factor,
                       _edit_stack_world, _edit_context, 0.125f, 0.125f, 1.0f);

      ImGui::Separator();

      ImGui::DragFloat2("Min-Max Depth (PS2)", &blur.min_max_depth_ps2,
                        _edit_stack_world, _edit_context, 0.0625f, 0.0f, 1.0f);

      if (not _settings.ui.hide_extra_effects_properties) {
         ImGui::SliderInt("Mode", &blur.mode, _edit_stack_world, _edit_context, 0, 1);

         ImGui::SetItemTooltip("Unknown property, may not do anything.");
      }

      ImGui::EndDisabled();
      ImGui::TreePop();
   }

   if (ImGui::TreeNodeEx("Motion Blur", ImGuiTreeNodeFlags_Framed)) {
      world::motion_blur& motion_blur = _world.effects.motion_blur;

      ImGui::Checkbox("Enabled", &motion_blur.enable, _edit_stack_world, _edit_context);

      ImGui::TreePop();
   }

   if (ImGui::TreeNodeEx("Scope Blur", ImGuiTreeNodeFlags_Framed)) {
      world::scope_blur& scope_blur = _world.effects.scope_blur;

      ImGui::Checkbox("Enabled", &scope_blur.enable, _edit_stack_world, _edit_context);

      ImGui::TreePop();
   }

   if (ImGui::TreeNodeEx("HDR", ImGuiTreeNodeFlags_Framed)) {
      world::hdr& hdr = _world.effects.hdr;

      ImGui::Checkbox("Enabled", &hdr.enable, _edit_stack_world, _edit_context);

      ImGui::BeginDisabled(not is_enabled(hdr.enable));

      ImGui::DragFloat("Down Size Factor", &hdr.down_size_factor,
                       _edit_stack_world, _edit_context, 0.125f, 0.0f, 1.0f);

      ImGui::DragInt("Bloom Pass Count", &hdr.num_bloom_passes,
                     _edit_stack_world, _edit_context, 1.0f, 1, 100);

      ImGui::DragFloat("Max Total Weight", &hdr.max_total_weight,
                       _edit_stack_world, _edit_context, 0.1f, 0.0f, 1e10f);

      ImGui::DragFloat("Glow Threshold", &hdr.glow_threshold, _edit_stack_world,
                       _edit_context, 0.1f, 0.0f, 1.0f);

      ImGui::DragFloat("Glow Factor", &hdr.glow_factor, _edit_stack_world,
                       _edit_context, 0.1f, 0.0f, 1.0f);

      ImGui::EndDisabled();
      ImGui::TreePop();
   }

   if (ImGui::TreeNodeEx("Shadow", ImGuiTreeNodeFlags_Framed)) {
      world::shadow& shadow = _world.effects.shadow;

      ImGui::Checkbox("Enabled", &shadow.enable, _edit_stack_world, _edit_context);

      ImGui::BeginDisabled(not is_enabled(shadow.enable));

      if (not _settings.ui.hide_extra_effects_properties) {
         ImGui::Checkbox("Blur Enabled", &shadow.blur_enable, _edit_stack_world,
                         _edit_context);

         ImGui::SetItemTooltip("Unused on PC, maybe unused on PS2/Xbox.");
      }

      ImGui::DragFloat("Intensity", &shadow.intensity, _edit_stack_world,
                       _edit_context, 0.1f, 0.0f, 1.0f);

      ImGui::EndDisabled();
      ImGui::TreePop();
   }

   if (ImGui::TreeNodeEx("Sun Flares", ImGuiTreeNodeFlags_Framed)) {
      for (world::sun_flare& sun_flare : _world.effects.sun_flares) {
         if (ImGui::TreeNode(&sun_flare, "Sun Flare - (%.3f %.3f)",
                             sun_flare.angle.pc.x, sun_flare.angle.pc.y)) {

            ImGui::DragFloat2("Angle", &sun_flare.angle, _edit_stack_world,
                              _edit_context);

            ImGui::ColorEdit3("Color", &sun_flare.color, _edit_stack_world,
                              _edit_context);

            ImGui::DragFloat("Size", &sun_flare.size, _edit_stack_world,
                             _edit_context, 1.0f, 0.0f, 1e10f);

            ImGui::Separator();

            ImGui::DragFloat("Flare Out Size", &sun_flare.flare_out_size,
                             _edit_stack_world, _edit_context, 1.0f, 0.0f, 1e10f);

            ImGui::DragInt("Num Flare Outs", &sun_flare.num_flare_outs,
                           _edit_stack_world, _edit_context, 1.0f, 1, 100000);

            ImGui::DragInt("Initial Flare Out Alpha", &sun_flare.initial_flare_out_alpha,
                           _edit_stack_world, _edit_context, 1.0f, 0, 255);

            ImGui::Separator();

            ImGui::EditHaloRing("Inner", &sun_flare.halo_inner_ring,
                                _edit_stack_world, _edit_context);

            ImGui::EditHaloRing("Middle", &sun_flare.halo_middle_ring,
                                _edit_stack_world, _edit_context);

            ImGui::EditHaloRing("Outter", &sun_flare.halo_outter_ring,
                                _edit_stack_world, _edit_context);

            ImGui::Separator();

            ImGui::ColorEdit4("Spike Color", &sun_flare.spike_color,
                              _edit_stack_world, _edit_context);

            ImGui::DragFloat("Spike Size", &sun_flare.spike_size,
                             _edit_stack_world, _edit_context, 1.0f, 0.0f, 1e10f);

            ImGui::TreePop();
         }
      }

      ImGui::TreePop();
   }

   ImGui::End();
}

}