#include "edits/bundle.hpp"
#include "edits/imgui_ext.hpp"
#include "math/matrix_funcs.hpp"
#include "math/vector_funcs.hpp"
#include "utility/srgb_conversion.hpp"
#include "utility/string_icompare.hpp"
#include "world/utility/animation.hpp"
#include "world/utility/world_utilities.hpp"
#include "world_edit.hpp"

#include <numbers>

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
   ImGui::SetNextWindowSize({640.0f * _display_scale, 550.0f * _display_scale},
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

         if (ImGui::BeginChild("##properties", {0.0f, 100.0f * _display_scale},
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

            if (selected_animation->local_translation) {
               ImGui::Separator();
               ImGui::TextWrapped(
                  "Local Translation animations do not have accurate "
                  "previews in WorldEdit.");
            }
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
                        _animation_editor_context.selected.key_movement = {0.0f, 0.0f,
                                                                           0.0f};
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
                        _animation_editor_context.selected
                           .key_rotation_movement = {0.0f, 0.0f, 0.0f};
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
                  else if (_animation_editor_config.match_tangents and
                           selected_animation->loop and
                           selected_animation->position_keys.size() > 1 and
                           selected_key == 0) {
                     edits::bundle_vector bundle;

                     bundle.push_back(
                        edits::make_set_vector_value(&selected_animation->position_keys,
                                                     0, &world::position_key::tangent,
                                                     tangent));

                     bundle.push_back(edits::make_set_vector_value(
                        &selected_animation->position_keys,
                        static_cast<int>(selected_animation->position_keys.size()) - 1,
                        &world::position_key::tangent_next, tangent));

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
                  else if (_animation_editor_config.match_tangents and
                           selected_animation->loop and
                           selected_animation->position_keys.size() > 1 and
                           (selected_key + 1) ==
                              selected_animation->position_keys.size()) {
                     edits::bundle_vector bundle;

                     bundle.push_back(
                        edits::make_set_vector_value(&selected_animation->position_keys,
                                                     selected_key,
                                                     &world::position_key::tangent_next,
                                                     tangent_next));

                     bundle.push_back(
                        edits::make_set_vector_value(&selected_animation->position_keys,
                                                     0, &world::position_key::tangent,
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
                  else if (_animation_editor_config.match_tangents and
                           selected_animation->loop and
                           selected_animation->rotation_keys.size() > 1 and
                           selected_key == 0) {
                     edits::bundle_vector bundle;

                     bundle.push_back(
                        edits::make_set_vector_value(&selected_animation->rotation_keys,
                                                     0, &world::rotation_key::tangent,
                                                     tangent));

                     bundle.push_back(edits::make_set_vector_value(
                        &selected_animation->rotation_keys,
                        static_cast<int>(selected_animation->rotation_keys.size()) - 1,
                        &world::rotation_key::tangent_next, tangent));

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
                  else if (_animation_editor_config.match_tangents and
                           selected_animation->loop and
                           selected_animation->rotation_keys.size() > 1 and
                           (selected_key + 1) ==
                              selected_animation->rotation_keys.size()) {
                     edits::bundle_vector bundle;

                     bundle.push_back(
                        edits::make_set_vector_value(&selected_animation->rotation_keys,
                                                     selected_key,
                                                     &world::rotation_key::tangent_next,
                                                     tangent_next));

                     bundle.push_back(
                        edits::make_set_vector_value(&selected_animation->rotation_keys,
                                                     0, &world::rotation_key::tangent,
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

   if (not _animation_editor_open) return;

   world::animation* selected_animation =
      world::find_entity(_world.animations, _animation_editor_context.selected.id);

   if (selected_animation) {
      quaternion base_rotation = {1.0f, 0.0f, 0.0f, 0.0f};
      float3 base_position = {0.0f, 0.0f, 0.0f};

      for (const world::animation_group& group : _world.animation_groups) {
         for (const world::animation_group::entry& entry : group.entries) {
            if (string::iequals(entry.animation, selected_animation->name)) {
               for (const world::object& object : _world.objects) {
                  if (string::iequals(entry.object, object.name)) {
                     base_rotation = object.rotation;
                     base_position = object.position;

                     break;
                  }
               }
            }
         }
      }

      const int32 selected_key = _animation_editor_context.selected.key;

      if (_animation_editor_context.selected.key_type == animation_key_type::position and
          selected_key < std::ssize(selected_animation->position_keys)) {
         const world::position_key& key =
            selected_animation->position_keys[selected_key];

         const float3 last_move_amount = _animation_editor_context.selected.key_movement;

         if (_gizmo.show_translate(key.position + base_position,
                                   quaternion{1.0f, 0.0f, 0.0f, 0.0f},
                                   _animation_editor_context.selected.key_movement)) {
            const float3 move_delta =
               (_animation_editor_context.selected.key_movement - last_move_amount);

            _edit_stack_world
               .apply(edits::make_set_vector_value(&selected_animation->position_keys,
                                                   selected_key,
                                                   &world::position_key::position,
                                                   key.position + move_delta),
                      _edit_context);
         }

         if (_gizmo.can_close_last_edit()) _edit_stack_world.close_last();
      }
      else if (_animation_editor_context.selected.key_type == animation_key_type::rotation and
               selected_key < std::ssize(selected_animation->rotation_keys)) {
         const world::rotation_key& key =
            selected_animation->rotation_keys[selected_key];

         const float4x4 key_transform =
            world::evaluate_animation(*selected_animation, base_rotation,
                                      base_position, key.time);

         const float3 key_position = {key_transform[3].x, key_transform[3].y,
                                      key_transform[3].z};

         const float3 last_rotation_amount =
            _animation_editor_context.selected.key_rotation_movement;

         if (_gizmo.show_rotate(key_position,
                                _animation_editor_context.selected.key_rotation_movement)) {
            constexpr float radians_to_degrees = 180.0f / std ::numbers::pi_v<float>;

            const float3 rotate_delta =
               (_animation_editor_context.selected.key_rotation_movement -
                last_rotation_amount) *
               radians_to_degrees;

            _edit_stack_world
               .apply(edits::make_set_vector_value(&selected_animation->rotation_keys,
                                                   selected_key,
                                                   &world::rotation_key::rotation,
                                                   key.rotation + rotate_delta),
                      _edit_context);
         }

         if (_gizmo.can_close_last_edit()) _edit_stack_world.close_last();
      }

      for (auto& key : selected_animation->position_keys) {
         float4x4 transform{};
         transform[3] = {key.position + base_position, 1.0f};

         _tool_visualizers.add_octahedron(transform,
                                          _settings.graphics.animation_position_key_color);
      }

      for (auto& key : selected_animation->rotation_keys) {
         float4x4 transform =
            world::evaluate_animation(*selected_animation, base_rotation,
                                      base_position, key.time);

         _tool_visualizers.add_arrow_wireframe(transform,
                                               float4{_settings.graphics.animation_rotation_key_color,
                                                      1.0f});
      }

      const uint32 spline_color =
         utility::pack_srgb_bgra({_settings.graphics.animation_spline_color, 1.0f});

      for (std::size_t i = 1; i < selected_animation->position_keys.size(); ++i) {
         if (selected_animation->position_keys[i - 1].transition ==
             world::animation_transition::linear) {
            _tool_visualizers.add_line(
               base_position + selected_animation->position_keys[i - 1].position,
               base_position + selected_animation->position_keys[i].position,
               spline_color);
         }
         else if (selected_animation->position_keys[i - 1].transition ==
                  world::animation_transition::spline) {
            const float time_start = selected_animation->position_keys[i - 1].time;
            const float time_distance = selected_animation->position_keys[i].time -
                                        selected_animation->position_keys[i - 1].time;

            for (float t = 0.0f; t < 64.0f; ++t) {
               float4x4 transform0 =
                  world::evaluate_animation(*selected_animation, base_rotation,
                                            base_position,
                                            time_start + ((t / 64.0f) * time_distance));
               float4x4 transform1 =
                  world::evaluate_animation(*selected_animation, base_rotation,
                                            base_position,
                                            time_start +
                                               (((t + 1) / 64.0f) * time_distance));

               _tool_visualizers.add_line(float3{transform0[3].x, transform0[3].y,
                                                 transform0[3].z},
                                          float3{transform1[3].x, transform1[3].y,
                                                 transform1[3].z},
                                          spline_color);
            }
         }
      }

      if (selected_animation->loop and not selected_animation->position_keys.empty()) {
         const world::position_key& last_key =
            selected_animation->position_keys.back();

         if (last_key.transition == world::animation_transition::linear) {
            _tool_visualizers.add_line(base_position + last_key.position,
                                       base_position +
                                          selected_animation->position_keys[0].position,
                                       spline_color);
         }
         else if (selected_animation->position_keys.back().transition ==
                  world::animation_transition::spline) {
            const float time_start = last_key.time;
            const float time_distance = selected_animation->runtime - last_key.time;

            for (float t = 0.0f; t < 64.0f; ++t) {
               float4x4 transform0 =
                  world::evaluate_animation(*selected_animation, base_rotation,
                                            base_position,
                                            time_start + ((t / 64.0f) * time_distance));
               float4x4 transform1 =
                  world::evaluate_animation(*selected_animation, base_rotation,
                                            base_position,
                                            time_start +
                                               (((t + 1) / 64.0f) * time_distance));

               _tool_visualizers.add_line(float3{transform0[3].x, transform0[3].y,
                                                 transform0[3].z},
                                          float3{transform1[3].x, transform1[3].y,
                                                 transform1[3].z},
                                          spline_color);
            }
         }
      }
   }
}

}