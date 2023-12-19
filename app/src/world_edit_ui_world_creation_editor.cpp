#include "world_edit.hpp"

#include "edits/add_property.hpp"
#include "edits/creation_entity_set.hpp"
#include "edits/imgui_ext.hpp"
#include "edits/set_value.hpp"
#include "math/vector_funcs.hpp"
#include "utility/string_icompare.hpp"
#include "world/utility/hintnode_traits.hpp"
#include "world/utility/path_properties.hpp"
#include "world/utility/region_properties.hpp"
#include "world/utility/snapping.hpp"
#include "world/utility/world_utilities.hpp"

#include <numbers>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using namespace std::literals;

namespace we {

namespace {

struct placement_traits {
   bool has_cycle_object_class = false;
   bool has_new_path = false;
   bool has_placement_rotation = true;
   bool has_point_at = true;
   bool has_placement_mode = true;
   bool has_lock_axis = true;
   bool has_placement_alignment = true;
   bool has_placement_ground = true;
   bool has_node_placement_insert = false;
   bool has_resize_to = false;
   bool has_from_bbox = false;
   bool has_from_line = false;
   bool has_draw_barrier = false;
   bool has_cycle_ai_planning = false;
};

auto surface_rotation(const float3 surface_normal,
                      const surface_rotation_axis rotation_axis) noexcept -> quaternion
{
   float3 axis = [rotation_axis]() {
      switch (rotation_axis) {
      case surface_rotation_axis::x:
         return float3{1.0f, 0.0f, 0.0f};
      case surface_rotation_axis::y:
         return float3{0.0f, 1.0f, 0.0f};
      case surface_rotation_axis::z:
         return float3{0.0f, 0.0f, 1.0f};
      case surface_rotation_axis::neg_x:
         return float3{-1.0f, 0.0f, 0.0f};
      case surface_rotation_axis::neg_y:
         return float3{0.0f, -1.0f, 0.0f};
      case surface_rotation_axis::neg_z:
         return float3{0.0f, 0.0f, -1.0f};
      default:
         std::unreachable();
      };
   }();

   return look_to_quat(surface_normal, axis);
}

auto align_position_to_grid(const float2 position, const float alignment) -> float2
{
   return float2{std::round(position.x / alignment) * alignment,
                 std::round(position.y / alignment) * alignment};
}

auto align_position_to_grid(const float3 position, const float alignment) -> float3
{
   return float3{std::round(position.x / alignment) * alignment, position.y,
                 std::round(position.z / alignment) * alignment};
}

}

void world_edit::ui_show_world_creation_editor() noexcept
{
   if (std::exchange(_entity_creation_context.activate_point_at, false)) {
      _entity_creation_config.placement_rotation = placement_rotation::manual_quaternion;
      _entity_creation_config.placement_mode = placement_mode::manual;

      _edit_stack_world.close_last(); // Make sure we don't coalesce with a previous point at.
      _entity_creation_context.using_point_at = true;
   }

   if (std::exchange(_entity_creation_context.activate_extend_to, false)) {
      _edit_stack_world.close_last();

      _entity_creation_context.using_extend_to = true;
      _entity_creation_context.using_shrink_to = false;
   }

   if (std::exchange(_entity_creation_context.activate_shrink_to, false)) {
      _edit_stack_world.close_last();

      _entity_creation_context.using_extend_to = false;
      _entity_creation_context.using_shrink_to = true;
   }

   if (std::exchange(_entity_creation_context.activate_from_object_bbox, false)) {
      _edit_stack_world.close_last();

      _entity_creation_context.using_from_object_bbox = true;
   }

   if (std::exchange(_entity_creation_context.activate_from_line, false)) {
      _edit_stack_world.close_last();

      _entity_creation_context.using_from_line = true;

      _entity_creation_context.from_line_start = std::nullopt;
   }

   if (std::exchange(_entity_creation_context.activate_draw_barrier, false)) {
      _edit_stack_world.close_last();

      _entity_creation_context.using_draw_barrier = true;
      _entity_creation_context.draw_barrier_start = std::nullopt;
      _entity_creation_context.draw_barrier_mid = std::nullopt;
   }

   if (std::exchange(_entity_creation_context.activate_pick_sector, false)) {
      _edit_stack_world.close_last();

      _entity_creation_context.using_pick_sector = true;
   }

   if (std::exchange(_entity_creation_context.finish_from_object_bbox, false)) {
      _entity_creation_context.using_from_object_bbox = false;

      place_creation_entity();
   }

   const bool rotate_entity_forward =
      std::exchange(_entity_creation_context.rotate_forward, false);
   const bool rotate_entity_back =
      std::exchange(_entity_creation_context.rotate_back, false);

   bool continue_creation = true;

   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});

   ImGui::Begin("Create", &continue_creation,
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_AlwaysAutoResize);

   world::creation_entity& creation_entity = *_interaction_targets.creation_entity;

   placement_traits traits{};

   const bool using_cursor_placement =
      _entity_creation_config.placement_mode == placement_mode::cursor and
      not _cursor_placement_undo_lock;

   if (std::holds_alternative<world::object>(creation_entity)) {
      const world::object& object = std::get<world::object>(creation_entity);

      ImGui::InputText("Name", &creation_entity, &world::object::name, &_edit_stack_world,
                       &_edit_context, [&](std::string* edited_value) {
                          *edited_value =
                             world::create_unique_name(_world.objects, *edited_value);
                       });

      ImGui::InputTextAutoComplete(
         "Class Name", &creation_entity, &world::object::class_name,
         &_edit_stack_world, &_edit_context, [&] {
            std::array<std::string_view, 6> entries;
            std::size_t matching_count = 0;

            _asset_libraries.odfs.view_existing(
               [&](const std::span<const assets::stable_string> assets) noexcept {
                  for (const std::string_view asset : assets) {
                     if (matching_count == entries.size()) break;
                     if (not asset.contains(object.class_name)) {
                        continue;
                     }

                     entries[matching_count] = asset;

                     ++matching_count;
                  }
               });

            return entries;
         });

      if (ImGui::BeginPopupContextItem("Class Name")) {
         if (ImGui::MenuItem("Open .odf in Text Editor")) {
            open_odf_in_text_editor(object.class_name);
         }

         ImGui::EndPopup();
      }

      ImGui::LayerPick<world::object>("Layer", &creation_entity,
                                      &_edit_stack_world, &_edit_context);

      if (string::iequals(_object_classes[object.class_name].definition->header.class_label,
                          "commandpost")) {
         ImGui::SeparatorText("Command Post");

         ImGui::Checkbox("Auto-Place Regions & Spawn Path",
                         &_entity_creation_config.command_post_auto_place_meta_entities);

         if (_entity_creation_config.command_post_auto_place_meta_entities) {
            ImGui::DragFloat("Capture Region Radius",
                             &_entity_creation_config.command_post_capture_radius);
            ImGui::DragFloat("Control Region Radius",
                             &_entity_creation_config.command_post_control_radius);
            ImGui::DragFloat("Control Region Height",
                             &_entity_creation_config.command_post_control_height);
            ImGui::DragFloat("Spawn Path Radius",
                             &_entity_creation_config.command_post_spawn_radius);
         }
      }

      if (not object.name.empty()) {
         ImGui::Checkbox("Auto-Add Object to Sectors",
                         &_entity_creation_config.auto_add_object_to_sectors);
      }

      ImGui::Separator();

      if (_entity_creation_config.placement_rotation == placement_rotation::manual_euler) {
         ImGui::DragRotationEuler("Rotation", &creation_entity, &world::object::rotation,
                                  &world::edit_context::euler_rotation,
                                  &_edit_stack_world, &_edit_context);
      }
      else {
         ImGui::DragQuat("Rotation", &creation_entity, &world::object::rotation,
                         &_edit_stack_world, &_edit_context);
      }

      if (const float3 old_position = object.position;
          ImGui::DragFloat3("Position", &creation_entity, &world::object::position,
                            &_edit_stack_world, &_edit_context)) {
         _entity_creation_context.lock_x_axis |=
            old_position.x != object.position.x;
         _entity_creation_context.lock_y_axis |=
            old_position.y != object.position.y;
         _entity_creation_context.lock_z_axis |=
            old_position.z != object.position.z;
      }

      if ((_entity_creation_config.placement_rotation == placement_rotation::surface or
           using_cursor_placement or rotate_entity_forward or rotate_entity_back) and
          not _entity_creation_context.using_point_at) {
         quaternion new_rotation = object.rotation;
         float3 new_position = object.position;
         float3 new_euler_rotation = _edit_context.euler_rotation;

         if (_entity_creation_config.placement_rotation == placement_rotation::surface and
             _cursor_surface_normalWS) {
            new_rotation =
               surface_rotation(*_cursor_surface_normalWS,
                                _entity_creation_config.surface_rotation_axis);
         }

         if (using_cursor_placement) {
            new_position = _cursor_positionWS;

            if (_entity_creation_config.placement_ground == placement_ground::bbox) {

               const math::bounding_box bbox =
                  object.rotation * _object_classes[object.class_name].model->bounding_box;

               new_position.y -= bbox.min.y;
            }

            if (_entity_creation_context.lock_x_axis) {
               new_position.x = object.position.x;
            }
            if (_entity_creation_context.lock_y_axis) {
               new_position.y = object.position.y;
            }
            if (_entity_creation_context.lock_z_axis) {
               new_position.z = object.position.z;
            }
            if (_entity_creation_config.placement_alignment ==
                placement_alignment::grid) {
               new_position = align_position_to_grid(new_position, _editor_grid_size);
            }
            else if (_entity_creation_config.placement_alignment ==
                     placement_alignment::snapping) {
               const std::optional<float3> snapped_position =
                  world::get_snapped_position(object, new_position, _world.objects,
                                              _entity_creation_config.snap_distance,
                                              _object_classes);

               if (snapped_position) new_position = *snapped_position;
            }
         }

         if (rotate_entity_forward or rotate_entity_back) {
            new_euler_rotation = {new_euler_rotation.x,
                                  std::fmod(new_euler_rotation.y +
                                               (rotate_entity_forward ? 15.0f : -15.0f),
                                            360.0f),
                                  new_euler_rotation.z};
            new_rotation = make_quat_from_euler(
               new_euler_rotation * std::numbers::pi_v<float> / 180.0f);
         }

         if (new_rotation != object.rotation or new_position != object.position) {
            _edit_stack_world.apply(edits::make_set_creation_location<world::object>(
                                       new_rotation, object.rotation, new_position,
                                       object.position, new_euler_rotation,
                                       _edit_context.euler_rotation),
                                    _edit_context, {.transparent = true});
         }
      }

      if (_entity_creation_context.using_point_at) {
         _tool_visualizers.add_line_overlay(_cursor_positionWS, object.position,
                                            0xffffffffu);

         const quaternion new_rotation =
            look_at_quat(_cursor_positionWS, object.position);

         if (new_rotation != object.rotation) {
            _edit_stack_world.apply(edits::make_set_creation_value(&world::object::rotation,
                                                                   new_rotation,
                                                                   object.rotation),
                                    _edit_context);
         }
      }

      ImGui::Separator();

      ImGui::SliderInt("Team", &creation_entity, &world::object::team,
                       &_edit_stack_world, &_edit_context, 0, 15, "%d",
                       ImGuiSliderFlags_AlwaysClamp);

      traits = {.has_cycle_object_class = true};
   }
   else if (std::holds_alternative<world::light>(creation_entity)) {
      const world::light& light = std::get<world::light>(creation_entity);

      ImGui::InputText("Name", &creation_entity, &world::light::name, &_edit_stack_world,
                       &_edit_context, [&](std::string* edited_value) {
                          *edited_value =
                             world::create_unique_name(_world.lights, *edited_value);
                       });

      ImGui::LayerPick<world::light>("Layer", &creation_entity,
                                     &_edit_stack_world, &_edit_context);

      ImGui::Separator();

      if (_entity_creation_config.placement_rotation == placement_rotation::manual_euler) {
         ImGui::DragRotationEuler("Rotation", &creation_entity, &world::light::rotation,
                                  &world::edit_context::euler_rotation,
                                  &_edit_stack_world, &_edit_context);
      }
      else {
         ImGui::DragQuat("Rotation", &creation_entity, &world::light::rotation,
                         &_edit_stack_world, &_edit_context);
      }

      if (const float3 old_position = light.position;
          ImGui::DragFloat3("Position", &creation_entity, &world::light::position,
                            &_edit_stack_world, &_edit_context)) {
         _entity_creation_context.lock_x_axis |=
            old_position.x != light.position.x;
         _entity_creation_context.lock_y_axis |=
            old_position.y != light.position.y;
         _entity_creation_context.lock_z_axis |=
            old_position.z != light.position.z;
      }

      if ((_entity_creation_config.placement_rotation == placement_rotation::surface or
           using_cursor_placement or rotate_entity_forward or rotate_entity_back) and
          not _entity_creation_context.using_point_at) {
         quaternion new_rotation = light.rotation;
         float3 new_position = light.position;
         float3 new_euler_rotation = _edit_context.euler_rotation;

         if (_entity_creation_config.placement_rotation == placement_rotation::surface and
             _cursor_surface_normalWS) {
            new_rotation =
               surface_rotation(*_cursor_surface_normalWS,
                                _entity_creation_config.surface_rotation_axis);
         }

         if (using_cursor_placement) {
            new_position = _cursor_positionWS;
            if (_entity_creation_config.placement_alignment ==
                placement_alignment::grid) {
               new_position = align_position_to_grid(new_position, _editor_grid_size);
            }
            else if (_entity_creation_config.placement_alignment ==
                     placement_alignment::snapping) {
               const std::optional<float3> snapped_position =
                  world::get_snapped_position(new_position, _world.objects,
                                              _entity_creation_config.snap_distance,
                                              _object_classes);

               if (snapped_position) new_position = *snapped_position;
            }

            if (_entity_creation_context.lock_x_axis) {
               new_position.x = light.position.x;
            }
            if (_entity_creation_context.lock_y_axis) {
               new_position.y = light.position.y;
            }
            if (_entity_creation_context.lock_z_axis) {
               new_position.z = light.position.z;
            }
         }

         if (rotate_entity_forward or rotate_entity_back) {
            new_euler_rotation = {new_euler_rotation.x,
                                  std::fmod(new_euler_rotation.y +
                                               (rotate_entity_forward ? 15.0f : -15.0f),
                                            360.0f),
                                  new_euler_rotation.z};
            new_rotation = make_quat_from_euler(
               new_euler_rotation * std::numbers::pi_v<float> / 180.0f);
         }

         if (new_rotation != light.rotation or new_position != light.position) {
            _edit_stack_world.apply(edits::make_set_creation_location<world::light>(
                                       new_rotation, light.rotation, new_position,
                                       light.position, new_euler_rotation,
                                       _edit_context.euler_rotation),
                                    _edit_context, {.transparent = true});
         }
      }

      if (_entity_creation_context.using_point_at) {
         _tool_visualizers.add_line_overlay(_cursor_positionWS, light.position,
                                            0xffffffffu);

         const quaternion new_rotation =
            look_at_quat(_cursor_positionWS, light.position);

         if (new_rotation != light.rotation) {
            _edit_stack_world.apply(edits::make_set_creation_value(&world::light::rotation,
                                                                   new_rotation,
                                                                   light.rotation),
                                    _edit_context);
         }
      }

      ImGui::Separator();

      ImGui::ColorEdit3("Color", &creation_entity, &world::light::color,
                        &_edit_stack_world, &_edit_context,
                        ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

      ImGui::Checkbox("Static", &creation_entity, &world::light::static_,
                      &_edit_stack_world, &_edit_context);
      ImGui::SameLine();
      ImGui::Checkbox("Shadow Caster", &creation_entity, &world::light::shadow_caster,
                      &_edit_stack_world, &_edit_context);
      ImGui::SameLine();
      ImGui::Checkbox("Specular Caster", &creation_entity, &world::light::specular_caster,
                      &_edit_stack_world, &_edit_context);

      ImGui::EnumSelect(
         "Light Type", &creation_entity, &world::light::light_type,
         &_edit_stack_world, &_edit_context,
         {enum_select_option{"Directional", world::light_type::directional},
          enum_select_option{"Point", world::light_type::point},
          enum_select_option{"Spot", world::light_type::spot},
          enum_select_option{"Directional Region Box",
                             world::light_type::directional_region_box},
          enum_select_option{"Directional Region Sphere",
                             world::light_type::directional_region_sphere},
          enum_select_option{"Directional Region Cylinder",
                             world::light_type::directional_region_cylinder}});

      ImGui::Separator();

      if (light.light_type == world::light_type::point or
          light.light_type == world::light_type::spot) {
         ImGui::DragFloat("Range", &creation_entity, &world::light::range,
                          &_edit_stack_world, &_edit_context);

         if (light.light_type == world::light_type::spot) {
            ImGui::DragFloat("Inner Cone Angle", &creation_entity,
                             &world::light::inner_cone_angle, &_edit_stack_world,
                             &_edit_context, 0.01f, 0.0f, light.outer_cone_angle,
                             "%.3f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::DragFloat("Outer Cone Angle", &creation_entity,
                             &world::light::outer_cone_angle, &_edit_stack_world,
                             &_edit_context, 0.01f, light.inner_cone_angle,
                             1.570f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
         }

         ImGui::Separator();
      }

      ImGui::InputTextAutoComplete(
         "Texture", &creation_entity, &world::light::texture,
         &_edit_stack_world, &_edit_context, [&] {
            std::array<std::string_view, 6> entries;
            std::size_t matching_count = 0;

            _asset_libraries.textures.view_existing(
               [&](const std::span<const assets::stable_string> assets) noexcept {
                  for (const std::string_view asset : assets) {
                     if (matching_count == entries.size()) break;
                     if (not asset.contains(light.texture)) {
                        continue;
                     }

                     entries[matching_count] = asset;

                     ++matching_count;
                  }
               });

            return entries;
         });

      if (world::is_directional_light(light) and not light.texture.empty()) {
         ImGui::DragFloat2("Directional Texture Tiling", &creation_entity,
                           &world::light::directional_texture_tiling,
                           &_edit_stack_world, &_edit_context, 0.01f);
         ImGui::DragFloat2("Directional Texture Offset", &creation_entity,
                           &world::light::directional_texture_offset,
                           &_edit_stack_world, &_edit_context, 0.01f);
      }

      if (is_region_light(light)) {
         ImGui::Separator();

         ImGui::InputText("Region Name", &creation_entity,
                          &world::light::region_name, &_edit_stack_world,
                          &_edit_context, [&](std::string* edited_value) {
                             *edited_value = world::create_unique_light_region_name(
                                _world.lights, _world.regions,
                                edited_value->empty() ? light.name : *edited_value);
                          });

         if (_entity_creation_config.placement_rotation ==
             placement_rotation::manual_euler) {
            ImGui::DragRotationEuler("Rotation", &creation_entity,
                                     &world::light::region_rotation,
                                     &world::edit_context::light_region_euler_rotation,
                                     &_edit_stack_world, &_edit_context);
         }
         else {
            ImGui::DragQuat("Region Rotation", &creation_entity,
                            &world::light::region_rotation, &_edit_stack_world,
                            &_edit_context);
         }

         ImGui::DragFloat3("Region Size", &creation_entity, &world::light::region_size,
                           &_edit_stack_world, &_edit_context);
      }

      traits = {.has_placement_ground = false};
   }
   else if (std::holds_alternative<world::path>(creation_entity)) {
      const world::path& path = std::get<world::path>(creation_entity);

      const world::path* existing_path = world::find_entity(_world.paths, path.name);

      if (existing_path) {
         ImGui::LabelText("Name", existing_path->name.c_str());
         ImGui::LayerPick("Layer", existing_path, &_edit_stack_world, &_edit_context);

         ImGui::EnumSelect("Path Type", existing_path, &world::path::type,
                           &_edit_stack_world, &_edit_context,
                           {enum_select_option{"None", world::path_type::none},
                            enum_select_option{"Entity Follow",
                                               world::path_type::entity_follow},
                            enum_select_option{"Formation", world::path_type::formation},
                            enum_select_option{"Patrol", world::path_type::patrol}});

         ImGui::EnumSelect(
            "Spline Type", existing_path, &world::path::spline_type,
            &_edit_stack_world, &_edit_context,
            {enum_select_option{"None", world::path_spline_type::none},
             enum_select_option{"Linear", world::path_spline_type::linear},
             enum_select_option{"Hermite", world::path_spline_type::hermite},
             enum_select_option{"Catmull-Rom", world::path_spline_type::catmull_rom}});

         ImGui::Separator();

         for (std::size_t i = 0; i < existing_path->properties.size(); ++i) {
            ImGui::InputKeyValue(existing_path, &world::path::properties, i,
                                 &_edit_stack_world, &_edit_context);
         }

         if (ImGui::BeginCombo("Add Property", "<select property>")) {
            for (const char* prop : world::get_path_properties(existing_path->type)) {
               if (ImGui::Selectable(prop)) {
                  _edit_stack_world.apply(edits::make_add_property(existing_path->id, prop),
                                          _edit_context);
               }
            }

            ImGui::EndCombo();
         }
      }
      else {
         ImGui::InputText("Name", &creation_entity, &world::path::name,
                          &_edit_stack_world, &_edit_context,
                          [&](std::string* edited_value) {
                             *edited_value =
                                world::create_unique_name(_world.paths,
                                                          edited_value->empty()
                                                             ? "Path 0"sv
                                                             : std::string_view{
                                                                  *edited_value});
                          });

         ImGui::LayerPick<world::path>("Layer", &creation_entity,
                                       &_edit_stack_world, &_edit_context);

         ImGui::EnumSelect("Path Type", &creation_entity, &world::path::type,
                           &_edit_stack_world, &_edit_context,
                           {enum_select_option{"None", world::path_type::none},
                            enum_select_option{"Entity Follow",
                                               world::path_type::entity_follow},
                            enum_select_option{"Formation", world::path_type::formation},
                            enum_select_option{"Patrol", world::path_type::patrol}});

         ImGui::EnumSelect(
            "Spline Type", &creation_entity, &world::path::spline_type,
            &_edit_stack_world, &_edit_context,
            {enum_select_option{"None", world::path_spline_type::none},
             enum_select_option{"Linear", world::path_spline_type::linear},
             enum_select_option{"Hermite", world::path_spline_type::hermite},
             enum_select_option{"Catmull-Rom", world::path_spline_type::catmull_rom}});
      }

      ImGui::Separator();

      if (path.nodes.size() != 1) std::terminate();

      ImGui::Text("Next Node");

      if (_entity_creation_config.placement_rotation == placement_rotation::manual_euler) {
         ImGui::DragRotationEulerPathNode("Rotation", &creation_entity,
                                          &world::edit_context::euler_rotation,
                                          &_edit_stack_world, &_edit_context);
      }
      else {
         ImGui::DragQuatPathNode("Rotation", &creation_entity,
                                 &_edit_stack_world, &_edit_context);
      }

      if (const float3 old_position = path.nodes[0].position;
          ImGui::DragFloat3PathNode("Position", &creation_entity,
                                    &_edit_stack_world, &_edit_context)) {
         _entity_creation_context.lock_x_axis |=
            old_position.x != path.nodes[0].position.x;
         _entity_creation_context.lock_y_axis |=
            old_position.y != path.nodes[0].position.y;
         _entity_creation_context.lock_z_axis |=
            old_position.z != path.nodes[0].position.z;
      }

      if ((_entity_creation_config.placement_rotation == placement_rotation::surface or
           using_cursor_placement or rotate_entity_forward or rotate_entity_back) and
          not _entity_creation_context.using_point_at) {
         quaternion new_rotation = path.nodes[0].rotation;
         float3 new_position = path.nodes[0].position;
         float3 new_euler_rotation = _edit_context.euler_rotation;

         if (_entity_creation_config.placement_rotation == placement_rotation::surface and
             _cursor_surface_normalWS) {
            new_rotation =
               surface_rotation(*_cursor_surface_normalWS,
                                _entity_creation_config.surface_rotation_axis);
         }

         if (using_cursor_placement) {
            new_position = _cursor_positionWS;

            if (_entity_creation_config.placement_alignment ==
                placement_alignment::grid) {
               new_position = align_position_to_grid(new_position, _editor_grid_size);
            }
            else if (_entity_creation_config.placement_alignment ==
                     placement_alignment::snapping) {
               const std::optional<float3> snapped_position =
                  world::get_snapped_position(new_position, _world.objects,
                                              _entity_creation_config.snap_distance,
                                              _object_classes);

               if (snapped_position) new_position = *snapped_position;
            }

            if (_entity_creation_context.lock_x_axis) {
               new_position.x = path.nodes[0].position.x;
            }
            if (_entity_creation_context.lock_y_axis) {
               new_position.y = path.nodes[0].position.y;
            }
            if (_entity_creation_context.lock_z_axis) {
               new_position.z = path.nodes[0].position.z;
            }
         }

         if (rotate_entity_forward or rotate_entity_back) {
            new_euler_rotation = {new_euler_rotation.x,
                                  std::fmod(new_euler_rotation.y +
                                               (rotate_entity_forward ? 15.0f : -15.0f),
                                            360.0f),
                                  new_euler_rotation.z};
            new_rotation = make_quat_from_euler(
               new_euler_rotation * std::numbers::pi_v<float> / 180.0f);
         }

         if (new_rotation != path.nodes[0].rotation or
             new_position != path.nodes[0].position) {
            _edit_stack_world.apply(edits::make_set_creation_path_node_location(
                                       new_rotation, path.nodes[0].rotation, new_position,
                                       path.nodes[0].position, new_euler_rotation,
                                       _edit_context.euler_rotation),
                                    _edit_context, {.transparent = true});
         }
      }

      if (_entity_creation_context.using_point_at) {
         _tool_visualizers.add_line_overlay(_cursor_positionWS,
                                            path.nodes[0].position, 0xffffffffu);

         const quaternion new_rotation =
            look_at_quat(_cursor_positionWS, path.nodes[0].position);

         if (new_rotation != path.nodes[0].rotation) {
            _edit_stack_world.apply(edits::make_set_creation_path_node_value(
                                       &world::path::node::rotation,
                                       new_rotation, path.nodes[0].rotation),
                                    _edit_context);
         }
      }

      if (existing_path and not existing_path->nodes.empty()) {
         if (_entity_creation_config.placement_node_insert ==
             placement_node_insert::nearest) {
            const world::clostest_node_result closest =
               find_closest_node(path.nodes[0].position, *existing_path);

            _tool_visualizers
               .add_line_overlay(path.nodes[0].position,
                                 existing_path->nodes[closest.index].position,
                                 0xffffffffu);

            if (closest.next_is_forward and
                closest.index + 1 < existing_path->nodes.size()) {
               _tool_visualizers
                  .add_line_overlay(path.nodes[0].position,
                                    existing_path->nodes[closest.index + 1].position,
                                    0xffffffffu);
            }
            else if (closest.index > 0) {
               _tool_visualizers
                  .add_line_overlay(path.nodes[0].position,
                                    existing_path->nodes[closest.index - 1].position,
                                    0xffffffffu);
            }
         }
         else {
            _tool_visualizers.add_line_overlay(path.nodes[0].position,
                                               existing_path->nodes.back().position,
                                               0xffffffffu);
         }
      }

      ImGui::Separator();

      if (ImGui::Button("New Path", {ImGui::CalcItemWidth(), 0.0f}) or
          std::exchange(_entity_creation_context.finish_current_path, false)) {

         _edit_stack_world.apply(edits::make_set_creation_value(
                                    &world::path::name,
                                    world::create_unique_name(_world.paths, path.name),
                                    path.name),
                                 _edit_context);
      }

      if (ImGui::IsItemHovered()) {
         ImGui::SetTooltip("Create another new path and stop adding "
                           "nodes to the current one.");
      }

      traits = {.has_new_path = true, .has_node_placement_insert = true};
   }
   else if (std::holds_alternative<world::region>(creation_entity)) {
      const world::region& region = std::get<world::region>(creation_entity);
      ImGui::InputText("Name", &creation_entity, &world::region::name, &_edit_stack_world,
                       &_edit_context, [&](std::string* edited_value) {
                          *edited_value =
                             world::create_unique_name(_world.regions, *edited_value);
                       });

      ImGui::LayerPick<world::region>("Layer", &creation_entity,
                                      &_edit_stack_world, &_edit_context);

      ImGui::Separator();

      const world::region_type start_region_type =
         world::get_region_type(region.description);
      world::region_type region_type = start_region_type;

      if (ImGui::EnumSelect(
             "Type", &region_type,
             {
                enum_select_option{"Typeless", world::region_type::typeless},
                enum_select_option{"Death Region", world::region_type::deathregion},
                enum_select_option{"Sound Stream", world::region_type::soundstream},
                enum_select_option{"Sound Static", world::region_type::soundstatic},
                enum_select_option{"Sound Space", world::region_type::soundspace},
                enum_select_option{"Sound Trigger", world::region_type::soundtrigger},
                enum_select_option{"Foley FX", world::region_type::foleyfx},
                enum_select_option{"Shadow", world::region_type::shadow},
                enum_select_option{"Map Bounds", world::region_type::mapbounds},
                enum_select_option{"Rumble", world::region_type::rumble},
                enum_select_option{"Reflection", world::region_type::reflection},
                enum_select_option{"Rain Shadow", world::region_type::rainshadow},
                enum_select_option{"Danger", world::region_type::danger},
                enum_select_option{"Damage Region", world::region_type::damage_region},
                enum_select_option{"AI Vis", world::region_type::ai_vis},
                enum_select_option{"Color Grading (Shader Patch)",
                                   world::region_type::colorgrading},
             })) {
         if (region_type != start_region_type) {
            _edit_stack_world.apply(edits::make_set_creation_value(&world::region::description,
                                                                   to_string(region_type),
                                                                   region.description),
                                    _edit_context);

            const world::region_allowed_shapes allowed_shapes =
               world::get_region_allowed_shapes(region_type);

            if (allowed_shapes == world::region_allowed_shapes::sphere and
                region.shape != world::region_shape::sphere) {
               _edit_stack_world
                  .apply(edits::make_set_creation_value(&world::region::shape,
                                                        world::region_shape::sphere,
                                                        region.shape),
                         _edit_context, {.closed = true, .transparent = true});
            }
            else if (allowed_shapes == world::region_allowed_shapes::box_cylinder and
                     region.shape == world::region_shape::sphere) {
               _edit_stack_world
                  .apply(edits::make_set_creation_value(&world::region::shape,
                                                        world::region_shape::box,
                                                        region.shape),
                         _edit_context, {.closed = true, .transparent = true});
            }
         }
      }

      switch (region_type) {
      case world::region_type::soundstream: {
         world::sound_stream_properties properties =
            world::unpack_region_sound_stream(region.description);

         ImGui::BeginGroup();

         bool value_changed = false;

         value_changed |= ImGui::InputText("Stream Name", &properties.sound_name);

         value_changed |=
            ImGui::DragFloat("Min Distance Divisor",
                             &properties.min_distance_divisor, 1.0f, 1.0f,
                             1000000.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

         if (value_changed) {
            _edit_stack_world.apply(edits::make_set_creation_value(
                                       &world::region::description,
                                       world::pack_region_sound_stream(properties),
                                       region.description),
                                    _edit_context);
         }

         ImGui::EndGroup();

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      } break;
      case world::region_type::soundstatic: {
         world::sound_stream_properties properties =
            world::unpack_region_sound_static(region.description);

         ImGui::BeginGroup();

         bool value_changed = false;

         value_changed |= ImGui::InputText("Sound Name", &properties.sound_name);

         value_changed |=
            ImGui::DragFloat("Min Distance Divisor",
                             &properties.min_distance_divisor, 1.0f, 1.0f,
                             1000000.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

         if (value_changed) {
            _edit_stack_world.apply(edits::make_set_creation_value(
                                       &world::region::description,
                                       world::pack_region_sound_static(properties),
                                       region.description),
                                    _edit_context);
         }

         ImGui::EndGroup();

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      } break;
      case world::region_type::soundspace: {
         world::sound_space_properties properties =
            world::unpack_region_sound_space(region.description);

         if (ImGui::InputText("Sound Space Name", &properties.sound_space_name)) {
            _edit_stack_world.apply(edits::make_set_creation_value(
                                       &world::region::description,
                                       world::pack_region_sound_space(properties),
                                       region.description),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      } break;
      case world::region_type::soundtrigger: {
         world::sound_trigger_properties properties =
            world::unpack_region_sound_trigger(region.description);

         if (ImGui::InputText("Region Name", &properties.region_name)) {
            _edit_stack_world.apply(edits::make_set_creation_value(
                                       &world::region::description,
                                       world::pack_region_sound_trigger(properties),
                                       region.description),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      } break;
      case world::region_type::foleyfx: {
         world::foley_fx_region_properties properties =
            world::unpack_region_foley_fx(region.description);

         if (ImGui::InputText("Group ID", &properties.group_id)) {
            _edit_stack_world
               .apply(edits::make_set_creation_value(&world::region::description,
                                                     world::pack_region_foley_fx(properties),
                                                     region.description),
                      _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      } break;
      case world::region_type::shadow: {
         world::shadow_region_properties properties =
            world::unpack_region_shadow(region.description);

         ImGui::BeginGroup();

         bool value_changed = false;

         if (float directional0 = properties.directional0.value_or(1.0f);
             ImGui::DragFloat("Directional Light 0 Strength", &directional0, 0.01f,
                              0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
            properties.directional0 = directional0;
            value_changed = true;
         }

         if (float directional1 = properties.directional1.value_or(1.0f);
             ImGui::DragFloat("Directional Light 1 Strength", &directional1, 0.01f,
                              0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
            properties.directional1 = directional1;
            value_changed = true;
         }

         if (float3 color_top = properties.color_top.value_or(float3{0.0f, 0.0f, 0.0f});
             ImGui::ColorEdit3("Ambient Light Top", &color_top.x)) {
            properties.color_top = color_top;
            value_changed = true;
         }

         if (float3 color_bottom =
                properties.color_bottom.value_or(float3{0.0f, 0.0f, 0.0f});
             ImGui::ColorEdit3("Ambient Light Bottom", &color_bottom.x)) {
            properties.color_bottom = color_bottom;
            value_changed = true;
         }

         value_changed |= ImGui::InputText("Environment Map", &properties.env_map);

         if (value_changed) {
            _edit_stack_world
               .apply(edits::make_set_creation_value(&world::region::description,
                                                     world::pack_region_shadow(properties),
                                                     region.description),
                      _edit_context);
         }

         ImGui::EndGroup();

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      } break;
      case world::region_type::rumble: {
         world::rumble_region_properties properties =
            world::unpack_region_rumble(region.description);

         ImGui::BeginGroup();

         bool value_changed = false;

         value_changed |= ImGui::InputText("Rumble Class", &properties.rumble_class);
         value_changed |=
            ImGui::InputText("Particle Effect", &properties.particle_effect);

         if (value_changed) {
            _edit_stack_world
               .apply(edits::make_set_creation_value(&world::region::description,
                                                     world::pack_region_rumble(properties),
                                                     region.description),
                      _edit_context);
         }

         ImGui::EndGroup();

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      } break;
      case world::region_type::damage_region: {
         world::damage_region_properties properties =
            world::unpack_region_damage(region.description);

         ImGui::BeginGroup();

         bool value_changed = false;

         if (float damage_rate = properties.damage_rate.value_or(0.0f);
             ImGui::DragFloat("Damage Rate", &damage_rate)) {
            properties.damage_rate = damage_rate;
            value_changed = true;
         }

         if (float person_scale = properties.person_scale.value_or(1.0f);
             ImGui::DragFloat("Person Scale", &person_scale, 0.01f)) {
            properties.person_scale = person_scale;
            value_changed = true;
         }

         if (float animal_scale = properties.animal_scale.value_or(1.0f);
             ImGui::DragFloat("Animal Scale", &animal_scale, 0.01f)) {
            properties.animal_scale = animal_scale;
            value_changed = true;
         }

         if (float droid_scale = properties.droid_scale.value_or(1.0f);
             ImGui::DragFloat("Droid Scale", &droid_scale, 0.01f)) {
            properties.droid_scale = droid_scale;
            value_changed = true;
         }

         if (float vehicle_scale = properties.vehicle_scale.value_or(1.0f);
             ImGui::DragFloat("Vehicle Scale", &vehicle_scale, 0.01f)) {
            properties.vehicle_scale = vehicle_scale;
            value_changed = true;
         }

         if (float building_scale = properties.building_scale.value_or(1.0f);
             ImGui::DragFloat("Building Scale", &building_scale, 0.01f)) {
            properties.building_scale = building_scale;
            value_changed = true;
         }

         if (float building_dead_scale = properties.building_scale.value_or(1.0f);
             ImGui::DragFloat("Building Dead Scale", &building_dead_scale, 0.01f)) {
            properties.building_dead_scale = building_dead_scale;
            value_changed = true;
         }

         if (float building_unbuilt_scale = properties.building_scale.value_or(1.0f);
             ImGui::DragFloat("Building Unbuilt Scale", &building_unbuilt_scale, 0.01f)) {
            properties.building_unbuilt_scale = building_unbuilt_scale;
            value_changed = true;
         }

         if (value_changed) {
            _edit_stack_world
               .apply(edits::make_set_creation_value(&world::region::description,
                                                     world::pack_region_damage(properties),
                                                     region.description),
                      _edit_context);
         }

         ImGui::EndGroup();

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      } break;
      case world::region_type::ai_vis: {
         world::ai_vis_region_properties properties =
            world::unpack_region_ai_vis(region.description);

         ImGui::BeginGroup();

         bool value_changed = false;

         if (float crouch = properties.crouch.value_or(1.0f);
             ImGui::DragFloat("Crouch", &crouch, 0.01f, 0.0f, 1e10f)) {
            properties.crouch = crouch;
            value_changed = true;
         }

         if (float stand = properties.stand.value_or(1.0f);
             ImGui::DragFloat("Stand", &stand, 0.01f, 0.0f, 1e10f)) {
            properties.stand = stand;
            value_changed = true;
         }

         if (value_changed) {
            _edit_stack_world
               .apply(edits::make_set_creation_value(&world::region::description,
                                                     world::pack_region_ai_vis(properties),
                                                     region.description),
                      _edit_context);
         }

         ImGui::EndGroup();

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      } break;
      case world::region_type::colorgrading: {
         world::colorgrading_region_properties properties =
            world::unpack_region_colorgrading(region.description);

         ImGui::BeginGroup();

         bool value_changed = false;

         value_changed |= ImGui::InputText("Config", &properties.config);
         value_changed |= ImGui::DragFloat("Fade Length", &properties.fade_length);

         if (value_changed) {
            _edit_stack_world.apply(edits::make_set_creation_value(
                                       &world::region::description,
                                       world::pack_region_colorgrading(properties),
                                       region.description),
                                    _edit_context);
         }

         ImGui::EndGroup();

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      } break;
      }

      ImGui::InputText("Description", &creation_entity, &world::region::description,
                       &_edit_stack_world, &_edit_context,
                       [&]([[maybe_unused]] std::string* edited_value) {});

      ImGui::Separator();

      if (_entity_creation_config.placement_rotation == placement_rotation::manual_euler) {
         ImGui::DragRotationEuler("Rotation", &creation_entity, &world::region::rotation,
                                  &world::edit_context::euler_rotation,
                                  &_edit_stack_world, &_edit_context);
      }
      else {
         ImGui::DragQuat("Rotation", &creation_entity, &world::region::rotation,
                         &_edit_stack_world, &_edit_context);
      }

      if (const float3 old_position = region.position;
          ImGui::DragFloat3("Position", &creation_entity, &world::region::position,
                            &_edit_stack_world, &_edit_context)) {
         _entity_creation_context.lock_x_axis |=
            old_position.x != region.position.x;
         _entity_creation_context.lock_y_axis |=
            old_position.y != region.position.y;
         _entity_creation_context.lock_z_axis |=
            old_position.z != region.position.z;
      }

      if ((_entity_creation_config.placement_rotation == placement_rotation::surface or
           using_cursor_placement or rotate_entity_forward or rotate_entity_back) and
          not _entity_creation_context.using_point_at) {
         quaternion new_rotation = region.rotation;
         float3 new_position = region.position;
         float3 new_euler_rotation = _edit_context.euler_rotation;

         if (_entity_creation_config.placement_rotation == placement_rotation::surface and
             _cursor_surface_normalWS) {
            new_rotation =
               surface_rotation(*_cursor_surface_normalWS,
                                _entity_creation_config.surface_rotation_axis);
         }

         if (using_cursor_placement) {
            new_position = _cursor_positionWS;

            if (_entity_creation_config.placement_ground == placement_ground::bbox) {
               switch (region.shape) {
               case world::region_shape::box: {
                  const std::array<float3, 8> bbox_corners =
                     math::to_corners({.min = -region.size, .max = region.size});

                  float min_y = std::numeric_limits<float>::max();
                  float max_y = std::numeric_limits<float>::lowest();

                  for (const float3& v : bbox_corners) {
                     const float3 rotated_corner = region.rotation * v;

                     min_y = std::min(rotated_corner.y, min_y);
                     max_y = std::max(rotated_corner.y, max_y);
                  }

                  new_position.y += (std::abs(max_y - min_y) / 2.0f);
               } break;
               case world::region_shape::sphere: {
                  new_position.y += length(region.size);
               } break;
               case world::region_shape::cylinder: {
                  const float cylinder_radius =
                     length(float2{region.size.x, region.size.z});
                  const std::array<float3, 8> bbox_corners = math::to_corners(
                     {.min = {-cylinder_radius, -region.size.y, -cylinder_radius},
                      .max = {cylinder_radius, region.size.y, cylinder_radius}});

                  float min_y = std::numeric_limits<float>::max();
                  float max_y = std::numeric_limits<float>::lowest();

                  for (const float3& v : bbox_corners) {
                     const float3 rotated_corner = region.rotation * v;

                     min_y = std::min(rotated_corner.y, min_y);
                     max_y = std::max(rotated_corner.y, max_y);
                  }

                  new_position.y += (std::abs(max_y - min_y) / 2.0f);
               } break;
               }
            }

            if (_entity_creation_config.placement_alignment ==
                placement_alignment::grid) {
               new_position = align_position_to_grid(new_position, _editor_grid_size);
            }
            else if (_entity_creation_config.placement_alignment ==
                     placement_alignment::snapping) {
               const std::optional<float3> snapped_position =
                  world::get_snapped_position(new_position, _world.objects,
                                              _entity_creation_config.snap_distance,
                                              _object_classes);

               if (snapped_position) new_position = *snapped_position;
            }

            if (_entity_creation_context.lock_x_axis) {
               new_position.x = region.position.x;
            }
            if (_entity_creation_context.lock_y_axis) {
               new_position.y = region.position.y;
            }
            if (_entity_creation_context.lock_z_axis) {
               new_position.z = region.position.z;
            }
         }

         if (rotate_entity_forward or rotate_entity_back) {
            new_euler_rotation = {new_euler_rotation.x,
                                  std::fmod(new_euler_rotation.y +
                                               (rotate_entity_forward ? 15.0f : -15.0f),
                                            360.0f),
                                  new_euler_rotation.z};
            new_rotation = make_quat_from_euler(
               new_euler_rotation * std::numbers::pi_v<float> / 180.0f);
         }

         if (new_rotation != region.rotation or new_position != region.position) {
            _edit_stack_world.apply(edits::make_set_creation_location<world::region>(
                                       new_rotation, region.rotation, new_position,
                                       region.position, new_euler_rotation,
                                       _edit_context.euler_rotation),
                                    _edit_context, {.transparent = true});
         }
      }

      if (_entity_creation_context.using_point_at) {
         _tool_visualizers.add_line_overlay(_cursor_positionWS, region.position,
                                            0xffffffffu);

         const quaternion new_rotation =
            look_at_quat(_cursor_positionWS, region.position);

         if (new_rotation != region.rotation) {
            _edit_stack_world.apply(edits::make_set_creation_value(&world::region::rotation,
                                                                   new_rotation,
                                                                   region.rotation),
                                    _edit_context);
         }
      }

      ImGui::Separator();

      switch (world::get_region_allowed_shapes(region_type)) {
      case world::region_allowed_shapes::all: {
         ImGui::EnumSelect("Shape", &creation_entity, &world::region::shape,
                           &_edit_stack_world, &_edit_context,
                           {enum_select_option{"Box", world::region_shape::box},
                            enum_select_option{"Sphere", world::region_shape::sphere},
                            enum_select_option{"Cylinder",
                                               world::region_shape::cylinder}});
      } break;
      case world::region_allowed_shapes::sphere: {
         ImGui::LabelText("Shape", "Sphere");
      } break;
      case world::region_allowed_shapes::box_cylinder: {
         ImGui::EnumSelect("Shape", &creation_entity, &world::region::shape,
                           &_edit_stack_world, &_edit_context,
                           {enum_select_option{"Box", world::region_shape::box},
                            enum_select_option{"Cylinder",
                                               world::region_shape::cylinder}});
      } break;
      }

      float3 region_size = region.size;

      switch (region.shape) {
      case world::region_shape::box: {
         ImGui::DragFloat3("Size", &region_size, 0.0f, 1e10f);
      } break;
      case world::region_shape::sphere: {
         float radius = length(region_size);

         if (ImGui::DragFloat("Radius", &radius, 0.1f)) {
            const float radius_sq = radius * radius;
            const float size = std::sqrt(radius_sq / 3.0f);

            region_size = {size, size, size};
         }
      } break;
      case world::region_shape::cylinder: {
         float height = region_size.y * 2.0f;

         if (ImGui::DragFloat("Height", &height, 0.1f, 0.0f, 1e10f)) {
            region_size.y = height / 2.0f;
         }

         float radius = length(float2{region_size.x, region_size.z});

         if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f, 1e10f)) {
            const float radius_sq = radius * radius;
            const float size = std::sqrt(radius_sq / 2.0f);

            region_size.x = size;
            region_size.z = size;
         }
      } break;
      }

      if (ImGui::Button("Extend To", {ImGui::CalcItemWidth(), 0.0f})) {
         _entity_creation_context.activate_extend_to = true;
      }
      if (ImGui::Button("Shrink To", {ImGui::CalcItemWidth(), 0.0f})) {
         _entity_creation_context.activate_shrink_to = true;
      }

      if (_entity_creation_context.using_extend_to or
          _entity_creation_context.using_shrink_to) {
         _entity_creation_config.placement_mode = placement_mode::manual;

         if (not _entity_creation_context.resize_start_size) {
            _entity_creation_context.resize_start_size = region.size;
         }

         _tool_visualizers.add_line_overlay(_cursor_positionWS, region.position,
                                            0xffffffffu);

         const float3 region_start_size = *_entity_creation_context.resize_start_size;

         switch (region.shape) {
         case world::region_shape::box: {
            const quaternion inverse_region_rotation = conjugate(region.rotation);

            const float3 cursorRS =
               inverse_region_rotation * (_cursor_positionWS - region.position);

            if (_entity_creation_context.using_extend_to) {
               region_size = max(abs(cursorRS), region_start_size);
            }
            else {
               region_size = min(abs(cursorRS), region_start_size);
            }
         } break;
         case world::region_shape::sphere: {
            const float start_radius = length(region_start_size);
            const float new_radius = distance(region.position, _cursor_positionWS);

            const float radius = _entity_creation_context.using_extend_to
                                    ? std::max(start_radius, new_radius)
                                    : std::min(start_radius, new_radius);
            const float radius_sq = radius * radius;
            const float size = std::sqrt(radius_sq / 3.0f);

            region_size = {size, size, size};
         } break;
         case world::region_shape::cylinder: {
            const float start_radius =
               length(float2{region_start_size.x, region_start_size.z});
            const float start_height = region_start_size.y;

            const quaternion inverse_region_rotation = conjugate(region.rotation);

            const float3 cursorRS =
               inverse_region_rotation * (_cursor_positionWS - region.position);

            const float new_radius = length(float2{cursorRS.x, cursorRS.z});
            const float new_height = std::abs(cursorRS.y);

            const float radius = std::max(start_radius, new_radius);
            const float radius_sq = radius * radius;
            const float size = std::sqrt(radius_sq / 2.0f);

            region_size = {size, std::max(start_height, new_height), size};
         }
         }
      }
      else {
         _entity_creation_context.resize_start_size = std::nullopt;
      }

      if (region_size != region.size) {
         _edit_stack_world.apply(edits::make_set_creation_value(&world::region::size,
                                                                region_size,
                                                                region.size),
                                 _edit_context);
      }

      ImGui::Separator();
      if (ImGui::Button("From Object Bounds", {ImGui::CalcItemWidth(), 0.0f})) {
         _entity_creation_context.activate_from_object_bbox = true;
      }

      if (_entity_creation_context.using_from_object_bbox and
          _interaction_targets.hovered_entity and
          std::holds_alternative<world::object_id>(*_interaction_targets.hovered_entity)) {
         _entity_creation_config.placement_rotation =
            placement_rotation::manual_quaternion;
         _entity_creation_config.placement_mode = placement_mode::manual;

         const world::object* object =
            world::find_entity(_world.objects,
                               std::get<world::object_id>(
                                  *_interaction_targets.hovered_entity));

         if (object) {
            math::bounding_box bbox =
               _object_classes[object->class_name].model->bounding_box;

            const float3 size = abs(bbox.max - bbox.min) / 2.0f;
            const float3 position =
               object->rotation * ((conjugate(object->rotation) * object->position) +
                                   ((bbox.min + bbox.max) / 2.0f));

            _edit_stack_world.apply(edits::make_set_creation_region_metrics(
                                       object->rotation, region.rotation, position,
                                       region.position, size, region.size),
                                    _edit_context);
         }
      }

      traits = {.has_resize_to = true, .has_from_bbox = true};
   }
   else if (std::holds_alternative<world::sector>(creation_entity)) {
      const world::sector& sector = std::get<world::sector>(creation_entity);

      ImGui::InputText("Name", &creation_entity, &world::sector::name, &_edit_stack_world,
                       &_edit_context, [&](std::string* edited_value) {
                          *edited_value =
                             world::create_unique_name(_world.sectors, *edited_value);
                       });

      const bool using_from_object_bbox =
         _entity_creation_context.using_from_object_bbox;

      if (using_from_object_bbox) ImGui::BeginDisabled();

      ImGui::DragFloat("Base", &creation_entity, &world::sector::base,
                       &_edit_stack_world, &_edit_context, 1.0f, 0.0f, 0.0f, "Y:%.3f");
      ImGui::DragFloat("Height", &creation_entity, &world::sector::height,
                       &_edit_stack_world, &_edit_context);

      if (sector.points.empty()) std::terminate();

      if (const float2 old_position = sector.points[0];
          ImGui::DragSectorPoint("Position", &creation_entity,
                                 &_edit_stack_world, &_edit_context)) {
         _entity_creation_context.lock_x_axis |=
            old_position.x != sector.points[0].x;
         _entity_creation_context.lock_z_axis |=
            old_position.y != sector.points[0].y;
      }

      if (using_cursor_placement and not using_from_object_bbox) {
         float2 new_position = sector.points[0];

         new_position = {_cursor_positionWS.x, _cursor_positionWS.z};

         if (_entity_creation_config.placement_alignment == placement_alignment::grid) {
            new_position = align_position_to_grid(new_position, _editor_grid_size);
         }
         else if (_entity_creation_config.placement_alignment ==
                  placement_alignment::snapping) {
            // What should snapping for sectors do?
            ImGui::Text("Snapping is currently unimplemented for "
                        "sectors. Sorry!");
         }

         if (_entity_creation_context.lock_x_axis) {
            new_position.x = sector.points[0].x;
         }
         if (_entity_creation_context.lock_z_axis) {
            new_position.y = sector.points[0].y;
         }

         if (new_position != sector.points[0]) {
            _edit_stack_world.apply(edits::make_set_creation_sector_point(new_position,
                                                                          sector.points[0]),
                                    _edit_context, {.transparent = true});
         }
      }

      if (ImGui::Button("New Sector", {ImGui::CalcItemWidth(), 0.0f}) or
          std::exchange(_entity_creation_context.finish_current_sector, false)) {

         _edit_stack_world.apply(
            edits::make_set_creation_value(&world::sector::name,
                                           world::create_unique_name(_world.sectors,
                                                                     sector.name),
                                           sector.name),
            _edit_context);
      }

      if (ImGui::IsItemHovered()) {
         ImGui::SetTooltip("Create another new sector and stop adding "
                           "points to the current one.");
      }

      if (using_from_object_bbox) ImGui::EndDisabled();

      ImGui::Checkbox("Auto-Fill Object List", &_entity_creation_config.auto_fill_sector);

      if (ImGui::IsItemHovered()) {
         ImGui::SetTooltip("Auto-Fill the sector's object list with objects "
                           "inside "
                           "the sector from active layers as points are added. "
                           "This "
                           "will add a separate entry to the Undo stack.");
      }

      ImGui::Separator();
      if (ImGui::Button("From Object Bounds", {ImGui::CalcItemWidth(), 0.0f})) {
         _entity_creation_context.activate_from_object_bbox = true;
      }

      if (const world::sector* existing_sector =
             world::find_entity(_world.sectors, sector.name);
          existing_sector and not existing_sector->points.empty() and
          not using_from_object_bbox) {
         const float2 start_point = existing_sector->points.back();
         const float2 mid_point = sector.points[0];
         const float2 end_point = existing_sector->points[0];

         const float3 line_bottom_start = {start_point.x, existing_sector->base,
                                           start_point.y};
         const float3 line_bottom_mid = {mid_point.x, sector.base, mid_point.y};
         const float3 line_bottom_end = {end_point.x, existing_sector->base,
                                         end_point.y};

         const float3 line_top_start = {start_point.x,
                                        existing_sector->base + existing_sector->height,
                                        start_point.y};
         const float3 line_top_mid = {mid_point.x, sector.base + sector.height,
                                      mid_point.y};
         const float3 line_top_end = {end_point.x,
                                      existing_sector->base + existing_sector->height,
                                      end_point.y};

         _tool_visualizers.add_line_overlay(line_bottom_start, line_bottom_mid,
                                            0xffffffffu);
         _tool_visualizers.add_line_overlay(line_top_start, line_top_mid, 0xffffffffu);
         _tool_visualizers.add_line_overlay(line_bottom_mid, line_bottom_end,
                                            0xffffffffu);
         _tool_visualizers.add_line_overlay(line_top_mid, line_top_end, 0xffffffffu);
      }
      if (using_from_object_bbox and _interaction_targets.hovered_entity and
          std::holds_alternative<world::object_id>(*_interaction_targets.hovered_entity)) {
         const world::object* object =
            world::find_entity(_world.objects,
                               std::get<world::object_id>(
                                  *_interaction_targets.hovered_entity));

         if (object) {
            std::array<float3, 8> corners = math::to_corners(
               _object_classes[object->class_name].model->bounding_box);

            for (auto& corner : corners) {
               corner = object->rotation * corner + object->position;
            }

            _edit_stack_world.apply(edits::make_set_creation_value(
                                       &world::sector::points,
                                       {float2{corners[0].x, corners[0].z},
                                        float2{corners[1].x, corners[1].z},
                                        float2{corners[2].x, corners[2].z},
                                        float2{corners[3].x, corners[3].z}},
                                       sector.points),
                                    _edit_context);
         }
      }

      traits = {.has_placement_rotation = false,
                .has_point_at = false,
                .has_placement_ground = false,
                .has_from_bbox = true};
   }
   else if (std::holds_alternative<world::portal>(creation_entity)) {
      const world::portal& portal = std::get<world::portal>(creation_entity);

      ImGui::InputText("Name", &creation_entity, &world::portal::name, &_edit_stack_world,
                       &_edit_context, [&](std::string* edited_value) {
                          *edited_value =
                             world::create_unique_name(_world.portals, *edited_value);
                       });

      ImGui::DragFloat("Width", &creation_entity, &world::portal::width,
                       &_edit_stack_world, &_edit_context, 1.0f, 0.25f, 1e10f);
      ImGui::DragFloat("Height", &creation_entity, &world::portal::height,
                       &_edit_stack_world, &_edit_context, 1.0f, 0.25f, 1e10f);

      ImGui::InputTextAutoComplete("Linked Sector 1", &creation_entity,
                                   &world::portal::sector1, &_edit_stack_world,
                                   &_edit_context, [&] {
                                      std::array<std::string_view, 6> entries;
                                      std::size_t matching_count = 0;

                                      for (const auto& sector : _world.sectors) {
                                         if (sector.name.contains(portal.sector1)) {
                                            if (matching_count == entries.size())
                                               break;

                                            entries[matching_count] = sector.name;

                                            ++matching_count;
                                         }
                                      }

                                      return entries;
                                   });

      if (ImGui::Button("Pick Linked Sector 1", {ImGui::CalcItemWidth(), 0.0f})) {
         _entity_creation_context.activate_pick_sector = true;
         _entity_creation_context.pick_sector_index = 1;
      }

      ImGui::InputTextAutoComplete("Linked Sector 2", &creation_entity,
                                   &world::portal::sector2, &_edit_stack_world,
                                   &_edit_context, [&] {
                                      std::array<std::string_view, 6> entries;
                                      std::size_t matching_count = 0;

                                      for (const auto& sector : _world.sectors) {
                                         if (sector.name.contains(portal.sector2)) {
                                            if (matching_count == entries.size())
                                               break;

                                            entries[matching_count] = sector.name;

                                            ++matching_count;
                                         }
                                      }

                                      return entries;
                                   });

      if (ImGui::Button("Pick Linked Sector 2", {ImGui::CalcItemWidth(), 0.0f})) {
         _entity_creation_context.activate_pick_sector = true;
         _entity_creation_context.pick_sector_index = 2;
      }

      if (_entity_creation_context.using_pick_sector and
          _interaction_targets.hovered_entity and
          std::holds_alternative<world::sector_id>(*_interaction_targets.hovered_entity)) {
         const world::sector* sector =
            find_entity(_world.sectors, std::get<world::sector_id>(
                                           *_interaction_targets.hovered_entity));

         if (sector) {
            std::string world::portal::*sector_member = nullptr;

            if (_entity_creation_context.pick_sector_index == 1) {
               sector_member = &world::portal::sector1;
            }
            else if (_entity_creation_context.pick_sector_index == 2) {
               sector_member = &world::portal::sector2;
            }

            _edit_stack_world.apply(edits::make_set_creation_value(sector_member,
                                                                   sector->name,
                                                                   portal.*sector_member),
                                    _edit_context);
         }
      }

      if (_entity_creation_config.placement_rotation == placement_rotation::manual_euler) {
         ImGui::DragRotationEuler("Rotation", &creation_entity, &world::portal::rotation,
                                  &world::edit_context::euler_rotation,
                                  &_edit_stack_world, &_edit_context);
      }
      else {
         ImGui::DragQuat("Rotation", &creation_entity, &world::portal::rotation,
                         &_edit_stack_world, &_edit_context);
      }

      if (const float3 old_position = portal.position;
          ImGui::DragFloat3("Position", &creation_entity, &world::portal::position,
                            &_edit_stack_world, &_edit_context)) {
         _entity_creation_context.lock_x_axis |=
            old_position.x != portal.position.x;
         _entity_creation_context.lock_y_axis |=
            old_position.y != portal.position.y;
         _entity_creation_context.lock_z_axis |=
            old_position.z != portal.position.z;
      }

      if ((_entity_creation_config.placement_rotation == placement_rotation::surface or
           using_cursor_placement or rotate_entity_forward or rotate_entity_back) and
          not _entity_creation_context.using_point_at and
          not _entity_creation_context.using_pick_sector) {
         quaternion new_rotation = portal.rotation;
         float3 new_position = portal.position;
         float3 new_euler_rotation = _edit_context.euler_rotation;

         if (_entity_creation_config.placement_rotation == placement_rotation::surface and
             _cursor_surface_normalWS) {
            new_rotation =
               surface_rotation(*_cursor_surface_normalWS,
                                _entity_creation_config.surface_rotation_axis);
         }

         if (using_cursor_placement) {
            new_position = _cursor_positionWS;

            if (_entity_creation_config.placement_ground == placement_ground::bbox) {
               new_position.y += (portal.height / 2.0f);
            }

            if (_entity_creation_context.lock_x_axis) {
               new_position.x = portal.position.x;
            }
            if (_entity_creation_context.lock_y_axis) {
               new_position.y = portal.position.y;
            }
            if (_entity_creation_context.lock_z_axis) {
               new_position.z = portal.position.z;
            }
         }

         if (rotate_entity_forward or rotate_entity_back) {
            new_euler_rotation = {new_euler_rotation.x,
                                  std::fmod(new_euler_rotation.y +
                                               (rotate_entity_forward ? 15.0f : -15.0f),
                                            360.0f),
                                  new_euler_rotation.z};
            new_rotation = make_quat_from_euler(
               new_euler_rotation * std::numbers::pi_v<float> / 180.0f);
         }

         if (new_rotation != portal.rotation or new_position != portal.position) {
            _edit_stack_world.apply(edits::make_set_creation_location<world::portal>(
                                       new_rotation, portal.rotation, new_position,
                                       portal.position, new_euler_rotation,
                                       _edit_context.euler_rotation),
                                    _edit_context, {.transparent = true});
         }
      }

      if (_entity_creation_context.using_point_at) {
         _tool_visualizers.add_line_overlay(_cursor_positionWS, portal.position,
                                            0xffffffffu);

         const quaternion new_rotation =
            look_at_quat(_cursor_positionWS, portal.position);

         if (new_rotation != portal.rotation) {
            _edit_stack_world.apply(edits::make_set_creation_value(&world::portal::rotation,
                                                                   new_rotation,
                                                                   portal.rotation),
                                    _edit_context);
         }
      }

      if (ImGui::Button("Extend To", {ImGui::CalcItemWidth(), 0.0f})) {
         _entity_creation_context.activate_extend_to = true;
      }
      if (ImGui::Button("Shrink To", {ImGui::CalcItemWidth(), 0.0f})) {
         _entity_creation_context.activate_shrink_to = true;
      }

      if (_entity_creation_context.using_extend_to or
          _entity_creation_context.using_shrink_to) {
         _entity_creation_config.placement_mode = placement_mode::manual;

         if (not _entity_creation_context.resize_portal_start_width) {
            _entity_creation_context.resize_portal_start_width = portal.width;
         }

         if (not _entity_creation_context.resize_portal_start_height) {
            _entity_creation_context.resize_portal_start_height = portal.height;
         }

         _tool_visualizers.add_line_overlay(_cursor_positionWS, portal.position,
                                            0xffffffffu);

         const quaternion inverse_rotation = conjugate(portal.rotation);

         const float3 positionPS = inverse_rotation * portal.position;
         const float3 cursor_positionPS = inverse_rotation * _cursor_positionWS;
         const float3 size = abs(positionPS - cursor_positionPS);

         const float width =
            _entity_creation_context.using_extend_to
               ? std::max(*_entity_creation_context.resize_portal_start_width,
                          size.x * 2.0f)
               : std::min(*_entity_creation_context.resize_portal_start_width,
                          size.x * 2.0f);
         const float height =
            _entity_creation_context.using_extend_to
               ? std::max(*_entity_creation_context.resize_portal_start_height,
                          size.y * 2.0f)
               : std::min(*_entity_creation_context.resize_portal_start_height,
                          size.y * 2.0f);

         if (width != portal.width or height != portal.height) {
            _edit_stack_world.apply(edits::make_set_creation_portal_size(width,
                                                                         portal.width, height,
                                                                         portal.height),
                                    _edit_context);
         }
      }
      else {
         _entity_creation_context.resize_portal_start_width = std::nullopt;
         _entity_creation_context.resize_portal_start_height = std::nullopt;
      }

      traits = {.has_placement_alignment = false, .has_resize_to = true};
   }
   else if (std::holds_alternative<world::hintnode>(creation_entity)) {
      const world::hintnode& hintnode = std::get<world::hintnode>(creation_entity);

      ImGui::InputText("Name", &creation_entity, &world::hintnode::name,
                       &_edit_stack_world, &_edit_context,
                       [&](std::string* edited_value) {
                          *edited_value =
                             world::create_unique_name(_world.hintnodes, *edited_value);
                       });

      ImGui::LayerPick<world::hintnode>("Layer", &creation_entity,
                                        &_edit_stack_world, &_edit_context);

      ImGui::Separator();

      if (_entity_creation_config.placement_rotation == placement_rotation::manual_euler) {
         ImGui::DragRotationEuler("Rotation", &creation_entity,
                                  &world::hintnode::rotation,
                                  &world::edit_context::euler_rotation,
                                  &_edit_stack_world, &_edit_context);
      }
      else {
         ImGui::DragQuat("Rotation", &creation_entity, &world::hintnode::rotation,
                         &_edit_stack_world, &_edit_context);
      }

      if (const float3 old_position = hintnode.position;
          ImGui::DragFloat3("Position", &creation_entity, &world::hintnode::position,
                            &_edit_stack_world, &_edit_context)) {
         _entity_creation_context.lock_x_axis |=
            old_position.x != hintnode.position.x;
         _entity_creation_context.lock_y_axis |=
            old_position.y != hintnode.position.y;
         _entity_creation_context.lock_z_axis |=
            old_position.z != hintnode.position.z;
      }

      if ((_entity_creation_config.placement_rotation == placement_rotation::surface or
           using_cursor_placement or rotate_entity_forward or rotate_entity_back) and
          not _entity_creation_context.using_point_at) {
         quaternion new_rotation = hintnode.rotation;
         float3 new_position = hintnode.position;
         float3 new_euler_rotation = _edit_context.euler_rotation;

         if (_entity_creation_config.placement_rotation == placement_rotation::surface and
             _cursor_surface_normalWS) {
            new_rotation =
               surface_rotation(*_cursor_surface_normalWS,
                                _entity_creation_config.surface_rotation_axis);
         }

         if (using_cursor_placement) {
            new_position = _cursor_positionWS;

            if (_entity_creation_config.placement_alignment ==
                placement_alignment::grid) {
               new_position = align_position_to_grid(new_position, _editor_grid_size);
            }
            else if (_entity_creation_config.placement_alignment ==
                     placement_alignment::snapping) {
               const std::optional<float3> snapped_position =
                  world::get_snapped_position(new_position, _world.objects,
                                              _entity_creation_config.snap_distance,
                                              _object_classes);

               if (snapped_position) new_position = *snapped_position;
            }

            if (_entity_creation_context.lock_x_axis) {
               new_position.x = hintnode.position.x;
            }
            if (_entity_creation_context.lock_y_axis) {
               new_position.y = hintnode.position.y;
            }
            if (_entity_creation_context.lock_z_axis) {
               new_position.z = hintnode.position.z;
            }
         }

         if (rotate_entity_forward or rotate_entity_back) {
            new_euler_rotation = {new_euler_rotation.x,
                                  std::fmod(new_euler_rotation.y +
                                               (rotate_entity_forward ? 15.0f : -15.0f),
                                            360.0f),
                                  new_euler_rotation.z};
            new_rotation = make_quat_from_euler(
               new_euler_rotation * std::numbers::pi_v<float> / 180.0f);
         }

         if (new_rotation != hintnode.rotation or new_position != hintnode.position) {
            _edit_stack_world.apply(edits::make_set_creation_location<world::hintnode>(
                                       new_rotation, hintnode.rotation, new_position,
                                       hintnode.position, new_euler_rotation,
                                       _edit_context.euler_rotation),
                                    _edit_context, {.transparent = true});
         }
      }

      if (_entity_creation_context.using_point_at) {
         _tool_visualizers.add_line_overlay(_cursor_positionWS,
                                            hintnode.position, 0xffffffffu);

         const quaternion new_rotation =
            look_at_quat(_cursor_positionWS, hintnode.position);

         if (new_rotation != hintnode.rotation) {
            _edit_stack_world.apply(edits::make_set_creation_value(&world::hintnode::rotation,
                                                                   new_rotation,
                                                                   hintnode.rotation),
                                    _edit_context);
         }
      }

      ImGui::Separator();

      ImGui::EnumSelect("Type", &creation_entity, &world::hintnode::type,
                        &_edit_stack_world, &_edit_context,
                        {enum_select_option{"Snipe", world::hintnode_type::snipe},
                         enum_select_option{"Patrol", world::hintnode_type::patrol},
                         enum_select_option{"Cover", world::hintnode_type::cover},
                         enum_select_option{"Access", world::hintnode_type::access},
                         enum_select_option{"Jet Jump", world::hintnode_type::jet_jump},
                         enum_select_option{"Mine", world::hintnode_type::mine},
                         enum_select_option{"Land", world::hintnode_type::land},
                         enum_select_option{"Fortification",
                                            world::hintnode_type::fortification},
                         enum_select_option{"Vehicle Cover",
                                            world::hintnode_type::vehicle_cover}});

      const world::hintnode_traits hintnode_traits =
         world::get_hintnode_traits(hintnode.type);

      if (hintnode_traits.has_command_post) {
         if (ImGui::BeginCombo("Command Post", hintnode.command_post.c_str())) {
            for (const auto& object : _world.objects) {
               const assets::odf::definition& definition =
                  *_object_classes[object.class_name].definition;

               if (string::iequals(definition.header.class_label,
                                   "commandpost")) {
                  if (ImGui::Selectable(object.name.c_str())) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::hintnode::command_post,
                                                object.name, hintnode.command_post),
                                             _edit_context);
                  }
               }
            }
            ImGui::EndCombo();
         }
      }

      if (hintnode_traits.has_primary_stance) {
         ImGui::EditFlags("Primary Stance", &creation_entity,
                          &world::hintnode::primary_stance, &_edit_stack_world,
                          &_edit_context,
                          {{"Stand", world::stance_flags::stand},
                           {"Crouch", world::stance_flags::crouch},
                           {"Prone", world::stance_flags::prone}});
      }

      if (hintnode_traits.has_secondary_stance) {
         ImGui::EditFlags("Secondary Stance", &creation_entity,
                          &world::hintnode::secondary_stance,
                          &_edit_stack_world, &_edit_context,
                          {{"Stand", world::stance_flags::stand},
                           {"Crouch", world::stance_flags::crouch},
                           {"Prone", world::stance_flags::prone},
                           {"Left", world::stance_flags::left},
                           {"Right", world::stance_flags::right}});
      }

      if (hintnode_traits.has_mode) {
         ImGui::EnumSelect("Mode", &creation_entity, &world::hintnode::mode,
                           &_edit_stack_world, &_edit_context,
                           {enum_select_option{"None", world::hintnode_mode::none},
                            enum_select_option{"Attack", world::hintnode_mode::attack},
                            enum_select_option{"Defend", world::hintnode_mode::defend},
                            enum_select_option{"Both", world::hintnode_mode::both}});
      }

      if (hintnode_traits.has_radius) {
         ImGui::DragFloat("Radius", &creation_entity, &world::hintnode::radius,
                          &_edit_stack_world, &_edit_context, 0.25f, 0.0f,
                          1e10f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      }

      traits = {.has_placement_ground = false};
   }
   else if (std::holds_alternative<world::barrier>(creation_entity)) {
      const world::barrier& barrier = std::get<world::barrier>(creation_entity);

      ImGui::InputText("Name", &creation_entity, &world::barrier::name,
                       &_edit_stack_world, &_edit_context,
                       [&](std::string* edited_value) {
                          *edited_value =
                             world::create_unique_name(_world.barriers, *edited_value);
                       });

      ImGui::DragBarrierRotation("Rotation", &creation_entity,
                                 &_edit_stack_world, &_edit_context);

      if (const float3 old_position = barrier.position;
          ImGui::DragFloat3("Position", &creation_entity, &world::barrier::position,
                            &_edit_stack_world, &_edit_context)) {
         _entity_creation_context.lock_x_axis |=
            old_position.x != barrier.position.x;
         _entity_creation_context.lock_y_axis |=
            old_position.y != barrier.position.y;
         _entity_creation_context.lock_z_axis |=
            old_position.z != barrier.position.z;
      }

      if ((using_cursor_placement or rotate_entity_forward or rotate_entity_back) and
          not _entity_creation_context.using_point_at) {
         float3 new_position = barrier.position;
         float new_rotation = barrier.rotation_angle;

         if (using_cursor_placement) {
            new_position = _cursor_positionWS;

            if (_entity_creation_config.placement_alignment ==
                placement_alignment::grid) {
               new_position = align_position_to_grid(new_position, _editor_grid_size);
            }
            else if (_entity_creation_config.placement_alignment ==
                     placement_alignment::snapping) {
               const std::optional<float3> snapped_position =
                  world::get_snapped_position(new_position, _world.objects,
                                              _entity_creation_config.snap_distance,
                                              _object_classes);

               if (snapped_position) {
                  new_position = *snapped_position;
               }
            }

            if (_entity_creation_context.lock_x_axis) {
               new_position.x = barrier.position.x;
            }
            if (_entity_creation_context.lock_y_axis) {
               new_position.y = barrier.position.y;
            }
            if (_entity_creation_context.lock_z_axis) {
               new_position.z = barrier.position.z;
            }
         }

         if (rotate_entity_forward or rotate_entity_back) {
            new_rotation =
               std::fmod(new_rotation + ((rotate_entity_forward ? 15.0f : -15.0f) *
                                         std::numbers::pi_v<float> / 180.0f),
                         std::numbers::pi_v<float> * 2.0f);
         }

         if (new_position != barrier.position or new_rotation != barrier.rotation_angle) {
            _edit_stack_world.apply(edits::make_set_creation_barrier_metrics(
                                       new_rotation, barrier.rotation_angle,
                                       new_position, barrier.position,
                                       barrier.size, barrier.size),
                                    _edit_context, {.transparent = true});
         }
      }

      if (_entity_creation_context.using_point_at) {
         _tool_visualizers.add_line_overlay(_cursor_positionWS,
                                            barrier.position, 0xffffffffu);

         const float3 cursor_directionWS =
            normalize(barrier.position - _cursor_positionWS);
         const float2 direction_xzWS =
            normalize(float2{cursor_directionWS.x, cursor_directionWS.z});

         const float new_angle = std::atan2(-direction_xzWS.x, -direction_xzWS.y) +
                                 std::numbers::pi_v<float>;

         if (new_angle != barrier.rotation_angle) {
            _edit_stack_world.apply(edits::make_set_creation_value(&world::barrier::rotation_angle,
                                                                   new_angle,
                                                                   barrier.rotation_angle),
                                    _edit_context);
         }
      }

      ImGui::DragFloat2XZ("Size", &creation_entity, &world::barrier::size,
                          &_edit_stack_world, &_edit_context, 1.0f, 0.0f, 1e10f);

      if (ImGui::Button("Extend To", {ImGui::CalcItemWidth(), 0.0f})) {
         _entity_creation_context.activate_extend_to = true;
      }
      if (ImGui::Button("Shrink To", {ImGui::CalcItemWidth(), 0.0f})) {
         _entity_creation_context.activate_shrink_to = true;
      }

      if (_entity_creation_context.using_extend_to or
          _entity_creation_context.using_shrink_to) {
         _entity_creation_config.placement_mode = placement_mode::manual;

         if (not _entity_creation_context.resize_barrier_start_position) {
            _entity_creation_context.resize_barrier_start_position = barrier.position;
            _entity_creation_context.resize_barrier_start_size = barrier.size;
         }

         const float3 barrier_start_position =
            *_entity_creation_context.resize_barrier_start_position;
         const float2 barrier_start_size =
            *_entity_creation_context.resize_barrier_start_size;

         _tool_visualizers.add_line_overlay(_cursor_positionWS,
                                            barrier_start_position, 0xffffffffu);

         const float4x4 barrier_rotation =
            make_rotation_matrix_from_euler({0.0f, barrier.rotation_angle, 0.0f});
         const float4x4 inverse_barrier_rotation = transpose(barrier_rotation);

         const float3 cursor_positionOS = inverse_barrier_rotation * _cursor_positionWS;
         const float3 barrier_positionOS =
            inverse_barrier_rotation * barrier_start_position;

         std::array<float3, 4> cornersOS{
            float3{-barrier_start_size.x, 0.0f, -barrier_start_size.y} + barrier_positionOS,
            float3{-barrier_start_size.x, 0.0f, barrier_start_size.y} + barrier_positionOS,
            float3{barrier_start_size.x, 0.0f, barrier_start_size.y} + barrier_positionOS,
            float3{barrier_start_size.x, 0.0f, -barrier_start_size.y} + barrier_positionOS,
         };

         const float3 cursor_vectorOS = cursor_positionOS - barrier_positionOS;

         if (cursor_vectorOS.x < 0.0f) {
            cornersOS[0].x = cursor_positionOS.x;
            cornersOS[1].x = cursor_positionOS.x;
         }
         else {
            cornersOS[2].x = cursor_positionOS.x;
            cornersOS[3].x = cursor_positionOS.x;
         }

         if (cursor_vectorOS.z < 0.0f) {
            cornersOS[0].z = cursor_positionOS.z;
            cornersOS[3].z = cursor_positionOS.z;
         }
         else {
            cornersOS[1].z = cursor_positionOS.z;
            cornersOS[2].z = cursor_positionOS.z;
         }

         float3 new_positionOS{};

         for (const auto& corner : cornersOS) {
            new_positionOS += corner;
         }

         new_positionOS /= 4.0f;

         float2 barrier_new_size = abs(float2{cornersOS[2].x, cornersOS[2].z} -
                                       float2{cornersOS[0].x, cornersOS[0].z}) /
                                   2.0f;

         if (_entity_creation_context.using_extend_to) {
            barrier_new_size = max(barrier_new_size, barrier_start_size);
         }
         else {
            barrier_new_size = min(barrier_new_size, barrier_start_size);
         }

         float3 barrier_new_position = barrier_rotation * new_positionOS;

         if (barrier_new_size.x == barrier_start_size.x) {
            barrier_new_position.x = barrier_start_position.x;
         }
         if (barrier_new_size.y == barrier_start_size.y) {
            barrier_new_position.z = barrier_start_position.z;
         }

         barrier_new_position.y = barrier_start_position.y;

         if (barrier_new_position != barrier_start_position or
             barrier_new_size != barrier_start_size) {
            _edit_stack_world.apply(edits::make_set_creation_barrier_metrics(
                                       barrier.rotation_angle, barrier.rotation_angle,
                                       barrier_new_position, barrier_start_position,
                                       barrier_new_size, barrier_start_size),
                                    _edit_context);
         }
      }
      else {
         _entity_creation_context.resize_barrier_start_position = std::nullopt;
         _entity_creation_context.resize_barrier_start_size = std::nullopt;
      }

      ImGui::Separator();
      if (ImGui::Button("From Object Bounds", {ImGui::CalcItemWidth(), 0.0f})) {
         _entity_creation_context.activate_from_object_bbox = true;
      }

      if (_entity_creation_context.using_from_object_bbox and
          _interaction_targets.hovered_entity and
          std::holds_alternative<world::object_id>(*_interaction_targets.hovered_entity)) {
         _entity_creation_config.placement_mode = placement_mode::manual;

         const world::object* object =
            world::find_entity(_world.objects,
                               std::get<world::object_id>(
                                  *_interaction_targets.hovered_entity));

         if (object) {
            math::bounding_box bbox =
               _object_classes[object->class_name].model->bounding_box;

            const float3 size = abs(bbox.max - bbox.min) / 2.0f;
            const float3 position =
               object->rotation * ((conjugate(object->rotation) * object->position) +
                                   ((bbox.min + bbox.max) / 2.0f));

            std::array<float3, 8> corners = math::to_corners(bbox);

            for (auto& corner : corners) {
               corner = object->rotation * corner + object->position;
            }

            const float rotation_angle =
               std::atan2(corners[1].z - corners[0].z, corners[1].x - corners[0].x);

            _edit_stack_world.apply(edits::make_set_creation_barrier_metrics(
                                       rotation_angle, barrier.rotation_angle,
                                       position, barrier.position,
                                       float2{size.x, size.z}, barrier.size),
                                    _edit_context);
         }
      }

      if (ImGui::Button("From Line", {ImGui::CalcItemWidth(), 0.0f})) {
         _entity_creation_context.activate_from_line = true;
      }

      if (ImGui::IsItemHovered()) {
         ImGui::SetTooltip("Place a line and then place resize and "
                           "reposition the barrier around that line.");
      }

      if (_entity_creation_context.using_from_line) {
         _entity_creation_config.placement_mode = placement_mode::manual;

         if (_entity_creation_context.from_line_start) {
            _tool_visualizers.add_line_overlay(*_entity_creation_context.from_line_start,
                                               _cursor_positionWS, 0xffffffffu);

            const float3 start = *_entity_creation_context.from_line_start;
            const float3 end = _cursor_positionWS;

            const float3 position = (start + end) / 2.0f;
            const float height = distance(start, end) / 2.0f;
            const float rotation_angle =
               std::atan2(start.x - end.x, start.z - end.z);

            _edit_stack_world.apply(edits::make_set_creation_barrier_metrics(
                                       rotation_angle, barrier.rotation_angle,
                                       position, barrier.position,
                                       float2{barrier.size.x, height}, barrier.size),
                                    _edit_context);

            if (std::exchange(_entity_creation_context.from_line_click, false)) {
               place_creation_entity();

               _entity_creation_context.using_from_line = false;
            }
         }
         else if (std::exchange(_entity_creation_context.from_line_click, false)) {
            _entity_creation_context.from_line_start = _cursor_positionWS;
         }
      }

      if (ImGui::Button("Draw Barrier", {ImGui::CalcItemWidth(), 0.0f})) {
         _entity_creation_context.activate_draw_barrier = true;
      }

      if (ImGui::IsItemHovered()) {
         ImGui::SetTooltip("Draw lines to create a barrier.");
      }

      if (_entity_creation_context.using_draw_barrier) {
         _entity_creation_config.placement_mode = placement_mode::manual;

         const bool click =
            std::exchange(_entity_creation_context.draw_barrier_click, false);

         if (click and not _entity_creation_context.draw_barrier_start) {
            _entity_creation_context.draw_barrier_start = _cursor_positionWS;
         }
         else if (click and not _entity_creation_context.draw_barrier_mid) {
            _entity_creation_context.draw_barrier_mid = _cursor_positionWS;
         }
         else if (click) {
            place_creation_entity();

            _entity_creation_context.draw_barrier_start = std::nullopt;
            _entity_creation_context.draw_barrier_mid = std::nullopt;
         }

         if (_entity_creation_context.draw_barrier_start and
             _entity_creation_context.draw_barrier_mid) {
            const float3 draw_barrier_start =
               *_entity_creation_context.draw_barrier_start;
            const float3 draw_barrier_mid = *_entity_creation_context.draw_barrier_mid;

            const float3 cursor_direction =
               normalize(_cursor_positionWS - draw_barrier_mid);

            const float3 extend_normal =
               normalize(float3{draw_barrier_mid.z, 0.0f, draw_barrier_mid.x} -
                         float3{draw_barrier_start.z, 0.0f, draw_barrier_start.x}) *
               float3{-1.0, 0.0f, 1.0};

            const float normal_sign =
               dot(cursor_direction, extend_normal) < 0.0f ? -1.0f : 1.0f;

            const float cursor_distance =
               distance(draw_barrier_mid, _cursor_positionWS);

            const float3 draw_barrier_end =
               draw_barrier_mid + extend_normal * cursor_distance * normal_sign;

            _tool_visualizers.add_line_overlay(draw_barrier_start,
                                               draw_barrier_mid, 0xffffffffu);
            _tool_visualizers.add_line_overlay(draw_barrier_mid,
                                               draw_barrier_end, 0xffffffffu);

            const float3 position = (draw_barrier_start + draw_barrier_end) / 2.0f;

            float rotation_angle =
               std::atan2(draw_barrier_start.x - draw_barrier_mid.x,
                          draw_barrier_start.z - draw_barrier_mid.z);

            if (draw_barrier_start.z - draw_barrier_mid.z < 0.0f) {
               rotation_angle += std::numbers::pi_v<float>;
            }

            const float inv_rotation_angle =
               (std::numbers::pi_v<float> * 2.0f) - rotation_angle;

            const float rot_sin = std::sin(inv_rotation_angle);
            const float rot_cos = -std::cos(inv_rotation_angle);

            const std::array<float2, 3> cornersWS{
               {{draw_barrier_start.x, draw_barrier_start.z},
                {draw_barrier_mid.x, draw_barrier_mid.z},
                {draw_barrier_end.x, draw_barrier_end.z}}};
            std::array<float2, 3> cornersOS{};

            for (std::size_t i = 0; i < cornersOS.size(); ++i) {
               const float2 cornerWS = cornersWS[i];

               cornersOS[i] = float2{cornerWS.x * rot_cos - cornerWS.y * rot_sin,
                                     cornerWS.x * rot_sin + cornerWS.y * rot_cos};
            }

            const float2 barrier_max =
               max(max(cornersOS[0], cornersOS[1]), cornersOS[2]);
            const float2 barrier_min =
               min(min(cornersOS[0], cornersOS[1]), cornersOS[2]);

            const float2 size = abs(barrier_max - barrier_min) / 2.0f;

            _edit_stack_world.apply(edits::make_set_creation_barrier_metrics(
                                       rotation_angle, barrier.rotation_angle, position,
                                       barrier.position, size, barrier.size),
                                    _edit_context);
         }
         else if (_entity_creation_context.draw_barrier_start) {
            _tool_visualizers.add_line_overlay(*_entity_creation_context.draw_barrier_start,
                                               _cursor_positionWS, 0xffffffffu);
         }
      }

      ImGui::EditFlags("Flags", &creation_entity, &world::barrier::flags,
                       &_edit_stack_world, &_edit_context,
                       {{"Soldier", world::ai_path_flags::soldier},
                        {"Hover", world::ai_path_flags::hover},
                        {"Small", world::ai_path_flags::small},
                        {"Medium", world::ai_path_flags::medium},
                        {"Huge", world::ai_path_flags::huge},
                        {"Flyer", world::ai_path_flags::flyer}});

      traits = {.has_placement_rotation = false,
                .has_placement_ground = false,
                .has_resize_to = true,
                .has_from_bbox = true,
                .has_from_line = true,
                .has_draw_barrier = true};
   }
   else if (std::holds_alternative<world::planning_hub>(creation_entity)) {
      const world::planning_hub& hub = std::get<world::planning_hub>(creation_entity);

      ImGui::InputText("Name", &creation_entity, &world::planning_hub::name,
                       &_edit_stack_world, &_edit_context,
                       [&](std::string* edited_value) {
                          *edited_value =
                             world::create_unique_name(_world.planning_hubs,
                                                       *edited_value);
                       });

      if (const float3 old_position = hub.position;
          ImGui::DragFloat3("Position", &creation_entity, &world::planning_hub::position,
                            &_edit_stack_world, &_edit_context)) {
         _entity_creation_context.lock_x_axis |= old_position.x != hub.position.x;
         _entity_creation_context.lock_y_axis |= old_position.y != hub.position.y;
         _entity_creation_context.lock_z_axis |= old_position.z != hub.position.z;
      }

      if (using_cursor_placement and not _entity_creation_context.hub_sizing_started) {
         float3 new_position = hub.position;

         if (using_cursor_placement) {
            new_position = _cursor_positionWS;

            if (_entity_creation_config.placement_alignment ==
                placement_alignment::grid) {
               new_position = align_position_to_grid(new_position, _editor_grid_size);
            }
            else if (_entity_creation_config.placement_alignment ==
                     placement_alignment::snapping) {
               const std::optional<float3> snapped_position =
                  world::get_snapped_position({new_position.x,
                                               _cursor_positionWS.y,
                                               new_position.y},
                                              _world.objects,
                                              _entity_creation_config.snap_distance,
                                              _object_classes);

               if (snapped_position) {
                  new_position = *snapped_position;
               }
            }

            if (_entity_creation_context.lock_x_axis) {
               new_position.x = hub.position.x;
            }
            if (_entity_creation_context.lock_y_axis) {
               new_position.y = hub.position.y;
            }
            if (_entity_creation_context.lock_z_axis) {
               new_position.z = hub.position.z;
            }
         }

         if (new_position != hub.position) {
            _edit_stack_world.apply(edits::make_set_creation_value(&world::planning_hub::position,
                                                                   new_position,
                                                                   hub.position),
                                    _edit_context, {.transparent = true});
         }
      }

      ImGui::DragFloat("Radius", &creation_entity, &world::planning_hub::radius,
                       &_edit_stack_world, &_edit_context, 1.0f, 0.0f, 1e10f);

      if (_entity_creation_context.hub_sizing_started) {
         _tool_visualizers.add_line_overlay(_cursor_positionWS, hub.position,
                                            0xffffffffu);

         const float new_radius = distance(_cursor_positionWS, hub.position);

         if (new_radius != hub.radius) {
            _edit_stack_world.apply(edits::make_set_creation_value(&world::planning_hub::radius,
                                                                   new_radius,
                                                                   hub.radius),
                                    _edit_context);
         }
      }

      traits = {
         .has_placement_rotation = false,
         .has_point_at = false,
         .has_placement_ground = false,
         .has_cycle_ai_planning = true,
      };
   }
   else if (std::holds_alternative<world::planning_connection>(creation_entity)) {
      const world::planning_connection& connection =
         std::get<world::planning_connection>(creation_entity);

      ImGui::InputText("Name", &creation_entity,
                       &world::planning_connection::name, &_edit_stack_world,
                       &_edit_context, [&](std::string* edited_value) {
                          *edited_value =
                             world::create_unique_name(_world.planning_connections,
                                                       *edited_value);
                       });

      ImGui::Text("Start: %s",
                  _world.planning_hubs[connection.start_hub_index].name.c_str());
      ImGui::Text("End: %s",
                  _world.planning_hubs[connection.end_hub_index].name.c_str());

      if (_entity_creation_context.connection_link_started and
          _interaction_targets.hovered_entity and
          std::holds_alternative<world::planning_hub_id>(
             *_interaction_targets.hovered_entity)) {
         const world::planning_hub_id end_id =
            std::get<world::planning_hub_id>(*_interaction_targets.hovered_entity);

         _edit_stack_world.apply(edits::make_set_creation_value(
                                    &world::planning_connection::end_hub_index,
                                    get_hub_index(_world.planning_hubs, end_id),
                                    connection.end_hub_index),
                                 _edit_context, {.transparent = true});
      }

      ImGui::EditFlags("Flags", &creation_entity, &world::planning_connection::flags,
                       &_edit_stack_world, &_edit_context,
                       {{"Soldier", world::ai_path_flags::soldier},
                        {"Hover", world::ai_path_flags::hover},
                        {"Small", world::ai_path_flags::small},
                        {"Medium", world::ai_path_flags::medium},
                        {"Huge", world::ai_path_flags::huge},
                        {"Flyer", world::ai_path_flags::flyer}});

      ImGui::Separator();

      ImGui::Checkbox("Jump", &creation_entity, &world::planning_connection::jump,
                      &_edit_stack_world, &_edit_context);
      ImGui::SameLine();
      ImGui::Checkbox("Jet Jump", &creation_entity, &world::planning_connection::jet_jump,
                      &_edit_stack_world, &_edit_context);
      ImGui::SameLine();
      ImGui::Checkbox("One Way", &creation_entity, &world::planning_connection::one_way,
                      &_edit_stack_world, &_edit_context);

      bool is_dynamic = connection.dynamic_group != 0;

      if (ImGui::Checkbox("Dynamic", &is_dynamic)) {
         _edit_stack_world
            .apply(edits::make_set_creation_value(&world::planning_connection::dynamic_group,
                                                  is_dynamic ? int8{1} : int8{0},
                                                  connection.dynamic_group),
                   _edit_context);
      }

      if (is_dynamic) {
         ImGui::SliderInt("Dynamic Group", &creation_entity,
                          &world::planning_connection::dynamic_group,
                          &_edit_stack_world, &_edit_context, 1, 8, "%d",
                          ImGuiSliderFlags_AlwaysClamp);
      }

      if (ImGui::CollapsingHeader("Forward Branch Weights")) {
         world::planning_branch_weights weights = connection.forward_weights;

         bool changed = false;

         ImGui::PushID("Forward Branch Weights");

         // clang-format off
         changed |= ImGui::DragFloat("Soldier", &weights.soldier, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
         changed |= ImGui::DragFloat("Hover", &weights.hover, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
         changed |= ImGui::DragFloat("Small", &weights.small, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
         changed |= ImGui::DragFloat("Medium", &weights.medium, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
         changed |= ImGui::DragFloat("Huge", &weights.huge, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
         // clang-format on

         ImGui::PopID();

         if (changed) {
            _edit_stack_world.apply(edits::make_set_creation_value(
                                       &world::planning_connection::forward_weights,
                                       weights, connection.forward_weights),
                                    _edit_context);
         }
      }

      if (ImGui::CollapsingHeader("Backward Branch Weights")) {
         world::planning_branch_weights weights = connection.backward_weights;

         ImGui::PushID("Backward Branch Weights");

         bool changed = false;

         // clang-format off
         changed |= ImGui::DragFloat("Soldier", &weights.soldier, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
         changed |= ImGui::DragFloat("Hover", &weights.hover, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
         changed |= ImGui::DragFloat("Small", &weights.small, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
         changed |= ImGui::DragFloat("Medium", &weights.medium, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
         changed |= ImGui::DragFloat("Huge", &weights.huge, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
         // clang-format on

         ImGui::PopID();

         if (changed) {
            _edit_stack_world.apply(edits::make_set_creation_value(
                                       &world::planning_connection::backward_weights,
                                       weights, connection.backward_weights),
                                    _edit_context);
         }
      }

      traits = {.has_placement_rotation = false,
                .has_point_at = false,
                .has_placement_mode = false,
                .has_lock_axis = false,
                .has_placement_alignment = false,
                .has_placement_ground = false,
                .has_cycle_ai_planning = true};
   }
   else if (std::holds_alternative<world::boundary>(creation_entity)) {
      const world::boundary& boundary = std::get<world::boundary>(creation_entity);

      ImGui::InputText("Name", &creation_entity, &world::boundary::name,
                       &_edit_stack_world, &_edit_context,
                       [&](std::string* edited_value) {
                          *edited_value =
                             world::create_unique_name(_world.boundaries, *edited_value);
                       });

      if (const float2 old_position = boundary.position;
          ImGui::DragFloat2XZ("Position", &creation_entity, &world::boundary::position,
                              &_edit_stack_world, &_edit_context, 0.25f)) {
         _entity_creation_context.lock_x_axis |=
            old_position.x != boundary.position.x;
         _entity_creation_context.lock_z_axis |=
            old_position.y != boundary.position.y;
      }

      if (using_cursor_placement and not _entity_creation_context.using_point_at) {
         float2 new_position = boundary.position;

         if (using_cursor_placement) {
            new_position = float2{_cursor_positionWS.x, _cursor_positionWS.z};

            if (_entity_creation_config.placement_alignment ==
                placement_alignment::grid) {
               new_position = align_position_to_grid(new_position, _editor_grid_size);
            }
            else if (_entity_creation_config.placement_alignment ==
                     placement_alignment::snapping) {
               const std::optional<float3> snapped_position =
                  world::get_snapped_position({new_position.x,
                                               _cursor_positionWS.y,
                                               new_position.y},
                                              _world.objects,
                                              _entity_creation_config.snap_distance,
                                              _object_classes);

               if (snapped_position) {
                  new_position = {snapped_position->x, snapped_position->z};
               }
            }

            if (_entity_creation_context.lock_x_axis) {
               new_position.x = boundary.position.x;
            }
            if (_entity_creation_context.lock_z_axis) {
               new_position.y = boundary.position.y; // NB: Usage of Y under lock Z.
            }
         }

         if (new_position != boundary.position) {
            _edit_stack_world.apply(edits::make_set_creation_value(&world::boundary::position,
                                                                   new_position,
                                                                   boundary.position),
                                    _edit_context, {.transparent = true});
         }
      }

      ImGui::DragFloat2XZ("Size", &creation_entity, &world::boundary::size,
                          &_edit_stack_world, &_edit_context, 1.0f, 0.0f, 1e10f);

      traits = {.has_placement_rotation = false,
                .has_point_at = false,
                .has_placement_ground = false};
   }

   if (traits.has_placement_rotation) {
      ImGui::SeparatorText("Rotation");

      if (ImGui::BeginTable("Rotation", 3,
                            ImGuiTableFlags_NoSavedSettings |
                               ImGuiTableFlags_SizingStretchSame)) {

         ImGui::TableNextColumn();
         if (ImGui::Selectable("Manual", _entity_creation_config.placement_rotation ==
                                            placement_rotation::manual_euler)) {
            _entity_creation_config.placement_rotation =
               placement_rotation::manual_euler;
         }

         ImGui::TableNextColumn();
         if (ImGui::Selectable("Manual (Quat)",
                               _entity_creation_config.placement_rotation ==
                                  placement_rotation::manual_quaternion)) {
            _entity_creation_config.placement_rotation =
               placement_rotation::manual_quaternion;
         }

         ImGui::TableNextColumn();
         if (ImGui::Selectable("From Surface", _entity_creation_config.placement_rotation ==
                                                  placement_rotation::surface)) {
            _entity_creation_config.placement_rotation = placement_rotation::surface;
         }
         ImGui::EndTable();
      }

      if (_entity_creation_config.placement_rotation == placement_rotation::surface) {
         ImGui::SeparatorText("Rotation Surface Axis");

         if (ImGui::BeginTable("Rotation Surface Axis", 3,
                               ImGuiTableFlags_NoSavedSettings |
                                  ImGuiTableFlags_SizingStretchSame)) {

            ImGui::TableNextColumn();
            if (ImGui::Selectable("X", _entity_creation_config.surface_rotation_axis ==
                                          surface_rotation_axis::x)) {
               _entity_creation_config.surface_rotation_axis =
                  surface_rotation_axis::x;
            }
            ImGui::TableNextColumn();
            if (ImGui::Selectable("Y", _entity_creation_config.surface_rotation_axis ==
                                          surface_rotation_axis::y)) {
               _entity_creation_config.surface_rotation_axis =
                  surface_rotation_axis::y;
            }
            ImGui::TableNextColumn();
            if (ImGui::Selectable("Z", _entity_creation_config.surface_rotation_axis ==
                                          surface_rotation_axis::z)) {
               _entity_creation_config.surface_rotation_axis =
                  surface_rotation_axis::z;
            }

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Negative X", _entity_creation_config.surface_rotation_axis ==
                                                   surface_rotation_axis::neg_x)) {
               _entity_creation_config.surface_rotation_axis =
                  surface_rotation_axis::neg_x;
            }
            ImGui::TableNextColumn();
            if (ImGui::Selectable("Negative Y", _entity_creation_config.surface_rotation_axis ==
                                                   surface_rotation_axis::neg_y)) {
               _entity_creation_config.surface_rotation_axis =
                  surface_rotation_axis::neg_y;
            }
            ImGui::TableNextColumn();
            if (ImGui::Selectable("Negative Z", _entity_creation_config.surface_rotation_axis ==
                                                   surface_rotation_axis::neg_z)) {
               _entity_creation_config.surface_rotation_axis =
                  surface_rotation_axis::neg_z;
            }

            ImGui::EndTable();
         }
      }
   }

   if (traits.has_point_at) {
      ImGui::Separator();

      if (ImGui::Button("Point At", {ImGui::CalcItemWidth(), 0.0f})) {
         _entity_creation_context.activate_point_at = true;
      }
   }

   if (traits.has_placement_mode) {
      ImGui::SeparatorText("Placement");

      if (ImGui::BeginTable("Placement", 2,
                            ImGuiTableFlags_NoSavedSettings |
                               ImGuiTableFlags_SizingStretchSame)) {

         ImGui::TableNextColumn();
         if (ImGui::Selectable("Manual", _entity_creation_config.placement_mode ==
                                            placement_mode::manual)) {
            _entity_creation_config.placement_mode = placement_mode::manual;
         }

         ImGui::TableNextColumn();

         if (ImGui::Selectable("At Cursor", _entity_creation_config.placement_mode ==
                                               placement_mode::cursor)) {
            _entity_creation_config.placement_mode = placement_mode::cursor;
         }
         ImGui::EndTable();
      }
   }

   if (_entity_creation_config.placement_mode == placement_mode::cursor) {
      if (traits.has_lock_axis) {
         ImGui::SeparatorText("Locked Position");

         if (ImGui::BeginTable("Locked Position", 3,
                               ImGuiTableFlags_NoSavedSettings |
                                  ImGuiTableFlags_SizingStretchSame)) {

            ImGui::TableNextColumn();
            ImGui::Selectable("X", &_entity_creation_context.lock_x_axis);
            ImGui::TableNextColumn();
            ImGui::Selectable("Y", &_entity_creation_context.lock_y_axis);
            ImGui::TableNextColumn();
            ImGui::Selectable("Z", &_entity_creation_context.lock_z_axis);

            ImGui::EndTable();
         }
      }

      if (traits.has_placement_alignment) {
         ImGui::SeparatorText("Align To");

         if (ImGui::BeginTable("Align To", 3,
                               ImGuiTableFlags_NoSavedSettings |
                                  ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableNextColumn();
            if (ImGui::Selectable("None", _entity_creation_config.placement_alignment ==
                                             placement_alignment::none)) {
               _entity_creation_config.placement_alignment = placement_alignment::none;
            }

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Grid", _entity_creation_config.placement_alignment ==
                                             placement_alignment::grid)) {
               _entity_creation_config.placement_alignment = placement_alignment::grid;
            }

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Snapping", _entity_creation_config.placement_alignment ==
                                                 placement_alignment::snapping)) {
               _entity_creation_config.placement_alignment =
                  placement_alignment::snapping;
            }

            ImGui::EndTable();
         }

         if (_entity_creation_config.placement_alignment == placement_alignment::grid) {
            ImGui::DragFloat("Grid Size", &_editor_grid_size, 1.0f, 1.0f, 1e10f,
                             "%.3f", ImGuiSliderFlags_AlwaysClamp);
         }
         else if (_entity_creation_config.placement_alignment ==
                  placement_alignment::snapping) {
            ImGui::DragFloat("Snap Distance",
                             &_entity_creation_config.snap_distance, 0.1f, 0.0f,
                             1e10f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
         }
      }

      if (traits.has_placement_ground) {
         ImGui::SeparatorText("Ground With");

         if (ImGui::BeginTable("Ground With", 2,
                               ImGuiTableFlags_NoSavedSettings |
                                  ImGuiTableFlags_SizingStretchSame)) {

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Origin", _entity_creation_config.placement_ground ==
                                               placement_ground::origin)) {
               _entity_creation_config.placement_ground = placement_ground::origin;
            }

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Bounding Box", _entity_creation_config.placement_ground ==
                                                     placement_ground::bbox)) {
               _entity_creation_config.placement_ground = placement_ground::bbox;
            }

            ImGui::EndTable();
         }
      }

      if (traits.has_node_placement_insert) {
         ImGui::SeparatorText("Node Insertion");

         if (ImGui::BeginTable("Node Insertion", 2,
                               ImGuiTableFlags_NoSavedSettings |
                                  ImGuiTableFlags_SizingStretchSame)) {

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Nearest", _entity_creation_config.placement_node_insert ==
                                                placement_node_insert::nearest)) {
               _entity_creation_config.placement_node_insert =
                  placement_node_insert::nearest;
            }

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Append", _entity_creation_config.placement_node_insert ==
                                               placement_node_insert::append)) {
               _entity_creation_config.placement_node_insert =
                  placement_node_insert::append;
            }

            ImGui::EndTable();
         }
      }
   }

   ImGui::End();

   if (_hotkeys_view_show) {
      ImGui::Begin("Hotkeys");

      ImGui::SeparatorText("Entity Creation");

      if (traits.has_cycle_object_class) {
         ImGui::Text("Cycle Object Class");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation", "Cycle Object Class")));
      }

      if (traits.has_new_path) {
         ImGui::Text("New Path");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation", "Cycle Object Class")));
      }

      if (traits.has_placement_rotation) {
         ImGui::Text("Change Rotation Mode");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation", "Change Rotation Mode")));
      }

      if (traits.has_point_at) {
         ImGui::Text("Point At");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation", "Start Point At")));
      }

      if (traits.has_placement_mode) {
         ImGui::Text("Change Placement Mode");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation",
                                   "Change Placement Mode")));
      }

      if (traits.has_lock_axis) {
         ImGui::Text("Lock X Position");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation", "Lock X Axis")));

         ImGui::Text("Lock Y Position");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation", "Lock Y Axis")));

         ImGui::Text("Lock Z Position");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation", "Lock Z Axis")));
      }

      if (traits.has_placement_alignment) {
         ImGui::Text("Change Alignment Mode");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation",
                                   "Change Alignment Mode")));
      }

      if (traits.has_placement_ground) {
         ImGui::Text("Change Grounding Mode");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation",
                                   "Change Grounding Mode")));
      }

      if (traits.has_resize_to) {
         ImGui::Text("Extend To");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation", "Start Extend To")));

         ImGui::Text("Shrink To");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation", "Start Shrink To")));
      }

      if (traits.has_from_bbox) {
         ImGui::Text("From Object Bounds");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation",
                                   "Start From Object BBOX")));
      }

      if (traits.has_from_line) {
         ImGui::Text("From Line");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation", "Start From Line")));
      }

      if (traits.has_draw_barrier) {
         ImGui::Text("Draw Barrier");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation", "Start Draw Barrier")));
      }

      if (traits.has_cycle_ai_planning) {
         ImGui::Text("Toggle AI Planning Entity Type");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Entity Creation (AI Planning)",
                                   "Toggle AI Planning Entity Type")));
      }

      ImGui::End();
   }

   if (not continue_creation) {
      _edit_stack_world.apply(edits::make_creation_entity_set(std::nullopt, creation_entity),
                              _edit_context);
   }

   if (_cursor_placement_undo_lock) {
      ImGui::SetTooltip(
         "At Cursor entity placement is disabled when undoing and "
         "redoing edits. Move the mouse to re-enable.");

      _cursor_placement_undo_lock =
         distance(_cursor_placement_lock_position,
                  float2{ImGui::GetMousePos().x, ImGui::GetMousePos().y}) <
         _settings.preferences.cursor_placement_reenable_distance;
   }
}

}