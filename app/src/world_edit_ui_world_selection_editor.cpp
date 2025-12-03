#include "world_edit.hpp"

#include "edits/add_property.hpp"
#include "edits/add_sector_object.hpp"
#include "edits/bundle.hpp"
#include "edits/creation_entity_set.hpp"
#include "edits/delete_branch_weight.hpp"
#include "edits/delete_path_property.hpp"
#include "edits/delete_sector_object.hpp"
#include "edits/delete_sector_point.hpp"
#include "edits/imgui_ext.hpp"
#include "edits/set_block.hpp"
#include "edits/set_class_name.hpp"
#include "edits/set_value.hpp"

#include "math/scalar_funcs.hpp"
#include "math/vector_funcs.hpp"

#include "utility/string_icompare.hpp"

#include "world/blocks/utility/accessors.hpp"
#include "world/blocks/utility/find.hpp"
#include "world/utility/grounding.hpp"
#include "world/utility/hintnode_traits.hpp"
#include "world/utility/multi_select_support.hpp"
#include "world/utility/object_properties.hpp"
#include "world/utility/path_properties.hpp"
#include "world/utility/raycast.hpp"
#include "world/utility/region_properties.hpp"
#include "world/utility/sector_fill.hpp"
#include "world/utility/selection_centre.hpp"
#include "world/utility/to_ui_string.hpp"
#include "world/utility/world_utilities.hpp"

#include <numbers>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

using namespace std::literals;

