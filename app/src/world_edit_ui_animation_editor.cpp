#include "edits/bundle.hpp"
#include "edits/imgui_ext.hpp"
#include "edits/insert_animation_key.hpp"
#include "math/matrix_funcs.hpp"
#include "math/vector_funcs.hpp"
#include "utility/srgb_conversion.hpp"
#include "utility/string_icompare.hpp"
#include "world/utility/animation.hpp"
#include "world/utility/raycast_animation.hpp"
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
      return "Hermite Spline";
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

template<typename T>
bool is_unique_key_time(const std::vector<T>& keys, const float time) noexcept
{
   for (const T& key : keys) {
      if (key.time == time) return false;
   }

   return true;
}

void convert_to_smooth_spline(world::animation& animation, float smoothness,
                              edits::stack<world::edit_context>& edit_stack,
                              world::edit_context& edit_context) noexcept
{
   edits::bundle_vector edit_bundle;

   edit_bundle.reserve(animation.position_keys.size() * 3);

   const int32 max_key = static_cast<int32>(std::ssize(animation.position_keys) - 1);

   for (int32 i = 0; i < std::ssize(animation.position_keys); ++i) {
      const world::position_key& key_back =
         animation.position_keys[std::clamp(i - 1, 0, max_key)];
      const world::position_key& key =
         animation.position_keys[std::clamp(i, 0, max_key)];
      const world::position_key& key_forward =
         animation.position_keys[std::clamp(i + 1, 0, max_key)];
      const world::position_key& key_forward_forward =
         animation.position_keys[std::clamp(i + 2, 0, max_key)];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, i,
                                      &world::position_key::transition,
                                      world::animation_transition::spline));
      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, i,
                                      &world::position_key::tangent,
                                      smoothness * (key_forward.position -
                                                    key_back.position)));
      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, i,
                                      &world::position_key::tangent_next,
                                      smoothness * (key_forward_forward.position -
                                                    key.position)));
   }

   edit_stack.apply(edits::make_bundle(std::move(edit_bundle)), edit_context);
}

void update_position_auto_tangents(world::animation& animation, int32 key_index,
                                   float3 key_new_position, float smoothness,
                                   edits::stack<world::edit_context>& edit_stack,
                                   world::edit_context& edit_context) noexcept
{
   edits::bundle_vector edit_bundle;

   edit_bundle.reserve(5);

   const int32 max_key = static_cast<int32>(std::ssize(animation.position_keys) - 1);

   {
      const world::position_key& key_forward_forward =
         animation.position_keys[std::clamp(key_index + 2, 0, max_key)];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, key_index,
                                      &world::position_key::position, key_new_position));
      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, key_index,
                                      &world::position_key::tangent_next,
                                      smoothness * (key_forward_forward.position -
                                                    key_new_position)));
   }

   if (key_index - 2 >= 0) {
      const world::position_key& key = animation.position_keys[key_index - 2];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, key_index - 2,
                                      &world::position_key::tangent_next,
                                      smoothness * (key_new_position - key.position)));
   }

   if (key_index - 1 >= 0) {
      const world::position_key& key_back =
         animation.position_keys[std::clamp(key_index - 2, 0, max_key)];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, key_index - 1,
                                      &world::position_key::tangent,
                                      smoothness *
                                         (key_new_position - key_back.position)));
   }

   if (key_index + 1 < std::ssize(animation.position_keys)) {
      const world::position_key& key_forward =
         animation.position_keys[std::clamp(key_index + 2, 0, max_key)];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, key_index + 1,
                                      &world::position_key::tangent,
                                      smoothness * (key_forward.position -
                                                    key_new_position)));
   }

   edit_stack.apply(edits::make_bundle(std::move(edit_bundle)), edit_context);
}

