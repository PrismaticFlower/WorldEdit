#include "edits/bundle.hpp"
#include "edits/imgui_ext.hpp"
#include "world/utility/world_utilities.hpp"
#include "world_edit.hpp"

#include <imgui.h>

namespace we {

namespace {

auto get_name(const world::animation_transition transition) -> const char*
{
   switch (transition) {
   case world::animation_transition::pop:
      return "Pop";
   case world::animation_transition::linear:
      return "Linear";
   case world::animation_transition::spline:
      return "Spline";
   default:
      return "<unknown>";
   }
}

template<typename T>
auto get_previous_key_time(const std::vector<T>& keys, const int32 index) noexcept
   -> float
{
   const int32 previous_index = index - 1;

   if (previous_index < 0) return 0.0f;

   return keys[previous_index].time;
}

template<typename T>
auto get_next_key_time(const std::vector<T>& keys, const int32 index,
                       const float max_time) noexcept -> float
{

   const int32 next_index = index + 1;

   if (next_index >= std::ssize(keys)) return max_time;

   return keys[next_index].time;
}

}

void world_edit::ui_show_animation_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({640.0f * _display_scale, 540.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({640.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   if (ImGui::Begin("Animation Editor", &_animation_editor_open)) {
      if (ImGui::BeginChild("Animations", {160.0f * _display_scale, 0.0f},
                            ImGuiChildFlags_ResizeX)) {
         ImGui::SeparatorText("Animations");

         for (auto& animation : _world.animations) {
            if (ImGui::Selectable(animation.name.c_str(),
                                  _animation_editor_context.selected.id ==
                                     animation.id)) {
               _animation_editor_context.selected = {.id = animation.id};
            }
         }
      }

      ImGui::EndChild();

      ImGui::SameLine();

      world::animation* selected_animation =
         world::find_entity(_world.animations,
                            _animation_editor_context.selected.id);

      if (ImGui::BeginChild("##selected") and selected_animation) {
         ImGui::SeparatorText(selected_animation->name.c_str());

         if (ImGui::BeginChild("##properties", {0.0f, 80.0f * _display_scale},
                               ImGuiChildFlags_ResizeY)) {

            ImGui::InputText("Name", &selected_animation->name, _edit_stack_world,
                             _edit_context, [](std::string* new_name) noexcept {
                                (void)new_name;

                                // TODO: Ensure name is unique!
                             });

            ImGui::DragFloat("Runtime", &selected_animation->runtime,
                             _edit_stack_world, _edit_context, 0.05f, 0.0f, 1e10f);

            ImGui::Checkbox("Loop", &selected_animation->loop,
                            _edit_stack_world, _edit_context);
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::Checkbox("Local Translation", &selected_animation->local_translation,
                            _edit_stack_world, _edit_context);
         }

         ImGui::EndChild();

         if (ImGui::BeginChild("##keys", {0.0f, 220.0 * _display_scale},
                               ImGuiChildFlags_ResizeY)) {
            if (ImGui::BeginChild("##position_keys",
                                  {(ImGui::GetWindowWidth() -
                                    ImGui::GetStyle().ItemSpacing.x) *
                                      0.5f,
                                   0.0f},
                                  ImGuiChildFlags_ResizeX)) {
               ImGui::SeparatorText("Position Keys");

               if (ImGui::BeginChild("##scroll_region")) {
                  for (int32 i = 0;
                       i < std::ssize(selected_animation->position_keys); ++i) {
                     ImGui::PushID(i);

                     const ImVec2 cursor = ImGui::GetCursorPos();

                     if (ImGui::Selectable("##select",
                                           _animation_editor_context.selected.key == i and
                                              _animation_editor_context.selected.key_type ==
                                                 animation_key_type::position)) {
                        _animation_editor_context.selected.key = i;
                        _animation_editor_context.selected.key_type =
                           animation_key_type::position;
                     }

                     ImGui::SetCursorPos(cursor);

                     ImGui::Text("%.2f", selected_animation->position_keys[i].time);

                     ImGui::PopID();
                  }
               }

               ImGui::EndChild();
            }

            ImGui::EndChild();

            ImGui::SameLine();

            if (ImGui::BeginChild("##rotation_keys")) {
               ImGui::SeparatorText("Rotation Keys");

               if (ImGui::BeginChild("##scroll_region")) {
                  for (int32 i = 0;
                       i < std::ssize(selected_animation->rotation_keys); ++i) {
                     ImGui::PushID(i);

                     const ImVec2 cursor = ImGui::GetCursorPos();

                     if (ImGui::Selectable("##select",
                                           _animation_editor_context.selected.key == i and
                                              _animation_editor_context.selected.key_type ==
                                                 animation_key_type::rotation)) {
                        _animation_editor_context.selected.key = i;
                        _animation_editor_context.selected.key_type =
                           animation_key_type::rotation;
                     }

                     ImGui::SetCursorPos(cursor);

                     ImGui::Text("%.2f", selected_animation->rotation_keys[i].time);

                     ImGui::PopID();
                  }
               }

               ImGui::EndChild();
            }

            ImGui::EndChild();
         }

         ImGui::EndChild();

         if (ImGui::BeginChild("##selected_key")) {
            const int32 selected_key = _animation_editor_context.selected.key;

            if (_animation_editor_context.selected.key_type == animation_key_type::position and
                selected_key < std::ssize(selected_animation->position_keys)) {
               ImGui::SeparatorText("Position Key");

               world::position_key& key =
                  selected_animation->position_keys[selected_key];

               if (float time = key.time;
                   ImGui::SliderFloat("Time", &time,
                                      get_previous_key_time(selected_animation->position_keys,
                                                            selected_key),
                                      get_next_key_time(selected_animation->position_keys,
                                                        selected_key,
                                                        selected_animation->runtime),
                                      "%.2f")) {
                  _edit_stack_world.apply(edits::make_set_vector_value(
                                             &selected_animation->position_keys, selected_key,
                                             &world::position_key::time, time),
                                          _edit_context);
               }

               if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

               if (float3 position = key.position;
                   ImGui::DragFloat3("Position", &position)) {
                  _edit_stack_world.apply(edits::make_set_vector_value(
                                             &selected_animation->position_keys, selected_key,
                                             &world::position_key::position, position),
                                          _edit_context);
               }

               if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

               if (ImGui::BeginCombo("Transition", get_name(key.transition))) {
                  if (ImGui::Selectable("Pop", key.transition ==
                                                  world::animation_transition::pop)) {
                     _edit_stack_world.apply(edits::make_set_vector_value(
                                                &selected_animation->position_keys,
                                                selected_key,
                                                &world::position_key::transition,
                                                world::animation_transition::pop),
                                             _edit_context, {.closed = true});
                  }

                  if (ImGui::Selectable("Linear",
                                        key.transition ==
                                           world::animation_transition::linear)) {
                     _edit_stack_world.apply(edits::make_set_vector_value(
                                                &selected_animation->position_keys,
                                                selected_key,
                                                &world::position_key::transition,
                                                world::animation_transition::linear),
                                             _edit_context, {.closed = true});
                  }

                  if (ImGui::Selectable("Spline",
                                        key.transition ==
                                           world::animation_transition::spline)) {
                     _edit_stack_world.apply(edits::make_set_vector_value(
                                                &selected_animation->position_keys,
                                                selected_key,
                                                &world::position_key::transition,
                                                world::animation_transition::spline),
                                             _edit_context, {.closed = true});
                  }

                  ImGui::EndCombo();
               }

               ImGui::BeginDisabled(key.transition !=
                                    world::animation_transition::spline);

               if (float3 tangent = key.tangent;
                   ImGui::DragFloat3("Tangent", &tangent)) {
                  if (_animation_editor_config.match_tangents and selected_key != 0) {
                     edits::bundle_vector bundle;

                     bundle.push_back(
                        edits::make_set_vector_value(&selected_animation->position_keys,
                                                     selected_key,
                                                     &world::position_key::tangent,
                                                     tangent));

                     bundle.push_back(
                        edits::make_set_vector_value(&selected_animation->position_keys,
                                                     selected_key - 1,
                                                     &world::position_key::tangent_next,
                                                     tangent));

                     _edit_stack_world.apply(edits::make_bundle(std::move(bundle)),
                                             _edit_context);
                  }
                  else {
                     _edit_stack_world.apply(edits::make_set_vector_value(
                                                &selected_animation->position_keys,
                                                selected_key,
                                                &world::position_key::tangent, tangent),
                                             _edit_context);
                  }
               }

               if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

               if (float3 tangent_next = key.tangent_next;
                   ImGui::DragFloat3("Tangent Next", &tangent_next)) {
                  if (_animation_editor_config.match_tangents and
                      (selected_key + 1) != selected_animation->position_keys.size()) {
                     edits::bundle_vector bundle;

                     bundle.push_back(
                        edits::make_set_vector_value(&selected_animation->position_keys,
                                                     selected_key,
                                                     &world::position_key::tangent_next,
                                                     tangent_next));

                     bundle.push_back(
                        edits::make_set_vector_value(&selected_animation->position_keys,
                                                     selected_key + 1,
                                                     &world::position_key::tangent,
                                                     tangent_next));

                     _edit_stack_world.apply(edits::make_bundle(std::move(bundle)),
                                             _edit_context);
                  }
                  else {
                     _edit_stack_world.apply(edits::make_set_vector_value(
                                                &selected_animation->position_keys,
                                                selected_key,
                                                &world::position_key::tangent_next,
                                                tangent_next),
                                             _edit_context);
                  }
               }

               if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

               ImGui::EndDisabled();
            }
            else if (_animation_editor_context.selected.key_type ==
                        animation_key_type::rotation and
                     _animation_editor_context.selected.key <
                        std::ssize(selected_animation->rotation_keys)) {
               ImGui::SeparatorText("Rotation Key");

               world::rotation_key& key =
                  selected_animation
                     ->rotation_keys[_animation_editor_context.selected.key];

               if (float time = key.time;
                   ImGui::SliderFloat("Time", &time,
                                      get_previous_key_time(selected_animation->rotation_keys,
                                                            selected_key),
                                      get_next_key_time(selected_animation->rotation_keys,
                                                        selected_key,
                                                        selected_animation->runtime),
                                      "%.2f")) {
                  _edit_stack_world.apply(edits::make_set_vector_value(
                                             &selected_animation->rotation_keys, selected_key,
                                             &world::rotation_key::time, time),
                                          _edit_context);
               }

               if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

               if (float3 rotation = key.rotation;
                   ImGui::DragFloat3("Rotation", &rotation)) {
                  _edit_stack_world.apply(edits::make_set_vector_value(
                                             &selected_animation->rotation_keys, selected_key,
                                             &world::rotation_key::rotation, rotation),
                                          _edit_context);
               }

               if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

               if (ImGui::BeginCombo("Transition", get_name(key.transition))) {
                  if (ImGui::Selectable("Pop", key.transition ==
                                                  world::animation_transition::pop)) {
                     _edit_stack_world.apply(edits::make_set_vector_value(
                                                &selected_animation->rotation_keys,
                                                selected_key,
                                                &world::rotation_key::transition,
                                                world::animation_transition::pop),
                                             _edit_context, {.closed = true});
                  }

                  if (ImGui::Selectable("Linear",
                                        key.transition ==
                                           world::animation_transition::linear)) {
                     _edit_stack_world.apply(edits::make_set_vector_value(
                                                &selected_animation->rotation_keys,
                                                selected_key,
                                                &world::rotation_key::transition,
                                                world::animation_transition::linear),
                                             _edit_context, {.closed = true});
                  }

                  if (ImGui::Selectable("Spline",
                                        key.transition ==
                                           world::animation_transition::spline)) {
                     _edit_stack_world.apply(edits::make_set_vector_value(
                                                &selected_animation->rotation_keys,
                                                selected_key,
                                                &world::rotation_key::transition,
                                                world::animation_transition::spline),
                                             _edit_context, {.closed = true});
                  }

                  ImGui::EndCombo();
               }

               ImGui::BeginDisabled(key.transition !=
                                    world::animation_transition::spline);

               if (float3 tangent = key.tangent;
                   ImGui::DragFloat3("Tangent", &tangent)) {
                  if (_animation_editor_config.match_tangents and selected_key != 0) {
                     edits::bundle_vector bundle;

                     bundle.push_back(
                        edits::make_set_vector_value(&selected_animation->rotation_keys,
                                                     selected_key,
                                                     &world::rotation_key::tangent,
                                                     tangent));

                     bundle.push_back(
                        edits::make_set_vector_value(&selected_animation->rotation_keys,
                                                     selected_key - 1,
                                                     &world::rotation_key::tangent_next,
                                                     tangent));

                     _edit_stack_world.apply(edits::make_bundle(std::move(bundle)),
                                             _edit_context);
                  }
                  else {
                     _edit_stack_world.apply(edits::make_set_vector_value(
                                                &selected_animation->rotation_keys,
                                                selected_key,
                                                &world::rotation_key::tangent, tangent),
                                             _edit_context);
                  }
               }

               if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

               if (float3 tangent_next = key.tangent_next;
                   ImGui::DragFloat3("Tangent Next", &tangent_next)) {
                  if (_animation_editor_config.match_tangents and
                      (selected_key + 1) != selected_animation->rotation_keys.size()) {
                     edits::bundle_vector bundle;

                     bundle.push_back(
                        edits::make_set_vector_value(&selected_animation->rotation_keys,
                                                     selected_key,
                                                     &world::rotation_key::tangent_next,
                                                     tangent_next));

                     bundle.push_back(
                        edits::make_set_vector_value(&selected_animation->rotation_keys,
                                                     selected_key + 1,
                                                     &world::rotation_key::tangent,
                                                     tangent_next));

                     _edit_stack_world.apply(edits::make_bundle(std::move(bundle)),
                                             _edit_context);
                  }
                  else {
                     _edit_stack_world.apply(edits::make_set_vector_value(
                                                &selected_animation->rotation_keys,
                                                selected_key,
                                                &world::rotation_key::tangent_next,
                                                tangent_next),
                                             _edit_context);
                  }
               }

               if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

               ImGui::EndDisabled();
            }
            else {
               ImGui::Text("Select a key to edit.");
            }
         }

         ImGui::EndChild();
      }

      ImGui::EndChild();
   }

   ImGui::End();
}

}