namespace we {

void world_edit::ui_show_world_selection_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({520.0f * _display_scale, 620.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({520.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   bool selection_open = true;

   if (ImGui::Begin("Selection", &selection_open, ImGuiWindowFlags_NoCollapse)) {
      if (_interaction_targets.selection.size() == 1) {
         const world::selected_entity& selected = _interaction_targets.selection[0];

         if (selected.is<world::object_id>()) {
            world::object* object =
               world::find_entity(_world.objects, selected.get<world::object_id>());

            if (object) {
               ImGui::SeparatorText("Object");

               ImGui::InputText("Name", &object->name, _edit_stack_world,
                                _edit_context, [&](std::string* edited_value) noexcept {
                                   *edited_value =
                                      world::create_unique_name(_world.objects,
                                                                *edited_value);
                                });

               if (ui_object_class_pick_widget(object)) {
                  std::vector<world::instance_property> new_instance_properties =
                     world::make_object_instance_properties(
                        *_object_classes[object->class_handle].definition,
                        object->instance_properties);

                  if (new_instance_properties != object->instance_properties) {
                     _edit_stack_world.apply(edits::make_set_value(&object->instance_properties,
                                                                   std::move(new_instance_properties)),
                                             _edit_context, {.transparent = true});
                  }
               }

               if (ImGui::BeginPopupContextItem("Class Name")) {
                  if (ImGui::MenuItem("Open .odf in Text Editor")) {
                     open_odf_in_text_editor(object->class_name);
                  }

                  if (ImGui::MenuItem("Show .odf in Explorer")) {
                     show_odf_in_explorer(object->class_name);
                  }

                  ImGui::EndPopup();
               }

               ImGui::LayerPick("Layer", &object->layer, _edit_stack_world,
                                _edit_context);

               ImGui::Separator();

               ImGui::DragQuat("Rotation", &object->rotation, _edit_stack_world,
                               _edit_context);
               ImGui::DragFloat3("Position", &object->position,
                                 _edit_stack_world, _edit_context);

               const bool ground_object =
                  ImGui::Button("Ground Object", {ImGui::CalcItemWidth(), 0.0f});

               ImGui::Separator();

               ImGui::SliderInt("Team", &object->team, _edit_stack_world, _edit_context,
                                0, 15, "%d", ImGuiSliderFlags_AlwaysClamp);

               ImGui::Separator();

               for (uint32 i = 0; i < object->instance_properties.size(); ++i) {
                  world::instance_property& prop = object->instance_properties[i];

                  if (prop.key.contains("Path")) {
                     ImGui::InputKeyValueAutoComplete(
                        &object->instance_properties, i, _edit_stack_world,
                        _edit_context, [&]() noexcept {
                           std::array<std::string_view, 6> entries;

                           std::size_t matching_count = 0;

                           for (auto& path : _world.paths) {
                              if (not path.name.contains(prop.value)) {
                                 continue;
                              }

                              entries[matching_count] = path.name;

                              ++matching_count;

                              if (matching_count == entries.size()) break;
                           }

                           return entries;
                        });
                  }
                  else if (prop.key.contains("Region")) {
                     ImGui::InputKeyValueAutoComplete(
                        &object->instance_properties, i, _edit_stack_world,
                        _edit_context, [&]() noexcept {
                           std::array<std::string_view, 6> entries;

                           std::size_t matching_count = 0;

                           for (auto& region : _world.regions) {
                              if (not region.description.contains(prop.value)) {
                                 continue;
                              }

                              entries[matching_count] = region.description;

                              ++matching_count;

                              if (matching_count == entries.size()) break;
                           }

                           return entries;
                        });
                  }
                  else {
                     ImGui::InputKeyValue(&object->instance_properties, i,
                                          _edit_stack_world, _edit_context);
                  }
               }

               if (ground_object) {
                  if (const std::optional<float3> grounded_position =
                         world::ground_object(*object, _world, _object_classes,
                                              _world_blocks_bvh_library,
                                              _world_layers_hit_mask);
                      grounded_position) {
                     _edit_stack_world.apply(edits::make_set_value(&object->position,
                                                                   *grounded_position),
                                             _edit_context, {.closed = true});
                  }
               }
            }
            else {
               selection_open = false;
            }
         }
         else if (selected.is<world::light_id>()) {
            world::light* light =
               world::find_entity(_world.lights, selected.get<world::light_id>());

            if (light) {
               ImGui::SeparatorText("Light");

               ImGui::InputText("Name", &light->name, _edit_stack_world, _edit_context,
                                [&](std::string* edited_value) noexcept {
                                   *edited_value =
                                      world::create_unique_name(_world.lights,
                                                                *edited_value);
                                });
               ImGui::LayerPick("Layer", &light->layer, _edit_stack_world, _edit_context);

               ImGui::Separator();

               ImGui::DragQuat("Rotation", &light->rotation, _edit_stack_world,
                               _edit_context);
               ImGui::DragFloat3("Position", &light->position,
                                 _edit_stack_world, _edit_context);

               ImGui::Separator();

               ImGui::ColorEdit3("Color", &light->color, _edit_stack_world, _edit_context,
                                 ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

               ImGui::Checkbox("Static", &light->static_, _edit_stack_world,
                               _edit_context);
               ImGui::SameLine();
               ImGui::Checkbox("Shadow Caster", &light->shadow_caster,
                               _edit_stack_world, _edit_context);
               ImGui::SameLine();
               ImGui::Checkbox("Specular Caster", &light->specular_caster,
                               _edit_stack_world, _edit_context);

               ImGui::EnumSelect("Light Type", &light->light_type,
                                 _edit_stack_world, _edit_context,
                                 {world::light_type::directional,
                                  world::light_type::point, world::light_type::spot,
                                  world::light_type::directional_region_box,
                                  world::light_type::directional_region_sphere,
                                  world::light_type::directional_region_cylinder});

               ImGui::Separator();

               if (light->light_type == world::light_type::point or
                   light->light_type == world::light_type::spot) {
                  ImGui::DragFloat("Range", &light->range, _edit_stack_world,
                                   _edit_context);

                  if (light->light_type == world::light_type::spot) {
                     ImGui::DragFloat("Inner Cone Angle", &light->inner_cone_angle,
                                      _edit_stack_world, _edit_context, 0.01f,
                                      0.0f, light->outer_cone_angle, "%.3f",
                                      ImGuiSliderFlags_AlwaysClamp);
                     ImGui::DragFloat("Outer Cone Angle", &light->outer_cone_angle,
                                      _edit_stack_world, _edit_context, 0.01f,
                                      light->inner_cone_angle,
                                      std::numbers::pi_v<float>, "%.3f",
                                      ImGuiSliderFlags_AlwaysClamp);
                  }

                  ImGui::Separator();
               }

               ui_texture_pick_widget("Texture", &light->texture);

               if (not light->texture.empty()) {
                  ImGui::EnumSelect("Texture Addressing", &light->texture_addressing,
                                    _edit_stack_world, _edit_context,
                                    {world::texture_addressing::wrap,
                                     world::texture_addressing::clamp});

                  if (world::is_directional_light(*light)) {
                     ImGui::DragFloat2("Directional Texture Tiling",
                                       &light->directional_texture_tiling,
                                       _edit_stack_world, _edit_context, 0.0625f);
                     ImGui::DragFloat2("Directional Texture Offset",
                                       &light->directional_texture_offset,
                                       _edit_stack_world, _edit_context, 0.0625f);
                  }
               }

               if (is_region_light(*light)) {
                  ImGui::Separator();

                  ImGui::InputText("Region Name", &light->region_name,
                                   _edit_stack_world, _edit_context,
                                   [&](std::string* edited_value) noexcept {
                                      *edited_value =
                                         world::create_unique_light_region_name(
                                            _world.lights, _world.regions,
                                            edited_value->empty() ? light->name
                                                                  : *edited_value);
                                   });
                  ImGui::DragQuat("Region Rotation", &light->region_rotation,
                                  _edit_stack_world, _edit_context);

                  if (ImGui::Button("Rotate Region", {ImGui::CalcItemWidth(), 0.0f})) {
                     _selection_edit_tool = selection_edit_tool::rotate_light_region;
                     _rotate_selection_amount = {0.0f, 0.0f, 0.0f};
                     _rotate_light_region_id = light->id;
                  }

                  switch (light->light_type) {
                  case world::light_type::directional_region_box: {
                     ImGui::DragFloat3("Region Size", &light->region_size,
                                       _edit_stack_world, _edit_context, 1.0f,
                                       0.0f, 1e10f);
                  } break;
                  case world::light_type::directional_region_sphere: {
                     float radius = length(light->region_size);

                     if (ImGui::DragFloat("Region Radius", &radius, 0.1f)) {
                        const float radius_sq = radius * radius;
                        const float size = sqrt(radius_sq / 3.0f);

                        _edit_stack_world.apply(edits::make_set_value(&light->region_size,
                                                                      {size, size, size}),
                                                _edit_context);
                     }

                     if (ImGui::IsItemDeactivated()) {
                        _edit_stack_world.close_last();
                     }

                  } break;
                  case world::light_type::directional_region_cylinder: {

                     if (float height = light->region_size.y * 2.0f;
                         ImGui::DragFloat("Region Height", &height, 0.1f, 0.0f, 1e10f)) {
                        _edit_stack_world.apply(
                           edits::make_set_value(&light->region_size,
                                                 float3{light->region_size.x, height / 2.0f,
                                                        light->region_size.z}),
                           _edit_context);
                     }

                     if (ImGui::IsItemDeactivated()) {
                        _edit_stack_world.close_last();
                     }

                     if (float radius = length(
                            float2{light->region_size.x, light->region_size.z});
                         ImGui::DragFloat("Region Radius", &radius, 0.1f, 0.0f, 1e10f)) {
                        const float radius_sq = radius * radius;
                        const float size = sqrt(radius_sq / 2.0f);

                        _edit_stack_world.apply(
                           edits::make_set_value(&light->region_size,
                                                 float3{size, light->region_size.y, size}),
                           _edit_context);
                     }

                     if (ImGui::IsItemDeactivated()) {
                        _edit_stack_world.close_last();
                     }
                  } break;
                  default:
                     break;
                  }
               }

               if (not _settings.ui.hide_extra_light_properties) {
                  if (light->light_type == world::light_type::spot or
                      world::is_directional_light(*light)) {
                     ImGui::EnumSelect("PS2 Blend Mode", &light->ps2_blend_mode,
                                       _edit_stack_world, _edit_context,
                                       {world::ps2_blend_mode::add,
                                        world::ps2_blend_mode::multiply,
                                        world::ps2_blend_mode::blend});
                  }

                  if (light->light_type == world::light_type::spot) {
                     ImGui::Checkbox("Bidirectional", &light->bidirectional);
                  }
               }
            }
            else {
               selection_open = false;
            }
         }
         else if (selected.is<world::path_id_node_mask>()) {
            const auto& [id, node_mask] = selected.get<world::path_id_node_mask>();

            world::path* path = world::find_entity(_world.paths, id);

            if (path) {
               ImGui::SeparatorText("Path");

               ImGui::InputText("Name", &path->name, _edit_stack_world, _edit_context,
                                [&](std::string* edited_value) noexcept {
                                   *edited_value =
                                      world::create_unique_name(_world.paths,
                                                                edited_value->empty()
                                                                   ? "Path 0"sv
                                                                   : std::string_view{
                                                                        *edited_value});
                                });
               ImGui::LayerPick("Layer", &path->layer, _edit_stack_world, _edit_context);

               ImGui::EnumSelect("Path Type", &path->type, _edit_stack_world,
                                 _edit_context,
                                 {world::path_type::none, world::path_type::entity_follow,
                                  world::path_type::formation,
                                  world::path_type::patrol});

               if (path->type == world::path_type::patrol) {
                  ImGui::EnumSelect("Spline Type", &path->spline_type,
                                    _edit_stack_world, _edit_context,
                                    {world::path_spline_type::none,
                                     world::path_spline_type::linear,
                                     world::path_spline_type::hermite,
                                     world::path_spline_type::catmull_rom});
               }

               if (not path->properties.empty()) ImGui::Separator();

               for (uint32 i = 0; i < path->properties.size(); ++i) {
                  ImGui::PushID(static_cast<int>(i));

                  bool delete_property = false;

                  ImGui::InputKeyValueWithDelete(&path->properties, i, &delete_property,
                                                 _edit_stack_world, _edit_context);

                  if (delete_property) {
                     _edit_stack_world
                        .apply(edits::make_delete_path_property(path->id, i, _world),
                               _edit_context);
                  }

                  ImGui::PopID();
               }

               if (ImGui::BeginCombo("Add Property", "<select property>")) {
                  for (const char* prop : world::get_path_properties(path->type)) {
                     if (ImGui::Selectable(prop)) {
                        _edit_stack_world.apply(edits::make_add_property(path->id, prop),
                                                _edit_context);
                     }
                  }

                  ImGui::EndCombo();
               }

               ImGui::SeparatorText("Selected Nodes");

               const std::size_t node_count =
                  std::min(path->nodes.size(), world::max_path_nodes);

               for (uint32 node_index = 0; node_index < node_count; ++node_index) {
                  if (not node_mask[node_index]) continue;

                  ImGui::PushID("Node");
                  ImGui::PushID(static_cast<int>(node_index));

                  ImGui::Text("Node %u", node_index);

                  ImGui::DragPathNodeRotation("Rotation", &path->nodes, node_index,
                                              _edit_stack_world, _edit_context);
                  ImGui::DragPathNodePosition("Position", &path->nodes, node_index,
                                              _edit_stack_world, _edit_context);

                  for (uint32 prop_index = 0;
                       prop_index < path->nodes[node_index].properties.size();
                       ++prop_index) {
                     ImGui::PushID(static_cast<int>(prop_index));

                     bool delete_property = false;

                     ImGui::InputKeyValueWithDelete(&path->nodes, node_index,
                                                    prop_index, &delete_property,
                                                    _edit_stack_world, _edit_context);

                     if (delete_property) {
                        _edit_stack_world.apply(edits::make_delete_path_node_property(
                                                   path->id, node_index,
                                                   prop_index, _world),
                                                _edit_context);
                     }

                     ImGui::PopID();
                  }

                  if (ImGui::BeginCombo("Add Property##node",
                                        "<select property>")) {
                     for (const char* prop :
                          world::get_path_node_properties(path->type)) {
                        if (ImGui::Selectable(prop)) {
                           _edit_stack_world.apply(edits::make_add_property(path->id, node_index,
                                                                            prop),
                                                   _edit_context);
                        }
                     }

                     ImGui::EndCombo();
                  }

                  ImGui::PopID();
                  ImGui::PopID();
                  ImGui::Separator();
               }

               ImGui::SeparatorText("Node List");

               for (uint32 node_index = 0; node_index < node_count; ++node_index) {
                  ImGui::PushID("NodeList");
                  ImGui::PushID(static_cast<int>(node_index));

                  const ImVec2 cursor = ImGui::GetCursorPos();

                  if (ImGui::Selectable("##node", node_mask[node_index])) {
                     if (node_mask[node_index]) {
                        _interaction_targets.selection.remove(
                           world::make_path_id_node_mask(path->id, node_index));
                     }
                     else {
                        _interaction_targets.selection.add(
                           world::make_path_id_node_mask(path->id, node_index));
                     }
                  }

                  ImGui::SetCursorPos(cursor);

                  ImGui::Text("Node %u", node_index);

                  ImGui::PopID();
                  ImGui::PopID();
               }

               if (ImGui::Button("Add Nodes", {ImGui::CalcItemWidth(), 0.0f})) {
                  _edit_stack_world.apply(edits::make_creation_entity_set(
                                             world::path{.name = path->name,
                                                         .layer = path->layer,
                                                         .nodes = {world::path::node{}},
                                                         .id = world::max_id},
                                             _object_classes),
                                          _edit_context);
                  _entity_creation_context = {};
               }

               if (ImGui::Button("Move Path", {ImGui::CalcItemWidth(), 0.0f})) {
                  _selection_edit_tool = selection_edit_tool::move_path;
                  _move_entire_path_id = id;
               }
            }
            else {
               selection_open = false;
            }
         }
         else if (selected.is<world::region_id>()) {
            world::region* region =
               world::find_entity(_world.regions, selected.get<world::region_id>());

            if (region) {
               ImGui::SeparatorText("Region");

               ImGui::InputText("Name", &region->name, _edit_stack_world,
                                _edit_context, [&](std::string* edited_value) noexcept {
                                   *edited_value =
                                      world::create_unique_name(_world.regions,
                                                                _world.lights,
                                                                *edited_value);
                                });
               ImGui::LayerPick("Layer", &region->layer, _edit_stack_world,
                                _edit_context);

               ImGui::Separator();

               const world::region_type start_region_type =
                  world::get_region_type(region->description);
               world::region_type region_type = start_region_type;

               if (ImGui::EnumSelect("Type", &region_type,
                                     {
                                        world::region_type::typeless,
                                        world::region_type::soundstream,
                                        world::region_type::soundstatic,
                                        world::region_type::soundspace,
                                        world::region_type::soundtrigger,
                                        world::region_type::foleyfx,
                                        world::region_type::shadow,
                                        world::region_type::mapbounds,
                                        world::region_type::rumble,
                                        world::region_type::reflection,
                                        world::region_type::rainshadow,
                                        world::region_type::danger,
                                        world::region_type::damage_region,
                                        world::region_type::ai_vis,
                                        world::region_type::colorgrading,
                                     })) {
                  if (region_type != start_region_type) {
                     _edit_stack_world.apply(edits::make_set_value(&region->description,
                                                                   get_default_description(
                                                                      region_type)),
                                             _edit_context);

                     const world::region_allowed_shapes allowed_shapes =
                        world::get_region_allowed_shapes(region_type);

                     if (allowed_shapes == world::region_allowed_shapes::box and
                         region->shape != world::region_shape::box) {
                        _edit_stack_world
                           .apply(edits::make_set_value(&region->shape,
                                                        world::region_shape::box),
                                  _edit_context,
                                  {.closed = true, .transparent = true});
                     }
                     else if (allowed_shapes == world::region_allowed_shapes::sphere and
                              region->shape != world::region_shape::sphere) {
                        _edit_stack_world
                           .apply(edits::make_set_value(&region->shape,
                                                        world::region_shape::sphere),
                                  _edit_context,
                                  {.closed = true, .transparent = true});
                     }
                     else if (allowed_shapes == world::region_allowed_shapes::box_cylinder and
                              region->shape == world::region_shape::sphere) {
                        _edit_stack_world
                           .apply(edits::make_set_value(&region->shape,
                                                        world::region_shape::box),
                                  _edit_context,
                                  {.closed = true, .transparent = true});
                     }
                  }
               }

               switch (region_type) {
               case world::region_type::typeless: {
               } break;
               case world::region_type::soundstream: {
                  world::sound_stream_properties properties =
                     world::unpack_region_sound_stream(region->description);

                  ImGui::BeginGroup();

                  bool value_changed = false;

                  value_changed |=
                     ImGui::InputText("Stream Name", &properties.sound_name);

                  value_changed |= ImGui::DragFloat("Min Distance Divisor",
                                                    &properties.min_distance_divisor,
                                                    1.0f, 1.0f, 1000000.0f, "%.3f",
                                                    ImGuiSliderFlags_AlwaysClamp);

                  if (value_changed) {
                     _edit_stack_world.apply(edits::make_set_value(&region->description,
                                                                   world::pack_region_sound_stream(
                                                                      properties)),
                                             _edit_context);
                  }

                  ImGui::EndGroup();

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::soundstatic: {
                  world::sound_stream_properties properties =
                     world::unpack_region_sound_static(region->description);

                  ImGui::BeginGroup();

                  bool value_changed = false;

                  value_changed |=
                     ImGui::InputText("Sound Name", &properties.sound_name);

                  value_changed |= ImGui::DragFloat("Min Distance Divisor",
                                                    &properties.min_distance_divisor,
                                                    1.0f, 1.0f, 1000000.0f, "%.3f",
                                                    ImGuiSliderFlags_AlwaysClamp);

                  if (value_changed) {
                     _edit_stack_world.apply(edits::make_set_value(&region->description,
                                                                   world::pack_region_sound_static(
                                                                      properties)),
                                             _edit_context);
                  }

                  ImGui::EndGroup();

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::soundspace: {
                  world::sound_space_properties properties =
                     world::unpack_region_sound_space(region->description);

                  if (ImGui::InputText("Sound Space Name", &properties.sound_space_name)) {
                     _edit_stack_world.apply(edits::make_set_value(&region->description,
                                                                   world::pack_region_sound_space(
                                                                      properties)),
                                             _edit_context);
                  }

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::soundtrigger: {
                  world::sound_trigger_properties properties =
                     world::unpack_region_sound_trigger(region->description);

                  if (ImGui::InputText("Region Name", &properties.region_name)) {
                     _edit_stack_world.apply(edits::make_set_value(&region->description,
                                                                   world::pack_region_sound_trigger(
                                                                      properties)),
                                             _edit_context);
                  }

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::foleyfx: {
                  world::foley_fx_region_properties properties =
                     world::unpack_region_foley_fx(region->description);

                  if (ImGui::InputText("Group ID", &properties.group_id)) {
                     _edit_stack_world.apply(edits::make_set_value(&region->description,
                                                                   world::pack_region_foley_fx(
                                                                      properties)),
                                             _edit_context);
                  }

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::shadow: {
                  world::shadow_region_properties properties =
                     world::unpack_region_shadow(region->description);

                  ImGui::BeginGroup();

                  bool value_changed = false;

                  if (float directional0 = properties.directional0.value_or(1.0f);
                      ImGui::DragFloat("Directional Light 0 Strength",
                                       &directional0, 0.01f, 0.0f, 1.0f, "%.3f",
                                       ImGuiSliderFlags_AlwaysClamp)) {
                     properties.directional0 = directional0;
                     value_changed = true;
                  }

                  if (float directional1 = properties.directional1.value_or(1.0f);
                      ImGui::DragFloat("Directional Light 1 Strength",
                                       &directional1, 0.01f, 0.0f, 1.0f, "%.3f",
                                       ImGuiSliderFlags_AlwaysClamp)) {
                     properties.directional1 = directional1;
                     value_changed = true;
                  }

                  if (float3 color_top = properties.color_top.value_or(
                         _world.global_lights.ambient_sky_color);
                      ImGui::ColorEdit3("Ambient Light Top", &color_top.x)) {
                     properties.color_top = color_top;
                     value_changed = true;
                  }

                  if (float3 color_bottom = properties.color_bottom.value_or(
                         _world.global_lights.ambient_ground_color);
                      ImGui::ColorEdit3("Ambient Light Bottom", &color_bottom.x)) {
                     properties.color_bottom = color_bottom;
                     value_changed = true;
                  }

                  if (std::optional<std::string> new_env_map =
                         ui_texture_pick_widget_untracked("Environment Map",
                                                          properties.env_map.c_str());
                      new_env_map) {
                     properties.env_map = std::move(*new_env_map);

                     value_changed = true;
                  }

                  if (value_changed) {
                     _edit_stack_world.apply(edits::make_set_value(&region->description,
                                                                   world::pack_region_shadow(
                                                                      properties)),
                                             _edit_context);
                  }

                  ImGui::EndGroup();

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::mapbounds: {
               } break;
               case world::region_type::rumble: {
                  world::rumble_region_properties properties =
                     world::unpack_region_rumble(region->description);

                  ImGui::BeginGroup();

                  bool value_changed = false;

                  value_changed |=
                     ImGui::InputText("Rumble Class", &properties.rumble_class);
                  value_changed |= ImGui::InputText("Particle Effect",
                                                    &properties.particle_effect);

                  if (value_changed) {
                     _edit_stack_world.apply(edits::make_set_value(&region->description,
                                                                   world::pack_region_rumble(
                                                                      properties)),
                                             _edit_context);
                  }

                  ImGui::EndGroup();

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::reflection:
               case world::region_type::rainshadow:
               case world::region_type::danger: {
               } break;
               case world::region_type::damage_region: {
                  world::damage_region_properties properties =
                     world::unpack_region_damage(region->description);

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

                  if (float building_dead_scale =
                         properties.building_dead_scale.value_or(1.0f);
                      ImGui::DragFloat("Building Dead Scale",
                                       &building_dead_scale, 0.01f)) {
                     properties.building_dead_scale = building_dead_scale;
                     value_changed = true;
                  }

                  if (float building_unbuilt_scale =
                         properties.building_unbuilt_scale.value_or(1.0f);
                      ImGui::DragFloat("Building Unbuilt Scale",
                                       &building_unbuilt_scale, 0.01f)) {
                     properties.building_unbuilt_scale = building_unbuilt_scale;
                     value_changed = true;
                  }

                  if (value_changed) {
                     _edit_stack_world.apply(edits::make_set_value(&region->description,
                                                                   world::pack_region_damage(
                                                                      properties)),
                                             _edit_context);
                  }

                  ImGui::EndGroup();

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::ai_vis: {
                  world::ai_vis_region_properties properties =
                     world::unpack_region_ai_vis(region->description);

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
                     _edit_stack_world.apply(edits::make_set_value(&region->description,
                                                                   world::pack_region_ai_vis(
                                                                      properties)),
                                             _edit_context);
                  }

                  ImGui::EndGroup();

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::colorgrading: {
                  world::colorgrading_region_properties properties =
                     world::unpack_region_colorgrading(region->description);

                  ImGui::BeginGroup();

                  bool value_changed = false;

                  value_changed |= ImGui::InputText("Config", &properties.config);
                  value_changed |=
                     ImGui::DragFloat("Fade Length", &properties.fade_length);

                  if (value_changed) {
                     _edit_stack_world.apply(edits::make_set_value(&region->description,
                                                                   world::pack_region_colorgrading(
                                                                      properties)),
                                             _edit_context);
                  }

                  ImGui::EndGroup();

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               }

               ImGui::InputText("Description", &region->description, _edit_stack_world,
                                _edit_context, [&](std::string*) noexcept {});

               ImGui::Separator();

               ImGui::DragQuat("Rotation", &region->rotation, _edit_stack_world,
                               _edit_context);
               ImGui::DragFloat3("Position", &region->position,
                                 _edit_stack_world, _edit_context);

               switch (region->shape) {
               case world::region_shape::box: {
                  ImGui::DragFloat3("Size", &region->size, _edit_stack_world,
                                    _edit_context, 1.0f, 0.0f, 1e10f);
               } break;
               case world::region_shape::sphere: {
                  float radius = length(region->size);

                  if (ImGui::DragFloat("Radius", &radius, 0.1f)) {

                     const float radius_sq = radius * radius;
                     const float size = sqrt(radius_sq / 3.0f);

                     _edit_stack_world.apply(edits::make_set_value(&region->size,
                                                                   {size, size, size}),
                                             _edit_context);
                  }

                  if (ImGui::IsItemDeactivated()) {
                     _edit_stack_world.close_last();
                  }

               } break;
               case world::region_shape::cylinder: {

                  if (float height = region->size.y * 2.0f;
                      ImGui::DragFloat("Height", &height, 0.1f, 0.0f, 1e10f)) {
                     _edit_stack_world
                        .apply(edits::make_set_value(&region->size,
                                                     float3{region->size.x, height / 2.0f,
                                                            region->size.z}),
                               _edit_context);
                  }

                  if (ImGui::IsItemDeactivated()) {
                     _edit_stack_world.close_last();
                  }

                  if (float radius = length(float2{region->size.x, region->size.z});
                      ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f, 1e10f)) {
                     const float radius_sq = radius * radius;
                     const float size = sqrt(radius_sq / 2.0f);

                     _edit_stack_world
                        .apply(edits::make_set_value(&region->size,
                                                     float3{size, region->size.y, size}),
                               _edit_context);
                  }

                  if (ImGui::IsItemDeactivated()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               }

               ImGui::Separator();

               switch (world::get_region_allowed_shapes(region_type)) {
               case world::region_allowed_shapes::all: {
                  ImGui::EnumSelect("Shape", &region->shape, _edit_stack_world,
                                    _edit_context,
                                    {world::region_shape::box, world::region_shape::sphere,
                                     world::region_shape::cylinder});
               } break;
               case world::region_allowed_shapes::box: {
                  ImGui::EnumSelect("Shape", &region->shape, _edit_stack_world,
                                    _edit_context, {world::region_shape::box});
               } break;
               case world::region_allowed_shapes::sphere: {
                  ImGui::EnumSelect("Shape", &region->shape, _edit_stack_world,
                                    _edit_context, {world::region_shape::sphere});
               } break;
               case world::region_allowed_shapes::box_cylinder: {
                  ImGui::EnumSelect("Shape", &region->shape, _edit_stack_world,
                                    _edit_context,
                                    {world::region_shape::box,
                                     world::region_shape::cylinder});
               } break;
               }
            }
            else {
               selection_open = false;
            }
         }
         else if (selected.is<world::sector_id>()) {
            world::sector* sector =
               world::find_entity(_world.sectors, selected.get<world::sector_id>());

            if (sector) {
               ImGui::SeparatorText("Sector");

               ImGui::InputText("Name", &sector->name, _edit_stack_world,
                                _edit_context, [&](std::string* edited_value) noexcept {
                                   *edited_value =
                                      world::create_unique_name(_world.sectors,
                                                                *edited_value);
                                });

               ImGui::DragFloat("Base", &sector->base, _edit_stack_world,
                                _edit_context, 1.0f, 0.0f, 0.0f, "Y:%.3f");
               ImGui::DragFloat("Height", &sector->height, _edit_stack_world,
                                _edit_context);

               ImGui::SeparatorText("Points");

               for (int i = 0; i < sector->points.size(); ++i) {
                  ImGui::BeginGroup();
                  ImGui::PushID(i);

                  float2 point = sector->points[i];

                  ImGui::DragSectorPoint("##point", &sector->points, i,
                                         _edit_stack_world, _edit_context);

                  ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                  if (ImGui::Button("Move")) {
                     _move_sector_point_id = sector->id;
                     _move_sector_point_index = i;
                     _selection_edit_tool = selection_edit_tool::move_sector_point;
                  }

                  const bool disable_delete = sector->points.size() <= 2;

                  if (disable_delete) ImGui::BeginDisabled();

                  ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                  if (ImGui::Button("X")) {
                     _edit_stack_world
                        .apply(edits::make_delete_sector_point(sector->id, i, _world),
                               _edit_context);
                  }

                  if (disable_delete) ImGui::EndDisabled();

                  ImGui::PopID();
                  ImGui::EndGroup();

                  if (ImGui::IsItemHovered()) {
                     const float3 bottom = float3{sector->points[i].x, sector->base,
                                                  sector->points[i].y};
                     const float3 top = float3{sector->points[i].x,
                                               sector->base + sector->height,
                                               sector->points[i].y};

                     _tool_visualizers.add_line_overlay(bottom - float3{1.0f, 0.0f, 0.0f},
                                                        bottom + float3{1.0f, 0.0f, 0.0f},
                                                        0xffffffff);
                     _tool_visualizers.add_line_overlay(bottom - float3{0.0f, 1.0f, 0.0f},
                                                        bottom + float3{0.0f, 1.0f, 0.0f},
                                                        0xffffffff);
                     _tool_visualizers.add_line_overlay(bottom - float3{0.0f, 0.0f, 1.0f},
                                                        bottom + float3{0.0f, 0.0f, 1.0f},
                                                        0xffffffff);

                     _tool_visualizers.add_line_overlay(top - float3{1.0f, 0.0f, 0.0f},
                                                        top + float3{1.0f, 0.0f, 0.0f},
                                                        0xffffffff);
                     _tool_visualizers.add_line_overlay(top - float3{0.0f, 1.0f, 0.0f},
                                                        top + float3{0.0f, 1.0f, 0.0f},
                                                        0xffffffff);
                     _tool_visualizers.add_line_overlay(top - float3{0.0f, 0.0f, 1.0f},
                                                        top + float3{0.0f, 0.0f, 1.0f},
                                                        0xffffffff);
                  }
               }

               if (ImGui::Button("Add Points", {ImGui::CalcItemWidth(), 0.0f})) {
                  _edit_stack_world.apply(edits::make_creation_entity_set(
                                             world::sector{.name = sector->name,
                                                           .base = sector->base,
                                                           .height = sector->height,
                                                           .points = {float2{0.0f, 0.0f}},
                                                           .id = world::max_id},
                                             _object_classes),
                                          _edit_context);
                  _entity_creation_context = {};
               }

               ImGui::SeparatorText("Contained Objects");

               if (ImGui::BeginTable("Contained Objects", 3,
                                     ImGuiTableFlags_SizingFixedFit)) {
                  for (int i = 0; i < sector->objects.size(); ++i) {
                     ImGui::PushID(i);

                     bool hovered = false;

                     ImGui::TableNextRow();

                     ImGui::TableNextColumn();
                     ImGui::Text(sector->objects[i].c_str());

                     hovered |= ImGui::IsItemHovered();

                     ImGui::TableNextColumn();
                     if (ImGui::SmallButton("X")) {
                        _edit_stack_world
                           .apply(edits::make_delete_sector_object(sector->id, i, _world),
                                  _edit_context);
                     }

                     hovered |= ImGui::IsItemHovered();

                     if (hovered) {
                        for (const auto& object : _world.objects) {
                           if (object.name == sector->objects[i]) {
                              _interaction_targets.hovered_entity = object.id;
                           }
                        }
                     }

                     ImGui::PopID();
                  }

                  ImGui::EndTable();
               }

               if (ImGui::Button("Auto-Fill Sector", {ImGui::CalcItemWidth(), 0.0f})) {
                  _edit_stack_world.apply(
                     edits::make_set_value(&sector->objects,
                                           world::sector_fill(*sector, _world.objects,
                                                              _object_classes)),
                     _edit_context);
               }

               if (ImGui::Button("Add Object", {ImGui::CalcItemWidth(), 0.0f})) {
                  _selection_edit_context.using_add_object_to_sector = true;
                  _selection_edit_context.sector_to_add_object_to = sector->id;
               }

               if (_selection_edit_context.sector_to_add_object_to == sector->id and
                   std::exchange(_selection_edit_context.add_hovered_object, false) and
                   _interaction_targets.hovered_entity and
                   _interaction_targets.hovered_entity->is<world::object_id>()) {
                  if (world::object* object =
                         world::find_entity(_world.objects,
                                            _interaction_targets.hovered_entity
                                               ->get<world::object_id>());
                      object) {
                     bool already_has_object = false;

                     for (const auto& other_object : sector->objects) {
                        if (string::iequals(object->name, other_object)) {
                           already_has_object = true;
                        }
                     }

                     if (not already_has_object) {
                        _edit_stack_world
                           .apply(edits::make_add_sector_object(sector->id,
                                                                object->name),
                                  _edit_context);
                     }
                  }
               }
            }
            else {
               selection_open = false;
            }
         }
         else if (selected.is<world::portal_id>()) {
            world::portal* portal =
               world::find_entity(_world.portals, selected.get<world::portal_id>());

            if (portal) {
               ImGui::SeparatorText("Portal");

               ImGui::InputText("Name", &portal->name, _edit_stack_world,
                                _edit_context, [&](std::string* edited_value) noexcept {
                                   *edited_value =
                                      world::create_unique_name(_world.portals,
                                                                *edited_value);
                                });

               ImGui::DragFloat("Width", &portal->width, _edit_stack_world,
                                _edit_context, 1.0f, 0.25f, 1e10f);
               ImGui::DragFloat("Height", &portal->height, _edit_stack_world,
                                _edit_context, 1.0f, 0.25f, 1e10f);

               ImGui::InputTextAutoComplete(
                  "Linked Sector 1", &portal->sector1, _edit_stack_world,
                  _edit_context, [&]() noexcept {
                     std::array<std::string_view, 6> entries;
                     std::size_t matching_count = 0;

                     for (const auto& sector : _world.sectors) {
                        if (sector.name.contains(portal->sector1)) {
                           if (matching_count == entries.size()) break;

                           entries[matching_count] = sector.name;

                           ++matching_count;
                        }
                     }

                     return entries;
                  });

               if (ImGui::Button("Pick Linked Sector 1",
                                 {ImGui::CalcItemWidth(), 0.0f})) {
                  _selection_edit_tool = selection_edit_tool::pick_sector;
                  _selection_pick_sector_context = {.index = selection_pick_sector_index::_1,
                                                    .id = portal->id};
               }

               ImGui::InputTextAutoComplete(
                  "Linked Sector 2", &portal->sector2, _edit_stack_world,
                  _edit_context, [&]() noexcept {
                     std::array<std::string_view, 6> entries;
                     std::size_t matching_count = 0;

                     for (const auto& sector : _world.sectors) {
                        if (sector.name.contains(portal->sector2)) {
                           if (matching_count == entries.size()) break;

                           entries[matching_count] = sector.name;

                           ++matching_count;
                        }
                     }

                     return entries;
                  });

               if (ImGui::Button("Pick Linked Sector 2",
                                 {ImGui::CalcItemWidth(), 0.0f})) {
                  _selection_edit_tool = selection_edit_tool::pick_sector;
                  _selection_pick_sector_context = {.index = selection_pick_sector_index::_2,
                                                    .id = portal->id};
               }

               ImGui::DragQuat("Rotation", &portal->rotation, _edit_stack_world,
                               _edit_context);
               ImGui::DragFloat3("Position", &portal->position,
                                 _edit_stack_world, _edit_context);
            }
            else {
               selection_open = false;
            }
         }
         else if (selected.is<world::hintnode_id>()) {
            world::hintnode* hintnode =
               world::find_entity(_world.hintnodes,
                                  selected.get<world::hintnode_id>());

            if (hintnode) {
               ImGui::SeparatorText("Hintnode");

               ImGui::InputText("Name", &hintnode->name, _edit_stack_world,
                                _edit_context, [&](std::string* edited_value) noexcept {
                                   *edited_value =
                                      world::create_unique_name(_world.hintnodes,
                                                                *edited_value);
                                });
               ImGui::LayerPick("Layer", &hintnode->layer, _edit_stack_world,
                                _edit_context);

               ImGui::Separator();

               ImGui::DragQuat("Rotation", &hintnode->rotation,
                               _edit_stack_world, _edit_context);
               ImGui::DragFloat3("Position", &hintnode->position,
                                 _edit_stack_world, _edit_context);

               ImGui::Separator();

               ImGui::EnumSelect(
                  "Type", &hintnode->type, _edit_stack_world, _edit_context,
                  {world::hintnode_type::snipe, world::hintnode_type::patrol,
                   world::hintnode_type::cover, world::hintnode_type::access,
                   world::hintnode_type::jet_jump, world::hintnode_type::mine,
                   world::hintnode_type::land, world::hintnode_type::fortification,
                   world::hintnode_type::vehicle_cover});

               const world::hintnode_traits hintnode_traits =
                  world::get_hintnode_traits(hintnode->type);

               if (hintnode_traits.has_command_post) {
                  if (ImGui::BeginCombo("Command Post",
                                        hintnode->command_post.c_str())) {
                     for (const auto& object : _world.objects) {
                        if (object.name.empty()) continue;

                        const assets::odf::definition& definition =
                           *_object_classes[object.class_handle].definition;

                        if (string::iequals(definition.header.class_label,
                                            "commandpost")) {
                           if (ImGui::Selectable(object.name.c_str())) {
                              _edit_stack_world
                                 .apply(edits::make_set_value(&hintnode->command_post,
                                                              object.name),
                                        _edit_context);
                           }
                        }
                     }
                     ImGui::EndCombo();
                  }
               }

               if (hintnode_traits.has_primary_stance) {
                  ImGui::EditFlags("Primary Stance", &hintnode->primary_stance,
                                   _edit_stack_world, _edit_context,
                                   {{"Stand", world::stance_flags::stand},
                                    {"Crouch", world::stance_flags::crouch},
                                    {"Prone", world::stance_flags::prone}});
               }

               if (hintnode_traits.has_secondary_stance) {
                  ImGui::EditFlags("Secondary Stance", &hintnode->secondary_stance,
                                   _edit_stack_world, _edit_context,
                                   {{"Stand", world::stance_flags::stand},
                                    {"Crouch", world::stance_flags::crouch},
                                    {"Prone", world::stance_flags::prone},
                                    {"Left", world::stance_flags::left},
                                    {"Right", world::stance_flags::right}});
               }

               if (hintnode_traits.has_mode) {
                  ImGui::EnumSelect("Mode", &hintnode->mode, _edit_stack_world,
                                    _edit_context,
                                    {world::hintnode_mode::none,
                                     world::hintnode_mode::attack,
                                     world::hintnode_mode::defend,
                                     world::hintnode_mode::both});
               }

               if (hintnode_traits.has_radius) {
                  ImGui::DragFloat("Radius", &hintnode->radius,
                                   _edit_stack_world, _edit_context, 0.25f, 0.0f,
                                   1e10f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
               }
            }
            else {
               selection_open = false;
            }
         }
         else if (selected.is<world::barrier_id>()) {
            world::barrier* barrier =
               world::find_entity(_world.barriers, selected.get<world::barrier_id>());

            if (barrier) {
               ImGui::SeparatorText("Barrier");

               ImGui::InputText("Name", &barrier->name, _edit_stack_world,
                                _edit_context, [&](std::string* edited_value) noexcept {
                                   *edited_value =
                                      world::create_unique_name(_world.barriers,
                                                                *edited_value);
                                });

               ImGui::DragBarrierRotation("Rotation", &barrier->rotation_angle,
                                          _edit_stack_world, _edit_context);

               ImGui::DragFloat3("Position", &barrier->position,
                                 _edit_stack_world, _edit_context, 0.25f);

               ImGui::DragFloat2XZ("Size", &barrier->size, _edit_stack_world,
                                   _edit_context, 1.0f, 0.0f, 1e10f);

               ImGui::EditFlags("Flags", &barrier->flags, _edit_stack_world,
                                _edit_context,
                                {{"Soldier", world::ai_path_flags::soldier},
                                 {"Hover", world::ai_path_flags::hover},
                                 {"Small", world::ai_path_flags::small},
                                 {"Medium", world::ai_path_flags::medium},
                                 {"Huge", world::ai_path_flags::huge},
                                 {"Flyer", world::ai_path_flags::flyer}});
            }
            else {
               selection_open = false;
            }
         }
         else if (selected.is<world::planning_hub_id>()) {
            world::planning_hub* hub =
               world::find_entity(_world.planning_hubs,
                                  selected.get<world::planning_hub_id>());

            if (hub) {
               ImGui::SeparatorText("AI Planning Hub");

               ImGui::InputText("Name", &hub->name, _edit_stack_world, _edit_context,
                                [&](std::string* edited_value) noexcept {
                                   *edited_value =
                                      world::create_unique_name(_world.planning_hubs,
                                                                *edited_value);
                                });

               ImGui::DragFloat3("Position", &hub->position, _edit_stack_world,
                                 _edit_context);

               ImGui::DragFloat("Radius", &hub->radius, _edit_stack_world,
                                _edit_context, 1.0f, 0.0f, 1e10f);

               ImGui::SeparatorText("Branch Weights");

               for (uint32 weights_index = 0;
                    weights_index < hub->weights.size(); ++weights_index) {
                  world::planning_branch_weights& weights =
                     hub->weights[weights_index];
                  world::planning_hub& target_hub =
                     _world.planning_hubs[weights.hub_index];
                  world::planning_connection& connection =
                     _world.planning_connections[weights.connection_index];

                  ImGui::BeginGroup();
                  ImGui::PushID(static_cast<int32>(target_hub.id));
                  ImGui::PushID(static_cast<int32>(connection.id));

                  if (ImGui::TreeNode("BranchWeight", "%s - %s",
                                      target_hub.name.c_str(),
                                      connection.name.c_str())) {
                     const auto weight_slider =
                        [&](const char* name,
                            float world::planning_branch_weights::* member) {
                           float value = weights.*member;

                           if (ImGui::SliderFloat(name, &value, 0.0f, 100.0f, "%.1f",
                                                  ImGuiSliderFlags_AlwaysClamp)) {
                              _edit_stack_world
                                 .apply(edits::make_set_vector_value(&hub->weights, weights_index,
                                                                     member, value),
                                        _edit_context);
                           }

                           if (ImGui::IsItemDeactivatedAfterEdit()) {
                              _edit_stack_world.close_last();
                           }
                        };

                     weight_slider("Soldier", &world::planning_branch_weights::soldier);
                     weight_slider("Hover", &world::planning_branch_weights::hover);
                     weight_slider("Small", &world::planning_branch_weights::small);
                     weight_slider("Medium", &world::planning_branch_weights::medium);
                     weight_slider("Huge", &world::planning_branch_weights::huge);
                     weight_slider("Flyer", &world::planning_branch_weights::flyer);

                     ImGui::Spacing();

                     ImGui::Separator();

                     if (ImGui::Button("Remove Branch Weight")) {
                        _edit_stack_world.apply(edits::make_delete_branch_weight(&hub->weights,
                                                                                 weights_index),
                                                _edit_context);

                        weights_index -= 1; // Adjust loop index.
                     }

                     ImGui::TreePop();
                  }

                  if (ImGui::BeginItemTooltip()) {
                     ImGui::Text("Soldier: %.1f", weights.soldier);
                     ImGui::Text("Hover: %.1f", weights.hover);
                     ImGui::Text("Small: %.1f", weights.small);
                     ImGui::Text("Medium: %.1f", weights.medium);
                     ImGui::Text("Huge: %.1f", weights.huge);
                     ImGui::Text("Flyer: %.1f", weights.flyer);

                     ImGui::EndTooltip();
                  }

                  ImGui::PopID();
                  ImGui::PopID();
                  ImGui::EndGroup();

                  if (ImGui::IsItemHovered()) {
                     _tool_visualizers.add_highlight(target_hub.id,
                                                     _settings.graphics.hover_color);
                     _tool_visualizers.add_highlight(connection.id,
                                                     _settings.graphics.hover_color);
                  }
               }

               if (ImGui::Button("New Branch Weight", {ImGui::CalcItemWidth(), 0.0f})) {
                  _world_draw_mask.planning_hubs = true;
                  _world_draw_mask.planning_connections = true;

                  _selection_edit_tool = selection_edit_tool::add_branch_weight;
                  _selection_add_branch_weight_context = {.from_hub_id = hub->id};
               }
            }
            else {
               selection_open = false;
            }
         }
         else if (selected.is<world::planning_connection_id>()) {
            world::planning_connection* connection =
               world::find_entity(_world.planning_connections,
                                  selected.get<world::planning_connection_id>());

            if (connection) {
               ImGui::SeparatorText("AI Planning Connection");

               ImGui::InputText("Name", &connection->name, _edit_stack_world,
                                _edit_context, [&](std::string* edited_value) noexcept {
                                   *edited_value =
                                      world::create_unique_name(_world.planning_connections,
                                                                *edited_value);
                                });

               ImGui::Text("Start: %s",
                           _world.planning_hubs[connection->start_hub_index]
                              .name.c_str());
               ImGui::Text("End: %s",
                           _world.planning_hubs[connection->end_hub_index].name.c_str());

               ImGui::EditFlags("Flags", &connection->flags, _edit_stack_world,
                                _edit_context,
                                {{"Soldier", world::ai_path_flags::soldier},
                                 {"Hover", world::ai_path_flags::hover},
                                 {"Small", world::ai_path_flags::small},
                                 {"Medium", world::ai_path_flags::medium},
                                 {"Huge", world::ai_path_flags::huge},
                                 {"Flyer", world::ai_path_flags::flyer}});

               ImGui::Separator();

               ImGui::Checkbox("Jump", &connection->jump, _edit_stack_world,
                               _edit_context);
               ImGui::SameLine();
               ImGui::Checkbox("Jet Jump", &connection->jet_jump,
                               _edit_stack_world, _edit_context);
               ImGui::SameLine();
               ImGui::Checkbox("One Way", &connection->one_way,
                               _edit_stack_world, _edit_context);

               bool is_dynamic = connection->dynamic_group != 0;

               if (ImGui::Checkbox("Dynamic", &is_dynamic)) {
                  _edit_stack_world.apply(edits::make_set_value(&connection->dynamic_group,
                                                                is_dynamic ? int8{1}
                                                                           : int8{0}),
                                          _edit_context);
               }

               if (is_dynamic) {
                  ImGui::SliderInt("Dynamic Group", &connection->dynamic_group,
                                   _edit_stack_world, _edit_context, 1, 8, "%d",
                                   ImGuiSliderFlags_AlwaysClamp);
               }
            }
            else {
               selection_open = false;
            }
         }
         else if (selected.is<world::boundary_id>()) {
            world::boundary* boundary =
               world::find_entity(_world.boundaries,
                                  selected.get<world::boundary_id>());

            if (boundary) {
               ImGui::SeparatorText("Boundary");

               ImGui::InputText("Name", &boundary->name, _edit_stack_world,
                                _edit_context, [&](std::string* edited_value) noexcept {
                                   *edited_value =
                                      world::create_unique_name(_world.boundaries,
                                                                *edited_value);
                                });
            }
            else {
               selection_open = false;
            }
         }
         else if (selected.is<world::measurement_id>()) {
            world::measurement* measurement =
               world::find_entity(_world.measurements,
                                  selected.get<world::measurement_id>());

            if (measurement) {
               ImGui::SeparatorText("Measurement");

               ImGui::InputText("Name", &measurement->name, _edit_stack_world,
                                _edit_context, [](std::string*) noexcept {});
               ImGui::DragFloat3("Start", &measurement->start,
                                 _edit_stack_world, _edit_context, 0.25f);
               ImGui::DragFloat3("End", &measurement->end, _edit_stack_world,
                                 _edit_context, 0.25f);
               ImGui::LabelText("Length", "%.2fm",
                                distance(measurement->start, measurement->end));
            }
            else {
               selection_open = false;
            }
         }
         else if (selected.is<world::block_id>()) {
            const world::block_id block_id = selected.get<world::block_id>();
            const std::optional<uint32> block_index =
               world::find_block(_world.blocks, block_id);

            if (block_index) {
               ImGui::SeparatorText("Block");

               ImGui::LayerPick("Layer",
                                &world::get_block_layer(_world.blocks,
                                                        block_id.type(), *block_index),
                                _edit_stack_world, _edit_context);

               if (block_id.type() == world::block_type::custom) {
                  const world::block_description_custom& block =
                     _world.blocks.custom.description[*block_index];

                  switch (block.mesh_description.type) {
                  case world::block_custom_mesh_type::stairway: {
                     const world::block_custom_mesh_description_stairway& stairway =
                        block.mesh_description.stairway;

                     float new_step_height = stairway.step_height;
                     float new_first_step_offset = stairway.first_step_offset;

                     ImGui::BeginGroup();

                     ImGui::DragFloat("Step Height", &new_step_height,
                                      0.125f * 0.25f, 0.125f, 1.0f, "%.3f",
                                      ImGuiSliderFlags_NoRoundToFormat);

                     ImGui::DragFloat("First Step Offset",
                                      &new_first_step_offset, 0.125f);

                     ImGui::EndGroup();

                     if (ImGui::IsItemEdited()) {
                        new_step_height = std::max(new_step_height, 0.015625f);

                        _edit_stack_world
                           .apply(edits::make_set_block_custom_metrics(
                                     *block_index, block.rotation, block.position,
                                     world::block_custom_mesh_description_stairway{
                                        .size = stairway.size,
                                        .step_height = new_step_height,
                                        .first_step_offset = new_first_step_offset}),
                                  _edit_context);
                     }

                     if (ImGui::IsItemDeactivated()) {
                        _edit_stack_world.close_last();
                     }
                  } break;
                  case world::block_custom_mesh_type::stairway_floating: {
                     const world::block_custom_mesh_description_stairway_floating& stairway =
                        block.mesh_description.stairway_floating;

                     float new_step_height = stairway.step_height;
                     float new_first_step_offset = stairway.first_step_offset;

                     ImGui::BeginGroup();

                     ImGui::DragFloat("Step Height", &new_step_height,
                                      0.125f * 0.25f, 0.125f, 1.0f, "%.3f",
                                      ImGuiSliderFlags_NoRoundToFormat);

                     ImGui::DragFloat("First Step Offset",
                                      &new_first_step_offset, 0.125f);

                     ImGui::EndGroup();

                     if (ImGui::IsItemEdited()) {
                        new_step_height = std::max(new_step_height, 0.015625f);

                        _edit_stack_world
                           .apply(edits::make_set_block_custom_metrics(
                                     *block_index, block.rotation, block.position,
                                     world::block_custom_mesh_description_stairway_floating{
                                        .size = stairway.size,
                                        .step_height = new_step_height,
                                        .first_step_offset = new_first_step_offset}),
                                  _edit_context);
                     }

                     if (ImGui::IsItemDeactivated()) {
                        _edit_stack_world.close_last();
                     }
                  } break;
                  case world::block_custom_mesh_type::ring: {
                     world::block_custom_mesh_description_ring ring =
                        block.mesh_description.ring;

                     const uint16 min_segments = 3;
                     const uint16 max_segments = 256;

                     bool checkbox_clicked = false;

                     ImGui::BeginGroup();

                     ImGui::DragFloat("Inner Radius", &ring.inner_radius, 1.0f,
                                      0.0f, 1e10f);
                     ImGui::DragFloat("Outer Radius", &ring.outer_radius, 1.0f,
                                      0.0f, 1e10f);
                     ImGui::SliderScalar("Segments", ImGuiDataType_U16, &ring.segments,
                                         &min_segments, &max_segments);
                     checkbox_clicked |=
                        ImGui::Checkbox("Flat Shading", &ring.flat_shading);
                     ImGui::DragFloat("Texture Loops", &ring.texture_loops);

                     ImGui::EndGroup();

                     ImGui::SetItemTooltip("How many times the texture wraps "
                                           "around the ring in the "
                                           "Unwrapped Texture Mode.");

                     if (ImGui::IsItemEdited() or checkbox_clicked) {
                        ring.inner_radius = std::max(ring.inner_radius, 0.0f);
                        ring.outer_radius = std::max(ring.outer_radius, 0.0f);
                        ring.segments = std::max(ring.segments, min_segments);

                        if (ring.inner_radius == 0.0f and ring.outer_radius == 0.0f) {
                           ring.inner_radius = 1.0f;
                        }

                        _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                                   *block_index, block.rotation,
                                                   block.position, ring),
                                                _edit_context);
                     }

                     if (ImGui::IsItemDeactivated() or checkbox_clicked) {
                        _edit_stack_world.close_last();
                     }
                  } break;
                  case world::block_custom_mesh_type::beveled_box: {
                     world::block_custom_mesh_description_beveled_box box =
                        block.mesh_description.beveled_box;

                     bool checkbox_clicked = false;

                     ImGui::BeginGroup();

                     ImGui::DragFloat("Bevel Amount", &box.amount, 0.0625f, 0.0f,
                                      std::min(std::min(box.size.x, box.size.y),
                                               box.size.z));
                     checkbox_clicked |= ImGui::Checkbox("Bevel Top", &box.bevel_top);
                     checkbox_clicked |=
                        ImGui::Checkbox("Bevel Sides", &box.bevel_sides);
                     checkbox_clicked |=
                        ImGui::Checkbox("Bevel Bottom", &box.bevel_bottom);

                     ImGui::EndGroup();

                     if (ImGui::IsItemEdited() or checkbox_clicked) {
                        box.amount = std::max(box.amount, 0.0f);

                        _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                                   *block_index, block.rotation,
                                                   block.position, box),
                                                _edit_context);
                     }

                     if (ImGui::IsItemDeactivated() or checkbox_clicked) {
                        _edit_stack_world.close_last();
                     }
                  } break;
                  case world::block_custom_mesh_type::curve: {
                     world::block_custom_mesh_description_curve curve =
                        block.mesh_description.curve;

                     const uint16 min_segments = 3;
                     const uint16 max_segments = 256;

                     ImGui::BeginGroup();

                     ImGui::DragFloat("Width", &curve.width, 1.0f, 0.0f, 1e10f);
                     ImGui::DragFloat("Height", &curve.height, 1.0f, 0.0f, 1e10f);
                     ImGui::SliderScalar("Segments", ImGuiDataType_U16, &curve.segments,
                                         &min_segments, &max_segments);
                     ImGui::DragFloat("Texture Loops", &curve.texture_loops);

                     ImGui::SetItemTooltip("How many times the texture wraps "
                                           "around the ring in the "
                                           "Unwrapped Texture Mode.");

                     ImGui::EndGroup();

                     if (ImGui::IsItemEdited()) {
                        curve.segments = std::max(curve.segments, min_segments);

                        if (curve.width == 0.0f and curve.height == 0.0f) {
                           curve.height = 1.0f;
                        }

                        _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                                   *block_index, block.rotation,
                                                   block.position, curve),
                                                _edit_context);
                     }

                     if (ImGui::IsItemDeactivated()) {
                        _edit_stack_world.close_last();
                     }
                  } break;
                  case world::block_custom_mesh_type::cylinder: {
                     world::block_custom_mesh_description_cylinder cylinder =
                        block.mesh_description.cylinder;

                     const uint16 min_segments = 3;
                     const uint16 max_segments = 256;

                     bool checkbox_clicked = false;

                     ImGui::BeginGroup();

                     ImGui::SliderScalar("Segments", ImGuiDataType_U16,
                                         &cylinder.segments, &min_segments,
                                         &max_segments);
                     checkbox_clicked |=
                        ImGui::Checkbox("Flat Shading", &cylinder.flat_shading);
                     ImGui::DragFloat("Texture Loops", &cylinder.texture_loops);

                     ImGui::SetItemTooltip("How many times the texture wraps "
                                           "around the ring in the "
                                           "Unwrapped Texture Mode.");

                     ImGui::EndGroup();

                     if (ImGui::IsItemEdited() or checkbox_clicked) {
                        cylinder.segments = std::max(cylinder.segments, min_segments);

                        _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                                   *block_index, block.rotation,
                                                   block.position, cylinder),
                                                _edit_context);
                     }

                     if (ImGui::IsItemDeactivated() or checkbox_clicked) {
                        _edit_stack_world.close_last();
                     }
                  } break;
                  case world::block_custom_mesh_type::cone: {
                     world::block_custom_mesh_description_cone cone =
                        block.mesh_description.cone;

                     const uint16 min_segments = 3;
                     const uint16 max_segments = 256;

                     bool checkbox_clicked = false;

                     ImGui::BeginGroup();

                     ImGui::SliderScalar("Segments", ImGuiDataType_U16, &cone.segments,
                                         &min_segments, &max_segments);
                     checkbox_clicked |=
                        ImGui::Checkbox("Flat Shading", &cone.flat_shading);

                     ImGui::EndGroup();

                     if (ImGui::IsItemEdited() or checkbox_clicked) {
                        cone.segments = std::max(cone.segments, min_segments);

                        _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                                   *block_index, block.rotation,
                                                   block.position, cone),
                                                _edit_context);
                     }

                     if (ImGui::IsItemDeactivated() or checkbox_clicked) {
                        _edit_stack_world.close_last();
                     }
                  } break;
                  case world::block_custom_mesh_type::arch: {
                     world::block_custom_mesh_description_arch arch =
                        block.mesh_description.arch;

                     const uint16 min_segments = 1;
                     const uint16 max_segments = 64;

                     ImGui::BeginGroup();

                     ImGui::DragFloat("Crown Length", &arch.crown_length, 0.5f,
                                      0.0f, arch.span_length);
                     ImGui::DragFloat("Crown Height", &arch.crown_height, 0.125f,
                                      0.0f, arch.size.y * 2.0f - arch.curve_height);
                     ImGui::DragFloat("Curve Height", &arch.curve_height, 0.5f,
                                      0.0f, arch.size.y * 2.0f);
                     ImGui::DragFloat("Span Length", &arch.span_length, 0.5f,
                                      0.0f, arch.size.x * 2.0f);
                     ImGui::SliderScalar("Segments", ImGuiDataType_U16, &arch.segments,
                                         &min_segments, &max_segments);

                     ImGui::EndGroup();

                     if (ImGui::IsItemEdited()) {
                        arch.segments = std::max(arch.segments, min_segments);

                        _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                                   *block_index, block.rotation,
                                                   block.position, arch),
                                                _edit_context);
                     }

                     if (ImGui::IsItemDeactivated()) {
                        _edit_stack_world.close_last();
                     }
                  } break;
                  }
               }
            }
            else {
               selection_open = false;
            }
         }
      }
      else {
         ui_show_world_selection_multi_editor();
      }

      ImGui::SeparatorText("Selection Tools");

      if (ImGui::Button("Move Selection", {ImGui::CalcItemWidth(), 0.0f})) {
         _selection_edit_tool = selection_edit_tool::move;
      }

      if (ImGui::Button("Move Selection with Cursor", {ImGui::CalcItemWidth(), 0.0f})) {
         _selection_edit_tool = selection_edit_tool::move_with_cursor;
      }

      if (ImGui::Button("Rotate Selection", {ImGui::CalcItemWidth(), 0.0f})) {
         _selection_edit_tool = selection_edit_tool::rotate;
         _rotate_selection_amount = {0.0f, 0.0f, 0.0f};
      }

      if (ImGui::Button("Rotate Selection Around Centre",
                        {ImGui::CalcItemWidth(), 0.0f})) {
         _selection_edit_tool = selection_edit_tool::rotate_around_centre;
         _rotate_selection_amount = {0.0f, 0.0f, 0.0f};

         _rotate_selection_centre =
            world::selection_centre_for_rotate_around(_world,
                                                      _interaction_targets.selection);
      }

      if (ImGui::Button("Resize Selected Entity", {ImGui::CalcItemWidth(), 0.0f})) {
         _selection_edit_tool = selection_edit_tool::resize_entity;
      }

      if (ImGui::Button("Match Transform", {ImGui::CalcItemWidth(), 0.0f})) {
         _selection_edit_tool = selection_edit_tool::match_transform;
         _selection_match_transform_context = {};
      }

      if (ImGui::Button("Align Selection", {ImGui::CalcItemWidth(), 0.0f})) {
         align_selection(_editor_grid_size);
      }

      if (ImGui::Button("Align Selection (Terrain Grid)",
                        {ImGui::CalcItemWidth(), 0.0f})) {
         align_selection(_world.terrain.grid_scale);
      }

      if (ImGui::Button("Ground Selection", {ImGui::CalcItemWidth(), 0.0f})) {
         ground_selection();
      }

      if (ImGui::Button("Set Selection Layer", {ImGui::CalcItemWidth(), 0.0f})) {
         _selection_edit_tool = selection_edit_tool::set_layer;
      }

      if (ImGui::Button("Focus on Selection", {ImGui::CalcItemWidth(), 0.0f})) {
         focus_on_selection();
      }
   }

   ImGui::End();

   if (not selection_open) _interaction_targets.selection.clear();

   if (_hotkeys_view_show) {
      ImGui::Begin("Hotkeys");

      ImGui::SeparatorText("Entity Editing");

      ImGui::Text("Move Selection");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing", "Move Selection")));

      ImGui::Text("Move Selection with Cursor");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing",
                                "Move Selection with Cursor")));

      ImGui::Text("Rotate Selection");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing", "Rotate Selection")));

      ImGui::Text("Rotate Selection Around Centre");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing",
                                "Rotate Selection Around Centre")));

      ImGui::Text("Resize Selected Entity");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing", "Resize Selected Entity")));

      ImGui::Text("Match Transform");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing", "Match Transform")));

      ImGui::Text("Align Selection");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing", "Align Selection")));

      ImGui::Text("Align Selection (Terrain Grid)");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing",
                                "Align Selection (Terrain Grid)")));

      ImGui::Text("Hide/Unhide Selection");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing", "Hide/Unhide Selection")));

      ImGui::Text("Ground Selection");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing", "Ground Selection")));

      ImGui::Text("New Entity from Selection");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing",
                                "New Entity from Selection")));

      ImGui::Text("Set Selection Layer");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing", "Set Selection Layer")));

      ImGui::Text("Focus on Selection");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing", "Focus on Selection")));

      ImGui::End();
   }
}