void update_auto_tangents(world::animation& animation, int32 key_index, float smoothness,
                          edits::stack<world::edit_context>& edit_stack,
                          world::edit_context& edit_context, bool transparent) noexcept
{
   edits::bundle_vector edit_bundle;

   edit_bundle.reserve(6);

   const int32 max_key = static_cast<int32>(std::ssize(animation.position_keys) - 1);

   const float3 key_new_position = animation.position_keys[key_index].position;

   {
      const world::position_key& key_back =
         animation.position_keys[std::clamp(key_index - 1, 0, max_key)];
      const world::position_key& key_forward =
         animation.position_keys[std::clamp(key_index + 1, 0, max_key)];
      const world::position_key& key_forward_forward =
         animation.position_keys[std::clamp(key_index + 2, 0, max_key)];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, key_index,
                                      &world::position_key::tangent,
                                      smoothness * (key_forward.position -
                                                    key_back.position)));
      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, key_index,
                                      &world::position_key::tangent_next,
                                      smoothness * (key_forward_forward.position -
                                                    key_new_position)));
   }

   if (key_index - 2 >= 0) {
      const world::position_key& key = animation.position_keys[key_index - 2];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, key_index - 2,
                                      &world::position_key::tangent_next,
                                      smoothness * (key_new_position - key.position)));
   }

   if (key_index - 1 >= 0) {
      const world::position_key& key_back =
         animation.position_keys[std::clamp(key_index - 2, 0, max_key)];
      const world::position_key& key =
         animation.position_keys[std::clamp(key_index - 1, 0, max_key)];
      const world::position_key& key_forward_forward =
         animation.position_keys[std::clamp(key_index + 1, 0, max_key)];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, key_index - 1,
                                      &world::position_key::tangent,
                                      smoothness *
                                         (key_new_position - key_back.position)));
      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, key_index - 1,
                                      &world::position_key::tangent_next,
                                      smoothness * (key_forward_forward.position -
                                                    key.position)));
   }

   if (key_index + 1 < std::ssize(animation.position_keys)) {
      const world::position_key& key_forward =
         animation.position_keys[std::clamp(key_index + 2, 0, max_key)];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, key_index + 1,
                                      &world::position_key::tangent,
                                      smoothness * (key_forward.position -
                                                    key_new_position)));
   }

   edit_stack.apply(edits::make_bundle(std::move(edit_bundle)), edit_context,
                    {.transparent = transparent});
}

}

