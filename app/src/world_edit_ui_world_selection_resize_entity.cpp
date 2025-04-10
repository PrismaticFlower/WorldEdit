#include "world_edit.hpp"

#include "edits/set_block.hpp"
#include "edits/set_value.hpp"

#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include "world/blocks/find.hpp"
#include "world/utility/world_utilities.hpp"

#include <imgui.h>

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we {

void world_edit::ui_show_world_selection_resize_entity() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});
   ImGui::SetNextWindowSizeConstraints({520.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   bool open = _selection_edit_tool == selection_edit_tool::resize_entity;
   bool resizable_entity = false;

   if (ImGui::Begin("Resize Entity", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::TextWrapped(
         "Resize any selected entities. Only lights, regions, "
         "portals, barriers, AI planning hubs, boundaries and blocks"
         "support resizing.");

      for (const world::selected_entity& selected : _interaction_targets.selection) {
         if (selected.is<world::light_id>()) {
            world::light* light =
               world::find_entity(_world.lights, selected.get<world::light_id>());

            if (light and light->light_type != world::light_type::directional) {
               switch (light->light_type) {
               case world::light_type::directional:
                  break;
               case world::light_type::point: {
                  float3 new_position = light->position;
                  float3 new_size = {light->range, light->range, light->range};

                  if (_gizmos.gizmo_size(
                         {
                            .name = "Light Size",
                            .instance = static_cast<int64>(light->id),
                            .alignment = _editor_grid_size,
                            .gizmo_rotation = quaternion{},
                         },
                         new_position, new_size)) {
                     float new_range = new_size.x;

                     if (new_size.y != light->range) new_range = new_size.y;
                     if (new_size.z != light->range) new_range = new_size.z;

                     _edit_stack_world.apply(edits::make_set_multi_value(&light->position,
                                                                         new_position,
                                                                         &light->range,
                                                                         new_range),
                                             _edit_context);
                  }
               } break;
               case world::light_type::spot: {
                  const float start_outer_radius =
                     light->range * std::tan(light->outer_cone_angle * 0.5f);

                  float new_range = light->range;
                  float new_outer_radius = start_outer_radius;

                  if (_gizmos.gizmo_cone_size(
                         {
                            .name = "Light Size",
                            .instance = static_cast<int64>(light->id),
                            .alignment = _editor_grid_size,
                            .gizmo_rotation = light->rotation,
                            .gizmo_positionWS = light->position,
                         },
                         new_range, new_outer_radius)) {
                     float new_inner_angle = light->inner_cone_angle;
                     float new_outer_angle = light->outer_cone_angle;

                     if (new_outer_radius != start_outer_radius) {
                        new_outer_angle =
                           std::atan(new_outer_radius / light->range) * 2.0f;

                        const float inner_radius =
                           light->range * std::tan(light->inner_cone_angle * 0.5f);

                        new_outer_angle =
                           std::atan(new_outer_radius / light->range) * 2.0f;

                        const float inner_scale =
                           start_outer_radius > 0.0f
                              ? (new_outer_radius / start_outer_radius)
                              : 1.0f;

                        new_inner_angle =
                           std::atan((inner_radius * inner_scale) / light->range) * 2.0f;
                     }

                     _edit_stack_world
                        .apply(edits::make_set_multi_value(&light->range, new_range,
                                                           &light->inner_cone_angle,
                                                           new_inner_angle,
                                                           &light->outer_cone_angle,
                                                           new_outer_angle),
                               _edit_context);
                  }
               } break;
               case world::light_type::directional_region_box: {
                  float3 new_position = light->position;
                  float3 new_size = light->region_size;

                  if (_gizmos.gizmo_size(
                         {
                            .name = "Light Size",
                            .instance = static_cast<int64>(light->id),
                            .alignment = _editor_grid_size,
                            .gizmo_rotation = light->region_rotation,
                         },
                         new_position, new_size)) {
                     _edit_stack_world.apply(edits::make_set_multi_value(&light->position,
                                                                         new_position,
                                                                         &light->region_size,
                                                                         new_size),
                                             _edit_context);
                  }
               } break;
               case world::light_type::directional_region_cylinder: {
                  const float start_radius =
                     length(float2{light->region_size.x, light->region_size.z});

                  float3 new_position = light->position;
                  float3 new_size = {start_radius, light->region_size.y, start_radius};

                  if (_gizmos.gizmo_size(
                         {
                            .name = "Light Size",
                            .instance = static_cast<int64>(light->id),
                            .alignment = _editor_grid_size,
                            .gizmo_rotation = light->region_rotation,
                         },
                         new_position, new_size)) {
                     const float new_radius =
                        new_size.x != start_radius ? new_size.x : new_size.z;
                     const float xz_size = std::sqrt((new_radius * new_radius) / 2.0f);

                     _edit_stack_world.apply(edits::make_set_multi_value(
                                                &light->position, new_position,
                                                &light->region_size,
                                                {xz_size, new_size.y, xz_size}),
                                             _edit_context);
                  }
               } break;
               case world::light_type::directional_region_sphere: {
                  const float start_radius = length(light->region_size);

                  float3 new_position = light->position;
                  float3 new_size = {start_radius, start_radius, start_radius};

                  if (_gizmos.gizmo_size(
                         {
                            .name = "Light Size",
                            .instance = static_cast<int64>(light->id),
                            .alignment = _editor_grid_size,
                            .gizmo_rotation = light->region_rotation,
                         },
                         new_position, new_size)) {
                     float new_radius = new_size.x;

                     if (new_size.y != start_radius) new_radius = new_size.y;
                     if (new_size.z != start_radius) new_radius = new_size.z;

                     const float size = std::sqrt((new_radius * new_radius) / 3.0f);

                     _edit_stack_world
                        .apply(edits::make_set_multi_value(&light->position, new_position,
                                                           &light->region_size,
                                                           {size, size, size}),
                               _edit_context);
                  }
               } break;
               }

               if (_gizmos.can_close_last_edit()) {
                  _edit_stack_world.close_last();
               }

               resizable_entity = true;
            }
         }
         else if (selected.is<world::region_id>()) {
            world::region* region =
               world::find_entity(_world.regions, selected.get<world::region_id>());

            if (region) {
               switch (region->shape) {
               case world::region_shape::box: {
                  float3 new_position = region->position;
                  float3 new_size = region->size;

                  if (_gizmos.gizmo_size(
                         {
                            .name = "Region Size",
                            .instance = static_cast<int64>(region->id),
                            .alignment = _editor_grid_size,
                            .gizmo_rotation = region->rotation,
                         },
                         new_position, new_size)) {
                     _edit_stack_world.apply(edits::make_set_multi_value(&region->position,
                                                                         new_position,
                                                                         &region->size,
                                                                         new_size),
                                             _edit_context);
                  }
               } break;
               case world::region_shape::cylinder: {
                  const float start_radius =
                     length(float2{region->size.x, region->size.z});

                  float3 new_position = region->position;
                  float3 new_size = {start_radius, region->size.y, start_radius};

                  if (_gizmos.gizmo_size(
                         {
                            .name = "Region Size",
                            .instance = static_cast<int64>(region->id),
                            .alignment = _editor_grid_size,
                            .gizmo_rotation = region->rotation,
                         },
                         new_position, new_size)) {
                     const float new_radius =
                        new_size.x != start_radius ? new_size.x : new_size.z;
                     const float xz_size = std::sqrt((new_radius * new_radius) / 2.0f);

                     _edit_stack_world.apply(
                        edits::make_set_multi_value(&region->position,
                                                    new_position, &region->size,
                                                    {xz_size, new_size.y, xz_size}),
                        _edit_context);
                  }
               } break;
               case world::region_shape::sphere: {
                  const float start_radius = length(region->size);

                  float3 new_position = region->position;
                  float3 new_size = {start_radius, start_radius, start_radius};

                  if (_gizmos.gizmo_size(
                         {
                            .name = "Region Size",
                            .instance = static_cast<int64>(region->id),
                            .alignment = _editor_grid_size,
                            .gizmo_rotation = region->rotation,
                         },
                         new_position, new_size)) {
                     float new_radius = new_size.x;

                     if (new_size.y != start_radius) new_radius = new_size.y;
                     if (new_size.z != start_radius) new_radius = new_size.z;

                     const float size = std::sqrt((new_radius * new_radius) / 3.0f);

                     _edit_stack_world
                        .apply(edits::make_set_multi_value(&region->position,
                                                           new_position, &region->size,
                                                           {size, size, size}),
                               _edit_context);
                  }
               } break;
               }

               if (_gizmos.can_close_last_edit()) {
                  _edit_stack_world.close_last();
               }

               resizable_entity = true;
            }
         }
         else if (selected.is<world::portal_id>()) {
            world::portal* portal =
               world::find_entity(_world.portals, selected.get<world::portal_id>());

            if (portal) {
               float3 new_position = portal->position;
               float3 new_size = {portal->width / 2.0f, portal->height / 2.0f, 0.0f};

               if (_gizmos.gizmo_size(
                      {
                         .name = "Portal Size",
                         .instance = static_cast<int64>(portal->id),
                         .alignment = _editor_grid_size,
                         .gizmo_rotation = portal->rotation,
                         .show_z_axis = false,
                      },
                      new_position, new_size)) {
                  _edit_stack_world.apply(edits::make_set_multi_value(
                                             &portal->position, new_position,
                                             &portal->width, new_size.x * 2.0f,
                                             &portal->height, new_size.y * 2.0f),
                                          _edit_context);
               }

               if (_gizmos.can_close_last_edit()) {
                  _edit_stack_world.close_last();
               }

               resizable_entity = true;
            }
         }
         else if (selected.is<world::barrier_id>()) {
            world::barrier* barrier =
               world::find_entity(_world.barriers, selected.get<world::barrier_id>());

            if (barrier) {
               float3 new_position = barrier->position;
               float3 new_size = {barrier->size.x, 0.0f, barrier->size.y};

               if (_gizmos.gizmo_size(
                      {
                         .name = "Barrier Size",
                         .instance = static_cast<int64>(barrier->id),
                         .alignment = _editor_grid_size,
                         .gizmo_rotation = make_quat_from_euler(
                            {0.0f, barrier->rotation_angle, 0.0f}),
                         .show_y_axis = false,
                      },
                      new_position, new_size)) {
                  _edit_stack_world
                     .apply(edits::make_set_multi_value(&barrier->position,
                                                        new_position, &barrier->size,
                                                        {new_size.x, new_size.z}),
                            _edit_context);
               }

               if (_gizmos.can_close_last_edit()) {
                  _edit_stack_world.close_last();
               }

               resizable_entity = true;
            }
         }
         else if (selected.is<world::planning_hub_id>()) {
            world::planning_hub* hub =
               world::find_entity(_world.planning_hubs,
                                  selected.get<world::planning_hub_id>());

            if (hub) {
               float3 new_position = hub->position;
               float3 new_size = {hub->radius, 0.0f, hub->radius};

               if (_gizmos.gizmo_size(
                      {
                         .name = "Hub Size",
                         .instance = static_cast<int64>(hub->id),
                         .alignment = _editor_grid_size,
                         .gizmo_rotation = quaternion{},
                         .show_y_axis = false,
                      },
                      new_position, new_size)) {
                  float new_radius = new_size.x;

                  if (new_size.z != hub->radius) new_radius = new_size.z;

                  _edit_stack_world.apply(edits::make_set_multi_value(&hub->position,
                                                                      new_position,
                                                                      &hub->radius,
                                                                      new_radius),
                                          _edit_context);
               }

               if (_gizmos.can_close_last_edit()) {
                  _edit_stack_world.close_last();
               }

               resizable_entity = true;
            }
         }
         else if (selected.is<world::boundary_id>()) {
            world::boundary* boundary =
               world::find_entity(_world.boundaries,
                                  selected.get<world::boundary_id>());

            if (boundary) {
               float3 new_position = boundary->position;
               float3 new_size = {boundary->size.x, 0.0f, boundary->size.y};

               if (_gizmos.gizmo_size(
                      {
                         .name = "Boundary Size",
                         .instance = static_cast<int64>(boundary->id),
                         .alignment = _editor_grid_size,
                         .gizmo_rotation = quaternion{},
                         .show_y_axis = false,
                      },
                      new_position, new_size)) {
                  _edit_stack_world
                     .apply(edits::make_set_multi_value(&boundary->position,
                                                        new_position, &boundary->size,
                                                        {new_size.x, new_size.z}),
                            _edit_context);
               }

               if (_gizmos.can_close_last_edit()) {
                  _edit_stack_world.close_last();
               }

               resizable_entity = true;
            }
         }
         else if (selected.is<world::block_id>()) {
            const world::block_id block_id = selected.get<world::block_id>();
            const std::optional<uint32> block_index =
               world::find_block(_world.blocks, block_id);

            if (block_index) {
               switch (block_id.type()) {
               case world::block_type::box: {
                  const world::block_description_box& box =
                     _world.blocks.boxes.description[*block_index];

                  float3 new_position = box.position;
                  float3 new_size = box.size;

                  if (_gizmos.gizmo_size(
                         {
                            .name = "Block Box Size",
                            .instance = static_cast<int64>(
                               _world.blocks.boxes.ids[*block_index]),
                            .alignment = _editor_grid_size,
                            .gizmo_rotation = box.rotation,
                         },
                         new_position, new_size)) {
                     _edit_stack_world.apply(edits::make_set_block_box_metrics(
                                                *block_index, box.rotation,
                                                new_position, new_size),
                                             _edit_context);
                  }

               } break;
               }
            }

            if (_gizmos.can_close_last_edit()) {
               _edit_stack_world.close_last();
            }
            resizable_entity = true;
         }
      }
   }

   if (not open or not resizable_entity) {
      _edit_stack_world.close_last();
      _selection_edit_tool = selection_edit_tool::none;
   }

   ImGui::End();
}
}