void world_edit::ui_show_world_selection_multi_editor() noexcept
{
   using world::multi_select_flags;

   world::multi_select_flags flags = multi_select_flags::all;
   world::multi_select_properties properties;

   for (const world::selected_entity& selected : _interaction_targets.selection) {
      if (selected.is<world::object_id>()) {
         const world::object* object =
            find_entity(_world.objects, selected.get<world::object_id>());

         if (not object) continue;

         multi_select_flags object_flags =
            multi_select_flags::has_layer | multi_select_flags::has_rotation |
            multi_select_flags::has_position | multi_select_flags::has_object;

         properties.layer.integrate(object->layer);
         properties.rotation.integrate(object->rotation);
         properties.position.integrate(object->position);

         properties.object.class_name.integrate(object->class_name);
         properties.object.team.integrate(object->team);
         properties.object.instance_properties.integrate(object->instance_properties);

         flags &= object_flags;
      }
      else if (selected.is<world::light_id>()) {
         const world::light* light =
            find_entity(_world.lights, selected.get<world::light_id>());

         if (not light) continue;

         multi_select_flags light_flags =
            multi_select_flags::has_layer | multi_select_flags::has_rotation |
            multi_select_flags::has_position | multi_select_flags::has_light;

         properties.layer.integrate(light->layer);
         properties.rotation.integrate(light->rotation);
         properties.position.integrate(light->position);

         properties.light.color.integrate(light->color);
         properties.light.static_.integrate(light->static_);
         properties.light.shadow_caster.integrate(light->shadow_caster);
         properties.light.specular_caster.integrate(light->specular_caster);
         properties.light.type.integrate(light->light_type);

         if (light->light_type == world::light_type::point or
             light->light_type == world::light_type::spot) {
            light_flags |= multi_select_flags::has_light_range;

            properties.light.range.integrate(light->range);

            if (light->light_type == world::light_type::spot) {
               light_flags |= multi_select_flags::has_light_cone_props;

               properties.light.inner_cone_angle.integrate(light->inner_cone_angle);
               properties.light.outer_cone_angle.integrate(light->outer_cone_angle);
            }
         }

         properties.light.texture.integrate(light->texture);

         if (not light->texture.empty()) {
            light_flags |= multi_select_flags::has_light_texture_addressing;

            properties.light.texture_addressing.integrate(light->texture_addressing);
         }

         if (world::is_directional_light(*light) and not light->texture.empty()) {
            light_flags |= multi_select_flags::has_light_directional_texture;

            properties.light.directional_texture_tiling.integrate(
               light->directional_texture_tiling);
            properties.light.directional_texture_offset.integrate(
               light->directional_texture_offset);
         }

         if (world::is_region_light(*light)) {
            light_flags |= multi_select_flags::has_light_region;

            properties.light.region_rotation.integrate(light->region_rotation);

            switch (light->light_type) {
            case world::light_type::directional_region_box: {
               light_flags |= multi_select_flags::has_box_size;

               properties.box_size.integrate(light->region_size);
            } break;
            case world::light_type::directional_region_sphere: {
               light_flags |= multi_select_flags::has_radius;

               properties.radius.integrate(length(light->region_size));
            } break;
            case world::light_type::directional_region_cylinder: {
               light_flags |= multi_select_flags::has_height |
                              multi_select_flags::has_radius;

               properties.height.integrate(light->region_size.y * 2.0f);
               properties.radius.integrate(
                  length(float2{light->region_size.x, light->region_size.z}));
            } break;
            default:
               break;
            }
         }

         if (not _settings.ui.hide_extra_light_properties) {
            light_flags |= multi_select_flags::has_light_ps2_blend_mode;

            if (light->light_type == world::light_type::spot or
                is_directional_light(*light)) {
               properties.light.ps2_blend_mode.integrate(light->ps2_blend_mode);
            }

            if (light->light_type == world::light_type::spot) {
               // Covered by multi_select_flags::has_light_cone_props
               properties.light.bidirectional.integrate(light->bidirectional);
            }
         }

         flags &= light_flags;
      }
      else if (selected.is<world::path_id_node_mask>()) {
         const auto& [id, node_mask] = selected.get<world::path_id_node_mask>();

         const world::path* path = find_entity(_world.paths, id);

         if (not path) continue;

         multi_select_flags path_flags =
            multi_select_flags::has_layer | multi_select_flags::has_rotation |
            multi_select_flags::has_position | multi_select_flags::has_path;

         properties.layer.integrate(path->layer);

         properties.path.type.integrate(path->type);

         if (path->type == world::path_type::patrol) {
            path_flags |= multi_select_flags::has_path_spline_type;

            properties.path.spline_type.integrate(path->spline_type);
         }

         properties.path.properties.integrate(path->properties);

         const std::size_t node_count =
            std::min(path->nodes.size(), world::max_path_nodes);

         for (uint32 node_index = 0; node_index < node_count; ++node_index) {
            if (not node_mask[node_index]) continue;

            properties.rotation.integrate(path->nodes[node_index].rotation);
            properties.position.integrate(path->nodes[node_index].position);

            properties.path.node_properties.integrate(path->nodes[node_index].properties);
         }

         flags &= path_flags;
      }
      else if (selected.is<world::region_id>()) {
         const world::region* region =
            find_entity(_world.regions, selected.get<world::region_id>());

         if (not region) continue;

         multi_select_flags region_flags =
            multi_select_flags::has_layer | multi_select_flags::has_rotation |
            multi_select_flags::has_position | multi_select_flags::has_region;

         properties.layer.integrate(region->layer);
         properties.rotation.integrate(region->rotation);
         properties.position.integrate(region->position);

         properties.region.description.integrate(region->description);

         switch (region->shape) {
         case world::region_shape::box: {
            region_flags |= multi_select_flags::has_box_size;

            properties.box_size.integrate(region->size);
         } break;
         case world::region_shape::sphere: {
            region_flags |= multi_select_flags::has_radius;

            properties.radius.integrate(length(region->size));
         } break;
         case world::region_shape::cylinder: {
            region_flags |=
               multi_select_flags::has_height | multi_select_flags::has_radius;

            properties.height.integrate(region->size.y * 2.0f);
            properties.radius.integrate(
               length(float2{region->size.x, region->size.z}));
         } break;
         }

         properties.region.shape.integrate(region->shape);

         const world::region_type region_type =
            world::get_region_type(region->description);

         properties.region.type.integrate(region_type);

         switch (region_type) {
         case world::region_type::typeless: {
         } break;
         case world::region_type::soundstream: {
            const world::sound_stream_properties sound_stream =
               world::unpack_region_sound_stream(region->description);

            properties.region.sound_stream.sound_name.integrate(sound_stream.sound_name);
            properties.region.sound_stream.min_distance_divisor.integrate(
               sound_stream.min_distance_divisor);

            region_flags |= multi_select_flags::has_region_sound_stream;
         } break;
         case world::region_type::soundstatic: {
            const world::sound_stream_properties sound_static =
               world::unpack_region_sound_static(region->description);

            properties.region.sound_static.sound_name.integrate(sound_static.sound_name);
            properties.region.sound_static.min_distance_divisor.integrate(
               sound_static.min_distance_divisor);

            region_flags |= multi_select_flags::has_region_sound_static;
         } break;
         case world::region_type::soundspace: {
            const world::sound_space_properties sound_space =
               world::unpack_region_sound_space(region->description);

            properties.region.sound_space.sound_space_name.integrate(
               sound_space.sound_space_name);

            region_flags |= multi_select_flags::has_region_sound_space;
         } break;
         case world::region_type::soundtrigger: {
            const world::sound_trigger_properties sound_trigger =
               world::unpack_region_sound_trigger(region->description);

            properties.region.sound_trigger.region_name.integrate(sound_trigger.region_name);

            region_flags |= multi_select_flags::has_region_sound_trigger;
         } break;
         case world::region_type::foleyfx: {
            const world::foley_fx_region_properties foley_fx =
               world::unpack_region_foley_fx(region->description);

            properties.region.foley_fx.group_id.integrate(foley_fx.group_id);

            region_flags |= multi_select_flags::has_region_foley_fx;
         } break;
         case world::region_type::shadow: {
            const world::shadow_region_properties shadow =
               world::unpack_region_shadow(region->description);

            properties.region.shadow.directional0.integrate(
               shadow.directional0.value_or(1.0f));
            properties.region.shadow.directional1.integrate(
               shadow.directional1.value_or(1.0f));
            properties.region.shadow.color_top.integrate(
               shadow.color_top.value_or(_world.global_lights.ambient_sky_color));
            properties.region.shadow.color_bottom.integrate(
               shadow.color_bottom.value_or(_world.global_lights.ambient_ground_color));
            properties.region.shadow.env_map.integrate(shadow.env_map);

            region_flags |= multi_select_flags::has_region_shadow;
         } break;
         case world::region_type::mapbounds: {
         } break;
         case world::region_type::rumble: {
            const world::rumble_region_properties rumble =
               world::unpack_region_rumble(region->description);

            properties.region.rumble.rumble_class.integrate(rumble.rumble_class);
            properties.region.rumble.particle_effect.integrate(rumble.particle_effect);

            region_flags |= multi_select_flags::has_region_rumble;
         } break;
         case world::region_type::reflection:
         case world::region_type::rainshadow:
         case world::region_type::danger: {
         } break;
         case world::region_type::damage_region: {
            const world::damage_region_properties damage =
               world::unpack_region_damage(region->description);

            properties.region.damage.damage_rate.integrate(
               damage.damage_rate.value_or(0.0f));
            properties.region.damage.person_scale.integrate(
               damage.person_scale.value_or(1.0f));
            properties.region.damage.animal_scale.integrate(
               damage.animal_scale.value_or(1.0f));
            properties.region.damage.droid_scale.integrate(
               damage.droid_scale.value_or(1.0f));
            properties.region.damage.vehicle_scale.integrate(
               damage.vehicle_scale.value_or(1.0f));
            properties.region.damage.building_scale.integrate(
               damage.building_scale.value_or(1.0f));
            properties.region.damage.building_dead_scale.integrate(
               damage.building_dead_scale.value_or(1.0f));
            properties.region.damage.building_unbuilt_scale.integrate(
               damage.building_unbuilt_scale.value_or(1.0f));

            region_flags |= multi_select_flags::has_region_damage;
         } break;
         case world::region_type::ai_vis: {
            const world::ai_vis_region_properties ai_vis =
               world::unpack_region_ai_vis(region->description);

            properties.region.ai_vis.crouch.integrate(ai_vis.crouch.value_or(1.0f));
            properties.region.ai_vis.stand.integrate(ai_vis.stand.value_or(1.0f));

            region_flags |= multi_select_flags::has_region_ai_vis;
         } break;
         case world::region_type::colorgrading: {
            const world::colorgrading_region_properties colorgrading =
               world::unpack_region_colorgrading(region->description);

            properties.region.colorgrading.config.integrate(colorgrading.config);
            properties.region.colorgrading.fade_length.integrate(colorgrading.fade_length);

            region_flags |= multi_select_flags::has_region_colorgrading;
         } break;
         }

         flags &= region_flags;
      }
      else if (selected.is<world::sector_id>()) {
         const world::sector* sector =
            find_entity(_world.sectors, selected.get<world::sector_id>());

         if (not sector) continue;

         multi_select_flags sector_flags =
            multi_select_flags::has_height | multi_select_flags::has_sector;

         properties.height.integrate(sector->height);

         properties.sector.base.integrate(sector->base);

         flags &= sector_flags;
      }
      else if (selected.is<world::portal_id>()) {
         const world::portal* portal =
            find_entity(_world.portals, selected.get<world::portal_id>());

         if (not portal) continue;

         multi_select_flags portal_flags =
            multi_select_flags::has_rotation |
            multi_select_flags::has_position | multi_select_flags::has_width |
            multi_select_flags::has_height | multi_select_flags::has_portal;

         properties.rotation.integrate(portal->rotation);
         properties.position.integrate(portal->position);
         properties.width.integrate(portal->width);
         properties.height.integrate(portal->height);

         properties.portal.sector1.integrate(portal->sector1);
         properties.portal.sector2.integrate(portal->sector2);

         flags &= portal_flags;
      }
      else if (selected.is<world::hintnode_id>()) {
         const world::hintnode* hintnode =
            find_entity(_world.hintnodes, selected.get<world::hintnode_id>());

         if (not hintnode) continue;

         multi_select_flags hintnode_flags =
            multi_select_flags::has_layer | multi_select_flags::has_rotation |
            multi_select_flags::has_position | multi_select_flags::has_hintnode;

         properties.layer.integrate(hintnode->layer);
         properties.rotation.integrate(hintnode->rotation);
         properties.position.integrate(hintnode->position);

         properties.hintnode.type.integrate(hintnode->type);

         const world::hintnode_traits hintnode_traits =
            world::get_hintnode_traits(hintnode->type);

         if (hintnode_traits.has_command_post) {
            hintnode_flags |= multi_select_flags::has_hintnode_command_post;

            properties.hintnode.command_post.integrate(hintnode->command_post);
         }

         if (hintnode_traits.has_primary_stance) {
            hintnode_flags |= multi_select_flags::has_hintnode_primary_stance;

            properties.hintnode.primary_stance.integrate(hintnode->primary_stance);
         }

         if (hintnode_traits.has_secondary_stance) {
            hintnode_flags |= multi_select_flags::has_hintnode_secondary_stance;

            properties.hintnode.secondary_stance.integrate(hintnode->secondary_stance);
         }

         if (hintnode_traits.has_mode) {
            hintnode_flags |= multi_select_flags::has_hintnode_mode;

            properties.hintnode.mode.integrate(hintnode->mode);
         }

         if (hintnode_traits.has_radius) {
            hintnode_flags |= multi_select_flags::has_hintnode_radius;

            properties.hintnode.radius.integrate(hintnode->radius);
         }

         flags &= hintnode_flags;
      }
      else if (selected.is<world::barrier_id>()) {
         const world::barrier* barrier =
            find_entity(_world.barriers, selected.get<world::barrier_id>());

         if (not barrier) continue;

         multi_select_flags barrier_flags = multi_select_flags::has_position |
                                            multi_select_flags::has_ai_flags |
                                            multi_select_flags::has_barrier;

         properties.position.integrate(barrier->position);
         properties.ai_flags.integrate(barrier->flags);

         properties.barrier.rotation_angle.integrate(barrier->rotation_angle);
         properties.barrier.size.integrate(barrier->size);

         flags &= barrier_flags;
      }
      else if (selected.is<world::planning_hub_id>()) {
         const world::planning_hub* hub =
            find_entity(_world.planning_hubs, selected.get<world::planning_hub_id>());

         if (not hub) continue;

         multi_select_flags hub_flags =
            multi_select_flags::has_position | multi_select_flags::has_radius;

         properties.position.integrate(hub->position);
         properties.radius.integrate(hub->radius);

         flags &= hub_flags;
      }
      else if (selected.is<world::planning_connection_id>()) {
         const world::planning_connection* connection =
            find_entity(_world.planning_connections,
                        selected.get<world::planning_connection_id>());

         if (not connection) continue;

         multi_select_flags connection_flags =
            multi_select_flags::has_ai_flags |
            multi_select_flags::has_planning_connection;

         properties.ai_flags.integrate(connection->flags);
         properties.planning_connection.jump.integrate(connection->jump);
         properties.planning_connection.jet_jump.integrate(connection->jet_jump);
         properties.planning_connection.one_way.integrate(connection->one_way);
         properties.planning_connection.dynamic.integrate(connection->dynamic_group != 0);
         properties.planning_connection.dynamic_group.integrate(connection->dynamic_group);

         flags &= connection_flags;
      }
      else if (selected.is<world::boundary_id>()) {
         const world::boundary* boundary =
            find_entity(_world.boundaries, selected.get<world::boundary_id>());

         if (not boundary) continue;

         multi_select_flags boundary_flags = multi_select_flags::has_boundary;

         flags &= boundary_flags;
      }
      else if (selected.is<world::measurement_id>()) {
         const world::measurement* measurement =
            find_entity(_world.measurements, selected.get<world::measurement_id>());

         if (not measurement) continue;

         multi_select_flags measurement_flags = multi_select_flags::has_measurement;

         flags &= measurement_flags;
      }
      else if (selected.is<world::block_id>()) {
         const world::block_id block_id = selected.get<we::world::block_id>();
         const std::optional<uint32> block_index = find_block(_world.blocks, block_id);

         if (not block_index) continue;

         multi_select_flags block_flags =
            multi_select_flags::has_layer | multi_select_flags::has_block;

         properties.layer.integrate(
            get_block_layer(_world.blocks, block_id.type(), *block_index));

         switch (block_id.type()) {
         case world::block_type::box: {
            block_flags |= multi_select_flags::has_rotation |
                           multi_select_flags::has_position |
                           multi_select_flags::has_box_size;

            const world::block_description_box& box =
               _world.blocks.boxes.description[*block_index];

            properties.rotation.integrate(box.rotation);
            properties.position.integrate(box.position);
            properties.box_size.integrate(box.size);
         } break;
         case world::block_type::ramp: {
            block_flags |= multi_select_flags::has_rotation |
                           multi_select_flags::has_position |
                           multi_select_flags::has_box_size;

            const world::block_description_ramp& ramp =
               _world.blocks.ramps.description[*block_index];

            properties.rotation.integrate(ramp.rotation);
            properties.position.integrate(ramp.position);
            properties.box_size.integrate(ramp.size);
         } break;
         case world::block_type::quad: {
         } break;
         case world::block_type::custom: {
            block_flags |= multi_select_flags::has_rotation |
                           multi_select_flags::has_position;

            const world::block_description_custom& block =
               _world.blocks.custom.description[*block_index];

            properties.rotation.integrate(block.rotation);
            properties.position.integrate(block.position);

            switch (block.mesh_description.type) {
            case world::block_custom_mesh_type::stairway: {
               block_flags |= multi_select_flags::has_box_size |
                              multi_select_flags::has_block_step_properties;

               const world::block_custom_mesh_description_stairway& stairway =
                  block.mesh_description.stairway;

               properties.box_size.integrate(stairway.size);
               properties.block.step_height.integrate(stairway.step_height);
               properties.block.first_step_offset.integrate(stairway.first_step_offset);
            } break;
            case world::block_custom_mesh_type::stairway_floating: {
               block_flags |= multi_select_flags::has_box_size |
                              multi_select_flags::has_block_step_properties;

               const world::block_custom_mesh_description_stairway_floating& stairway =
                  block.mesh_description.stairway_floating;

               properties.box_size.integrate(stairway.size);
               properties.block.step_height.integrate(stairway.step_height);
               properties.block.first_step_offset.integrate(stairway.first_step_offset);
            } break;
            case world::block_custom_mesh_type::ring: {
               block_flags |= multi_select_flags::has_height |
                              multi_select_flags::has_block_segments |
                              multi_select_flags::has_block_flat_shading |
                              multi_select_flags::has_block_texture_loops |
                              multi_select_flags::has_block_ring_properties;

               const world::block_custom_mesh_description_ring& ring =
                  block.mesh_description.ring;

               properties.height.integrate(ring.height);
               properties.block.segments.integrate(ring.segments);
               properties.block.flat_shading.integrate(ring.flat_shading);
               properties.block.texture_loops.integrate(ring.texture_loops);

               properties.block.ring.inner_radius.integrate(ring.inner_radius);
               properties.block.ring.outer_radius.integrate(ring.outer_radius);
            } break;
            case world::block_custom_mesh_type::beveled_box: {
               block_flags |= multi_select_flags::has_box_size |
                              multi_select_flags::has_block_beveled_box_properties;

               const world::block_custom_mesh_description_beveled_box& beveled_box =
                  block.mesh_description.beveled_box;

               properties.box_size.integrate(beveled_box.size);
               properties.block.beveled_box.amount.integrate(beveled_box.amount);
               properties.block.beveled_box.bevel_top.integrate(beveled_box.bevel_top);
               properties.block.beveled_box.bevel_sides.integrate(beveled_box.bevel_sides);
               properties.block.beveled_box.bevel_bottom.integrate(beveled_box.bevel_bottom);
            } break;
            case world::block_custom_mesh_type::curve: {
               block_flags |= multi_select_flags::has_width |
                              multi_select_flags::has_height |
                              multi_select_flags::has_block_segments |
                              multi_select_flags::has_block_texture_loops;

               const world::block_custom_mesh_description_curve& curve =
                  block.mesh_description.curve;

               properties.width.integrate(curve.width);
               properties.height.integrate(curve.height);
               properties.block.segments.integrate(curve.segments);
               properties.block.texture_loops.integrate(curve.texture_loops);
            } break;
            case world::block_custom_mesh_type::cylinder: {
               block_flags |= multi_select_flags::has_box_size |
                              multi_select_flags::has_block_segments |
                              multi_select_flags::has_block_flat_shading |
                              multi_select_flags::has_block_texture_loops;

               const world::block_custom_mesh_description_cylinder& cylinder =
                  block.mesh_description.cylinder;

               properties.box_size.integrate(cylinder.size);
               properties.block.segments.integrate(cylinder.segments);
               properties.block.flat_shading.integrate(cylinder.flat_shading);
               properties.block.texture_loops.integrate(cylinder.texture_loops);
            } break;
            case world::block_custom_mesh_type::cone: {
               block_flags |= multi_select_flags::has_box_size |
                              multi_select_flags::has_block_segments |
                              multi_select_flags::has_block_flat_shading;

               const world::block_custom_mesh_description_cone& cone =
                  block.mesh_description.cone;

               properties.box_size.integrate(cone.size);
               properties.block.segments.integrate(cone.segments);
               properties.block.flat_shading.integrate(cone.flat_shading);
            } break;
            case world::block_custom_mesh_type::arch: {
               block_flags |= multi_select_flags::has_box_size |
                              multi_select_flags::has_block_segments |
                              multi_select_flags::has_block_arch_properties;

               const world::block_custom_mesh_description_arch& arch =
                  block.mesh_description.arch;

               properties.box_size.integrate(arch.size);
               properties.block.segments.integrate(arch.segments);

               properties.block.arch.crown_length.integrate(arch.crown_length);
               properties.block.arch.crown_height.integrate(arch.crown_height);
               properties.block.arch.curve_height.integrate(arch.curve_height);
               properties.block.arch.span_length.integrate(arch.span_length);
            } break;
            }
         } break;
         case world::block_type::hemisphere: {
            block_flags |= multi_select_flags::has_rotation |
                           multi_select_flags::has_position |
                           multi_select_flags::has_box_size;

            const world::block_description_hemisphere& hemisphere =
               _world.blocks.hemispheres.description[*block_index];

            properties.rotation.integrate(hemisphere.rotation);
            properties.position.integrate(hemisphere.position);
            properties.box_size.integrate(hemisphere.size);
         } break;
         case world::block_type::pyramid: {
            block_flags |= multi_select_flags::has_rotation |
                           multi_select_flags::has_position |
                           multi_select_flags::has_box_size;

            const world::block_description_pyramid& pyramid =
               _world.blocks.pyramids.description[*block_index];

            properties.rotation.integrate(pyramid.rotation);
            properties.position.integrate(pyramid.position);
            properties.box_size.integrate(pyramid.size);
         } break;
         case world::block_type::terrain_cut_box: {
            block_flags |= multi_select_flags::has_rotation |
                           multi_select_flags::has_position |
                           multi_select_flags::has_box_size;

            const world::block_description_terrain_cut_box& box =
               _world.blocks.terrain_cut_boxes.description[*block_index];

            properties.rotation.integrate(box.rotation);
            properties.position.integrate(box.position);
            properties.box_size.integrate(box.size);
         } break;
         }

         flags &= block_flags;
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_layer)) {
      if (int8 layer = properties.layer.value_or(0);
          ImGui::BeginCombo("Layer",
                            properties.layer.is_different() ? "<different>"
                            : layer >= _world.layer_descriptions.size()
                               ? ""
                               : _world.layer_descriptions[layer].name.c_str())) {

         for (int8 i = 0; i < std::ssize(_world.layer_descriptions); ++i) {
            if (ImGui::Selectable(_world.layer_descriptions[i].name.c_str(),
                                  not properties.layer.is_different() and layer == i)) {
               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.layer.count());

               const int8 new_layer = i;

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::object_id>()) {
                     world::object* object =
                        world::find_entity(_world.objects,
                                           selected.get<world::object_id>());

                     if (not object) continue;

                     edit_bundle.push_back(
                        edits::make_set_value(&object->layer, new_layer));
                  }
                  else if (selected.is<world::light_id>()) {
                     world::light* light =
                        world::find_entity(_world.lights,
                                           selected.get<world::light_id>());

                     if (not light) continue;

                     edit_bundle.push_back(
                        edits::make_set_value(&light->layer, new_layer));
                  }
                  else if (selected.is<world::path_id_node_mask>()) {
                     const auto& [id, node_mask] =
                        selected.get<world::path_id_node_mask>();

                     world::path* path = world::find_entity(_world.paths, id);

                     if (not path) continue;

                     edit_bundle.push_back(
                        edits::make_set_value(&path->layer, new_layer));
                  }
                  else if (selected.is<world::region_id>()) {
                     world::region* region =
                        world::find_entity(_world.regions,
                                           selected.get<world::region_id>());

                     if (not region) continue;

                     edit_bundle.push_back(
                        edits::make_set_value(&region->layer, new_layer));
                  }
                  else if (selected.is<world::hintnode_id>()) {
                     world::hintnode* hintnode =
                        world::find_entity(_world.hintnodes,
                                           selected.get<world::hintnode_id>());

                     if (not hintnode) continue;

                     edit_bundle.push_back(
                        edits::make_set_value(&hintnode->layer, new_layer));
                  }
                  else if (selected.is<world::block_id>()) {
                     const world::block_id block_id =
                        selected.get<world::block_id>();
                     const std::optional<uint32> block_index =
                        world::find_block(_world.blocks, block_id);

                     if (not block_index) continue;

                     edit_bundle.push_back(
                        edits::make_set_value(&world::get_block_layer(_world.blocks,
                                                                      block_id.type(),
                                                                      *block_index),
                                              new_layer));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context, {.closed = true});
            }
         }

         ImGui::EndCombo();
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_rotation)) {
      if (quaternion rotation = properties.rotation.value_or({});
          ImGui::DragQuat("Rotation", &rotation, 0.001f,
                          properties.rotation.is_different() ? "<different>" : nullptr)) {
         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.rotation.count());

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::object_id>()) {
               world::object* object =
                  world::find_entity(_world.objects, selected.get<world::object_id>());

               if (not object) continue;

               edit_bundle.push_back(edits::make_set_value(&object->rotation, rotation));
            }
            else if (selected.is<world::light_id>()) {
               world::light* light =
                  world::find_entity(_world.lights, selected.get<world::light_id>());

               if (not light) continue;

               edit_bundle.push_back(edits::make_set_value(&light->rotation, rotation));
            }
            else if (selected.is<world::path_id_node_mask>()) {
               const auto& [id, node_mask] =
                  selected.get<world::path_id_node_mask>();

               world::path* path = world::find_entity(_world.paths, id);

               if (not path) continue;

               const std::size_t node_count =
                  std::min(path->nodes.size(), world::max_path_nodes);

               for (uint32 node_index = 0; node_index < node_count; ++node_index) {
                  if (not node_mask[node_index]) continue;

                  edit_bundle.push_back(
                     edits::make_set_vector_value(&path->nodes, node_index,
                                                  &world::path::node::rotation,
                                                  rotation));
               }
            }
            else if (selected.is<world::region_id>()) {
               world::region* region =
                  world::find_entity(_world.regions, selected.get<world::region_id>());

               if (not region) continue;

               edit_bundle.push_back(edits::make_set_value(&region->rotation, rotation));
            }
            else if (selected.is<world::portal_id>()) {
               world::portal* portal =
                  world::find_entity(_world.portals, selected.get<world::portal_id>());

               if (not portal) continue;

               edit_bundle.push_back(edits::make_set_value(&portal->rotation, rotation));
            }
            else if (selected.is<world::hintnode_id>()) {
               world::hintnode* hintnode =
                  world::find_entity(_world.hintnodes,
                                     selected.get<world::hintnode_id>());

               if (not hintnode) continue;

               edit_bundle.push_back(
                  edits::make_set_value(&hintnode->rotation, rotation));
            }
            else if (selected.is<world::block_id>()) {
               const world::block_id block_id = selected.get<world::block_id>();
               const std::optional<uint32> block_index =
                  world::find_block(_world.blocks, block_id);

               if (not block_index) continue;

               switch (block_id.type()) {
               case world::block_type::box: {
                  const world::block_description_box& box =
                     _world.blocks.boxes.description[*block_index];

                  edit_bundle.push_back(
                     edits::make_set_block_box_metrics(*block_index, rotation,
                                                       box.position, box.size));
               } break;
               case world::block_type::ramp: {
                  const world::block_description_ramp& ramp =
                     _world.blocks.ramps.description[*block_index];

                  edit_bundle.push_back(
                     edits::make_set_block_ramp_metrics(*block_index, rotation,
                                                        ramp.position, ramp.size));
               } break;
               case world::block_type::quad: {
               } break;
               case world::block_type::custom: {
                  const world::block_description_custom& block =
                     _world.blocks.custom.description[*block_index];

                  switch (block.mesh_description.type) {
                  case world::block_custom_mesh_type::stairway: {
                     const world::block_custom_mesh_description_stairway& stairway =
                        block.mesh_description.stairway;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, rotation,
                                                             block.position, stairway));
                  } break;
                  case world::block_custom_mesh_type::stairway_floating: {
                     const world::block_custom_mesh_description_stairway_floating& stairway =
                        block.mesh_description.stairway_floating;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, rotation,
                                                             block.position, stairway));
                  } break;
                  case world::block_custom_mesh_type::ring: {
                     const world::block_custom_mesh_description_ring& ring =
                        block.mesh_description.ring;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, rotation,
                                                             block.position, ring));
                  } break;
                  case world::block_custom_mesh_type::beveled_box: {
                     const world::block_custom_mesh_description_beveled_box& beveled_box =
                        block.mesh_description.beveled_box;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, rotation,
                                                             block.position,
                                                             beveled_box));
                  } break;
                  case world::block_custom_mesh_type::curve: {
                     const world::block_custom_mesh_description_curve& curve =
                        block.mesh_description.curve;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, rotation,
                                                             block.position, curve));
                  } break;
                  case world::block_custom_mesh_type::cylinder: {
                     const world::block_custom_mesh_description_cylinder& cylinder =
                        block.mesh_description.cylinder;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, rotation,
                                                             block.position, cylinder));
                  } break;
                  case world::block_custom_mesh_type::cone: {
                     const world::block_custom_mesh_description_cone& cone =
                        block.mesh_description.cone;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, rotation,
                                                             block.position, cone));
                  } break;
                  case world::block_custom_mesh_type::arch: {
                     const world::block_custom_mesh_description_arch& arch =
                        block.mesh_description.arch;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, rotation,
                                                             block.position, arch));
                  } break;
                  }
               } break;
               case world::block_type::hemisphere: {
                  const world::block_description_hemisphere& hemisphere =
                     _world.blocks.hemispheres.description[*block_index];

                  edit_bundle.push_back(
                     edits::make_set_block_hemisphere_metrics(*block_index, rotation,
                                                              hemisphere.position,
                                                              hemisphere.size));
               } break;
               case world::block_type::pyramid: {
                  const world::block_description_pyramid& pyramid =
                     _world.blocks.pyramids.description[*block_index];

                  edit_bundle.push_back(
                     edits::make_set_block_pyramid_metrics(*block_index, rotation,
                                                           pyramid.position,
                                                           pyramid.size));
               } break;
               case world::block_type::terrain_cut_box: {
                  const world::block_description_terrain_cut_box& box =
                     _world.blocks.terrain_cut_boxes.description[*block_index];

                  edit_bundle.push_back(
                     edits::make_set_block_terrain_cut_box_metrics(*block_index, rotation,
                                                                   box.position,
                                                                   box.size));
               } break;
               }
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context);
      }

      if (ImGui::IsItemDeactivatedAfterEdit()) {
         _edit_stack_world.close_last();
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_position)) {
      float position_x = properties.position.x.value_or(0.0f);
      float position_y = properties.position.y.value_or(0.0f);
      float position_z = properties.position.z.value_or(0.0f);

      const float item_inner_spacing = ImGui::GetStyle().ItemInnerSpacing.x;

      const float item_width_one =
         std::max(1.0f,
                  floorf((ImGui::CalcItemWidth() - item_inner_spacing * 2.0f) / 3.0f));
      const float item_width_last =
         std::max(1.0f, floorf(ImGui::CalcItemWidth() -
                               (item_width_one + item_inner_spacing) * 2.0f));

      ImGui::BeginGroup();
      ImGui::PushID("Position");
      ImGui::SetNextItemWidth(item_width_one);
      const bool x_edited =
         ImGui::DragFloat("##X", &position_x, 1.0f, 0.0f, 0.0f,
                          properties.position.x.is_different() ? "<different>"
                                                               : "X:%.3f");

      ImGui::SameLine(0, item_inner_spacing);
      ImGui::SetNextItemWidth(item_width_one);
      const bool y_edited =
         ImGui::DragFloat("##Y", &position_y, 1.0f, 0.0f, 0.0f,
                          properties.position.y.is_different() ? "<different>"
                                                               : "Y:%.3f");

      ImGui::SameLine(0, item_inner_spacing);
      ImGui::SetNextItemWidth(item_width_last);
      const bool z_edited =
         ImGui::DragFloat("##Z", &position_z, 1.0f, 0.0f, 0.0f,
                          properties.position.z.is_different() ? "<different>"
                                                               : "Z:%.3f");
      ImGui::SameLine(0, item_inner_spacing);

      ImGui::PopID();

      ImGui::TextUnformatted("Position");

      ImGui::EndGroup();

      if (x_edited or y_edited or z_edited) {
         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.position.count());

         const auto edit_position = [&](const float3& position) {
            return float3{
               x_edited ? position_x : position.x,
               y_edited ? position_y : position.y,
               z_edited ? position_z : position.z,
            };
         };

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::object_id>()) {
               world::object* object =
                  world::find_entity(_world.objects, selected.get<world::object_id>());

               if (not object) continue;

               edit_bundle.push_back(
                  edits::make_set_value(&object->position,
                                        edit_position(object->position)));
            }
            else if (selected.is<world::light_id>()) {
               world::light* light =
                  world::find_entity(_world.lights, selected.get<world::light_id>());

               if (not light) continue;

               edit_bundle.push_back(
                  edits::make_set_value(&light->position,
                                        edit_position(light->position)));
            }
            else if (selected.is<world::path_id_node_mask>()) {
               const auto& [id, node_mask] =
                  selected.get<world::path_id_node_mask>();

               world::path* path = world::find_entity(_world.paths, id);

               if (not path) continue;

               const std::size_t node_count =
                  std::min(path->nodes.size(), world::max_path_nodes);

               for (uint32 node_index = 0; node_index < node_count; ++node_index) {
                  if (not node_mask[node_index]) continue;

                  edit_bundle.push_back(edits::make_set_vector_value(
                     &path->nodes, node_index, &world::path::node::position,
                     edit_position(path->nodes[node_index].position)));
               }
            }
            else if (selected.is<world::region_id>()) {
               world::region* region =
                  world::find_entity(_world.regions, selected.get<world::region_id>());

               if (not region) continue;

               edit_bundle.push_back(
                  edits::make_set_value(&region->position,
                                        edit_position(region->position)));
            }
            else if (selected.is<world::portal_id>()) {
               world::portal* portal =
                  world::find_entity(_world.portals, selected.get<world::portal_id>());

               if (not portal) continue;

               edit_bundle.push_back(
                  edits::make_set_value(&portal->position,
                                        edit_position(portal->position)));
            }
            else if (selected.is<world::hintnode_id>()) {
               world::hintnode* hintnode =
                  world::find_entity(_world.hintnodes,
                                     selected.get<world::hintnode_id>());

               if (not hintnode) continue;

               edit_bundle.push_back(
                  edits::make_set_value(&hintnode->position,
                                        edit_position(hintnode->position)));
            }
            else if (selected.is<world::barrier_id>()) {
               world::barrier* barrier =
                  world::find_entity(_world.barriers,
                                     selected.get<world::barrier_id>());

               if (not barrier) continue;

               edit_bundle.push_back(
                  edits::make_set_value(&barrier->position,
                                        edit_position(barrier->position)));
            }
            else if (selected.is<world::planning_hub_id>()) {
               world::planning_hub* hub =
                  world::find_entity(_world.planning_hubs,
                                     selected.get<world::planning_hub_id>());

               if (not hub) continue;

               edit_bundle.push_back(
                  edits::make_set_value(&hub->position, edit_position(hub->position)));
            }
            else if (selected.is<world::block_id>()) {
               const world::block_id block_id = selected.get<world::block_id>();
               const std::optional<uint32> block_index =
                  world::find_block(_world.blocks, block_id);

               if (not block_index) continue;

               switch (block_id.type()) {
               case world::block_type::box: {
                  const world::block_description_box& box =
                     _world.blocks.boxes.description[*block_index];

                  edit_bundle.push_back(
                     edits::make_set_block_box_metrics(*block_index, box.rotation,
                                                       edit_position(box.position),
                                                       box.size));
               } break;
               case world::block_type::ramp: {
                  const world::block_description_ramp& ramp =
                     _world.blocks.ramps.description[*block_index];

                  edit_bundle.push_back(
                     edits::make_set_block_ramp_metrics(*block_index, ramp.rotation,
                                                        edit_position(ramp.position),
                                                        ramp.size));
               } break;
               case world::block_type::quad: {
               } break;
               case world::block_type::custom: {
                  const world::block_description_custom& block =
                     _world.blocks.custom.description[*block_index];

                  switch (block.mesh_description.type) {
                  case world::block_custom_mesh_type::stairway: {
                     const world::block_custom_mesh_description_stairway& stairway =
                        block.mesh_description.stairway;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             edit_position(block.position),
                                                             stairway));
                  } break;
                  case world::block_custom_mesh_type::stairway_floating: {
                     const world::block_custom_mesh_description_stairway_floating& stairway =
                        block.mesh_description.stairway_floating;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             edit_position(block.position),
                                                             stairway));
                  } break;
                  case world::block_custom_mesh_type::ring: {
                     const world::block_custom_mesh_description_ring& ring =
                        block.mesh_description.ring;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             edit_position(block.position),
                                                             ring));
                  } break;
                  case world::block_custom_mesh_type::beveled_box: {
                     const world::block_custom_mesh_description_beveled_box& beveled_box =
                        block.mesh_description.beveled_box;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             edit_position(block.position),
                                                             beveled_box));
                  } break;
                  case world::block_custom_mesh_type::curve: {
                     const world::block_custom_mesh_description_curve& curve =
                        block.mesh_description.curve;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             edit_position(block.position),
                                                             curve));
                  } break;
                  case world::block_custom_mesh_type::cylinder: {
                     const world::block_custom_mesh_description_cylinder& cylinder =
                        block.mesh_description.cylinder;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             edit_position(block.position),
                                                             cylinder));
                  } break;
                  case world::block_custom_mesh_type::cone: {
                     const world::block_custom_mesh_description_cone& cone =
                        block.mesh_description.cone;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             edit_position(block.position),
                                                             cone));
                  } break;
                  case world::block_custom_mesh_type::arch: {
                     const world::block_custom_mesh_description_arch& arch =
                        block.mesh_description.arch;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             edit_position(block.position),
                                                             arch));
                  } break;
                  }
               } break;
               case world::block_type::hemisphere: {
                  const world::block_description_hemisphere& hemisphere =
                     _world.blocks.hemispheres.description[*block_index];

                  edit_bundle.push_back(edits::make_set_block_hemisphere_metrics(
                     *block_index, hemisphere.rotation,
                     edit_position(hemisphere.position), hemisphere.size));
               } break;
               case world::block_type::pyramid: {
                  const world::block_description_pyramid& pyramid =
                     _world.blocks.pyramids.description[*block_index];

                  edit_bundle.push_back(
                     edits::make_set_block_pyramid_metrics(*block_index, pyramid.rotation,
                                                           edit_position(pyramid.position),
                                                           pyramid.size));
               } break;
               case world::block_type::terrain_cut_box: {
                  const world::block_description_terrain_cut_box& box =
                     _world.blocks.terrain_cut_boxes.description[*block_index];

                  edit_bundle.push_back(edits::make_set_block_terrain_cut_box_metrics(
                     *block_index, box.rotation, edit_position(box.position), box.size));
               } break;
               }
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context);
      }

      if (ImGui::IsItemDeactivatedAfterEdit()) {
         _edit_stack_world.close_last();
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_box_size)) {
      float size_x = properties.box_size.x.value_or(1.0f);
      float size_y = properties.box_size.y.value_or(1.0f);
      float size_z = properties.box_size.z.value_or(1.0f);

      const float item_inner_spacing = ImGui::GetStyle().ItemInnerSpacing.x;

      const float item_width_one =
         std::max(1.0f,
                  floorf((ImGui::CalcItemWidth() - item_inner_spacing * 2.0f) / 3.0f));
      const float item_width_last =
         std::max(1.0f, floorf(ImGui::CalcItemWidth() -
                               (item_width_one + item_inner_spacing) * 2.0f));

      ImGui::BeginGroup();
      ImGui::PushID("BoxSize");
      ImGui::SetNextItemWidth(item_width_one);
      const bool x_edited =
         ImGui::DragFloat("##X", &size_x, 0.125f, 0.0f, 1e10f,
                          properties.box_size.x.is_different() ? "<different>"
                                                               : "X:%.3f");

      ImGui::SameLine(0, item_inner_spacing);
      ImGui::SetNextItemWidth(item_width_one);
      const bool y_edited =
         ImGui::DragFloat("##Y", &size_y, 0.125f, 0.0f, 1e10f,
                          properties.box_size.y.is_different() ? "<different>"
                                                               : "Y:%.3f");

      ImGui::SameLine(0, item_inner_spacing);
      ImGui::SetNextItemWidth(item_width_last);
      const bool z_edited =
         ImGui::DragFloat("##Z", &size_z, 0.125f, 0.0f, 1e10f,
                          properties.box_size.z.is_different() ? "<different>"
                                                               : "Z:%.3f");
      ImGui::SameLine(0, item_inner_spacing);

      ImGui::PopID();

      ImGui::TextUnformatted("Size");

      ImGui::EndGroup();

      if (x_edited or y_edited or z_edited) {
         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.box_size.count());

         const auto edit_size = [&](const float3& size) {
            return float3{
               x_edited ? size_x : size.x,
               y_edited ? size_y : size.y,
               z_edited ? size_z : size.z,
            };
         };

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::light_id>()) {
               world::light* light =
                  world::find_entity(_world.lights, selected.get<world::light_id>());

               if (not light) continue;

               if (light->light_type == world::light_type::directional_region_box) {
                  edit_bundle.push_back(
                     edits::make_set_value(&light->region_size,
                                           edit_size(light->region_size)));
               }
            }
            else if (selected.is<world::region_id>()) {
               world::region* region =
                  world::find_entity(_world.regions, selected.get<world::region_id>());

               if (not region) continue;

               if (region->shape == world::region_shape::box) {
                  edit_bundle.push_back(
                     edits::make_set_value(&region->size, edit_size(region->size)));
               }
            }
            else if (selected.is<world::block_id>()) {
               const world::block_id block_id = selected.get<world::block_id>();
               const std::optional<uint32> block_index =
                  world::find_block(_world.blocks, block_id);

               if (not block_index) continue;

               switch (block_id.type()) {
               case world::block_type::box: {
                  const world::block_description_box& box =
                     _world.blocks.boxes.description[*block_index];

                  edit_bundle.push_back(
                     edits::make_set_block_box_metrics(*block_index,
                                                       box.rotation, box.position,
                                                       edit_size(box.size)));
               } break;
               case world::block_type::ramp: {
                  const world::block_description_ramp& ramp =
                     _world.blocks.ramps.description[*block_index];

                  edit_bundle.push_back(
                     edits::make_set_block_ramp_metrics(*block_index, ramp.rotation,
                                                        ramp.position,
                                                        edit_size(ramp.size)));
               } break;
               case world::block_type::quad: {
               } break;
               case world::block_type::custom: {
                  const world::block_description_custom& block =
                     _world.blocks.custom.description[*block_index];

                  switch (block.mesh_description.type) {
                  case world::block_custom_mesh_type::stairway: {
                     world::block_custom_mesh_description_stairway stairway =
                        block.mesh_description.stairway;

                     stairway.size = edit_size(stairway.size);

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             block.position, stairway));
                  } break;
                  case world::block_custom_mesh_type::stairway_floating: {
                     world::block_custom_mesh_description_stairway_floating stairway =
                        block.mesh_description.stairway_floating;

                     stairway.size = edit_size(stairway.size);

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             block.position, stairway));
                  } break;
                  case world::block_custom_mesh_type::ring: {
                  } break;
                  case world::block_custom_mesh_type::beveled_box: {
                     world::block_custom_mesh_description_beveled_box beveled_box =
                        block.mesh_description.beveled_box;

                     beveled_box.size = edit_size(beveled_box.size);

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             block.position,
                                                             beveled_box));
                  } break;
                  case world::block_custom_mesh_type::curve: {
                  } break;
                  case world::block_custom_mesh_type::cylinder: {
                     world::block_custom_mesh_description_cylinder cylinder =
                        block.mesh_description.cylinder;

                     cylinder.size = edit_size(cylinder.size);

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             block.position, cylinder));
                  } break;
                  case world::block_custom_mesh_type::cone: {
                     world::block_custom_mesh_description_cone cone =
                        block.mesh_description.cone;

                     cone.size = edit_size(cone.size);

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             block.position, cone));
                  } break;
                  case world::block_custom_mesh_type::arch: {
                     world::block_custom_mesh_description_arch arch =
                        block.mesh_description.arch;

                     arch.size = edit_size(arch.size);

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             block.position, arch));
                  } break;
                  }
               } break;
               case world::block_type::hemisphere: {
                  const world::block_description_hemisphere& hemisphere =
                     _world.blocks.hemispheres.description[*block_index];

                  edit_bundle.push_back(edits::make_set_block_hemisphere_metrics(
                     *block_index, hemisphere.rotation, hemisphere.position,
                     edit_size(hemisphere.size)));
               } break;
               case world::block_type::pyramid: {
                  const world::block_description_pyramid& pyramid =
                     _world.blocks.pyramids.description[*block_index];

                  edit_bundle.push_back(
                     edits::make_set_block_pyramid_metrics(*block_index, pyramid.rotation,
                                                           pyramid.position,
                                                           edit_size(pyramid.size)));
               } break;
               case world::block_type::terrain_cut_box: {
                  const world::block_description_terrain_cut_box& box =
                     _world.blocks.terrain_cut_boxes.description[*block_index];

                  edit_bundle.push_back(edits::make_set_block_terrain_cut_box_metrics(
                     *block_index, box.rotation, box.position, edit_size(box.size)));
               } break;
               }
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context);
      }

      if (ImGui::IsItemDeactivatedAfterEdit()) {
         _edit_stack_world.close_last();
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_width)) {
      if (float width = properties.width.value_or(1.0f);
          ImGui::DragFloat("Width", &width, 0.125f, 0.0f, 1e10f,
                           properties.width.is_different() ? "<different>" : "%.3f")) {
         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.width.count());

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::portal_id>()) {
               world::portal* portal =
                  world::find_entity(_world.portals, selected.get<world::portal_id>());

               if (not portal) continue;

               edit_bundle.push_back(edits::make_set_value(&portal->width, width));
            }
            else if (selected.is<world::block_id>()) {
               const world::block_id block_id = selected.get<world::block_id>();
               const std::optional<uint32> block_index =
                  world::find_block(_world.blocks, block_id);

               if (not block_index) continue;

               switch (block_id.type()) {
               case world::block_type::box: {
               } break;
               case world::block_type::ramp: {
               } break;
               case world::block_type::quad: {
               } break;
               case world::block_type::custom: {
                  const world::block_description_custom& block =
                     _world.blocks.custom.description[*block_index];

                  switch (block.mesh_description.type) {
                  case world::block_custom_mesh_type::stairway: {
                  } break;
                  case world::block_custom_mesh_type::stairway_floating: {
                  } break;
                  case world::block_custom_mesh_type::ring: {
                  } break;
                  case world::block_custom_mesh_type::beveled_box: {
                  } break;
                  case world::block_custom_mesh_type::curve: {
                     world::block_custom_mesh_description_curve curve =
                        block.mesh_description.curve;

                     curve.width = width;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             block.position, curve));
                  } break;
                  case world::block_custom_mesh_type::cylinder: {
                  } break;
                  case world::block_custom_mesh_type::cone: {
                  } break;
                  case world::block_custom_mesh_type::arch: {
                  } break;
                  }
               } break;
               case world::block_type::hemisphere: {
               } break;
               case world::block_type::pyramid: {
               } break;
               case world::block_type::terrain_cut_box: {
               } break;
               }
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context);

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_height)) {
      if (float height = properties.height.value_or(1.0f);
          ImGui::DragFloat("Height", &height, 0.125f, 0.0f, 1e10f,
                           properties.height.is_different() ? "<different>" : "%.3f")) {
         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.height.count());

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::light_id>()) {
               world::light* light =
                  world::find_entity(_world.lights, selected.get<world::light_id>());

               if (not light) continue;

               if (light->light_type == world::light_type::directional_region_cylinder) {
                  edit_bundle.push_back(
                     edits::make_set_value(&light->region_size.y, height / 2.0f));
               }
            }
            else if (selected.is<world::region_id>()) {
               world::region* region =
                  world::find_entity(_world.regions, selected.get<world::region_id>());

               if (not region) continue;

               if (region->shape == world::region_shape::cylinder) {
                  edit_bundle.push_back(
                     edits::make_set_value(&region->size.y, height / 2.0f));
               }
            }
            else if (selected.is<world::sector_id>()) {
               world::sector* sector =
                  world::find_entity(_world.sectors, selected.get<world::sector_id>());

               if (not sector) continue;

               edit_bundle.push_back(edits::make_set_value(&sector->height, height));
            }
            else if (selected.is<world::portal_id>()) {
               world::portal* portal =
                  world::find_entity(_world.portals, selected.get<world::portal_id>());

               if (not portal) continue;

               edit_bundle.push_back(edits::make_set_value(&portal->height, height));
            }
            else if (selected.is<world::block_id>()) {
               const world::block_id block_id = selected.get<world::block_id>();
               const std::optional<uint32> block_index =
                  world::find_block(_world.blocks, block_id);

               if (not block_index) continue;

               switch (block_id.type()) {
               case world::block_type::box: {
               } break;
               case world::block_type::ramp: {
               } break;
               case world::block_type::quad: {
               } break;
               case world::block_type::custom: {
                  const world::block_description_custom& block =
                     _world.blocks.custom.description[*block_index];

                  switch (block.mesh_description.type) {
                  case world::block_custom_mesh_type::stairway: {
                  } break;
                  case world::block_custom_mesh_type::stairway_floating: {
                  } break;
                  case world::block_custom_mesh_type::ring: {
                     world::block_custom_mesh_description_ring ring =
                        block.mesh_description.ring;

                     ring.height = height;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             block.position, ring));
                  } break;
                  case world::block_custom_mesh_type::beveled_box: {
                  } break;
                  case world::block_custom_mesh_type::curve: {
                     world::block_custom_mesh_description_curve curve =
                        block.mesh_description.curve;

                     curve.height = height;

                     edit_bundle.push_back(
                        edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                             block.position, curve));

                  } break;
                  case world::block_custom_mesh_type::cylinder: {
                  } break;
                  case world::block_custom_mesh_type::cone: {
                  } break;
                  case world::block_custom_mesh_type::arch: {
                  } break;
                  }
               } break;
               case world::block_type::hemisphere: {
               } break;
               case world::block_type::pyramid: {
               } break;
               case world::block_type::terrain_cut_box: {
               } break;
               }
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context);

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_radius)) {
      if (float radius = properties.radius.value_or(1.0f);
          ImGui::DragFloat("Radius", &radius, 0.125f, 0.0f, 1e10f,
                           properties.radius.is_different() ? "<different>" : "%.3f")) {
         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.radius.count());

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::light_id>()) {
               world::light* light =
                  world::find_entity(_world.lights, selected.get<world::light_id>());

               if (not light) continue;

               if (light->light_type == world::light_type::directional_region_sphere) {
                  const float radius_sq = radius * radius;
                  const float size = sqrt(radius_sq / 3.0f);

                  edit_bundle.push_back(edits::make_set_value(&light->region_size,
                                                              {size, size, size}));
               }
               else if (light->light_type ==
                        world::light_type::directional_region_cylinder) {
                  const float radius_sq = radius * radius;
                  const float size = sqrt(radius_sq / 2.0f);

                  edit_bundle.push_back(
                     edits::make_set_value(&light->region_size,
                                           float3{size, light->region_size.y, size}));
               }
            }
            else if (selected.is<world::region_id>()) {
               world::region* region =
                  world::find_entity(_world.regions, selected.get<world::region_id>());

               if (not region) continue;

               if (region->shape == world::region_shape::sphere) {
                  const float radius_sq = radius * radius;
                  const float size = sqrt(radius_sq / 3.0f);

                  edit_bundle.push_back(
                     edits::make_set_value(&region->size, {size, size, size}));
               }
               else if (region->shape == world::region_shape::cylinder) {
                  const float radius_sq = radius * radius;
                  const float size = sqrt(radius_sq / 2.0f);

                  edit_bundle.push_back(
                     edits::make_set_value(&region->size,
                                           float3{size, region->size.y, size}));
               }
            }
            else if (selected.is<world::planning_hub_id>()) {
               world::planning_hub* hub =
                  world::find_entity(_world.planning_hubs,
                                     selected.get<world::planning_hub_id>());

               if (not hub) continue;

               edit_bundle.push_back(edits::make_set_value(&hub->radius, radius));
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context);
      }

      if (ImGui::IsItemDeactivatedAfterEdit()) {
         _edit_stack_world.close_last();
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_object)) {
      ImGui::Separator();

      // Class Name
      {
         const std::optional<lowercase_string> new_class_name =
            ui_object_class_pick_widget_untracked(
               lowercase_string{properties.object.class_name.value_or("")},
               properties.object.class_name.is_different() ? "<different>" : nullptr);

         if (new_class_name) {
            bool first_edit = true;

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::object_id>()) {
                  world::object* object =
                     world::find_entity(_world.objects,
                                        selected.get<world::object_id>());

                  if (not object) continue;

                  _edit_stack_world.apply(edits::make_set_class_name(object, *new_class_name,
                                                                     _object_classes),
                                          _edit_context,
                                          {.transparent =
                                              not std::exchange(first_edit, false)});

                  std::vector<world::instance_property> new_instance_properties =
                     world::make_object_instance_properties(
                        *_object_classes[object->class_handle].definition,
                        object->instance_properties);

                  if (new_instance_properties != object->instance_properties) {
                     _edit_stack_world.apply(edits::make_set_value(&object->instance_properties,
                                                                   std::move(new_instance_properties)),
                                             _edit_context, {.transparent = true});
                  }
               }
            }
         }
      }

      if (int team = properties.object.team.value_or(0);
          ImGui::SliderInt("Team", &team, 0, 15,
                           properties.object.team.is_different() ? "<different>" : "%d",
                           ImGuiSliderFlags_AlwaysClamp)) {
         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.object.team.count());

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::object_id>()) {
               world::object* object =
                  world::find_entity(_world.objects, selected.get<world::object_id>());

               if (not object) continue;

               edit_bundle.push_back(edits::make_set_value(&object->team, team));
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context);

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }

      if (properties.object.instance_properties.has_common_keys()) {
         ImGui::Separator();

         const std::span<const world::instance_property> ref_properties =
            properties.object.instance_properties.properties_or_default();

         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.object.instance_properties.count());

         bool property_edited = false;

         ImGui::BeginGroup();
         ImGui::PushStyleColor(ImGuiCol_TextDisabled,
                               ImGui::GetStyleColorVec4(ImGuiCol_Text));

         for (uint32 i = 0; i < ref_properties.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));

            const world::instance_property& prop = ref_properties[i];

            const bool is_different_value =
               properties.object.instance_properties.is_different_value(i);

            const std::string_view value =
               is_different_value ? "" : std::string_view{prop.value};
            const char* hint = is_different_value ? "<different>" : "";

            absl::InlinedVector<char, 256> value_buffer{value.begin(), value.end()};

            bool edited = false;

            if (prop.key.contains("Path")) {
               edited = ImGui::InputTextWithHintAutoComplete(
                  prop.key.c_str(), hint, &value_buffer, [&]() noexcept {
                     std::array<std::string_view, 6> entries;

                     std::size_t matching_count = 0;

                     for (auto& path : _world.paths) {
                        if (not path.name.contains(value)) {
                           continue;
                        }

                        entries[matching_count] = path.name;

                        ++matching_count;

                        if (matching_count == entries.size()) break;
                     }

                     return entries;
                  });
            }
            else if (prop.key.contains("Region")) {
               edited = ImGui::InputTextWithHintAutoComplete(
                  prop.key.c_str(), hint, &value_buffer, [&]() noexcept {
                     std::array<std::string_view, 6> entries;

                     std::size_t matching_count = 0;

                     for (auto& region : _world.regions) {
                        if (not region.description.contains(value)) {
                           continue;
                        }

                        entries[matching_count] = region.description;

                        ++matching_count;

                        if (matching_count == entries.size()) break;
                     }

                     return entries;
                  });
            }
            else {
               edited = ImGui::InputTextWithHint(prop.key.c_str(), hint, &value_buffer);
            }

            if (edited) {
               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::object_id>()) {
                     world::object* object =
                        world::find_entity(_world.objects,
                                           selected.get<world::object_id>());

                     if (not object) continue;

                     edit_bundle.push_back(edits::make_set_vector_value(
                        &object->instance_properties, i, &world::instance_property::value,
                        std::string{value_buffer.begin(), value_buffer.end()}));
                  }
               }

               property_edited = true;
            }

            ImGui::PopID();
         }

         ImGui::PopStyleColor();
         ImGui::EndGroup();

         if (property_edited) {
            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_light)) {
      ImGui::Separator();

      // Color
      {
         float3 color = properties.light.color.value_or(float3{0.5f, 0.5f, 0.5f});

         const float slider_width =
            ((ImGui::CalcItemWidth() - ImGui::GetFrameHeight() -
              ImGui::GetStyle().ItemInnerSpacing.x) -
             ImGui::GetStyle().ItemInnerSpacing.x * 2.0f) /
            3.0f;

         const bool is_different = properties.light.color.is_different();
         bool edited = false;

         ImGui::BeginGroup();

         ImGui::SetNextItemWidth(slider_width);
         edited |= ImGui::DragFloat("##R", &color.x, 1.0f / 255.0f, 0.0f, 1e10f,
                                    is_different ? "<different>" : "R: %.3f");

         ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
         ImGui::SetNextItemWidth(slider_width);
         edited |= ImGui::DragFloat("##G", &color.y, 1.0f / 255.0f, 0.0f, 1e10f,
                                    is_different ? "<different>" : "G: %.3f");
         ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
         ImGui::SetNextItemWidth(slider_width);
         edited |= ImGui::DragFloat("##B", &color.z, 1.0f / 255.0f, 0.0f, 1e10f,
                                    is_different ? "<different>" : "B: %.3f");

         ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
         edited |= ImGui::ColorEdit3("##Picker", &color.x,
                                     ImGuiColorEditFlags_NoInputs |
                                        ImGuiColorEditFlags_Float |
                                        ImGuiColorEditFlags_HDR);

         ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
         ImGui::TextUnformatted("Color");

         ImGui::EndGroup();

         if (edited) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.light.color.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::light_id>()) {
                  world::light* light =
                     world::find_entity(_world.lights,
                                        selected.get<world::light_id>());

                  if (not light) continue;

                  edit_bundle.push_back(edits::make_set_value(&light->color, color));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }

      if (bool static_ = properties.light.static_.value_or(false);
          ImGui::CheckboxTristate("Static", &static_,
                                  properties.light.static_.is_different())) {

         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.light.static_.count());

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::light_id>()) {
               world::light* light =
                  world::find_entity(_world.lights, selected.get<world::light_id>());

               if (not light) continue;

               edit_bundle.push_back(edits::make_set_value(&light->static_, static_));
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context, {.closed = true});
      }

      ImGui::SameLine();

      if (bool shadow_caster = properties.light.shadow_caster.value_or(false);
          ImGui::CheckboxTristate("Shadow Caster", &shadow_caster,
                                  properties.light.shadow_caster.is_different())) {

         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.light.shadow_caster.count());

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::light_id>()) {
               world::light* light =
                  world::find_entity(_world.lights, selected.get<world::light_id>());

               if (not light) continue;

               edit_bundle.push_back(
                  edits::make_set_value(&light->shadow_caster, shadow_caster));
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context, {.closed = true});
      }

      ImGui::SameLine();

      if (bool specular_caster = properties.light.specular_caster.value_or(false);
          ImGui::CheckboxTristate("Specular Caster", &specular_caster,
                                  properties.light.specular_caster.is_different())) {

         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.light.specular_caster.count());

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::light_id>()) {
               world::light* light =
                  world::find_entity(_world.lights, selected.get<world::light_id>());

               if (not light) continue;

               edit_bundle.push_back(
                  edits::make_set_value(&light->specular_caster, specular_caster));
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context, {.closed = true});
      }

      if (world::light_type type =
             properties.light.type.value_or(world::light_type::point);
          ImGui::BeginCombo("Type", properties.light.type.is_different()
                                       ? "<different>"
                                       : world::to_ui_string(type))) {
         for (const world::light_type other_type :
              {world::light_type::directional, world::light_type::point,
               world::light_type::spot, world::light_type::directional_region_box,
               world::light_type::directional_region_sphere,
               world::light_type::directional_region_cylinder}) {
            if (ImGui::Selectable(world::to_ui_string(other_type),
                                  properties.light.type.is_different()
                                     ? false
                                     : type == other_type)) {
               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.light.type.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::light_id>()) {
                     world::light* light =
                        world::find_entity(_world.lights,
                                           selected.get<world::light_id>());

                     if (not light) continue;

                     edit_bundle.push_back(
                        edits::make_set_value(&light->light_type, other_type));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context, {.closed = true});
            }
         }

         ImGui::EndCombo();
      }

      ImGui::Separator();

      if (are_flags_set(flags, multi_select_flags::has_light_range)) {
         if (float range = properties.light.range.value_or(0.0f);
             ImGui::DragFloat("Range", &range, 1.0f, 0.0f, 1e10f,
                              properties.light.range.is_different()
                                 ? "<different>"
                                 : "%.3f")) {

            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.light.range.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::light_id>()) {
                  world::light* light =
                     world::find_entity(_world.lights,
                                        selected.get<world::light_id>());

                  if (not light) continue;

                  edit_bundle.push_back(edits::make_set_value(&light->range, range));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_light_cone_props)) {
         if (float inner_cone_angle = properties.light.inner_cone_angle.value_or(0.0f);
             ImGui::DragFloat("Inner Cone Angle", &inner_cone_angle, 0.01f,
                              0.0f, std::numbers::pi_v<float>,
                              properties.light.inner_cone_angle.is_different()
                                 ? "<different>"
                                 : "%.3f")) {

            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.light.inner_cone_angle.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::light_id>()) {
                  world::light* light =
                     world::find_entity(_world.lights,
                                        selected.get<world::light_id>());

                  if (not light) continue;

                  edit_bundle.push_back(
                     edits::make_set_multi_value(&light->inner_cone_angle,
                                                 inner_cone_angle, &light->outer_cone_angle,
                                                 std::max(inner_cone_angle,
                                                          light->outer_cone_angle)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         if (float outer_cone_angle = properties.light.outer_cone_angle.value_or(
                std::numbers::pi_v<float> * 0.5f);
             ImGui::DragFloat("Outer Cone Angle", &outer_cone_angle, 0.01f,
                              0.0f, std::numbers::pi_v<float>,
                              properties.light.outer_cone_angle.is_different()
                                 ? "<different>"
                                 : "%.3f")) {

            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.light.outer_cone_angle.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::light_id>()) {
                  world::light* light =
                     world::find_entity(_world.lights,
                                        selected.get<world::light_id>());

                  if (not light) continue;

                  edit_bundle.push_back(
                     edits::make_set_multi_value(&light->inner_cone_angle,
                                                 std::min(outer_cone_angle,
                                                          light->inner_cone_angle),
                                                 &light->outer_cone_angle,
                                                 outer_cone_angle));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         ImGui::Separator();
      }

      // Texture
      {
         const bool is_different = properties.light.texture.is_different();
         const std::string& texture = properties.light.texture.value_or("");

         if (std::optional<std::string> new_texture =
                ui_texture_pick_widget_untracked("Texture", texture.c_str(),
                                                 is_different ? "<different>" : nullptr);
             new_texture) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.light.texture.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::light_id>()) {
                  world::light* light =
                     world::find_entity(_world.lights,
                                        selected.get<world::light_id>());

                  if (not light) continue;

                  edit_bundle.push_back(
                     edits::make_set_value(&light->texture, *new_texture));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context, {.closed = true});
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_light_texture_addressing)) {
         if (world::texture_addressing addressing =
                properties.light.texture_addressing.value_or(
                   world::texture_addressing::wrap);
             ImGui::BeginCombo("Texture Addressing",
                               properties.light.texture_addressing.is_different()
                                  ? "<different>"
                                  : world::to_ui_string(addressing))) {
            for (const world::texture_addressing other_addressing :
                 {world::texture_addressing::wrap, world::texture_addressing::clamp}) {
               if (ImGui::Selectable(world::to_ui_string(other_addressing),
                                     properties.light.texture_addressing.is_different()
                                        ? false
                                        : addressing == other_addressing)) {
                  edits::bundle_vector edit_bundle;
                  edit_bundle.reserve(properties.light.texture_addressing.count());

                  for (const world::selected_entity& selected :
                       _interaction_targets.selection) {
                     if (selected.is<world::light_id>()) {
                        world::light* light =
                           world::find_entity(_world.lights,
                                              selected.get<world::light_id>());

                        if (not light) continue;

                        edit_bundle.push_back(
                           edits::make_set_value(&light->texture_addressing,
                                                 other_addressing));
                     }
                  }

                  _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                          _edit_context, {.closed = true});
               }
            }

            ImGui::EndCombo();
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_light_directional_texture)) {
         // Directional Texture Tiling
         {
            float directional_texture_tiling_x =
               properties.light.directional_texture_tiling.x.value_or(1.0f);

            float directional_texture_tiling_y =
               properties.light.directional_texture_tiling.y.value_or(1.0f);

            const float item_inner_spacing = ImGui::GetStyle().ItemInnerSpacing.x;

            const float item_width_one =
               std::max(1.0f,
                        floorf((ImGui::CalcItemWidth() - item_inner_spacing) / 2.0f));
            const float item_width_last =
               std::max(1.0f, floorf(ImGui::CalcItemWidth() -
                                     (item_width_one + item_inner_spacing)));

            ImGui::BeginGroup();
            ImGui::PushID("Directional Texture Tiling");
            ImGui::SetNextItemWidth(item_width_one);
            const bool x_edited =
               ImGui::DragFloat("##X", &directional_texture_tiling_x, 0.0625f,
                                0.0f, 0.0f,
                                properties.light.directional_texture_tiling.x.is_different()
                                   ? "<different>"
                                   : "X:%.3f");

            ImGui::SameLine(0, item_inner_spacing);
            ImGui::SetNextItemWidth(item_width_last);
            const bool y_edited =
               ImGui::DragFloat("##Y", &directional_texture_tiling_y, 0.0625f,
                                0.0f, 0.0f,
                                properties.light.directional_texture_tiling.y.is_different()
                                   ? "<different>"
                                   : "Y:%.3f");

            ImGui::PopID();

            ImGui::SameLine(0, item_inner_spacing);
            ImGui::TextUnformatted("Directional Texture Tiling");

            ImGui::EndGroup();

            if (x_edited or y_edited) {

               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.light.directional_texture_tiling.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::light_id>()) {
                     world::light* light =
                        world::find_entity(_world.lights,
                                           selected.get<world::light_id>());

                     if (not light) continue;

                     float2 directional_texture_tiling = {
                        x_edited ? directional_texture_tiling_x
                                 : light->directional_texture_tiling.x,
                        y_edited ? directional_texture_tiling_y
                                 : light->directional_texture_tiling.y,
                     };

                     edit_bundle.push_back(
                        edits::make_set_value(&light->directional_texture_tiling,
                                              directional_texture_tiling));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context);
            }

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }

         // Directional Texture Offset
         {
            float directional_texture_offset_x =
               properties.light.directional_texture_offset.x.value_or(1.0f);

            float directional_texture_offset_y =
               properties.light.directional_texture_offset.y.value_or(1.0f);

            const float item_inner_spacing = ImGui::GetStyle().ItemInnerSpacing.x;

            const float item_width_one =
               std::max(1.0f,
                        floorf((ImGui::CalcItemWidth() - item_inner_spacing) / 2.0f));
            const float item_width_last =
               std::max(1.0f, floorf(ImGui::CalcItemWidth() -
                                     (item_width_one + item_inner_spacing)));

            ImGui::BeginGroup();
            ImGui::PushID("Directional Texture Offset");
            ImGui::SetNextItemWidth(item_width_one);
            const bool x_edited =
               ImGui::DragFloat("##X", &directional_texture_offset_x, 0.0625f,
                                0.0f, 0.0f,
                                properties.light.directional_texture_offset.x.is_different()
                                   ? "<different>"
                                   : "X:%.3f");

            ImGui::SameLine(0, item_inner_spacing);
            ImGui::SetNextItemWidth(item_width_last);
            const bool y_edited =
               ImGui::DragFloat("##Y", &directional_texture_offset_y, 0.0625f,
                                0.0f, 0.0f,
                                properties.light.directional_texture_offset.y.is_different()
                                   ? "<different>"
                                   : "Y:%.3f");

            ImGui::PopID();

            ImGui::SameLine(0, item_inner_spacing);
            ImGui::TextUnformatted("Directional Texture Offset");

            ImGui::EndGroup();

            if (x_edited or y_edited) {

               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.light.directional_texture_offset.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::light_id>()) {
                     world::light* light =
                        world::find_entity(_world.lights,
                                           selected.get<world::light_id>());

                     if (not light) continue;

                     float2 directional_texture_offset = {
                        x_edited ? directional_texture_offset_x
                                 : light->directional_texture_offset.x,
                        y_edited ? directional_texture_offset_y
                                 : light->directional_texture_offset.y,
                     };

                     edit_bundle.push_back(
                        edits::make_set_value(&light->directional_texture_offset,
                                              directional_texture_offset));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context);
            }

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }

         ImGui::Separator();
      }

      if (are_flags_set(flags, multi_select_flags::has_light_region)) {
         ImGui::Separator();

         if (quaternion rotation = properties.light.region_rotation.value_or({});
             ImGui::DragQuat("Region Rotation", &rotation, 0.001f,
                             properties.light.region_rotation.is_different()
                                ? "<different>"
                                : "%.3f")) {

            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.light.region_rotation.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::light_id>()) {
                  world::light* light =
                     world::find_entity(_world.lights,
                                        selected.get<world::light_id>());

                  if (not light) continue;

                  edit_bundle.push_back(
                     edits::make_set_value(&light->region_rotation, rotation));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }

      if (not _settings.ui.hide_extra_light_properties) {
         if (are_flags_set(flags, multi_select_flags::has_light_ps2_blend_mode)) {

            if (world::ps2_blend_mode blend_mode =
                   properties.light.ps2_blend_mode.value_or(world::ps2_blend_mode::add);
                ImGui::BeginCombo("PS2 Blend Mode",
                                  properties.light.ps2_blend_mode.is_different()
                                     ? "<different>"
                                     : world::to_ui_string(blend_mode))) {
               for (const world::ps2_blend_mode other_blend_mode :
                    {world::ps2_blend_mode::add, world::ps2_blend_mode::multiply,
                     world::ps2_blend_mode::blend}) {
                  if (ImGui::Selectable(world::to_ui_string(other_blend_mode),
                                        properties.light.ps2_blend_mode.is_different()
                                           ? false
                                           : blend_mode == other_blend_mode)) {
                     edits::bundle_vector edit_bundle;
                     edit_bundle.reserve(properties.light.ps2_blend_mode.count());

                     for (const world::selected_entity& selected :
                          _interaction_targets.selection) {
                        if (selected.is<world::light_id>()) {
                           world::light* light =
                              world::find_entity(_world.lights,
                                                 selected.get<world::light_id>());

                           if (not light) continue;

                           edit_bundle.push_back(
                              edits::make_set_value(&light->ps2_blend_mode,
                                                    other_blend_mode));
                        }
                     }

                     _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                             _edit_context, {.closed = true});
                  }
               }

               ImGui::EndCombo();
            }
         }

         if (are_flags_set(flags, multi_select_flags::has_light_cone_props)) {
            if (bool bidirectional = properties.light.bidirectional.value_or(false);
                ImGui::CheckboxTristate("Bidirectional", &bidirectional,
                                        properties.light.bidirectional.is_different())) {

               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.light.bidirectional.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::light_id>()) {
                     world::light* light =
                        world::find_entity(_world.lights,
                                           selected.get<world::light_id>());

                     if (not light) continue;

                     edit_bundle.push_back(
                        edits::make_set_value(&light->bidirectional, bidirectional));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context, {.closed = true});
            }
         }
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_path)) {
      if (world::path_type type = properties.path.type.value_or(world::path_type::none);
          ImGui::BeginCombo("Type", properties.path.type.is_different()
                                       ? "<different>"
                                       : world::to_ui_string(type))) {
         for (const world::path_type other_type :
              {world::path_type::none, world::path_type::entity_follow,
               world::path_type::formation, world::path_type::patrol}) {
            if (ImGui::Selectable(world::to_ui_string(other_type),
                                  properties.path.type.is_different()
                                     ? false
                                     : type == other_type)) {
               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.path.type.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::path_id_node_mask>()) {
                     const auto& [id, node_mask] =
                        selected.get<world::path_id_node_mask>();

                     world::path* path = world::find_entity(_world.paths, id);

                     if (not path) continue;

                     edit_bundle.push_back(
                        edits::make_set_value(&path->type, other_type));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context, {.closed = true});
            }
         }

         ImGui::EndCombo();
      }

      if (are_flags_set(flags, multi_select_flags::has_path_spline_type)) {
         if (world::path_spline_type type =
                properties.path.spline_type.value_or(world::path_spline_type::none);
             ImGui::BeginCombo("Spline Type", properties.path.spline_type.is_different()
                                                 ? "<different>"
                                                 : world::to_ui_string(type))) {
            for (const world::path_spline_type other_type :
                 {world::path_spline_type::none, world::path_spline_type::linear,
                  world::path_spline_type::hermite,
                  world::path_spline_type::catmull_rom}) {
               if (ImGui::Selectable(world::to_ui_string(other_type),
                                     properties.path.spline_type.is_different()
                                        ? false
                                        : type == other_type)) {
                  edits::bundle_vector edit_bundle;
                  edit_bundle.reserve(properties.path.spline_type.count());

                  for (const world::selected_entity& selected :
                       _interaction_targets.selection) {
                     if (selected.is<world::path_id_node_mask>()) {
                        const auto& [id, node_mask] =
                           selected.get<world::path_id_node_mask>();

                        world::path* path = world::find_entity(_world.paths, id);

                        if (not path) continue;

                        edit_bundle.push_back(
                           edits::make_set_value(&path->spline_type, other_type));
                     }
                  }

                  _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                          _edit_context, {.closed = true});
               }
            }

            ImGui::EndCombo();
         }
      }

      if (properties.path.properties.has_common_keys()) {
         ImGui::Separator();

         const std::span<const world::path::property> ref_properties =
            properties.path.properties.properties_or_default();

         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.path.properties.count());

         bool property_edited = false;

         ImGui::BeginGroup();
         ImGui::PushStyleColor(ImGuiCol_TextDisabled,
                               ImGui::GetStyleColorVec4(ImGuiCol_Text));

         for (uint32 i = 0; i < ref_properties.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));

            const world::path::property& prop = ref_properties[i];

            const bool is_different_value =
               properties.path.properties.is_different_value(i);

            const std::string_view value =
               is_different_value ? "" : std::string_view{prop.value};
            const char* hint = is_different_value ? "<different>" : "";

            absl::InlinedVector<char, 256> value_buffer{value.begin(), value.end()};

            if (ImGui::InputTextWithHint(prop.key.c_str(), hint, &value_buffer)) {
               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::path_id_node_mask>()) {
                     const auto& [id, node_mask] =
                        selected.get<world::path_id_node_mask>();

                     world::path* path = world::find_entity(_world.paths, id);

                     if (not path) continue;

                     edit_bundle.push_back(edits::make_set_vector_value(
                        &path->properties, i, &world::path::property::value,
                        std::string{value_buffer.begin(), value_buffer.end()}));
                  }
               }

               property_edited = true;
            }

            ImGui::PopID();
         }

         ImGui::PopStyleColor();
         ImGui::EndGroup();

         if (property_edited) {
            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }

      if (properties.path.node_properties.has_common_keys()) {
         ImGui::Separator();
         ImGui::PushID("node_properties");

         const std::span<const world::path::property> ref_properties =
            properties.path.node_properties.properties_or_default();

         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.path.node_properties.count());

         bool property_edited = false;

         ImGui::BeginGroup();
         ImGui::PushStyleColor(ImGuiCol_TextDisabled,
                               ImGui::GetStyleColorVec4(ImGuiCol_Text));

         for (uint32 property_index = 0; property_index < ref_properties.size();
              ++property_index) {
            ImGui::PushID(static_cast<int>(property_index));

            const world::path::property& prop = ref_properties[property_index];

            const bool is_different_value =
               properties.path.node_properties.is_different_value(property_index);

            const std::string_view value =
               is_different_value ? "" : std::string_view{prop.value};
            const char* hint = is_different_value ? "<different>" : "";

            absl::InlinedVector<char, 256> value_buffer{value.begin(), value.end()};

            if (ImGui::InputTextWithHint(prop.key.c_str(), hint, &value_buffer)) {
               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::path_id_node_mask>()) {
                     const auto& [id, node_mask] =
                        selected.get<world::path_id_node_mask>();

                     world::path* path = world::find_entity(_world.paths, id);

                     if (not path) continue;

                     const std::size_t node_count =
                        std::min(path->nodes.size(), world::max_path_nodes);

                     for (uint32 node_index = 0; node_index < node_count; ++node_index) {
                        if (not node_mask[node_index]) continue;
                        edit_bundle.push_back(edits::make_set_path_node_property_value(
                           &path->nodes, node_index, property_index,
                           std::string{value_buffer.begin(), value_buffer.end()}));
                     }
                  }
               }

               property_edited = true;
            }

            ImGui::PopID();
            ImGui::PopID();
         }

         ImGui::PopStyleColor();
         ImGui::EndGroup();

         if (property_edited) {
            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_region)) {
      if (world::region_type type =
             properties.region.type.value_or(world::region_type::typeless);
          ImGui::BeginCombo("Type", properties.region.type.is_different()
                                       ? "<different>"
                                       : world::to_ui_string(type))) {
         for (const world::region_type other_type : {
                 world::region_type::typeless,
                 world::region_type::soundstream,
                 world::region_type::soundstatic,
                 world::region_type::soundspace,
                 world::region_type::soundtrigger,
                 world::region_type::foleyfx,
                 world::region_type::shadow,
                 world::region_type::mapbounds,
                 world::region_type::rumble,
                 world::region_type::reflection,
                 world::region_type::rainshadow,
                 world::region_type::danger,
                 world::region_type::damage_region,
                 world::region_type::ai_vis,
                 world::region_type::colorgrading,
              }) {
            if (ImGui::Selectable(world::to_ui_string(other_type),
                                  properties.region.type.is_different()
                                     ? false
                                     : type == other_type)) {
               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.region.type.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::region_id>()) {
                     world::region* region =
                        world::find_entity(_world.regions,
                                           selected.get<world::region_id>());

                     if (not region) continue;
                     if (world::get_region_type(region->description) == other_type) {
                        continue;
                     }

                     edit_bundle.push_back(
                        edits::make_set_value(&region->description,
                                              get_default_description(other_type)));

                     const world::region_allowed_shapes allowed_shapes =
                        world::get_region_allowed_shapes(other_type);

                     if (allowed_shapes == world::region_allowed_shapes::box and
                         region->shape != world::region_shape::box) {
                        edit_bundle.push_back(
                           edits::make_set_value(&region->shape,
                                                 world::region_shape::box));
                     }
                     else if (allowed_shapes == world::region_allowed_shapes::sphere and
                              region->shape != world::region_shape::sphere) {
                        edit_bundle.push_back(
                           edits::make_set_value(&region->shape,
                                                 world::region_shape::sphere));
                     }
                     else if (allowed_shapes == world::region_allowed_shapes::box_cylinder and
                              region->shape == world::region_shape::sphere) {
                        edit_bundle.push_back(
                           edits::make_set_value(&region->shape,
                                                 world::region_shape::box));
                     }
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context, {.closed = true});
            }
         }

         ImGui::EndCombo();
      }

      // Save the region description before we possibly edit it below.
      absl::InlinedVector<char, 256> region_description_buffer;

      if (not properties.region.description.is_different()) {
         const std::string_view description =
            properties.region.description.value_or("");

         region_description_buffer = {description.begin(), description.end()};
      }

      if (are_flags_set(flags, multi_select_flags::has_region_sound_stream)) {
         // Stream Name
         {
            const bool is_different =
               properties.region.sound_stream.sound_name.is_different();
            const std::string& stream_name =
               properties.region.sound_stream.sound_name.value_or("");

            absl::InlinedVector<char, 256> stream_name_buffer;

            if (not is_different) {
               stream_name_buffer = {stream_name.begin(), stream_name.end()};
            }

            ImGui::PushStyleColor(ImGuiCol_TextDisabled,
                                  ImGui::GetStyleColorVec4(ImGuiCol_Text));

            if (ImGui::InputTextWithHint("Stream Name", is_different ? "<different>" : "",
                                         &stream_name_buffer)) {
               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.region.sound_stream.sound_name.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::region_id>()) {
                     world::region* region =
                        world::find_entity(_world.regions,
                                           selected.get<world::region_id>());

                     if (not region) continue;

                     world::sound_stream_properties sound_stream =
                        world::unpack_region_sound_stream(region->description);

                     sound_stream.sound_name =
                        std::string{stream_name_buffer.begin(),
                                    stream_name_buffer.end()};

                     edit_bundle.push_back(
                        edits::make_set_value(&region->description,
                                              world::pack_region_sound_stream(
                                                 sound_stream)));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context);
            }

            ImGui::PopStyleColor();

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }

         if (float min_distance_divisor =
                properties.region.sound_stream.min_distance_divisor.value_or(0.0f);
             ImGui::DragFloat(
                "Min Distance Divisor", &min_distance_divisor, 1.0f, 1.0f, 1e10f,
                properties.region.sound_stream.min_distance_divisor.is_different()
                   ? "<different>"
                   : "%.3f",
                ImGuiSliderFlags_AlwaysClamp)) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(
               properties.region.sound_stream.min_distance_divisor.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::sound_stream_properties sound_stream =
                     world::unpack_region_sound_stream(region->description);

                  sound_stream.min_distance_divisor = min_distance_divisor;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_sound_stream(sound_stream)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_region_sound_static)) {
         // Sound Name
         {
            const bool is_different =
               properties.region.sound_static.sound_name.is_different();
            const std::string& sound_name =
               properties.region.sound_static.sound_name.value_or("");

            absl::InlinedVector<char, 256> sound_name_buffer;

            if (not is_different) {
               sound_name_buffer = {sound_name.begin(), sound_name.end()};
            }

            ImGui::PushStyleColor(ImGuiCol_TextDisabled,
                                  ImGui::GetStyleColorVec4(ImGuiCol_Text));

            if (ImGui::InputTextWithHint("Sound Name", is_different ? "<different>" : "",
                                         &sound_name_buffer)) {
               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.region.sound_static.sound_name.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::region_id>()) {
                     world::region* region =
                        world::find_entity(_world.regions,
                                           selected.get<world::region_id>());

                     if (not region) continue;

                     world::sound_static_properties sound_static =
                        world::unpack_region_sound_static(region->description);

                     sound_static.sound_name =
                        std::string{sound_name_buffer.begin(),
                                    sound_name_buffer.end()};

                     edit_bundle.push_back(
                        edits::make_set_value(&region->description,
                                              world::pack_region_sound_static(
                                                 sound_static)));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context);
            }

            ImGui::PopStyleColor();

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }

         if (float min_distance_divisor =
                properties.region.sound_static.min_distance_divisor.value_or(0.0f);
             ImGui::DragFloat(
                "Min Distance Divisor", &min_distance_divisor, 1.0f, 1.0f, 1e10f,
                properties.region.sound_static.min_distance_divisor.is_different()
                   ? "<different>"
                   : "%.3f",
                ImGuiSliderFlags_AlwaysClamp)) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(
               properties.region.sound_static.min_distance_divisor.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::sound_static_properties sound_static =
                     world::unpack_region_sound_static(region->description);

                  sound_static.min_distance_divisor = min_distance_divisor;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_sound_static(sound_static)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_region_sound_space)) {
         const bool is_different =
            properties.region.sound_space.sound_space_name.is_different();
         const std::string& sound_space_name =
            properties.region.sound_space.sound_space_name.value_or("");

         absl::InlinedVector<char, 256> sound_space_name_buffer;

         if (not is_different) {
            sound_space_name_buffer = {sound_space_name.begin(),
                                       sound_space_name.end()};
         }

         ImGui::PushStyleColor(ImGuiCol_TextDisabled,
                               ImGui::GetStyleColorVec4(ImGuiCol_Text));

         if (ImGui::InputTextWithHint("Sound Space Name",
                                      is_different ? "<different>" : "",
                                      &sound_space_name_buffer)) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.sound_space.sound_space_name.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::sound_space_properties sound_space =
                     world::unpack_region_sound_space(region->description);

                  sound_space.sound_space_name =
                     std::string{sound_space_name_buffer.begin(),
                                 sound_space_name_buffer.end()};

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_sound_space(sound_space)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         ImGui::PopStyleColor();

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_region_sound_trigger)) {
         const bool is_different =
            properties.region.sound_trigger.region_name.is_different();
         const std::string& region_name =
            properties.region.sound_trigger.region_name.value_or("");

         absl::InlinedVector<char, 256> region_name_buffer;

         if (not is_different) {
            region_name_buffer = {region_name.begin(), region_name.end()};
         }

         ImGui::PushStyleColor(ImGuiCol_TextDisabled,
                               ImGui::GetStyleColorVec4(ImGuiCol_Text));

         if (ImGui::InputTextWithHint("Region Name", is_different ? "<different>" : "",
                                      &region_name_buffer)) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.sound_trigger.region_name.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::sound_trigger_properties sound_trigger =
                     world::unpack_region_sound_trigger(region->description);

                  sound_trigger.region_name =
                     std::string{region_name_buffer.begin(), region_name_buffer.end()};

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_sound_trigger(sound_trigger)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         ImGui::PopStyleColor();

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_region_foley_fx)) {
         const bool is_different = properties.region.foley_fx.group_id.is_different();
         const std::string& group_id =
            properties.region.foley_fx.group_id.value_or("");

         absl::InlinedVector<char, 256> group_id_buffer;

         if (not is_different) {
            group_id_buffer = {group_id.begin(), group_id.end()};
         }

         ImGui::PushStyleColor(ImGuiCol_TextDisabled,
                               ImGui::GetStyleColorVec4(ImGuiCol_Text));

         if (ImGui::InputTextWithHint("Group ID", is_different ? "<different>" : "",
                                      &group_id_buffer)) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.foley_fx.group_id.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::foley_fx_region_properties foley_fx =
                     world::unpack_region_foley_fx(region->description);

                  foley_fx.group_id =
                     std::string{group_id_buffer.begin(), group_id_buffer.end()};

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_foley_fx(foley_fx)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         ImGui::PopStyleColor();

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_region_shadow)) {
         if (float directional0 = properties.region.shadow.directional0.value_or(0.5f);
             ImGui::DragFloat("Directional Light 0 Strength", &directional0,
                              0.01f, 0.0f, 1.0f,
                              properties.region.shadow.directional0.is_different()
                                 ? "<different>"
                                 : "%.3f",
                              ImGuiSliderFlags_AlwaysClamp)) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.shadow.directional0.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::shadow_region_properties shadow =
                     world::unpack_region_shadow(region->description);

                  shadow.directional0 = directional0;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_shadow(shadow)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         if (float directional1 = properties.region.shadow.directional1.value_or(0.5f);
             ImGui::DragFloat("Directional Light 1 Strength", &directional1,
                              0.01f, 0.0f, 1.0f,
                              properties.region.shadow.directional1.is_different()
                                 ? "<different>"
                                 : "%.3f",
                              ImGuiSliderFlags_AlwaysClamp)) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.shadow.directional1.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::shadow_region_properties shadow =
                     world::unpack_region_shadow(region->description);

                  shadow.directional1 = directional1;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_shadow(shadow)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         // Ambient Light Top
         {
            float3 color =
               properties.region.shadow.color_top.value_or(float3{0.5f, 0.5f, 0.5f});

            const float slider_width =
               ((ImGui::CalcItemWidth() - ImGui::GetFrameHeight() -
                 ImGui::GetStyle().ItemInnerSpacing.x) -
                ImGui::GetStyle().ItemInnerSpacing.x * 2.0f) /
               3.0f;

            const bool is_different =
               properties.region.shadow.color_top.is_different();
            bool edited = false;

            ImGui::PushID("Top");
            ImGui::BeginGroup();

            ImGui::SetNextItemWidth(slider_width);
            edited |= ImGui::DragFloat("##R", &color.x, 1.0f / 255.0f, 0.0f, 1e10f,
                                       is_different ? "<different>" : "R: %.3f");

            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::SetNextItemWidth(slider_width);
            edited |= ImGui::DragFloat("##G", &color.y, 1.0f / 255.0f, 0.0f, 1e10f,
                                       is_different ? "<different>" : "G: %.3f");
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::SetNextItemWidth(slider_width);
            edited |= ImGui::DragFloat("##B", &color.z, 1.0f / 255.0f, 0.0f, 1e10f,
                                       is_different ? "<different>" : "B: %.3f");

            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            edited |= ImGui::ColorEdit3("##Picker", &color.x,
                                        ImGuiColorEditFlags_NoInputs |
                                           ImGuiColorEditFlags_Float |
                                           ImGuiColorEditFlags_HDR);

            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::TextUnformatted("Ambient Light Top");

            ImGui::EndGroup();
            ImGui::PopID();

            if (edited) {
               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.region.shadow.color_top.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::region_id>()) {
                     world::region* region =
                        world::find_entity(_world.regions,
                                           selected.get<world::region_id>());

                     if (not region) continue;

                     world::shadow_region_properties shadow =
                        world::unpack_region_shadow(region->description);

                     shadow.color_top = color;

                     edit_bundle.push_back(
                        edits::make_set_value(&region->description,
                                              world::pack_region_shadow(shadow)));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context);
            }

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }

         // Ambient Light Bottom
         {
            float3 color = properties.region.shadow.color_bottom.value_or(
               float3{0.5f, 0.5f, 0.5f});

            const float slider_width =
               ((ImGui::CalcItemWidth() - ImGui::GetFrameHeight() -
                 ImGui::GetStyle().ItemInnerSpacing.x) -
                ImGui::GetStyle().ItemInnerSpacing.x * 2.0f) /
               3.0f;

            const bool is_different =
               properties.region.shadow.color_bottom.is_different();
            bool edited = false;

            ImGui::PushID("Bottom");
            ImGui::BeginGroup();

            ImGui::SetNextItemWidth(slider_width);
            edited |= ImGui::DragFloat("##R", &color.x, 1.0f / 255.0f, 0.0f, 1e10f,
                                       is_different ? "<different>" : "R: %.3f");

            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::SetNextItemWidth(slider_width);
            edited |= ImGui::DragFloat("##G", &color.y, 1.0f / 255.0f, 0.0f, 1e10f,
                                       is_different ? "<different>" : "G: %.3f");
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::SetNextItemWidth(slider_width);
            edited |= ImGui::DragFloat("##B", &color.z, 1.0f / 255.0f, 0.0f, 1e10f,
                                       is_different ? "<different>" : "B: %.3f");

            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            edited |= ImGui::ColorEdit3("##Picker", &color.x,
                                        ImGuiColorEditFlags_NoInputs |
                                           ImGuiColorEditFlags_Float |
                                           ImGuiColorEditFlags_HDR);

            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::TextUnformatted("Ambient Light Bottom");

            ImGui::EndGroup();
            ImGui::PopID();

            if (edited) {
               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.region.shadow.color_bottom.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::region_id>()) {
                     world::region* region =
                        world::find_entity(_world.regions,
                                           selected.get<world::region_id>());

                     if (not region) continue;

                     world::shadow_region_properties shadow =
                        world::unpack_region_shadow(region->description);

                     shadow.color_bottom = color;

                     edit_bundle.push_back(
                        edits::make_set_value(&region->description,
                                              world::pack_region_shadow(shadow)));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context);
            }

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }

         // Env Map
         {
            const bool is_different = properties.region.shadow.env_map.is_different();
            const std::string& env_map =
               properties.region.shadow.env_map.value_or("");

            if (std::optional<std::string> new_env_map =
                   ui_texture_pick_widget_untracked("Environment Map", env_map.c_str(),
                                                    is_different ? "<different>" : nullptr);
                new_env_map) {
               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.region.shadow.env_map.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::region_id>()) {
                     world::region* region =
                        world::find_entity(_world.regions,
                                           selected.get<world::region_id>());

                     if (not region) continue;

                     world::shadow_region_properties shadow =
                        world::unpack_region_shadow(region->description);

                     shadow.env_map = *new_env_map;

                     edit_bundle.push_back(
                        edits::make_set_value(&region->description,
                                              world::pack_region_shadow(shadow)));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context, {.closed = true});
            }
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_region_rumble)) {
         // Rumble Class
         {
            const bool is_different =
               properties.region.rumble.rumble_class.is_different();
            const std::string& rumble_class =
               properties.region.rumble.rumble_class.value_or("");

            absl::InlinedVector<char, 256> rumble_class_buffer;

            if (not is_different) {
               rumble_class_buffer = {rumble_class.begin(), rumble_class.end()};
            }

            ImGui::PushStyleColor(ImGuiCol_TextDisabled,
                                  ImGui::GetStyleColorVec4(ImGuiCol_Text));

            if (ImGui::InputTextWithHint("Rumble Class",
                                         is_different ? "<different>" : "",
                                         &rumble_class_buffer)) {
               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.region.rumble.rumble_class.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::region_id>()) {
                     world::region* region =
                        world::find_entity(_world.regions,
                                           selected.get<world::region_id>());

                     if (not region) continue;

                     world::rumble_region_properties rumble =
                        world::unpack_region_rumble(region->description);

                     rumble.rumble_class = std::string{rumble_class_buffer.begin(),
                                                       rumble_class_buffer.end()};

                     edit_bundle.push_back(
                        edits::make_set_value(&region->description,
                                              world::pack_region_rumble(rumble)));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context);
            }

            ImGui::PopStyleColor();

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }

         // Particle Effect
         {
            const bool is_different =
               properties.region.rumble.particle_effect.is_different();
            const std::string& particle_effect =
               properties.region.rumble.particle_effect.value_or("");

            absl::InlinedVector<char, 256> particle_effect_buffer;

            if (not is_different) {
               particle_effect_buffer = {particle_effect.begin(),
                                         particle_effect.end()};
            }

            ImGui::PushStyleColor(ImGuiCol_TextDisabled,
                                  ImGui::GetStyleColorVec4(ImGuiCol_Text));

            if (ImGui::InputTextWithHint("Particle Effect",
                                         is_different ? "<different>" : "",
                                         &particle_effect_buffer)) {
               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.region.rumble.particle_effect.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::region_id>()) {
                     world::region* region =
                        world::find_entity(_world.regions,
                                           selected.get<world::region_id>());

                     if (not region) continue;

                     world::rumble_region_properties rumble =
                        world::unpack_region_rumble(region->description);

                     rumble.particle_effect =
                        std::string{particle_effect_buffer.begin(),
                                    particle_effect_buffer.end()};

                     edit_bundle.push_back(
                        edits::make_set_value(&region->description,
                                              world::pack_region_rumble(rumble)));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context);
            }

            ImGui::PopStyleColor();

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_region_damage)) {
         if (float damage_rate = properties.region.damage.damage_rate.value_or(0.0f);
             ImGui::DragFloat("Damage Rate", &damage_rate, 1.0f, 0.0f, 0.0f,
                              properties.region.damage.damage_rate.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.damage.damage_rate.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::damage_region_properties damage =
                     world::unpack_region_damage(region->description);

                  damage.damage_rate = damage_rate;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_damage(damage)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         if (float person_scale = properties.region.damage.person_scale.value_or(0.0f);
             ImGui::DragFloat("Person Scale", &person_scale, 0.01f, 0.0f, 0.0f,
                              properties.region.damage.person_scale.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.damage.person_scale.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::damage_region_properties damage =
                     world::unpack_region_damage(region->description);

                  damage.person_scale = person_scale;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_damage(damage)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         if (float animal_scale = properties.region.damage.animal_scale.value_or(0.0f);
             ImGui::DragFloat("Animal Scale", &animal_scale, 0.01f, 0.0f, 0.0f,
                              properties.region.damage.animal_scale.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.damage.animal_scale.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::damage_region_properties damage =
                     world::unpack_region_damage(region->description);

                  damage.animal_scale = animal_scale;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_damage(damage)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         if (float droid_scale = properties.region.damage.droid_scale.value_or(0.0f);
             ImGui::DragFloat("Droid Scale", &droid_scale, 0.01f, 0.0f, 0.0f,
                              properties.region.damage.droid_scale.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.damage.droid_scale.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::damage_region_properties damage =
                     world::unpack_region_damage(region->description);

                  damage.droid_scale = droid_scale;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_damage(damage)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         if (float vehicle_scale =
                properties.region.damage.vehicle_scale.value_or(0.0f);
             ImGui::DragFloat("Vehicle Scale", &vehicle_scale, 0.01f, 0.0f, 0.0f,
                              properties.region.damage.vehicle_scale.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.damage.vehicle_scale.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::damage_region_properties damage =
                     world::unpack_region_damage(region->description);

                  damage.vehicle_scale = vehicle_scale;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_damage(damage)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         if (float building_scale =
                properties.region.damage.building_scale.value_or(0.0f);
             ImGui::DragFloat("Building Scale", &building_scale, 0.01f, 0.0f, 0.0f,
                              properties.region.damage.building_scale.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.damage.building_scale.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::damage_region_properties damage =
                     world::unpack_region_damage(region->description);

                  damage.building_scale = building_scale;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_damage(damage)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         if (float building_dead_scale =
                properties.region.damage.building_dead_scale.value_or(0.0f);
             ImGui::DragFloat("Building Dead Scale", &building_dead_scale,
                              0.01f, 0.0f, 0.0f,
                              properties.region.damage.building_dead_scale.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.damage.building_dead_scale.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::damage_region_properties damage =
                     world::unpack_region_damage(region->description);

                  damage.building_dead_scale = building_dead_scale;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_damage(damage)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         if (float building_unbuilt_scale =
                properties.region.damage.building_unbuilt_scale.value_or(0.0f);
             ImGui::DragFloat("Building Unbuilt Scale", &building_unbuilt_scale,
                              0.01f, 0.0f, 0.0f,
                              properties.region.damage.building_unbuilt_scale.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.damage.building_unbuilt_scale.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::damage_region_properties damage =
                     world::unpack_region_damage(region->description);

                  damage.building_unbuilt_scale = building_unbuilt_scale;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_damage(damage)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_region_ai_vis)) {
         if (float crouch = properties.region.ai_vis.crouch.value_or(0.5f);
             ImGui::DragFloat("Crouch", &crouch, 0.01f, 0.0f, 1e10f,
                              properties.region.ai_vis.crouch.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.ai_vis.crouch.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::ai_vis_region_properties ai_vis =
                     world::unpack_region_ai_vis(region->description);

                  ai_vis.crouch = crouch;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_ai_vis(ai_vis)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         if (float stand = properties.region.ai_vis.stand.value_or(0.5f);
             ImGui::DragFloat("Stand", &stand, 0.01f, 0.0f, 1e10f,
                              properties.region.ai_vis.stand.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.ai_vis.stand.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::ai_vis_region_properties ai_vis =
                     world::unpack_region_ai_vis(region->description);

                  ai_vis.stand = stand;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_ai_vis(ai_vis)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_region_colorgrading)) {
         // Config
         {
            const bool is_different =
               properties.region.colorgrading.config.is_different();
            const std::string& config =
               properties.region.colorgrading.config.value_or("");

            absl::InlinedVector<char, 256> config_buffer;

            if (not is_different) {
               config_buffer = {config.begin(), config.end()};
            }

            ImGui::PushStyleColor(ImGuiCol_TextDisabled,
                                  ImGui::GetStyleColorVec4(ImGuiCol_Text));

            if (ImGui::InputTextWithHint("Config", is_different ? "<different>" : "",
                                         &config_buffer)) {
               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.region.colorgrading.config.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::region_id>()) {
                     world::region* region =
                        world::find_entity(_world.regions,
                                           selected.get<world::region_id>());

                     if (not region) continue;

                     world::colorgrading_region_properties colorgrading =
                        world::unpack_region_colorgrading(region->description);

                     colorgrading.config =
                        std::string{config_buffer.begin(), config_buffer.end()};

                     edit_bundle.push_back(
                        edits::make_set_value(&region->description,
                                              world::pack_region_colorgrading(
                                                 colorgrading)));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context);
            }

            ImGui::PopStyleColor();

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }

         if (float fade_length =
                properties.region.colorgrading.fade_length.value_or(0.0f);
             ImGui::DragFloat("Fade Length", &fade_length, 1.0f, 0.0f, 0.0f,
                              properties.region.colorgrading.fade_length.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.colorgrading.fade_length.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  world::colorgrading_region_properties colorgrading =
                     world::unpack_region_colorgrading(region->description);

                  colorgrading.fade_length = fade_length;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           world::pack_region_colorgrading(colorgrading)));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }

      // Description
      {
         const bool is_different = properties.region.description.is_different();

         ImGui::PushStyleColor(ImGuiCol_TextDisabled,
                               ImGui::GetStyleColorVec4(ImGuiCol_Text));

         if (ImGui::InputTextWithHint("Description", is_different ? "<different>" : "",
                                      &region_description_buffer)) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.region.description.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::region_id>()) {
                  world::region* region =
                     world::find_entity(_world.regions,
                                        selected.get<world::region_id>());

                  if (not region) continue;

                  edit_bundle.push_back(
                     edits::make_set_value(&region->description,
                                           std::string{region_description_buffer.begin(),
                                                       region_description_buffer.end()}));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         ImGui::PopStyleColor();

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }

      ImGui::Separator();

      if (world::region_shape shape =
             properties.region.shape.value_or(world::region_shape::box);
          ImGui::BeginCombo("Shape", properties.region.shape.is_different()
                                        ? "<different>"
                                        : world::to_ui_string(shape))) {
         for (const world::region_shape other_shape : {
                 world::region_shape::box,
                 world::region_shape::sphere,
                 world::region_shape::cylinder,
              }) {
            if (ImGui::Selectable(world::to_ui_string(other_shape),
                                  properties.region.shape.is_different()
                                     ? false
                                     : shape == other_shape)) {
               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.region.shape.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::region_id>()) {
                     world::region* region =
                        world::find_entity(_world.regions,
                                           selected.get<world::region_id>());

                     if (not region) continue;
                     if (not is_region_allowed_shape(world::get_region_type(
                                                        region->description),
                                                     other_shape)) {
                        continue;
                     }

                     edit_bundle.push_back(
                        edits::make_set_value(&region->shape, other_shape));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context, {.closed = true});
            }
         }

         ImGui::EndCombo();
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_sector)) {
      if (float base = properties.sector.base.value_or(0.0f);
          ImGui::DragFloat("Base", &base, 1.0f, 0.0f, 0.0f,
                           properties.sector.base.is_different() ? "<different>"
                                                                 : "%.3f")) {
         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.sector.base.count());

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::sector_id>()) {
               world::sector* sector =
                  world::find_entity(_world.sectors, selected.get<world::sector_id>());

               if (not sector) continue;

               edit_bundle.push_back(edits::make_set_value(&sector->base, base));
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context);
      }

      if (ImGui::IsItemDeactivatedAfterEdit()) {
         _edit_stack_world.close_last();
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_portal)) {
      // Sector 1
      {
         const bool is_different = properties.portal.sector1.is_different();
         const std::string_view sector1 = properties.portal.sector1.value_or("");

         absl::InlinedVector<char, 256> sector1_buffer;

         if (not is_different) {
            sector1_buffer = {sector1.begin(), sector1.end()};
         }

         ImGui::PushStyleColor(ImGuiCol_TextDisabled,
                               ImGui::GetStyleColorVec4(ImGuiCol_Text));

         if (ImGui::InputTextWithHintAutoComplete(
                "Linked Sector 1", is_different ? "<different>" : "",
                &sector1_buffer, [&]() noexcept {
                   std::array<std::string_view, 6> entries;
                   std::size_t matching_count = 0;

                   for (const auto& sector : _world.sectors) {
                      if (sector.name.contains(sector1)) {
                         if (matching_count == entries.size()) break;

                         entries[matching_count] = sector.name;

                         ++matching_count;
                      }
                   }

                   return entries;
                })) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.portal.sector1.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::portal_id>()) {
                  world::portal* portal =
                     world::find_entity(_world.portals,
                                        selected.get<world::portal_id>());

                  if (not portal) continue;

                  edit_bundle.push_back(
                     edits::make_set_value(&portal->sector1,
                                           std::string{sector1_buffer.begin(),
                                                       sector1_buffer.end()}));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         ImGui::PopStyleColor();

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }

      // Sector 2
      {
         const bool is_different = properties.portal.sector2.is_different();
         const std::string_view sector2 = properties.portal.sector2.value_or("");

         absl::InlinedVector<char, 256> sector2_buffer;

         if (not is_different) {
            sector2_buffer = {sector2.begin(), sector2.end()};
         }

         ImGui::PushStyleColor(ImGuiCol_TextDisabled,
                               ImGui::GetStyleColorVec4(ImGuiCol_Text));

         if (ImGui::InputTextWithHintAutoComplete(
                "Linked Sector 2", is_different ? "<different>" : "",
                &sector2_buffer, [&]() noexcept {
                   std::array<std::string_view, 6> entries;
                   std::size_t matching_count = 0;

                   for (const auto& sector : _world.sectors) {
                      if (sector.name.contains(sector2)) {
                         if (matching_count == entries.size()) break;

                         entries[matching_count] = sector.name;

                         ++matching_count;
                      }
                   }

                   return entries;
                })) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.portal.sector2.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::portal_id>()) {
                  world::portal* portal =
                     world::find_entity(_world.portals,
                                        selected.get<world::portal_id>());

                  if (not portal) continue;

                  edit_bundle.push_back(
                     edits::make_set_value(&portal->sector2,
                                           std::string{sector2_buffer.begin(),
                                                       sector2_buffer.end()}));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         ImGui::PopStyleColor();

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_hintnode)) {
      ImGui::Separator();

      if (world::hintnode_type type =
             properties.hintnode.type.value_or(world::hintnode_type::snipe);
          ImGui::BeginCombo("Type", properties.hintnode.type.is_different()
                                       ? "<different>"
                                       : world::to_ui_string(type))) {
         for (const world::hintnode_type other_type : {
                 world::hintnode_type::snipe,
                 world::hintnode_type::patrol,
                 world::hintnode_type::cover,
                 world::hintnode_type::access,
                 world::hintnode_type::jet_jump,
                 world::hintnode_type::mine,
                 world::hintnode_type::land,
                 world::hintnode_type::fortification,
              }) {
            if (ImGui::Selectable(world::to_ui_string(other_type),
                                  properties.hintnode.type.is_different()
                                     ? false
                                     : type == other_type)) {
               edits::bundle_vector edit_bundle;
               edit_bundle.reserve(properties.hintnode.type.count());

               for (const world::selected_entity& selected :
                    _interaction_targets.selection) {
                  if (selected.is<world::hintnode_id>()) {
                     world::hintnode* hintnode =
                        world::find_entity(_world.hintnodes,
                                           selected.get<world::hintnode_id>());

                     if (not hintnode) continue;

                     edit_bundle.push_back(
                        edits::make_set_value(&hintnode->type, other_type));
                  }
               }

               _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                       _edit_context, {.closed = true});
            }
         }

         ImGui::EndCombo();
      }

      if (are_flags_set(flags, multi_select_flags::has_hintnode_command_post)) {
         if (ImGui::BeginCombo(
                "Command Post",
                properties.hintnode.command_post.is_different()
                   ? "<different>"
                   : properties.hintnode.command_post.value_or("").c_str())) {
            for (const world::object& object : _world.objects) {
               if (object.name.empty()) continue;

               const assets::odf::definition& definition =
                  *_object_classes[object.class_handle].definition;

               if (string::iequals(definition.header.class_label,
                                   "commandpost")) {
                  if (ImGui::Selectable(object.name.c_str())) {
                     edits::bundle_vector edit_bundle;
                     edit_bundle.reserve(properties.hintnode.type.count());

                     for (const world::selected_entity& selected :
                          _interaction_targets.selection) {
                        if (selected.is<world::hintnode_id>()) {
                           world::hintnode* hintnode =
                              world::find_entity(_world.hintnodes,
                                                 selected.get<world::hintnode_id>());

                           if (not hintnode) continue;

                           edit_bundle.push_back(
                              edits::make_set_value(&hintnode->command_post,
                                                    object.name));
                        }
                     }

                     _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                             _edit_context, {.closed = true});
                  }
               }
            }
            ImGui::EndCombo();
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_hintnode_primary_stance)) {
         const world::stance_flags stance_flags =
            properties.hintnode.primary_stance.value_or_default();

         world::stance_flags or_stance_flags = world::stance_flags::none;
         world::stance_flags and_stance_flags = world::stance_flags::primary_stance_mask;

         ImGui::SeparatorText("Primary Stance");

         ImGui::BeginGroup();

         if (ImGui::Selectable(properties.hintnode.primary_stance.is_stand_different()
                                  ? "Stand <different>"
                                  : "Stand",
                               are_flags_set(stance_flags, world::stance_flags::stand))) {
            if (are_flags_set(stance_flags, world::stance_flags::stand)) {
               and_stance_flags = ~world::stance_flags::stand;
            }
            else {
               or_stance_flags = world::stance_flags::stand;
            }
         }

         if (ImGui::Selectable(properties.hintnode.primary_stance.is_crouch_different()
                                  ? "Crouch <different>"
                                  : "Crouch",
                               are_flags_set(stance_flags, world::stance_flags::crouch))) {
            if (are_flags_set(stance_flags, world::stance_flags::crouch)) {
               and_stance_flags = ~world::stance_flags::crouch;
            }
            else {
               or_stance_flags = world::stance_flags::crouch;
            }
         }

         if (ImGui::Selectable(properties.hintnode.primary_stance.is_prone_different()
                                  ? "Prone <different>"
                                  : "Prone",
                               are_flags_set(stance_flags, world::stance_flags::prone))) {
            if (are_flags_set(stance_flags, world::stance_flags::prone)) {
               and_stance_flags = ~world::stance_flags::prone;
            }
            else {
               or_stance_flags = world::stance_flags::prone;
            }
         }

         ImGui::EndGroup();

         if (ImGui::IsItemDeactivated()) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.hintnode.primary_stance.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::hintnode_id>()) {
                  world::hintnode* hintnode =
                     world::find_entity(_world.hintnodes,
                                        selected.get<world::hintnode_id>());

                  if (not hintnode) continue;

                  edit_bundle.push_back(
                     edits::make_set_value(&hintnode->primary_stance,
                                           (hintnode->primary_stance | or_stance_flags) &
                                              and_stance_flags));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context, {.closed = true});
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_hintnode_secondary_stance)) {
         const world::stance_flags stance_flags =
            properties.hintnode.secondary_stance.value_or_default();

         world::stance_flags or_stance_flags = world::stance_flags::none;
         world::stance_flags and_stance_flags =
            world::stance_flags::secondary_stance_mask;

         ImGui::SeparatorText("Secondary Stance");

         ImGui::BeginGroup();
         ImGui::PushID("Secondary");

         if (ImGui::Selectable(properties.hintnode.secondary_stance.is_stand_different()
                                  ? "Stand <different>"
                                  : "Stand",
                               are_flags_set(stance_flags, world::stance_flags::stand))) {
            if (are_flags_set(stance_flags, world::stance_flags::stand)) {
               and_stance_flags = ~world::stance_flags::stand;
            }
            else {
               or_stance_flags = world::stance_flags::stand;
            }
         }

         if (ImGui::Selectable(properties.hintnode.secondary_stance.is_crouch_different()
                                  ? "Crouch <different>"
                                  : "Crouch",
                               are_flags_set(stance_flags, world::stance_flags::crouch))) {
            if (are_flags_set(stance_flags, world::stance_flags::crouch)) {
               and_stance_flags = ~world::stance_flags::crouch;
            }
            else {
               or_stance_flags = world::stance_flags::crouch;
            }
         }

         if (ImGui::Selectable(properties.hintnode.secondary_stance.is_prone_different()
                                  ? "Prone <different>"
                                  : "Prone",
                               are_flags_set(stance_flags, world::stance_flags::prone))) {
            if (are_flags_set(stance_flags, world::stance_flags::prone)) {
               and_stance_flags = ~world::stance_flags::prone;
            }
            else {
               or_stance_flags = world::stance_flags::prone;
            }
         }

         if (ImGui::Selectable(properties.hintnode.secondary_stance.is_left_different()
                                  ? "Left <different>"
                                  : "Left",
                               are_flags_set(stance_flags, world::stance_flags::left))) {
            if (are_flags_set(stance_flags, world::stance_flags::left)) {
               and_stance_flags = ~world::stance_flags::left;
            }
            else {
               or_stance_flags = world::stance_flags::left;
            }
         }

         if (ImGui::Selectable(properties.hintnode.secondary_stance.is_right_different()
                                  ? "Right <different>"
                                  : "Right",
                               are_flags_set(stance_flags, world::stance_flags::right))) {
            if (are_flags_set(stance_flags, world::stance_flags::right)) {
               and_stance_flags = ~world::stance_flags::right;
            }
            else {
               or_stance_flags = world::stance_flags::right;
            }
         }

         ImGui::PopID();
         ImGui::EndGroup();

         if (ImGui::IsItemDeactivated()) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.hintnode.secondary_stance.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::hintnode_id>()) {
                  world::hintnode* hintnode =
                     world::find_entity(_world.hintnodes,
                                        selected.get<world::hintnode_id>());

                  if (not hintnode) continue;

                  edit_bundle.push_back(
                     edits::make_set_value(&hintnode->secondary_stance,
                                           (hintnode->secondary_stance | or_stance_flags) &
                                              and_stance_flags));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context, {.closed = true});
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_hintnode_mode)) {
         if (world::hintnode_mode mode =
                properties.hintnode.mode.value_or(world::hintnode_mode::none);
             ImGui::BeginCombo("Mode", properties.hintnode.mode.is_different()
                                          ? "<different>"
                                          : world::to_ui_string(mode))) {
            for (const world::hintnode_mode other_mode : {
                    world::hintnode_mode::none,
                    world::hintnode_mode::attack,
                    world::hintnode_mode::defend,
                    world::hintnode_mode::both,
                 }) {
               if (ImGui::Selectable(world::to_ui_string(other_mode),
                                     properties.hintnode.mode.is_different()
                                        ? false
                                        : mode == other_mode)) {
                  edits::bundle_vector edit_bundle;
                  edit_bundle.reserve(properties.hintnode.mode.count());

                  for (const world::selected_entity& selected :
                       _interaction_targets.selection) {
                     if (selected.is<world::hintnode_id>()) {
                        world::hintnode* hintnode =
                           world::find_entity(_world.hintnodes,
                                              selected.get<world::hintnode_id>());

                        if (not hintnode) continue;

                        edit_bundle.push_back(
                           edits::make_set_value(&hintnode->mode, other_mode));
                     }
                  }

                  _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                          _edit_context, {.closed = true});
               }
            }

            ImGui::EndCombo();
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_hintnode_radius)) {
         if (float radius = properties.hintnode.radius.value_or(0.0f);
             ImGui::DragFloat("Radius", &radius, 0.25f, 0.0f, 1e10f,
                              properties.hintnode.radius.is_different()
                                 ? "<different>"
                                 : "%.3f",
                              ImGuiSliderFlags_AlwaysClamp)) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.hintnode.radius.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::hintnode_id>()) {
                  world::hintnode* hintnode =
                     world::find_entity(_world.hintnodes,
                                        selected.get<world::hintnode_id>());

                  if (not hintnode) continue;

                  edit_bundle.push_back(edits::make_set_value(&hintnode->radius, radius));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_barrier)) {
      ImGui::Separator();

      if (float rotation_degrees = properties.barrier.rotation_angle.value_or(0.0f) *
                                   180.0f / std::numbers::pi_v<float>;
          ImGui::DragFloat("Rotation", &rotation_degrees, 1.0f, 0.0f, 0.0f,
                           properties.barrier.rotation_angle.is_different()
                              ? "<different>"
                              : "%.3f")) {
         const float rotation = rotation_degrees / 180.0f * std::numbers::pi_v<float>;

         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.barrier.rotation_angle.count());

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::barrier_id>()) {
               world::barrier* barrier =
                  world::find_entity(_world.barriers,
                                     selected.get<world::barrier_id>());

               if (not barrier) continue;

               edit_bundle.push_back(
                  edits::make_set_value(&barrier->rotation_angle, rotation));
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context);
      }

      if (ImGui::IsItemDeactivatedAfterEdit()) {
         _edit_stack_world.close_last();
      }

      // Size
      {
         float size_x = properties.barrier.size.x.value_or(1.0f);
         float size_y = properties.barrier.size.y.value_or(1.0f);

         const float item_inner_spacing = ImGui::GetStyle().ItemInnerSpacing.x;

         const float item_width_one =
            std::max(1.0f, floorf((ImGui::CalcItemWidth() - item_inner_spacing) / 2.0f));
         const float item_width_last =
            std::max(1.0f, floorf(ImGui::CalcItemWidth() -
                                  (item_width_one + item_inner_spacing)));

         ImGui::BeginGroup();
         ImGui::PushID("SizeXZ");
         ImGui::SetNextItemWidth(item_width_one);
         const bool x_edited =
            ImGui::DragFloat("##X", &size_x, 0.0625f, 0.0f, 0.0f,
                             properties.barrier.size.x.is_different()
                                ? "<different>"
                                : "X:%.3f");

         ImGui::SameLine(0, item_inner_spacing);
         ImGui::SetNextItemWidth(item_width_last);
         const bool y_edited =
            ImGui::DragFloat("##Y", &size_y, 0.0625f, 0.0f, 0.0f,
                             properties.barrier.size.y.is_different()
                                ? "<different>"
                                : "Y:%.3f");

         ImGui::PopID();

         ImGui::SameLine(0, item_inner_spacing);
         ImGui::TextUnformatted("Size");

         ImGui::EndGroup();

         if (x_edited or y_edited) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.barrier.size.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::barrier_id>()) {
                  world::barrier* barrier =
                     world::find_entity(_world.barriers,
                                        selected.get<world::barrier_id>());

                  if (not barrier) continue;

                  float2 size = {
                     x_edited ? size_x : barrier->size.x,
                     y_edited ? size_y : barrier->size.y,
                  };

                  edit_bundle.push_back(edits::make_set_value(&barrier->size, size));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_planning_connection)) {
      if (bool jump = properties.planning_connection.jump.value_or(true);
          ImGui::CheckboxTristate("Jump", &jump,
                                  properties.planning_connection.jump.is_different())) {

         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.planning_connection.jump.count());

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::planning_connection_id>()) {
               world::planning_connection* connection =
                  world::find_entity(_world.planning_connections,
                                     selected.get<world::planning_connection_id>());

               if (not connection) continue;

               edit_bundle.push_back(edits::make_set_value(&connection->jump, jump));
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context, {.closed = true});
      }

      ImGui::SameLine();

      if (bool jet_jump = properties.planning_connection.jet_jump.value_or(true);
          ImGui::CheckboxTristate("Jet Jump", &jet_jump,
                                  properties.planning_connection.jet_jump.is_different())) {

         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.planning_connection.jet_jump.count());

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::planning_connection_id>()) {
               world::planning_connection* connection =
                  world::find_entity(_world.planning_connections,
                                     selected.get<world::planning_connection_id>());

               if (not connection) continue;

               edit_bundle.push_back(
                  edits::make_set_value(&connection->jet_jump, jet_jump));
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context, {.closed = true});
      }

      ImGui::SameLine();

      if (bool one_way = properties.planning_connection.one_way.value_or(true);
          ImGui::CheckboxTristate("One Way", &one_way,
                                  properties.planning_connection.one_way.is_different())) {

         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.planning_connection.one_way.count());

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::planning_connection_id>()) {
               world::planning_connection* connection =
                  world::find_entity(_world.planning_connections,
                                     selected.get<world::planning_connection_id>());

               if (not connection) continue;

               edit_bundle.push_back(
                  edits::make_set_value(&connection->one_way, one_way));
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context, {.closed = true});
      }

      {

         bool is_dynamic = properties.planning_connection.dynamic.value_or(false);
         int8 dynamic_group =
            properties.planning_connection.dynamic_group.value_or(0);

         bool edited = false;
         bool close = false;

         if (ImGui::CheckboxTristate("Dynamic", &is_dynamic,
                                     properties.planning_connection.dynamic.is_different())) {
            edited |= true;
            close = true;

            dynamic_group = is_dynamic ? 1 : 0;
         }

         if (is_dynamic or
             properties.planning_connection.dynamic_group.is_different()) {
            const int8 min_group = 1;
            const int8 max_group = 8;

            edited |=
               ImGui::SliderScalar("Dynamic Group", ImGuiDataType_S8,
                                   &dynamic_group, &min_group, &max_group,
                                   properties.planning_connection.dynamic_group.is_different()
                                      ? "<different>"
                                      : nullptr,
                                   ImGuiSliderFlags_AlwaysClamp);
         }

         if (edited) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.planning_connection.dynamic_group.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::planning_connection_id>()) {
                  world::planning_connection* connection =
                     world::find_entity(_world.planning_connections,
                                        selected.get<world::planning_connection_id>());

                  if (not connection) continue;

                  edit_bundle.push_back(edits::make_set_value(&connection->dynamic_group,
                                                              dynamic_group));
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context, {.closed = close});

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_ai_flags)) {
      const world::ai_path_flags ai_flags = properties.ai_flags.value_or_default();

      world::ai_path_flags or_ai_flags = world::ai_path_flags::none;
      world::ai_path_flags and_ai_flags = world::ai_path_flags::all;

      ImGui::SeparatorText("Flags");

      ImGui::BeginGroup();

      if (ImGui::Selectable(properties.ai_flags.is_soldier_different()
                               ? "Soldier <different>"
                               : "Soldier",
                            are_flags_set(ai_flags, world::ai_path_flags::soldier))) {
         if (are_flags_set(ai_flags, world::ai_path_flags::soldier)) {
            and_ai_flags = ~world::ai_path_flags::soldier;
         }
         else {
            or_ai_flags = world::ai_path_flags::soldier;
         }
      }

      if (ImGui::Selectable(properties.ai_flags.is_hover_different()
                               ? "Hover <different>"
                               : "Hover",
                            are_flags_set(ai_flags, world::ai_path_flags::hover))) {
         if (are_flags_set(ai_flags, world::ai_path_flags::hover)) {
            and_ai_flags = ~world::ai_path_flags::hover;
         }
         else {
            or_ai_flags = world::ai_path_flags::hover;
         }
      }

      if (ImGui::Selectable(properties.ai_flags.is_small_different()
                               ? "Small <different>"
                               : "Small",
                            are_flags_set(ai_flags, world::ai_path_flags::small))) {
         if (are_flags_set(ai_flags, world::ai_path_flags::small)) {
            and_ai_flags = ~world::ai_path_flags::small;
         }
         else {
            or_ai_flags = world::ai_path_flags::small;
         }
      }

      if (ImGui::Selectable(properties.ai_flags.is_medium_different()
                               ? "Medium <different>"
                               : "Medium",
                            are_flags_set(ai_flags, world::ai_path_flags::medium))) {
         if (are_flags_set(ai_flags, world::ai_path_flags::medium)) {
            and_ai_flags = ~world::ai_path_flags::medium;
         }
         else {
            or_ai_flags = world::ai_path_flags::medium;
         }
      }

      if (ImGui::Selectable(properties.ai_flags.is_huge_different()
                               ? "Huge <different>"
                               : "Huge",
                            are_flags_set(ai_flags, world::ai_path_flags::huge))) {
         if (are_flags_set(ai_flags, world::ai_path_flags::huge)) {
            and_ai_flags = ~world::ai_path_flags::huge;
         }
         else {
            or_ai_flags = world::ai_path_flags::huge;
         }
      }

      if (ImGui::Selectable(properties.ai_flags.is_flyer_different()
                               ? "Flyer <different>"
                               : "Flyer",
                            are_flags_set(ai_flags, world::ai_path_flags::flyer))) {
         if (are_flags_set(ai_flags, world::ai_path_flags::flyer)) {
            and_ai_flags = ~world::ai_path_flags::flyer;
         }
         else {
            or_ai_flags = world::ai_path_flags::flyer;
         }
      }

      ImGui::EndGroup();

      if (ImGui::IsItemDeactivated()) {
         edits::bundle_vector edit_bundle;
         edit_bundle.reserve(properties.ai_flags.count());

         for (const world::selected_entity& selected : _interaction_targets.selection) {
            if (selected.is<world::barrier_id>()) {
               world::barrier* barrier =
                  world::find_entity(_world.barriers,
                                     selected.get<world::barrier_id>());

               if (not barrier) continue;

               edit_bundle.push_back(
                  edits::make_set_value(&barrier->flags, (barrier->flags | or_ai_flags) &
                                                            and_ai_flags));
            }
            else if (selected.is<world::planning_connection_id>()) {
               world::planning_connection* connection =
                  world::find_entity(_world.planning_connections,
                                     selected.get<world::planning_connection_id>());

               if (not connection) continue;

               edit_bundle.push_back(
                  edits::make_set_value(&connection->flags,
                                        (connection->flags | or_ai_flags) & and_ai_flags));
            }
         }

         _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                 _edit_context, {.closed = true});
      }
   }

   if (are_flags_set(flags, multi_select_flags::has_block)) {
      ImGui::Separator();

      if (are_flags_set(flags, multi_select_flags::has_block_step_properties)) {
         if (float step_height = properties.block.step_height.value_or(0.25f);
             ImGui::DragFloat("Step Height", &step_height, 0.125f * 0.25f, 0.125f, 1.0f,
                              properties.block.step_height.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.block.step_height.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::block_id>()) {
                  const world::block_id block_id = selected.get<world::block_id>();
                  const std::optional<uint32> block_index =
                     world::find_block(_world.blocks, block_id);

                  if (not block_index) continue;

                  switch (block_id.type()) {
                  case world::block_type::box: {
                  } break;
                  case world::block_type::ramp: {
                  } break;
                  case world::block_type::quad: {
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     switch (block.mesh_description.type) {
                     case world::block_custom_mesh_type::stairway: {
                        world::block_custom_mesh_description_stairway stairway =
                           block.mesh_description.stairway;

                        stairway.step_height = step_height;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position,
                                                                stairway));
                     } break;
                     case world::block_custom_mesh_type::stairway_floating: {
                        world::block_custom_mesh_description_stairway_floating stairway =
                           block.mesh_description.stairway_floating;

                        stairway.step_height = step_height;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position,
                                                                stairway));
                     } break;
                     case world::block_custom_mesh_type::ring: {
                     } break;
                     case world::block_custom_mesh_type::beveled_box: {
                     } break;
                     case world::block_custom_mesh_type::curve: {
                     } break;
                     case world::block_custom_mesh_type::cylinder: {
                     } break;
                     case world::block_custom_mesh_type::cone: {
                     } break;
                     case world::block_custom_mesh_type::arch: {
                     } break;
                     }
                  } break;
                  case world::block_type::hemisphere: {
                  } break;
                  case world::block_type::pyramid: {
                  } break;
                  case world::block_type::terrain_cut_box: {
                  } break;
                  }
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }

         if (float first_step_offset =
                properties.block.first_step_offset.value_or(0.0f);
             ImGui::DragFloat("First Step Offset", &first_step_offset, 0.125f, 0.0f, 0.0f,
                              properties.block.first_step_offset.is_different()
                                 ? "<different>"
                                 : "%.3f")) {

            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.block.first_step_offset.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::block_id>()) {
                  const world::block_id block_id = selected.get<world::block_id>();
                  const std::optional<uint32> block_index =
                     world::find_block(_world.blocks, block_id);

                  if (not block_index) continue;

                  switch (block_id.type()) {
                  case world::block_type::box: {
                  } break;
                  case world::block_type::ramp: {
                  } break;
                  case world::block_type::quad: {
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     switch (block.mesh_description.type) {
                     case world::block_custom_mesh_type::stairway: {
                        world::block_custom_mesh_description_stairway stairway =
                           block.mesh_description.stairway;

                        stairway.first_step_offset = first_step_offset;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position,
                                                                stairway));
                     } break;
                     case world::block_custom_mesh_type::stairway_floating: {
                        world::block_custom_mesh_description_stairway_floating stairway =
                           block.mesh_description.stairway_floating;

                        stairway.first_step_offset = first_step_offset;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position,
                                                                stairway));
                     } break;
                     case world::block_custom_mesh_type::ring: {
                     } break;
                     case world::block_custom_mesh_type::beveled_box: {
                     } break;
                     case world::block_custom_mesh_type::curve: {
                     } break;
                     case world::block_custom_mesh_type::cylinder: {
                     } break;
                     case world::block_custom_mesh_type::cone: {
                     } break;
                     case world::block_custom_mesh_type::arch: {
                     } break;
                     }
                  } break;
                  case world::block_type::hemisphere: {
                  } break;
                  case world::block_type::pyramid: {
                  } break;
                  case world::block_type::terrain_cut_box: {
                  } break;
                  }
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_block_segments)) {
         const uint16 min_segments = 3;
         const uint16 max_segments = 256;

         if (uint16 segments = properties.block.segments.value_or(16);
             ImGui::SliderScalar("Segments", ImGuiDataType_U16, &segments,
                                 &min_segments, &max_segments,
                                 properties.block.segments.is_different()
                                    ? "<different>"
                                    : nullptr)) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.block.segments.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::block_id>()) {
                  const world::block_id block_id = selected.get<world::block_id>();
                  const std::optional<uint32> block_index =
                     world::find_block(_world.blocks, block_id);

                  if (not block_index) continue;

                  switch (block_id.type()) {
                  case world::block_type::box: {
                  } break;
                  case world::block_type::ramp: {
                  } break;
                  case world::block_type::quad: {
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     switch (block.mesh_description.type) {
                     case world::block_custom_mesh_type::stairway: {
                     } break;
                     case world::block_custom_mesh_type::stairway_floating: {
                     } break;
                     case world::block_custom_mesh_type::ring: {
                        world::block_custom_mesh_description_ring ring =
                           block.mesh_description.ring;

                        ring.segments = segments;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position, ring));
                     } break;
                     case world::block_custom_mesh_type::beveled_box: {
                     } break;
                     case world::block_custom_mesh_type::curve: {
                        world::block_custom_mesh_description_curve curve =
                           block.mesh_description.curve;

                        curve.segments = segments;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position, curve));
                     } break;
                     case world::block_custom_mesh_type::cylinder: {
                        world::block_custom_mesh_description_cylinder cylinder =
                           block.mesh_description.cylinder;

                        cylinder.segments = segments;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position,
                                                                cylinder));
                     } break;
                     case world::block_custom_mesh_type::cone: {
                        world::block_custom_mesh_description_cone cone =
                           block.mesh_description.cone;

                        cone.segments = segments;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position, cone));
                     } break;
                     case world::block_custom_mesh_type::arch: {
                        world::block_custom_mesh_description_arch arch =
                           block.mesh_description.arch;

                        arch.segments = segments;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position, arch));
                     } break;
                     }
                  } break;
                  case world::block_type::hemisphere: {
                  } break;
                  case world::block_type::pyramid: {
                  } break;
                  case world::block_type::terrain_cut_box: {
                  } break;
                  }
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_block_flat_shading)) {
         if (bool flat_shading = properties.block.flat_shading.value_or(true);
             ImGui::CheckboxTristate("Flat Shading", &flat_shading,
                                     properties.block.flat_shading.is_different())) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.block.flat_shading.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::block_id>()) {
                  const world::block_id block_id = selected.get<world::block_id>();
                  const std::optional<uint32> block_index =
                     world::find_block(_world.blocks, block_id);

                  if (not block_index) continue;

                  switch (block_id.type()) {
                  case world::block_type::box: {
                  } break;
                  case world::block_type::ramp: {
                  } break;
                  case world::block_type::quad: {
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     switch (block.mesh_description.type) {
                     case world::block_custom_mesh_type::stairway: {
                     } break;
                     case world::block_custom_mesh_type::stairway_floating: {
                     } break;
                     case world::block_custom_mesh_type::ring: {
                        world::block_custom_mesh_description_ring ring =
                           block.mesh_description.ring;

                        ring.flat_shading = flat_shading;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position, ring));
                     } break;
                     case world::block_custom_mesh_type::beveled_box: {
                     } break;
                     case world::block_custom_mesh_type::curve: {
                     } break;
                     case world::block_custom_mesh_type::cylinder: {
                        world::block_custom_mesh_description_cylinder cylinder =
                           block.mesh_description.cylinder;

                        cylinder.flat_shading = flat_shading;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position,
                                                                cylinder));
                     } break;
                     case world::block_custom_mesh_type::cone: {
                        world::block_custom_mesh_description_cone cone =
                           block.mesh_description.cone;

                        cone.flat_shading = flat_shading;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position, cone));
                     } break;
                     case world::block_custom_mesh_type::arch: {
                     } break;
                     }
                  } break;
                  case world::block_type::hemisphere: {
                  } break;
                  case world::block_type::pyramid: {
                  } break;
                  case world::block_type::terrain_cut_box: {
                  } break;
                  }
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context, {.closed = true});
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_block_texture_loops)) {
         if (float texture_loops = properties.block.texture_loops.value_or(1.0f);
             ImGui::DragFloat("Texture Loops", &texture_loops, 1.0f, 0.0f, 0.0f,
                              properties.block.texture_loops.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.block.texture_loops.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::block_id>()) {
                  const world::block_id block_id = selected.get<world::block_id>();
                  const std::optional<uint32> block_index =
                     world::find_block(_world.blocks, block_id);

                  if (not block_index) continue;

                  switch (block_id.type()) {
                  case world::block_type::box: {
                  } break;
                  case world::block_type::ramp: {
                  } break;
                  case world::block_type::quad: {
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     switch (block.mesh_description.type) {
                     case world::block_custom_mesh_type::stairway: {
                     } break;
                     case world::block_custom_mesh_type::stairway_floating: {
                     } break;
                     case world::block_custom_mesh_type::ring: {
                        world::block_custom_mesh_description_ring ring =
                           block.mesh_description.ring;

                        ring.texture_loops = texture_loops;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position, ring));
                     } break;
                     case world::block_custom_mesh_type::beveled_box: {
                     } break;
                     case world::block_custom_mesh_type::curve: {
                        world::block_custom_mesh_description_curve curve =
                           block.mesh_description.curve;

                        curve.texture_loops = texture_loops;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position, curve));
                     } break;
                     case world::block_custom_mesh_type::cylinder: {
                        world::block_custom_mesh_description_cylinder cylinder =
                           block.mesh_description.cylinder;

                        cylinder.texture_loops = texture_loops;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position,
                                                                cylinder));
                     } break;
                     case world::block_custom_mesh_type::cone: {
                     } break;
                     case world::block_custom_mesh_type::arch: {
                     } break;
                     }
                  } break;
                  case world::block_type::hemisphere: {
                  } break;
                  case world::block_type::pyramid: {
                  } break;
                  case world::block_type::terrain_cut_box: {
                  } break;
                  }
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_block_ring_properties)) {
         if (float inner_radius = properties.block.ring.inner_radius.value_or(16.0f);
             ImGui::DragFloat("Inner Radius", &inner_radius, 1.0f, 0.0f, 1e10f,
                              properties.block.ring.inner_radius.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.block.ring.inner_radius.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::block_id>()) {
                  const world::block_id block_id = selected.get<world::block_id>();
                  const std::optional<uint32> block_index =
                     world::find_block(_world.blocks, block_id);

                  if (not block_index) continue;

                  switch (block_id.type()) {
                  case world::block_type::box: {
                  } break;
                  case world::block_type::ramp: {
                  } break;
                  case world::block_type::quad: {
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     switch (block.mesh_description.type) {
                     case world::block_custom_mesh_type::stairway: {
                     } break;
                     case world::block_custom_mesh_type::stairway_floating: {
                     } break;
                     case world::block_custom_mesh_type::ring: {
                        world::block_custom_mesh_description_ring ring =
                           block.mesh_description.ring;

                        ring.inner_radius = inner_radius;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position, ring));
                     } break;
                     case world::block_custom_mesh_type::beveled_box: {
                     } break;
                     case world::block_custom_mesh_type::curve: {
                     } break;
                     case world::block_custom_mesh_type::cylinder: {
                     } break;
                     case world::block_custom_mesh_type::cone: {
                     } break;
                     case world::block_custom_mesh_type::arch: {
                     } break;
                     }
                  } break;
                  case world::block_type::hemisphere: {
                  } break;
                  case world::block_type::pyramid: {
                  } break;
                  case world::block_type::terrain_cut_box: {
                  } break;
                  }
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }

         if (float outer_radius = properties.block.ring.outer_radius.value_or(4.0f);
             ImGui::DragFloat("Outer Radius", &outer_radius, 1.0f, 0.0f, 1e10f,
                              properties.block.ring.outer_radius.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.block.ring.outer_radius.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::block_id>()) {
                  const world::block_id block_id = selected.get<world::block_id>();
                  const std::optional<uint32> block_index =
                     world::find_block(_world.blocks, block_id);

                  if (not block_index) continue;

                  switch (block_id.type()) {
                  case world::block_type::box: {
                  } break;
                  case world::block_type::ramp: {
                  } break;
                  case world::block_type::quad: {
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     switch (block.mesh_description.type) {
                     case world::block_custom_mesh_type::stairway: {
                     } break;
                     case world::block_custom_mesh_type::stairway_floating: {
                     } break;
                     case world::block_custom_mesh_type::ring: {
                        world::block_custom_mesh_description_ring ring =
                           block.mesh_description.ring;

                        ring.outer_radius = outer_radius;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position, ring));
                     } break;
                     case world::block_custom_mesh_type::beveled_box: {
                     } break;
                     case world::block_custom_mesh_type::curve: {
                     } break;
                     case world::block_custom_mesh_type::cylinder: {
                     } break;
                     case world::block_custom_mesh_type::cone: {
                     } break;
                     case world::block_custom_mesh_type::arch: {
                     } break;
                     }
                  } break;
                  case world::block_type::hemisphere: {
                  } break;
                  case world::block_type::pyramid: {
                  } break;
                  case world::block_type::terrain_cut_box: {
                  } break;
                  }
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_block_beveled_box_properties)) {
         if (float amount = properties.block.beveled_box.amount.value_or(0.125f);
             ImGui::DragFloat("Bevel Amount", &amount, 0.0625f, 0.0f, 1e10f,
                              properties.block.beveled_box.amount.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.block.beveled_box.amount.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::block_id>()) {
                  const world::block_id block_id = selected.get<world::block_id>();
                  const std::optional<uint32> block_index =
                     world::find_block(_world.blocks, block_id);

                  if (not block_index) continue;

                  switch (block_id.type()) {
                  case world::block_type::box: {
                  } break;
                  case world::block_type::ramp: {
                  } break;
                  case world::block_type::quad: {
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     switch (block.mesh_description.type) {
                     case world::block_custom_mesh_type::stairway: {
                     } break;
                     case world::block_custom_mesh_type::stairway_floating: {
                     } break;
                     case world::block_custom_mesh_type::ring: {
                     } break;
                     case world::block_custom_mesh_type::beveled_box: {
                        world::block_custom_mesh_description_beveled_box beveled_box =
                           block.mesh_description.beveled_box;

                        beveled_box.amount = amount;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position,
                                                                beveled_box));
                     } break;
                     case world::block_custom_mesh_type::curve: {
                     } break;
                     case world::block_custom_mesh_type::cylinder: {
                     } break;
                     case world::block_custom_mesh_type::cone: {
                     } break;
                     case world::block_custom_mesh_type::arch: {
                     } break;
                     }
                  } break;
                  case world::block_type::hemisphere: {
                  } break;
                  case world::block_type::pyramid: {
                  } break;
                  case world::block_type::terrain_cut_box: {
                  } break;
                  }
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }

         if (bool bevel_top = properties.block.beveled_box.bevel_top.value_or(true);
             ImGui::CheckboxTristate("Bevel Top", &bevel_top,
                                     properties.block.beveled_box.bevel_top.is_different())) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.block.beveled_box.bevel_top.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::block_id>()) {
                  const world::block_id block_id = selected.get<world::block_id>();
                  const std::optional<uint32> block_index =
                     world::find_block(_world.blocks, block_id);

                  if (not block_index) continue;

                  switch (block_id.type()) {
                  case world::block_type::box: {
                  } break;
                  case world::block_type::ramp: {
                  } break;
                  case world::block_type::quad: {
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     switch (block.mesh_description.type) {
                     case world::block_custom_mesh_type::stairway: {
                     } break;
                     case world::block_custom_mesh_type::stairway_floating: {
                     } break;
                     case world::block_custom_mesh_type::ring: {
                     } break;
                     case world::block_custom_mesh_type::beveled_box: {
                        world::block_custom_mesh_description_beveled_box beveled_box =
                           block.mesh_description.beveled_box;

                        beveled_box.bevel_top = bevel_top;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position,
                                                                beveled_box));
                     } break;
                     case world::block_custom_mesh_type::curve: {
                     } break;
                     case world::block_custom_mesh_type::cylinder: {
                     } break;
                     case world::block_custom_mesh_type::cone: {
                     } break;
                     case world::block_custom_mesh_type::arch: {
                     } break;
                     }
                  } break;
                  case world::block_type::hemisphere: {
                  } break;
                  case world::block_type::pyramid: {
                  } break;
                  case world::block_type::terrain_cut_box: {
                  } break;
                  }
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context, {.closed = true});
         }

         if (bool bevel_sides = properties.block.beveled_box.bevel_sides.value_or(true);
             ImGui::CheckboxTristate("Bevel Sides", &bevel_sides,
                                     properties.block.beveled_box.bevel_sides.is_different())) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.block.beveled_box.bevel_sides.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::block_id>()) {
                  const world::block_id block_id = selected.get<world::block_id>();
                  const std::optional<uint32> block_index =
                     world::find_block(_world.blocks, block_id);

                  if (not block_index) continue;

                  switch (block_id.type()) {
                  case world::block_type::box: {
                  } break;
                  case world::block_type::ramp: {
                  } break;
                  case world::block_type::quad: {
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     switch (block.mesh_description.type) {
                     case world::block_custom_mesh_type::stairway: {
                     } break;
                     case world::block_custom_mesh_type::stairway_floating: {
                     } break;
                     case world::block_custom_mesh_type::ring: {
                     } break;
                     case world::block_custom_mesh_type::beveled_box: {
                        world::block_custom_mesh_description_beveled_box beveled_box =
                           block.mesh_description.beveled_box;

                        beveled_box.bevel_sides = bevel_sides;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position,
                                                                beveled_box));
                     } break;
                     case world::block_custom_mesh_type::curve: {
                     } break;
                     case world::block_custom_mesh_type::cylinder: {
                     } break;
                     case world::block_custom_mesh_type::cone: {
                     } break;
                     case world::block_custom_mesh_type::arch: {
                     } break;
                     }
                  } break;
                  case world::block_type::hemisphere: {
                  } break;
                  case world::block_type::pyramid: {
                  } break;
                  case world::block_type::terrain_cut_box: {
                  } break;
                  }
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context, {.closed = true});
         }

         if (bool bevel_bottom =
                properties.block.beveled_box.bevel_bottom.value_or(true);
             ImGui::CheckboxTristate("Bevel Bottom", &bevel_bottom,
                                     properties.block.beveled_box.bevel_bottom.is_different())) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.block.beveled_box.bevel_bottom.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::block_id>()) {
                  const world::block_id block_id = selected.get<world::block_id>();
                  const std::optional<uint32> block_index =
                     world::find_block(_world.blocks, block_id);

                  if (not block_index) continue;

                  switch (block_id.type()) {
                  case world::block_type::box: {
                  } break;
                  case world::block_type::ramp: {
                  } break;
                  case world::block_type::quad: {
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     switch (block.mesh_description.type) {
                     case world::block_custom_mesh_type::stairway: {
                     } break;
                     case world::block_custom_mesh_type::stairway_floating: {
                     } break;
                     case world::block_custom_mesh_type::ring: {
                     } break;
                     case world::block_custom_mesh_type::beveled_box: {
                        world::block_custom_mesh_description_beveled_box beveled_box =
                           block.mesh_description.beveled_box;

                        beveled_box.bevel_bottom = bevel_bottom;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position,
                                                                beveled_box));
                     } break;
                     case world::block_custom_mesh_type::curve: {
                     } break;
                     case world::block_custom_mesh_type::cylinder: {
                     } break;
                     case world::block_custom_mesh_type::cone: {
                     } break;
                     case world::block_custom_mesh_type::arch: {
                     } break;
                     }
                  } break;
                  case world::block_type::hemisphere: {
                  } break;
                  case world::block_type::pyramid: {
                  } break;
                  case world::block_type::terrain_cut_box: {
                  } break;
                  }
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context, {.closed = true});
         }
      }

      if (are_flags_set(flags, multi_select_flags::has_block_arch_properties)) {
         if (float crown_length = properties.block.arch.crown_length.value_or(2.0f);
             ImGui::DragFloat("Crown Length", &crown_length, 0.5f, 0.0f, 1e10f,
                              properties.block.arch.crown_length.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.block.arch.crown_length.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::block_id>()) {
                  const world::block_id block_id = selected.get<world::block_id>();
                  const std::optional<uint32> block_index =
                     world::find_block(_world.blocks, block_id);

                  if (not block_index) continue;

                  switch (block_id.type()) {
                  case world::block_type::box: {
                  } break;
                  case world::block_type::ramp: {
                  } break;
                  case world::block_type::quad: {
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     switch (block.mesh_description.type) {
                     case world::block_custom_mesh_type::stairway: {
                     } break;
                     case world::block_custom_mesh_type::stairway_floating: {
                     } break;
                     case world::block_custom_mesh_type::ring: {
                     } break;
                     case world::block_custom_mesh_type::beveled_box: {
                     } break;
                     case world::block_custom_mesh_type::curve: {
                     } break;
                     case world::block_custom_mesh_type::cylinder: {
                     } break;
                     case world::block_custom_mesh_type::cone: {
                     } break;
                     case world::block_custom_mesh_type::arch: {
                        world::block_custom_mesh_description_arch arch =
                           block.mesh_description.arch;

                        arch.crown_length = crown_length;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position, arch));
                     } break;
                     }
                  } break;
                  case world::block_type::hemisphere: {
                  } break;
                  case world::block_type::pyramid: {
                  } break;
                  case world::block_type::terrain_cut_box: {
                  } break;
                  }
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }

         if (float crown_height = properties.block.arch.crown_height.value_or(0.25f);
             ImGui::DragFloat("Crown Height", &crown_height, 0.125f, 0.0f, 1e10f,
                              properties.block.arch.crown_height.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.block.arch.crown_height.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::block_id>()) {
                  const world::block_id block_id = selected.get<world::block_id>();
                  const std::optional<uint32> block_index =
                     world::find_block(_world.blocks, block_id);

                  if (not block_index) continue;

                  switch (block_id.type()) {
                  case world::block_type::box: {
                  } break;
                  case world::block_type::ramp: {
                  } break;
                  case world::block_type::quad: {
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     switch (block.mesh_description.type) {
                     case world::block_custom_mesh_type::stairway: {
                     } break;
                     case world::block_custom_mesh_type::stairway_floating: {
                     } break;
                     case world::block_custom_mesh_type::ring: {
                     } break;
                     case world::block_custom_mesh_type::beveled_box: {
                     } break;
                     case world::block_custom_mesh_type::curve: {
                     } break;
                     case world::block_custom_mesh_type::cylinder: {
                     } break;
                     case world::block_custom_mesh_type::cone: {
                     } break;
                     case world::block_custom_mesh_type::arch: {
                        world::block_custom_mesh_description_arch arch =
                           block.mesh_description.arch;

                        arch.crown_height = crown_height;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position, arch));
                     } break;
                     }
                  } break;
                  case world::block_type::hemisphere: {
                  } break;
                  case world::block_type::pyramid: {
                  } break;
                  case world::block_type::terrain_cut_box: {
                  } break;
                  }
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }

         if (float curve_height = properties.block.arch.curve_height.value_or(1.0f);
             ImGui::DragFloat("Curve Height", &curve_height, 0.5f, 0.0f, 1e10f,
                              properties.block.arch.curve_height.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.block.arch.curve_height.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::block_id>()) {
                  const world::block_id block_id = selected.get<world::block_id>();
                  const std::optional<uint32> block_index =
                     world::find_block(_world.blocks, block_id);

                  if (not block_index) continue;

                  switch (block_id.type()) {
                  case world::block_type::box: {
                  } break;
                  case world::block_type::ramp: {
                  } break;
                  case world::block_type::quad: {
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     switch (block.mesh_description.type) {
                     case world::block_custom_mesh_type::stairway: {
                     } break;
                     case world::block_custom_mesh_type::stairway_floating: {
                     } break;
                     case world::block_custom_mesh_type::ring: {
                     } break;
                     case world::block_custom_mesh_type::beveled_box: {
                     } break;
                     case world::block_custom_mesh_type::curve: {
                     } break;
                     case world::block_custom_mesh_type::cylinder: {
                     } break;
                     case world::block_custom_mesh_type::cone: {
                     } break;
                     case world::block_custom_mesh_type::arch: {
                        world::block_custom_mesh_description_arch arch =
                           block.mesh_description.arch;

                        arch.curve_height = curve_height;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position, arch));
                     } break;
                     }
                  } break;
                  case world::block_type::hemisphere: {
                  } break;
                  case world::block_type::pyramid: {
                  } break;
                  case world::block_type::terrain_cut_box: {
                  } break;
                  }
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }

         if (float span_length = properties.block.arch.span_length.value_or(8.0f);
             ImGui::DragFloat("Span Length", &span_length, 0.5f, 0.0f, 1e10f,
                              properties.block.arch.span_length.is_different()
                                 ? "<different>"
                                 : "%.3f")) {
            edits::bundle_vector edit_bundle;
            edit_bundle.reserve(properties.block.arch.span_length.count());

            for (const world::selected_entity& selected : _interaction_targets.selection) {
               if (selected.is<world::block_id>()) {
                  const world::block_id block_id = selected.get<world::block_id>();
                  const std::optional<uint32> block_index =
                     world::find_block(_world.blocks, block_id);

                  if (not block_index) continue;

                  switch (block_id.type()) {
                  case world::block_type::box: {
                  } break;
                  case world::block_type::ramp: {
                  } break;
                  case world::block_type::quad: {
                  } break;
                  case world::block_type::custom: {
                     const world::block_description_custom& block =
                        _world.blocks.custom.description[*block_index];

                     switch (block.mesh_description.type) {
                     case world::block_custom_mesh_type::stairway: {
                     } break;
                     case world::block_custom_mesh_type::stairway_floating: {
                     } break;
                     case world::block_custom_mesh_type::ring: {
                     } break;
                     case world::block_custom_mesh_type::beveled_box: {
                     } break;
                     case world::block_custom_mesh_type::curve: {
                     } break;
                     case world::block_custom_mesh_type::cylinder: {
                     } break;
                     case world::block_custom_mesh_type::cone: {
                     } break;
                     case world::block_custom_mesh_type::arch: {
                        world::block_custom_mesh_description_arch arch =
                           block.mesh_description.arch;

                        arch.span_length = span_length;

                        edit_bundle.push_back(
                           edits::make_set_block_custom_metrics(*block_index,
                                                                block.rotation,
                                                                block.position, arch));
                     } break;
                     }
                  } break;
                  case world::block_type::hemisphere: {
                  } break;
                  case world::block_type::pyramid: {
                  } break;
                  case world::block_type::terrain_cut_box: {
                  } break;
                  }
               }
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(edit_bundle)),
                                    _edit_context);

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               _edit_stack_world.close_last();
            }
         }
      }
   }
}

}