void world_edit::ui_show_animation_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({640.0f * _display_scale, 610.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({640.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   std::optional<int32> hovered_position_key;
   std::optional<int32> hovered_rotation_key;

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

         if (ImGui::BeginChild("##properties", {0.0f, 160.0f * _display_scale},
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

            ImGui::SeparatorText("Edit Options");

            ImGui::Checkbox("Match Tangents", &_animation_editor_config.match_tangents);

            ImGui::SetItemTooltip(
               "When editing the tangent values of a key match the Tangent "
               "and Tangent Next values of neighbouring keys as needed.");

            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

            ImGui::Checkbox("Auto Tangents", &_animation_editor_config.auto_tangents);

            ImGui::SetItemTooltip(
               "When editing position keys set the values of key tangents "
               "as needed to automatically to create a Catmull-Rom or Cardinal "
               "spline.");

            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

            if (ImGui::Button("Convert to Smooth Spline")) {
               _animation_editor_config.auto_tangents = true;

               convert_to_smooth_spline(*selected_animation,
                                        _animation_editor_config.auto_tangent_smoothness,
                                        _edit_stack_world, _edit_context);
            }

            ImGui::SetItemTooltip(
               "Change the transition type of all position keys to Hermite "
               "Spline and auto-fill "
               "the tangents to create a Catmull-Rom or Cardinal spline.");

            if (float tension = 1.0f - _animation_editor_config.auto_tangent_smoothness;
                ImGui::SliderFloat("Auto Tangent Tension", &tension, 0.0f, 1.0f,
                                   "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
               _animation_editor_config.auto_tangent_smoothness = 1.0f - tension;
            }

            ImGui::SetItemTooltip("The tension value used for auto tangents. A "
                                  "value of 0.5 gives a Catmull-Rom Spline.");
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

               if (ImGui::BeginChild("##scroll_region",
                                     {0.0f, ImGui::GetContentRegionAvail().y -
                                               84.0f * _display_scale})) {
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

                     if (ImGui::IsItemHovered()) hovered_position_key = i;

                     ImGui::SetCursorPos(cursor);

                     ImGui::Text("%.2f", selected_animation->position_keys[i].time);

                     ImGui::PopID();
                  }
               }

               ImGui::EndChild();

               ImGui::SeparatorText("New Key");

               ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

               ImGui::SliderFloat("##new_key_time",
                                  &_animation_editor_context.new_position_key_time,
                                  0.0f, selected_animation->runtime,
                                  "Time: %.2f", ImGuiSliderFlags_AlwaysClamp);

               ImGui::BeginDisabled(
                  not is_unique_key_time(selected_animation->position_keys,
                                         _animation_editor_context.new_position_key_time));

               const float button_width = (ImGui::GetContentRegionAvail().x -
                                           ImGui::GetStyle().ItemInnerSpacing.x) *
                                          0.5f;

               if (ImGui::Button("Add", {button_width, 0.0f})) {
                  std::optional<int32> previous_key_index;

                  for (int32 i = 0;
                       i < std::ssize(selected_animation->position_keys); ++i) {
                     if (_animation_editor_context.new_position_key_time >=
                         selected_animation->position_keys[i].time) {
                        previous_key_index = i;
                     }
                     else {
                        break;
                     }
                  }

                  const int32 insert_before_index =
                     previous_key_index ? *previous_key_index + 1 : 0;
                  const world::position_key new_key =
                     world::make_position_key_for_time(*selected_animation,
                                                       _animation_editor_context.new_position_key_time);

                  _edit_stack_world.apply(edits::make_insert_animation_key(
                                             &selected_animation->position_keys,
                                             insert_before_index, new_key),
                                          _edit_context);

                  if (_animation_editor_config.auto_tangents) {
                     update_auto_tangents(*selected_animation,
                                          insert_before_index, 0.5f,
                                          _edit_stack_world, _edit_context, true);
                  }

                  _animation_editor_context.selected.key_type =
                     animation_key_type::position;
                  _animation_editor_context.selected.key = insert_before_index;
               }

               ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

               ImGui::Button("Place", {button_width, 0.0f});

               ImGui::SetItemTooltip(
                  "Add a new key and place it with the cursor.");

               ImGui::EndDisabled();
            }

            ImGui::EndChild();

            ImGui::SameLine();

            if (ImGui::BeginChild("##rotation_keys")) {
               ImGui::SeparatorText("Rotation Keys");

               if (ImGui::BeginChild("##scroll_region",
                                     {0.0f, ImGui::GetContentRegionAvail().y -
                                               84.0f * _display_scale})) {
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

                     if (ImGui::IsItemHovered()) hovered_rotation_key = i;

                     ImGui::SetCursorPos(cursor);

                     ImGui::Text("%.2f", selected_animation->rotation_keys[i].time);

                     ImGui::PopID();
                  }
               }

               ImGui::EndChild();

               ImGui::SeparatorText("New Key");

               ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

               ImGui::SliderFloat("##new_key_time",
                                  &_animation_editor_context.new_rotation_key_time,
                                  0.0f, selected_animation->runtime,
                                  "Time: %.2f", ImGuiSliderFlags_AlwaysClamp);

               ImGui::BeginDisabled(
                  not is_unique_key_time(selected_animation->rotation_keys,
                                         _animation_editor_context.new_rotation_key_time));

               if (ImGui::Button("Add", {ImGui::GetContentRegionAvail().x, 0.0f})) {
                  std::optional<int32> previous_key_index;

                  for (int32 i = 0;
                       i < std::ssize(selected_animation->rotation_keys); ++i) {
                     if (_animation_editor_context.new_rotation_key_time >=
                         selected_animation->rotation_keys[i].time) {
                        previous_key_index = i;
                     }
                     else {
                        break;
                     }
                  }

                  const int32 insert_before_index =
                     previous_key_index ? *previous_key_index + 1 : 0;
                  const world::rotation_key new_key =
                     world::make_rotation_key_for_time(*selected_animation,
                                                       _animation_editor_context.new_rotation_key_time);

                  _edit_stack_world.apply(edits::make_insert_animation_key(
                                             &selected_animation->rotation_keys,
                                             insert_before_index, new_key),
                                          _edit_context);

                  _animation_editor_context.selected.key_type =
                     animation_key_type::rotation;
                  _animation_editor_context.selected.key = insert_before_index;
               }

               ImGui::EndDisabled();
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
                  if (_animation_editor_config.auto_tangents) {
                     update_position_auto_tangents(*selected_animation,
                                                   selected_key, position,
                                                   _animation_editor_config.auto_tangent_smoothness,
                                                   _edit_stack_world, _edit_context);
                  }
                  else {
                     _edit_stack_world.apply(edits::make_set_vector_value(
                                                &selected_animation->position_keys,
                                                selected_key,
                                                &world::position_key::position, position),
                                             _edit_context);
                  }
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

                  if (ImGui::Selectable("Hermite Spline",
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

                  if (ImGui::Selectable("Hermite Spline",
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

      if (not _gizmo.want_capture_mouse()) {
         const graphics::camera_ray cursor_ray =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         if (std::optional<int32> hit =
                world::raycast_position_keys(cursor_ray.origin, cursor_ray.direction,
                                             *selected_animation, base_rotation,
                                             base_position, 1.0f)) {
            hovered_position_key = *hit;
         }

         if (std::optional<int32> hit =
                world::raycast_rotation_keys(cursor_ray.origin, cursor_ray.direction,
                                             *selected_animation, base_rotation,
                                             base_position, 1.0f)) {
            hovered_rotation_key = *hit;
         }
      }

      if (_animation_editor_context.select) {
         if (hovered_position_key) {
            _animation_editor_context.selected.key = *hovered_position_key;
            _animation_editor_context.selected.key_type = animation_key_type::position;
         }
         else if (hovered_rotation_key) {
            _animation_editor_context.selected.key = *hovered_rotation_key;
            _animation_editor_context.selected.key_type = animation_key_type::rotation;
         }

         _animation_editor_context.select = false;
      }

      if (_animation_editor_context.select_behind) {
         if (hovered_rotation_key) {
            _animation_editor_context.selected.key = *hovered_rotation_key;
            _animation_editor_context.selected.key_type = animation_key_type::rotation;
         }
         else if (hovered_position_key) {
            _animation_editor_context.selected.key = *hovered_position_key;
            _animation_editor_context.selected.key_type = animation_key_type::position;
         }

         _animation_editor_context.select_behind = false;
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

            if (_animation_editor_config.auto_tangents) {
               update_position_auto_tangents(*selected_animation, selected_key,
                                             key.position + move_delta,
                                             _animation_editor_config.auto_tangent_smoothness,
                                             _edit_stack_world, _edit_context);
            }
            else {
               _edit_stack_world
                  .apply(edits::make_set_vector_value(&selected_animation->position_keys,
                                                      selected_key,
                                                      &world::position_key::position,
                                                      key.position + move_delta),
                         _edit_context);
            }
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

      if (hovered_position_key and
          *hovered_position_key < std::ssize(selected_animation->position_keys)) {
         const world::position_key& hovered_key =
            selected_animation->position_keys[*hovered_position_key];

         float4x4 transform{};
         transform[3] = {hovered_key.position + base_position, 1.0f};

         _tool_visualizers.add_octahedron_wireframe(transform,
                                                    _settings.graphics.hover_color);
      }

      if (hovered_rotation_key and
          *hovered_rotation_key < std::ssize(selected_animation->rotation_keys)) {
         const world::rotation_key& hovered_key =
            selected_animation->rotation_keys[*hovered_rotation_key];

         float4x4 transform =
            world::evaluate_animation(*selected_animation, base_rotation,
                                      base_position, hovered_key.time);

         _tool_visualizers.add_arrow_wireframe(transform,
                                               float4{_settings.graphics.hover_color,
                                                      1.0f});
      }

      if (_hotkeys_view_show) {
         ImGui::Begin("Hotkeys");

         ImGui::SeparatorText("Animation Editing");

         ImGui::Text("Select");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Animation Editing", "Select")));

         ImGui::Text("Select (Behind)");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Animation Editing", "Select (Behind)")));

         ImGui::End();
      }
   }
}

}