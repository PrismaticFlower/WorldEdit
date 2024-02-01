#include "world_edit.hpp"

#include "edits/add_property.hpp"
#include "edits/add_sector_object.hpp"
#include "edits/creation_entity_set.hpp"
#include "edits/delete_path_property.hpp"
#include "edits/delete_sector_object.hpp"
#include "edits/delete_sector_point.hpp"
#include "edits/imgui_ext.hpp"
#include "edits/set_value.hpp"
#include "utility/string_icompare.hpp"
#include "world/utility/grounding.hpp"
#include "world/utility/hintnode_traits.hpp"
#include "world/utility/object_properties.hpp"
#include "world/utility/path_properties.hpp"
#include "world/utility/raycast.hpp"
#include "world/utility/region_properties.hpp"
#include "world/utility/sector_fill.hpp"
#include "world/utility/selection_centre.hpp"
#include "world/utility/world_utilities.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using namespace std::literals;

namespace we {

void world_edit::ui_show_world_selection_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSizeConstraints({520.0f * _display_scale, 620.0f * _display_scale},
                                       {std::numeric_limits<float>::max(),
                                        620.0f * _display_scale});

   bool selection_open = true;

   if (ImGui::Begin("Selection", &selection_open, ImGuiWindowFlags_NoCollapse)) {
      const bool is_multi_select = _interaction_targets.selection.size() > 1;

      int id_offset = 0;

      for (int selected_index = 0;
           selected_index < _interaction_targets.selection.size(); ++selected_index) {
         ImGui::PushID(selected_index + id_offset);

         const world::selected_entity& selected =
            _interaction_targets.selection[selected_index];

         if (std::holds_alternative<world::object_id>(selected)) {
            world::object* object =
               world::find_entity(_world.objects, std::get<world::object_id>(selected));

            if (not object) {
               ImGui::PopID();

               continue;
            }

            if (is_multi_select) {
               bool keep_selected = true;

               if (not ImGui::CollapsingHeader("Object", &keep_selected,
                                               ImGuiTreeNodeFlags_DefaultOpen) and
                   keep_selected) {
                  ImGui::PopID();

                  continue;
               }
               else if (not keep_selected) {
                  _interaction_targets.selection.remove(object->id);

                  selected_index -= 1;
                  id_offset += 1;

                  ImGui::PopID();

                  continue;
               }
            }
            else {
               ImGui::SeparatorText("Object");
            }

            ImGui::InputText("Name", object, &world::object::name, &_edit_stack_world,
                             &_edit_context, [&](std::string* edited_value) {
                                *edited_value =
                                   world::create_unique_name(_world.objects,
                                                             *edited_value);
                             });
            ImGui::InputTextAutoComplete(
               "Class Name", object, &world::object::class_name,
               &_edit_stack_world, &_edit_context, [&] {
                  std::array<std::string_view, 6> entries;
                  std::size_t matching_count = 0;

                  _asset_libraries.odfs.view_existing(
                     [&](const std::span<const assets::stable_string> assets) noexcept {
                        for (const std::string_view asset : assets) {
                           if (matching_count == entries.size()) break;
                           if (not asset.contains(object->class_name)) {
                              continue;
                           }

                           entries[matching_count] = asset;

                           ++matching_count;
                        }
                     });

                  return entries;
               });

            if (ImGui::IsItemDeactivatedAfterEdit()) {
               std::vector<world::instance_property> new_instance_properties =
                  world::make_object_instance_properties(
                     *_object_classes[object->class_name].definition,
                     object->instance_properties);

               if (new_instance_properties != object->instance_properties) {
                  _edit_stack_world
                     .apply(edits::make_set_value(object->id, &world::object::instance_properties,
                                                  std::move(new_instance_properties),
                                                  object->instance_properties),
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

            ImGui::LayerPick("Layer", object, &_edit_stack_world, &_edit_context);

            ImGui::Separator();

            ImGui::DragQuat("Rotation", object, &world::object::rotation,
                            &_edit_stack_world, &_edit_context);
            ImGui::DragFloat3("Position", object, &world::object::position,
                              &_edit_stack_world, &_edit_context);

            const bool ground_object =
               ImGui::Button("Ground Object", {ImGui::CalcItemWidth(), 0.0f});

            ImGui::Separator();

            ImGui::SliderInt("Team", object, &world::object::team,
                             &_edit_stack_world, &_edit_context, 0, 15, "%d",
                             ImGuiSliderFlags_AlwaysClamp);

            ImGui::Separator();

            for (std::size_t i = 0; i < object->instance_properties.size(); ++i) {
               world::instance_property& prop = object->instance_properties[i];

               if (prop.key.contains("Path")) {
                  ImGui::InputKeyValueAutoComplete(
                     object, &world::object::instance_properties, i,
                     &_edit_stack_world, &_edit_context, [&] {
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
                     object, &world::object::instance_properties, i,
                     &_edit_stack_world, &_edit_context, [&] {
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
                  ImGui::InputKeyValue(object, &world::object::instance_properties,
                                       i, &_edit_stack_world, &_edit_context);
               }
            }

            if (ground_object) {
               if (const std::optional<float3> grounded_position =
                      world::ground_object(*object, _world, _object_classes,
                                           _world_layers_hit_mask);
                   grounded_position) {
                  _edit_stack_world.apply(edits::make_set_value(object->id,
                                                                &world::object::position,
                                                                *grounded_position,
                                                                object->position),
                                          _edit_context, {.closed = true});
               }
            }
         }
         else if (std::holds_alternative<world::light_id>(selected)) {
            world::light* light =
               world::find_entity(_world.lights, std::get<world::light_id>(selected));

            if (not light) {
               ImGui::PopID();

               continue;
            }

            if (is_multi_select) {
               bool keep_selected = true;

               if (not ImGui::CollapsingHeader("Light", &keep_selected,
                                               ImGuiTreeNodeFlags_DefaultOpen) and
                   keep_selected) {
                  ImGui::PopID();

                  continue;
               }
               else if (not keep_selected) {
                  _interaction_targets.selection.remove(light->id);

                  selected_index -= 1;
                  id_offset += 1;

                  ImGui::PopID();

                  continue;
               }
            }
            else {
               ImGui::SeparatorText("Light");
            }

            ImGui::InputText("Name", light, &world::light::name, &_edit_stack_world,
                             &_edit_context, [&](std::string* edited_value) {
                                *edited_value =
                                   world::create_unique_name(_world.lights,
                                                             *edited_value);
                             });
            ImGui::LayerPick("Layer", light, &_edit_stack_world, &_edit_context);

            ImGui::Separator();

            ImGui::DragQuat("Rotation", light, &world::light::rotation,
                            &_edit_stack_world, &_edit_context);
            ImGui::DragFloat3("Position", light, &world::light::position,
                              &_edit_stack_world, &_edit_context);

            ImGui::Separator();

            ImGui::ColorEdit3("Color", light, &world::light::color,
                              &_edit_stack_world, &_edit_context,
                              ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

            ImGui::Checkbox("Static", light, &world::light::static_,
                            &_edit_stack_world, &_edit_context);
            ImGui::SameLine();
            ImGui::Checkbox("Shadow Caster", light, &world::light::shadow_caster,
                            &_edit_stack_world, &_edit_context);
            ImGui::SameLine();
            ImGui::Checkbox("Specular Caster", light, &world::light::specular_caster,
                            &_edit_stack_world, &_edit_context);

            ImGui::EnumSelect(
               "Light Type", light, &world::light::light_type,
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

            if (light->light_type == world::light_type::point or
                light->light_type == world::light_type::spot) {
               ImGui::DragFloat("Range", light, &world::light::range,
                                &_edit_stack_world, &_edit_context);

               if (light->light_type == world::light_type::spot) {
                  ImGui::DragFloat("Inner Cone Angle", light,
                                   &world::light::inner_cone_angle,
                                   &_edit_stack_world, &_edit_context, 0.01f,
                                   0.0f, light->outer_cone_angle, "%.3f",
                                   ImGuiSliderFlags_AlwaysClamp);
                  ImGui::DragFloat("Outer Cone Angle", light,
                                   &world::light::outer_cone_angle, &_edit_stack_world,
                                   &_edit_context, 0.01f, light->inner_cone_angle,
                                   1.570f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
               }

               ImGui::Separator();
            }

            ImGui::InputTextAutoComplete(
               "Texture", light, &world::light::texture, &_edit_stack_world,
               &_edit_context, [&] {
                  std::array<std::string_view, 6> entries;
                  std::size_t matching_count = 0;

                  _asset_libraries.textures.view_existing(
                     [&](const std::span<const assets::stable_string> assets) noexcept {
                        for (const std::string_view asset : assets) {
                           if (matching_count == entries.size()) break;
                           if (not asset.contains(light->texture)) {
                              continue;
                           }

                           entries[matching_count] = asset;

                           ++matching_count;
                        }
                     });

                  return entries;
               });

            if (world::is_directional_light(*light) and not light->texture.empty()) {
               ImGui::DragFloat2("Directional Texture Tiling", light,
                                 &world::light::directional_texture_tiling,
                                 &_edit_stack_world, &_edit_context, 0.01f);
               ImGui::DragFloat2("Directional Texture Offset", light,
                                 &world::light::directional_texture_offset,
                                 &_edit_stack_world, &_edit_context, 0.01f);
            }

            ImGui::Separator();

            if (is_region_light(*light)) {
               ImGui::InputText("Region Name", light,
                                &world::light::region_name, &_edit_stack_world,
                                &_edit_context, [&](std::string* edited_value) {
                                   *edited_value =
                                      world::create_unique_light_region_name(
                                         _world.lights, _world.regions,
                                         edited_value->empty() ? light->name
                                                               : *edited_value);
                                });
               ImGui::DragQuat("Region Rotation", light, &world::light::region_rotation,
                               &_edit_stack_world, &_edit_context);
               ImGui::DragFloat3("Region Size", light, &world::light::region_size,
                                 &_edit_stack_world, &_edit_context);
            }
         }
         else if (std::holds_alternative<world::path_id_node_pair>(selected)) {
            auto [id, node_index] = std::get<world::path_id_node_pair>(selected);

            world::path* path = world::find_entity(_world.paths, id);

            if (not path or node_index >= path->nodes.size()) {
               ImGui::PopID();

               continue;
            }

            if (is_multi_select) {
               bool keep_selected = true;

               if (not ImGui::CollapsingHeader("Path Node", &keep_selected,
                                               ImGuiTreeNodeFlags_DefaultOpen) and
                   keep_selected) {
                  ImGui::PopID();

                  continue;
               }
               else if (not keep_selected) {
                  _interaction_targets.selection.remove(
                     world::path_id_node_pair{id, node_index});

                  selected_index -= 1;
                  id_offset += 1;

                  ImGui::PopID();

                  continue;
               }
            }
            else {
               ImGui::SeparatorText("Path Node");
            }

            ImGui::InputText("Name", path, &world::path::name, &_edit_stack_world,
                             &_edit_context, [&](std::string* edited_value) {
                                *edited_value =
                                   world::create_unique_name(_world.paths,
                                                             edited_value->empty()
                                                                ? "Path 0"sv
                                                                : std::string_view{
                                                                     *edited_value});
                             });
            ImGui::LayerPick("Layer", path, &_edit_stack_world, &_edit_context);

            ImGui::EnumSelect("Path Type", path, &world::path::type,
                              &_edit_stack_world, &_edit_context,
                              {enum_select_option{"None", world::path_type::none},
                               enum_select_option{"Entity Follow",
                                                  world::path_type::entity_follow},
                               enum_select_option{"Formation", world::path_type::formation},
                               enum_select_option{"Patrol", world::path_type::patrol}});

            ImGui::EnumSelect(
               "Spline Type", path, &world::path::spline_type,
               &_edit_stack_world, &_edit_context,
               {enum_select_option{"None", world::path_spline_type::none},
                enum_select_option{"Linear", world::path_spline_type::linear},
                enum_select_option{"Hermite", world::path_spline_type::hermite},
                enum_select_option{"Catmull-Rom", world::path_spline_type::catmull_rom}});

            if (not path->properties.empty()) ImGui::Separator();

            for (std::size_t i = 0; i < path->properties.size(); ++i) {
               ImGui::PushID(static_cast<int>(i));

               bool delete_property = false;

               ImGui::InputKeyValueWithDelete(path, &world::path::properties, i,
                                              &delete_property,
                                              &_edit_stack_world, &_edit_context);

               if (delete_property) {
                  _edit_stack_world.apply(edits::make_delete_path_property(path->id,
                                                                           i, _world),
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

            ImGui::Separator();
            ImGui::Text("Node %i", static_cast<int>(node_index));

            ImGui::DragQuat("Rotation", path, node_index, &world::path::node::rotation,
                            &_edit_stack_world, &_edit_context);
            ImGui::DragFloat3("Position", path, node_index, &world::path::node::position,
                              &_edit_stack_world, &_edit_context);

            ImGui::PushID("Node");

            for (std::size_t prop_index = 0;
                 prop_index < path->nodes[node_index].properties.size(); ++prop_index) {
               ImGui::PushID(static_cast<int>(prop_index));

               bool delete_property = false;

               ImGui::InputKeyValueWithDelete(path, node_index, prop_index,
                                              &delete_property,
                                              &_edit_stack_world, &_edit_context);

               if (delete_property) {
                  _edit_stack_world.apply(edits::make_delete_path_node_property(
                                             path->id, node_index, prop_index, _world),
                                          _edit_context);
               }

               ImGui::PopID();
            }

            ImGui::PopID();

            if (ImGui::BeginCombo("Add Property##node", "<select property>")) {
               for (const char* prop : world::get_path_node_properties(path->type)) {
                  if (ImGui::Selectable(prop)) {
                     _edit_stack_world.apply(edits::make_add_property(path->id, node_index,
                                                                      prop),
                                             _edit_context);
                  }
               }

               ImGui::EndCombo();
            }

            ImGui::Separator();

            if (ImGui::Button("Add Nodes", {ImGui::CalcItemWidth(), 0.0f})) {
               _edit_stack_world.apply(edits::make_creation_entity_set(
                                          world::path{.name = path->name,
                                                      .layer = path->layer,
                                                      .nodes = {world::path::node{}},
                                                      .id = world::max_id},
                                          _interaction_targets.creation_entity),
                                       _edit_context);
               _entity_creation_context = {};
            }

            if (ImGui::Button("Move Path", {ImGui::CalcItemWidth(), 0.0f})) {
               _selection_edit_tool = selection_edit_tool::move_path;
               _move_selection_amount = {0.0f, 0.0f, 0.0f};
               _move_entire_path_id = id;
            }
         }
         else if (std::holds_alternative<world::region_id>(selected)) {
            world::region* region =
               world::find_entity(_world.regions, std::get<world::region_id>(selected));

            if (not region) {
               ImGui::PopID();

               continue;
            }

            if (is_multi_select) {
               bool keep_selected = true;

               if (not ImGui::CollapsingHeader("Region", &keep_selected,
                                               ImGuiTreeNodeFlags_DefaultOpen) and
                   keep_selected) {
                  ImGui::PopID();

                  continue;
               }
               else if (not keep_selected) {
                  _interaction_targets.selection.remove(region->id);

                  selected_index -= 1;
                  id_offset += 1;

                  ImGui::PopID();

                  continue;
               }
            }
            else {
               ImGui::SeparatorText("Region");
            }

            ImGui::InputText("Name", region, &world::region::name, &_edit_stack_world,
                             &_edit_context, [&](std::string* edited_value) {
                                *edited_value =
                                   world::create_unique_name(_world.regions,
                                                             *edited_value);
                             });
            ImGui::LayerPick("Layer", region, &_edit_stack_world, &_edit_context);

            ImGui::Separator();

            const world::region_type start_region_type =
               world::get_region_type(region->description);
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
                  _edit_stack_world.apply(edits::make_set_value(region->id,
                                                                &world::region::description,
                                                                to_string(region_type),
                                                                region->description),
                                          _edit_context);

                  const world::region_allowed_shapes allowed_shapes =
                     world::get_region_allowed_shapes(region_type);

                  if (allowed_shapes == world::region_allowed_shapes::sphere and
                      region->shape != world::region_shape::sphere) {
                     _edit_stack_world
                        .apply(edits::make_set_value(region->id, &world::region::shape,
                                                     world::region_shape::sphere,
                                                     region->shape),
                               _edit_context, {.closed = true, .transparent = true});
                  }
                  else if (allowed_shapes == world::region_allowed_shapes::box_cylinder and
                           region->shape == world::region_shape::sphere) {
                     _edit_stack_world.apply(
                        edits::make_set_value(region->id, &world::region::shape,
                                              world::region_shape::box, region->shape),
                        _edit_context, {.closed = true, .transparent = true});
                  }
               }
            }

            switch (region_type) {
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
                  _edit_stack_world.apply(
                     edits::make_set_value(region->id, &world::region::description,
                                           world::pack_region_sound_stream(properties),
                                           region->description),
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

               value_changed |= ImGui::InputText("Sound Name", &properties.sound_name);

               value_changed |= ImGui::DragFloat("Min Distance Divisor",
                                                 &properties.min_distance_divisor,
                                                 1.0f, 1.0f, 1000000.0f, "%.3f",
                                                 ImGuiSliderFlags_AlwaysClamp);

               if (value_changed) {
                  _edit_stack_world.apply(
                     edits::make_set_value(region->id, &world::region::description,
                                           world::pack_region_sound_static(properties),
                                           region->description),
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
                  _edit_stack_world.apply(
                     edits::make_set_value(region->id, &world::region::description,
                                           world::pack_region_sound_space(properties),
                                           region->description),
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
                  _edit_stack_world.apply(
                     edits::make_set_value(region->id, &world::region::description,
                                           world::pack_region_sound_trigger(properties),
                                           region->description),
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
                  _edit_stack_world
                     .apply(edits::make_set_value(region->id, &world::region::description,
                                                  world::pack_region_foley_fx(properties),
                                                  region->description),
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

               if (float3 color_top =
                      properties.color_top.value_or(float3{0.0f, 0.0f, 0.0f});
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

               value_changed |=
                  ImGui::InputText("Environment Map", &properties.env_map);

               if (value_changed) {
                  _edit_stack_world
                     .apply(edits::make_set_value(region->id, &world::region::description,
                                                  world::pack_region_shadow(properties),
                                                  region->description),
                            _edit_context);
               }

               ImGui::EndGroup();

               if (ImGui::IsItemDeactivatedAfterEdit()) {
                  _edit_stack_world.close_last();
               }
            } break;
            case world::region_type::rumble: {
               world::rumble_region_properties properties =
                  world::unpack_region_rumble(region->description);

               ImGui::BeginGroup();

               bool value_changed = false;

               value_changed |=
                  ImGui::InputText("Rumble Class", &properties.rumble_class);
               value_changed |=
                  ImGui::InputText("Particle Effect", &properties.particle_effect);

               if (value_changed) {
                  _edit_stack_world
                     .apply(edits::make_set_value(region->id, &world::region::description,
                                                  world::pack_region_rumble(properties),
                                                  region->description),
                            _edit_context);
               }

               ImGui::EndGroup();

               if (ImGui::IsItemDeactivatedAfterEdit()) {
                  _edit_stack_world.close_last();
               }
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

               if (float building_dead_scale = properties.building_scale.value_or(1.0f);
                   ImGui::DragFloat("Building Dead Scale", &building_dead_scale, 0.01f)) {
                  properties.building_dead_scale = building_dead_scale;
                  value_changed = true;
               }

               if (float building_unbuilt_scale =
                      properties.building_scale.value_or(1.0f);
                   ImGui::DragFloat("Building Unbuilt Scale",
                                    &building_unbuilt_scale, 0.01f)) {
                  properties.building_unbuilt_scale = building_unbuilt_scale;
                  value_changed = true;
               }

               if (value_changed) {
                  _edit_stack_world
                     .apply(edits::make_set_value(region->id, &world::region::description,
                                                  world::pack_region_damage(properties),
                                                  region->description),
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
                  _edit_stack_world
                     .apply(edits::make_set_value(region->id, &world::region::description,
                                                  world::pack_region_ai_vis(properties),
                                                  region->description),
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
                  _edit_stack_world.apply(
                     edits::make_set_value(region->id, &world::region::description,
                                           world::pack_region_colorgrading(properties),
                                           region->description),
                     _edit_context);
               }

               ImGui::EndGroup();

               if (ImGui::IsItemDeactivatedAfterEdit()) {
                  _edit_stack_world.close_last();
               }
            } break;
            }

            ImGui::InputText("Description", region, &world::region::description,
                             &_edit_stack_world, &_edit_context,
                             [&](std::string*) {});

            ImGui::Separator();

            ImGui::DragQuat("Rotation", region, &world::region::rotation,
                            &_edit_stack_world, &_edit_context);
            ImGui::DragFloat3("Position", region, &world::region::position,
                              &_edit_stack_world, &_edit_context);

            switch (region->shape) {
            case world::region_shape::box: {
               ImGui::DragFloat3("Size", region, &world::region::size, &_edit_stack_world,
                                 &_edit_context, 1.0f, 0.0f, 1e10f);
            } break;
            case world::region_shape::sphere: {
               float radius = length(region->size);

               if (ImGui::DragFloat("Radius", &radius, 0.1f)) {

                  const float radius_sq = radius * radius;
                  const float size = std::sqrt(radius_sq / 3.0f);

                  _edit_stack_world.apply(edits::make_set_value(region->id,
                                                                &world::region::size,
                                                                {size, size, size},
                                                                region->size),
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
                     .apply(edits::make_set_value(region->id, &world::region::size,
                                                  float3{region->size.x, height / 2.0f,
                                                         region->size.z},
                                                  region->size),
                            _edit_context);
               }

               if (ImGui::IsItemDeactivated()) {
                  _edit_stack_world.close_last();
               }

               if (float radius = length(float2{region->size.x, region->size.z});
                   ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f, 1e10f)) {
                  const float radius_sq = radius * radius;
                  const float size = std::sqrt(radius_sq / 2.0f);

                  _edit_stack_world
                     .apply(edits::make_set_value(region->id, &world::region::size,
                                                  float3{size, region->size.y, size},
                                                  region->size),
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
               ImGui::EnumSelect("Shape", region, &world::region::shape,
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
               ImGui::EnumSelect("Shape", region, &world::region::shape,
                                 &_edit_stack_world, &_edit_context,
                                 {enum_select_option{"Box", world::region_shape::box},
                                  enum_select_option{"Cylinder",
                                                     world::region_shape::cylinder}});
            } break;
            }
         }
         else if (std::holds_alternative<world::sector_id>(selected)) {
            world::sector* sector =
               world::find_entity(_world.sectors, std::get<world::sector_id>(selected));

            if (not sector) {
               ImGui::PopID();

               continue;
            }

            if (is_multi_select) {
               bool keep_selected = true;

               if (not ImGui::CollapsingHeader("Sector", &keep_selected,
                                               ImGuiTreeNodeFlags_DefaultOpen) and
                   keep_selected) {
                  ImGui::PopID();

                  continue;
               }
               else if (not keep_selected) {
                  _interaction_targets.selection.remove(sector->id);

                  selected_index -= 1;
                  id_offset += 1;

                  ImGui::PopID();

                  continue;
               }
            }
            else {
               ImGui::SeparatorText("Sector");
            }

            ImGui::InputText("Name", sector, &world::sector::name, &_edit_stack_world,
                             &_edit_context, [&](std::string* edited_value) {
                                *edited_value =
                                   world::create_unique_name(_world.sectors,
                                                             *edited_value);
                             });

            ImGui::DragFloat("Base", sector, &world::sector::base, &_edit_stack_world,
                             &_edit_context, 1.0f, 0.0f, 0.0f, "Y:%.3f");
            ImGui::DragFloat("Height", sector, &world::sector::height,
                             &_edit_stack_world, &_edit_context);

            ImGui::SeparatorText("Points");

            for (int i = 0; i < sector->points.size(); ++i) {
               ImGui::BeginGroup();
               ImGui::PushID(i);

               float2 point = sector->points[i];

               ImGui::DragSectorPoint("##point", sector, i, &_edit_stack_world,
                                      &_edit_context);

               ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
               if (ImGui::Button("Move")) {
                  _move_sector_point_id = sector->id;
                  _move_sector_point_index = i;
                  _move_selection_amount = {0.0f, 0.0f, 0.0f};
                  _selection_edit_tool = selection_edit_tool::move_sector_point;
               }

               const bool disable_delete = sector->points.size() <= 2;

               if (disable_delete) ImGui::BeginDisabled();

               ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
               if (ImGui::Button("X")) {
                  _edit_stack_world.apply(edits::make_delete_sector_point(sector->id,
                                                                          i, _world),
                                          _edit_context);
               }

               if (disable_delete) ImGui::EndDisabled();

               ImGui::PopID();
               ImGui::EndGroup();

               if (ImGui::IsItemHovered()) {
                  const float3 bottom =
                     float3{sector->points[i].x, sector->base, sector->points[i].y};
                  const float3 top =
                     float3{sector->points[i].x, sector->base + sector->height,
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
                                          _interaction_targets.creation_entity),
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
                  edits::make_set_value(sector->id, &world::sector::objects,
                                        world::sector_fill(*sector, _world.objects,
                                                           _object_classes),
                                        sector->objects),
                  _edit_context);
            }

            if (ImGui::Button("Add Object", {ImGui::CalcItemWidth(), 0.0f})) {
               _selection_edit_context.using_add_object_to_sector = true;
               _selection_edit_context.sector_to_add_object_to = sector->id;
            }

            if (_selection_edit_context.sector_to_add_object_to == sector->id and
                std::exchange(_selection_edit_context.add_hovered_object, false) and
                _interaction_targets.hovered_entity and
                std::holds_alternative<world::object_id>(
                   *_interaction_targets.hovered_entity)) {
               if (world::object* object =
                      world::find_entity(_world.objects,
                                         std::get<world::object_id>(
                                            *_interaction_targets.hovered_entity));
                   object) {
                  bool already_has_object = false;

                  for (const auto& other_object : sector->objects) {
                     if (string::iequals(object->name, other_object)) {
                        already_has_object = true;
                     }
                  }

                  if (not already_has_object) {
                     _edit_stack_world
                        .apply(edits::make_add_sector_object(sector->id, object->name),
                               _edit_context);
                  }
               }
            }
         }
         else if (std::holds_alternative<world::portal_id>(selected)) {
            world::portal* portal =
               world::find_entity(_world.portals, std::get<world::portal_id>(selected));

            if (not portal) {
               ImGui::PopID();

               continue;
            }

            if (is_multi_select) {
               bool keep_selected = true;

               if (not ImGui::CollapsingHeader("Portal", &keep_selected,
                                               ImGuiTreeNodeFlags_DefaultOpen) and
                   keep_selected) {
                  ImGui::PopID();

                  continue;
               }
               else if (not keep_selected) {
                  _interaction_targets.selection.remove(portal->id);

                  selected_index -= 1;
                  id_offset += 1;

                  ImGui::PopID();

                  continue;
               }
            }
            else {
               ImGui::SeparatorText("Portal");
            }

            ImGui::InputText("Name", portal, &world::portal::name, &_edit_stack_world,
                             &_edit_context, [&](std::string* edited_value) {
                                *edited_value =
                                   world::create_unique_name(_world.portals,
                                                             *edited_value);
                             });

            ImGui::DragFloat("Width", portal, &world::portal::width,
                             &_edit_stack_world, &_edit_context, 1.0f, 0.25f, 1e10f);
            ImGui::DragFloat("Height", portal, &world::portal::height,
                             &_edit_stack_world, &_edit_context, 1.0f, 0.25f, 1e10f);

            ImGui::InputTextAutoComplete("Linked Sector 1", portal,
                                         &world::portal::sector1,
                                         &_edit_stack_world, &_edit_context, [&] {
                                            std::array<std::string_view, 6> entries;
                                            std::size_t matching_count = 0;

                                            for (const auto& sector : _world.sectors) {
                                               if (sector.name.contains(portal->sector1)) {
                                                  if (matching_count == entries.size())
                                                     break;

                                                  entries[matching_count] =
                                                     sector.name;

                                                  ++matching_count;
                                               }
                                            }

                                            return entries;
                                         });

            ImGui::InputTextAutoComplete("Linked Sector 2", portal,
                                         &world::portal::sector2,
                                         &_edit_stack_world, &_edit_context, [&] {
                                            std::array<std::string_view, 6> entries;
                                            std::size_t matching_count = 0;

                                            for (const auto& sector : _world.sectors) {
                                               if (sector.name.contains(portal->sector2)) {
                                                  if (matching_count == entries.size())
                                                     break;

                                                  entries[matching_count] =
                                                     sector.name;

                                                  ++matching_count;
                                               }
                                            }

                                            return entries;
                                         });

            ImGui::DragQuat("Rotation", portal, &world::portal::rotation,
                            &_edit_stack_world, &_edit_context);
            ImGui::DragFloat3("Position", portal, &world::portal::position,
                              &_edit_stack_world, &_edit_context);
         }
         else if (std::holds_alternative<world::hintnode_id>(selected)) {
            world::hintnode* hintnode =
               world::find_entity(_world.hintnodes,
                                  std::get<world::hintnode_id>(selected));

            if (not hintnode) {
               ImGui::PopID();

               continue;
            }

            if (is_multi_select) {
               bool keep_selected = true;

               if (not ImGui::CollapsingHeader("Hintnode", &keep_selected,
                                               ImGuiTreeNodeFlags_DefaultOpen) and
                   keep_selected) {
                  ImGui::PopID();

                  continue;
               }
               else if (not keep_selected) {
                  _interaction_targets.selection.remove(hintnode->id);

                  selected_index -= 1;
                  id_offset += 1;

                  ImGui::PopID();

                  continue;
               }
            }
            else {
               ImGui::SeparatorText("Hintnode");
            }

            ImGui::InputText("Name", hintnode, &world::hintnode::name, &_edit_stack_world,
                             &_edit_context, [&](std::string* edited_value) {
                                *edited_value =
                                   world::create_unique_name(_world.hintnodes,
                                                             *edited_value);
                             });
            ImGui::LayerPick("Layer", hintnode, &_edit_stack_world, &_edit_context);

            ImGui::Separator();

            ImGui::DragQuat("Rotation", hintnode, &world::hintnode::rotation,
                            &_edit_stack_world, &_edit_context);
            ImGui::DragFloat3("Position", hintnode, &world::hintnode::position,
                              &_edit_stack_world, &_edit_context);

            ImGui::Separator();

            ImGui::EnumSelect(
               "Type", hintnode, &world::hintnode::type, &_edit_stack_world,
               &_edit_context,
               {enum_select_option{"Snipe", world::hintnode_type::snipe},
                enum_select_option{"Patrol", world::hintnode_type::patrol},
                enum_select_option{"Cover", world::hintnode_type::cover},
                enum_select_option{"Access", world::hintnode_type::access},
                enum_select_option{"Jet Jump", world::hintnode_type::jet_jump},
                enum_select_option{"Mine", world::hintnode_type::mine},
                enum_select_option{"Land", world::hintnode_type::land},
                enum_select_option{"Fortification", world::hintnode_type::fortification},
                enum_select_option{"Vehicle Cover", world::hintnode_type::vehicle_cover}});

            const world::hintnode_traits hintnode_traits =
               world::get_hintnode_traits(hintnode->type);

            if (hintnode_traits.has_command_post) {
               if (ImGui::BeginCombo("Command Post", hintnode->command_post.c_str())) {
                  for (const auto& object : _world.objects) {
                     const assets::odf::definition& definition =
                        *_object_classes[object.class_name].definition;

                     if (string::iequals(definition.header.class_label,
                                         "commandpost")) {
                        if (ImGui::Selectable(object.name.c_str())) {
                           _edit_stack_world
                              .apply(edits::make_set_value(hintnode->id,
                                                           &world::hintnode::command_post,
                                                           object.name,
                                                           hintnode->command_post),
                                     _edit_context);
                        }
                     }
                  }
                  ImGui::EndCombo();
               }
            }

            if (hintnode_traits.has_primary_stance) {
               ImGui::EditFlags("Primary Stance", hintnode,
                                &world::hintnode::primary_stance,
                                &_edit_stack_world, &_edit_context,
                                {{"Stand", world::stance_flags::stand},
                                 {"Crouch", world::stance_flags::crouch},
                                 {"Prone", world::stance_flags::prone}});
            }

            if (hintnode_traits.has_secondary_stance) {
               ImGui::EditFlags("Secondary Stance", hintnode,
                                &world::hintnode::secondary_stance,
                                &_edit_stack_world, &_edit_context,
                                {{"Stand", world::stance_flags::stand},
                                 {"Crouch", world::stance_flags::crouch},
                                 {"Prone", world::stance_flags::prone},
                                 {"Left", world::stance_flags::left},
                                 {"Right", world::stance_flags::right}});
            }

            if (hintnode_traits.has_mode) {
               ImGui::EnumSelect(
                  "Mode", hintnode, &world::hintnode::mode, &_edit_stack_world,
                  &_edit_context,
                  {enum_select_option{"None", world::hintnode_mode::none},
                   enum_select_option{"Attack", world::hintnode_mode::attack},
                   enum_select_option{"Defend", world::hintnode_mode::defend},
                   enum_select_option{"Both", world::hintnode_mode::both}});
            }

            if (hintnode_traits.has_radius) {
               ImGui::DragFloat("Radius", hintnode, &world::hintnode::radius,
                                &_edit_stack_world, &_edit_context, 0.25f, 0.0f,
                                1e10f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            }
         }
         else if (std::holds_alternative<world::barrier_id>(selected)) {
            world::barrier* barrier =
               world::find_entity(_world.barriers,
                                  std::get<world::barrier_id>(selected));

            if (not barrier) {
               ImGui::PopID();

               continue;
            }

            if (is_multi_select) {
               bool keep_selected = true;

               if (not ImGui::CollapsingHeader("Barrier", &keep_selected,
                                               ImGuiTreeNodeFlags_DefaultOpen) and
                   keep_selected) {
                  ImGui::PopID();

                  continue;
               }
               else if (not keep_selected) {
                  _interaction_targets.selection.remove(barrier->id);

                  selected_index -= 1;
                  id_offset += 1;

                  ImGui::PopID();

                  continue;
               }
            }
            else {
               ImGui::SeparatorText("Barrier");
            }

            ImGui::InputText("Name", barrier, &world::barrier::name, &_edit_stack_world,
                             &_edit_context, [&](std::string* edited_value) {
                                *edited_value =
                                   world::create_unique_name(_world.barriers,
                                                             *edited_value);
                             });

            ImGui::DragBarrierRotation("Rotation", barrier, &_edit_stack_world,
                                       &_edit_context);

            ImGui::DragFloat3("Position", barrier, &world::barrier::position,
                              &_edit_stack_world, &_edit_context, 0.25f);

            ImGui::DragFloat2XZ("Size", barrier, &world::barrier::size,
                                &_edit_stack_world, &_edit_context, 1.0f, 0.0f, 1e10f);

            ImGui::EditFlags("Flags", barrier, &world::barrier::flags,
                             &_edit_stack_world, &_edit_context,
                             {{"Soldier", world::ai_path_flags::soldier},
                              {"Hover", world::ai_path_flags::hover},
                              {"Small", world::ai_path_flags::small},
                              {"Medium", world::ai_path_flags::medium},
                              {"Huge", world::ai_path_flags::huge},
                              {"Flyer", world::ai_path_flags::flyer}});
         }
         else if (std::holds_alternative<world::planning_hub_id>(selected)) {
            world::planning_hub* hub =
               world::find_entity(_world.planning_hubs,
                                  std::get<world::planning_hub_id>(selected));

            if (not hub) {
               ImGui::PopID();

               continue;
            }

            if (is_multi_select) {
               bool keep_selected = true;

               if (not ImGui::CollapsingHeader("AI Planning Hub", &keep_selected,
                                               ImGuiTreeNodeFlags_DefaultOpen) and
                   keep_selected) {
                  ImGui::PopID();

                  continue;
               }
               else if (not keep_selected) {
                  _interaction_targets.selection.remove(hub->id);

                  selected_index -= 1;
                  id_offset += 1;

                  ImGui::PopID();

                  continue;
               }
            }
            else {
               ImGui::SeparatorText("AI Planning Hub");
            }

            ImGui::InputText("Name", hub, &world::planning_hub::name, &_edit_stack_world,
                             &_edit_context, [&](std::string* edited_value) {
                                *edited_value =
                                   world::create_unique_name(_world.planning_hubs,
                                                             *edited_value);
                             });

            ImGui::DragFloat3("Position", hub, &world::planning_hub::position,
                              &_edit_stack_world, &_edit_context);

            ImGui::DragFloat("Radius", hub, &world::planning_hub::radius,
                             &_edit_stack_world, &_edit_context, 1.0f, 0.0f, 1e10f);
         }
         else if (std::holds_alternative<world::planning_connection_id>(selected)) {
            world::planning_connection* connection =
               world::find_entity(_world.planning_connections,
                                  std::get<world::planning_connection_id>(selected));

            if (not connection) {
               ImGui::PopID();

               continue;
            }

            if (is_multi_select) {
               bool keep_selected = true;

               if (not ImGui::CollapsingHeader("AI Planning Connection", &keep_selected,
                                               ImGuiTreeNodeFlags_DefaultOpen) and
                   keep_selected) {
                  ImGui::PopID();

                  continue;
               }
               else if (not keep_selected) {
                  _interaction_targets.selection.remove(connection->id);

                  selected_index -= 1;
                  id_offset += 1;

                  ImGui::PopID();

                  continue;
               }
            }
            else {
               ImGui::SeparatorText("AI Planning Connection");
            }

            ImGui::InputText("Name", connection, &world::planning_connection::name,
                             &_edit_stack_world, &_edit_context,
                             [&](std::string* edited_value) {
                                *edited_value =
                                   world::create_unique_name(_world.planning_connections,
                                                             *edited_value);
                             });

            ImGui::Text("Start: %s",
                        _world.planning_hubs[connection->start_hub_index].name.c_str());
            ImGui::Text("End: %s",
                        _world.planning_hubs[connection->end_hub_index].name.c_str());

            ImGui::EditFlags("Flags", connection, &world::planning_connection::flags,
                             &_edit_stack_world, &_edit_context,
                             {{"Soldier", world::ai_path_flags::soldier},
                              {"Hover", world::ai_path_flags::hover},
                              {"Small", world::ai_path_flags::small},
                              {"Medium", world::ai_path_flags::medium},
                              {"Huge", world::ai_path_flags::huge},
                              {"Flyer", world::ai_path_flags::flyer}});

            ImGui::Separator();

            ImGui::Checkbox("Jump", connection, &world::planning_connection::jump,
                            &_edit_stack_world, &_edit_context);
            ImGui::SameLine();
            ImGui::Checkbox("Jet Jump", connection, &world::planning_connection::jet_jump,
                            &_edit_stack_world, &_edit_context);
            ImGui::SameLine();
            ImGui::Checkbox("One Way", connection, &world::planning_connection::one_way,
                            &_edit_stack_world, &_edit_context);

            bool is_dynamic = connection->dynamic_group != 0;

            if (ImGui::Checkbox("Dynamic", &is_dynamic)) {
               _edit_stack_world
                  .apply(edits::make_set_value(connection->id,
                                               &world::planning_connection::dynamic_group,
                                               is_dynamic ? int8{1} : int8{0},
                                               connection->dynamic_group),
                         _edit_context);
            }

            if (is_dynamic) {
               ImGui::SliderInt("Dynamic Group", connection,
                                &world::planning_connection::dynamic_group,
                                &_edit_stack_world, &_edit_context, 1, 8, "%d",
                                ImGuiSliderFlags_AlwaysClamp);
            }

            if (ImGui::CollapsingHeader("Forward Branch Weights")) {
               world::planning_branch_weights weights = connection->forward_weights;

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
                  _edit_stack_world
                     .apply(edits::make_set_value(connection->id,
                                                  &world::planning_connection::forward_weights,
                                                  weights, connection->forward_weights),
                            _edit_context);
               }
            }

            if (ImGui::CollapsingHeader("Backward Branch Weights")) {
               world::planning_branch_weights weights = connection->backward_weights;

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
                  _edit_stack_world
                     .apply(edits::make_set_value(connection->id,
                                                  &world::planning_connection::backward_weights,
                                                  weights, connection->backward_weights),
                            _edit_context);
               }
            }
         }
         else if (std::holds_alternative<world::boundary_id>(selected)) {
            world::boundary* boundary =
               world::find_entity(_world.boundaries,
                                  std::get<world::boundary_id>(selected));

            if (not boundary) {
               ImGui::PopID();

               continue;
            }

            if (is_multi_select) {
               bool keep_selected = true;

               if (not ImGui::CollapsingHeader("Boundary", &keep_selected,
                                               ImGuiTreeNodeFlags_DefaultOpen) and
                   keep_selected) {
                  ImGui::PopID();

                  continue;
               }
               else if (not keep_selected) {
                  _interaction_targets.selection.remove(boundary->id);

                  selected_index -= 1;
                  id_offset += 1;

                  ImGui::PopID();

                  continue;
               }
            }
            else {
               ImGui::SeparatorText("Boundary");
            }

            ImGui::InputText("Name", boundary, &world::boundary::name, &_edit_stack_world,
                             &_edit_context, [&](std::string* edited_value) {
                                *edited_value =
                                   world::create_unique_name(_world.boundaries,
                                                             *edited_value);
                             });

            ImGui::DragFloat2XZ("Position", boundary, &world::boundary::position,
                                &_edit_stack_world, &_edit_context, 0.25f);

            ImGui::DragFloat2XZ("Size", boundary, &world::boundary::size,
                                &_edit_stack_world, &_edit_context, 1.0f, 0.0f, 1e10f);
         }
         else if (std::holds_alternative<world::measurement_id>(selected)) {
            world::measurement* measurement =
               world::find_entity(_world.measurements,
                                  std::get<world::measurement_id>(selected));

            if (not measurement) {
               ImGui::PopID();

               continue;
            }

            if (is_multi_select) {
               bool keep_selected = true;

               if (not ImGui::CollapsingHeader("Measurement", &keep_selected,
                                               ImGuiTreeNodeFlags_DefaultOpen) and
                   keep_selected) {
                  ImGui::PopID();

                  continue;
               }
               else if (not keep_selected) {
                  _interaction_targets.selection.remove(measurement->id);

                  selected_index -= 1;
                  id_offset += 1;

                  ImGui::PopID();

                  continue;
               }
            }
            else {
               ImGui::SeparatorText("Measurement");
            }

            ImGui::InputText("Name", measurement, &world::measurement::name,
                             &_edit_stack_world, &_edit_context, [](std::string*) {});
            ImGui::DragFloat3("Start", measurement, &world::measurement::start,
                              &_edit_stack_world, &_edit_context, 0.25f);
            ImGui::DragFloat3("End", measurement, &world::measurement::end,
                              &_edit_stack_world, &_edit_context, 0.25f);
            ImGui::LabelText("Length", "%.2fm",
                             distance(measurement->start, measurement->end));
         }

         ImGui::PopID();
      }

      ImGui::SeparatorText("Selection Tools");

      if (ImGui::Button("Move Selection", {ImGui::CalcItemWidth(), 0.0f})) {
         _selection_edit_tool = selection_edit_tool::move;
         _move_selection_amount = {0.0f, 0.0f, 0.0f};
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

      if (ImGui::Button("Align Selection (Terrain Grid)",
                        {ImGui::CalcItemWidth(), 0.0f})) {
         align_selection();
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

      ImGui::Text("Align Selection (Terrain Grid)");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing",
                                "Align Selection (Terrain Grid)")));

      ImGui::Text("Hide Selection");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Entity Editing", "Hide Selection")));

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

}