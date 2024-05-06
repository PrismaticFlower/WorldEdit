#include "edits/add_animation.hpp"
#include "edits/bundle.hpp"
#include "edits/delete_animation.hpp"
#include "edits/delete_animation_key.hpp"
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
#include <misc/cpp/imgui_stdlib.h>

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

auto clamp_position(int32 index, const world::animation& animation) noexcept -> int32
{
   if (animation.loop) {
      const int32 count = static_cast<int32>(std::ssize(animation.position_keys));

      return (index + count) % count;
   }
   else {
      const int32 max_key =
         static_cast<int32>(std::ssize(animation.position_keys) - 1);

      return std::clamp(index, 0, max_key);
   }
}

void convert_to_smooth_spline(world::animation& animation, float smoothness,
                              edits::stack<world::edit_context>& edit_stack,
                              world::edit_context& edit_context) noexcept
{
   edits::bundle_vector edit_bundle;

   edit_bundle.reserve(animation.position_keys.size() * 3);

   for (int32 i = 0; i < std::ssize(animation.position_keys); ++i) {
      const world::position_key& key_back =
         animation.position_keys[clamp_position(i - 1, animation)];
      const world::position_key& key =
         animation.position_keys[clamp_position(i, animation)];
      const world::position_key& key_forward =
         animation.position_keys[clamp_position(i + 1, animation)];
      const world::position_key& key_forward_forward =
         animation.position_keys[clamp_position(i + 2, animation)];

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

   {
      const world::position_key& key_forward_forward =
         animation.position_keys[clamp_position(key_index + 2, animation)];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, key_index,
                                      &world::position_key::position, key_new_position));
      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys, key_index,
                                      &world::position_key::tangent_next,
                                      smoothness * (key_forward_forward.position -
                                                    key_new_position)));
   }

   if (key_index - 2 >= 0 or animation.loop) {
      const world::position_key& key =
         animation.position_keys[clamp_position(key_index - 2, animation)];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys,
                                      clamp_position(key_index - 2, animation),
                                      &world::position_key::tangent_next,
                                      smoothness * (key_new_position - key.position)));
   }

   if (key_index - 1 >= 0 or animation.loop) {
      const world::position_key& key_back =
         animation.position_keys[clamp_position(key_index - 2, animation)];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys,
                                      clamp_position(key_index - 1, animation),
                                      &world::position_key::tangent,
                                      smoothness *
                                         (key_new_position - key_back.position)));
   }

   if (key_index + 1 < std::ssize(animation.position_keys) or animation.loop) {
      const world::position_key& key_forward =
         animation.position_keys[clamp_position(key_index + 2, animation)];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys,
                                      clamp_position(key_index + 1, animation),
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

   const float3 key_new_position = animation.position_keys[key_index].position;

   {
      const world::position_key& key_back =
         animation.position_keys[clamp_position(key_index - 1, animation)];
      const world::position_key& key_forward =
         animation.position_keys[clamp_position(key_index + 1, animation)];
      const world::position_key& key_forward_forward =
         animation.position_keys[clamp_position(key_index + 2, animation)];

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

   if (key_index - 2 >= 0 or animation.loop) {
      const world::position_key& key =
         animation.position_keys[clamp_position(key_index - 2, animation)];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys,
                                      clamp_position(key_index - 2, animation),
                                      &world::position_key::tangent_next,
                                      smoothness * (key_new_position - key.position)));
   }

   if (key_index - 1 >= 0 or animation.loop) {
      const world::position_key& key_back =
         animation.position_keys[clamp_position(key_index - 2, animation)];
      const world::position_key& key =
         animation.position_keys[clamp_position(key_index - 1, animation)];
      const world::position_key& key_forward_forward =
         animation.position_keys[clamp_position(key_index + 1, animation)];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys,
                                      clamp_position(key_index - 1, animation),
                                      &world::position_key::tangent,
                                      smoothness *
                                         (key_new_position - key_back.position)));
      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys,
                                      clamp_position(key_index - 1, animation),
                                      &world::position_key::tangent_next,
                                      smoothness * (key_forward_forward.position -
                                                    key.position)));
   }

   if (key_index + 1 < std::ssize(animation.position_keys) or animation.loop) {
      const world::position_key& key_forward =
         animation.position_keys[clamp_position(key_index + 2, animation)];

      edit_bundle.push_back(
         edits::make_set_vector_value(&animation.position_keys,
                                      clamp_position(key_index + 1, animation),
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
   ImGui::SetNextWindowSize({640.0f * _display_scale, 698.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({640.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   std::optional<int32> hovered_position_key;
   std::optional<int32> hovered_rotation_key;
   std::optional<float> hovered_position_time;
   std::optional<float> hovered_rotation_time;

   if (ImGui::Begin("Animation Editor", &_animation_editor_open)) {
      if (ImGui::BeginChild("Animations", {160.0f * _display_scale, 0.0f},
                            ImGuiChildFlags_ResizeX)) {
         ImGui::SeparatorText("Animations");

         if (ImGui::BeginChild("##scroll_region",
                               {0.0f, ImGui::GetContentRegionAvail().y -
                                         110.0f * _display_scale})) {
            for (int32 i = 0; i < std::ssize(_world.animations); ++i) {
               ImGui::PushID(i);

               if (ImGui::Selectable(_world.animations[i].name.c_str(),
                                     _animation_editor_context.selected.id ==
                                        _world.animations[i].id)) {
                  _animation_editor_context.selected = {
                     .id = _world.animations[i].id};
               }

               ImGui::PopID();
            }
         }

         ImGui::EndChild();

         ImGui::BeginDisabled(_animation_editor_context.selected.id ==
                              world::animation_id{world::max_id});

         if (ImGui::Button("Delete", {ImGui::GetContentRegionAvail().x, 0.0f})) {
            world::animation* selected_animation =
               world::find_entity(_world.animations,
                                  _animation_editor_context.selected.id);

            if (selected_animation) {
               _edit_stack_world
                  .apply(edits::make_delete_animation(static_cast<uint32>(
                                                         selected_animation -
                                                         _world.animations.data()),
                                                      _world),
                         _edit_context);
            }

            _animation_editor_context.selected = {};
         }

         ImGui::EndDisabled();

         ImGui::SeparatorText("New Animation");

         ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

         ImGui::InputTextWithHint("##new_name", "i.e door_close",
                                  &_animation_editor_config.new_animation_name);

         ImGui::BeginDisabled(_animation_editor_config.new_animation_name.empty());

         if (ImGui::Button("Add", {ImGui::GetContentRegionAvail().x, 0.0f})) {
            _edit_stack_world.apply(
               edits::make_add_animation(world::animation{
                  .name = world::create_unique_name(_world.animations,
                                                    _animation_editor_config.new_animation_name),
                  .id = _world.next_id.animations.aquire()}),
               _edit_context);

            _animation_editor_config.new_animation_name.clear();
            _animation_editor_context.selected = {.id = _world.animations.back().id};
         }

         ImGui::EndDisabled();
      }

      ImGui::EndChild();

      ImGui::SameLine();

      world::animation* selected_animation =
         world::find_entity(_world.animations,
                            _animation_editor_context.selected.id);

      if (ImGui::BeginChild("##selected") and selected_animation) {
         ImGui::SeparatorText(selected_animation->name.c_str());

         if (ImGui::BeginChild("##properties", {0.0f, 232.0f * _display_scale},
                               ImGuiChildFlags_ResizeY)) {

            ImGui::InputText("Name", &selected_animation->name, _edit_stack_world,
                             _edit_context, [this](std::string* new_name) noexcept {
                                *new_name = world::create_unique_name(_world.animations,
                                                                      *new_name);
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

            ImGui::SeparatorText("Playback Preview");

            if (ImGui::SliderFloat("Playback Time",
                                   &_animation_editor_context.selected.playback_time,
                                   0.0f, selected_animation->runtime, "%.2f")) {
               _animation_editor_context.selected.playback_state =
                  animation_playback_state::paused;
            }

            const float button_width =
               (ImGui::CalcItemWidth() - ImGui::GetStyle().ItemInnerSpacing.x) * 0.5f;

            if (_animation_editor_context.selected.playback_state ==
                animation_playback_state::stopped) {
               if (ImGui::Button("Play", {button_width, 0.0f})) {
                  _animation_editor_context.selected.playback_state =
                     animation_playback_state::play;
                  _animation_editor_context.selected.playback_tick_start =
                     std::chrono::steady_clock::now();
               }
            }
            else if (_animation_editor_context.selected.playback_state ==
                     animation_playback_state::paused) {
               if (ImGui::Button("Resume", {button_width, 0.0f})) {
                  _animation_editor_context.selected.playback_state =
                     animation_playback_state::play;
                  _animation_editor_context.selected.playback_tick_start =
                     std::chrono::steady_clock::now();
               }
            }
            else if (_animation_editor_context.selected.playback_state ==
                     animation_playback_state::play) {
               if (ImGui::Button("Pause", {button_width, 0.0f})) {
                  _animation_editor_context.selected.playback_state =
                     animation_playback_state::paused;
               }
            }

            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

            if (ImGui::Button("Stop", {button_width, 0.0f})) {
               _animation_editor_context.selected.playback_state =
                  animation_playback_state::stopped;
               _animation_editor_context.selected.playback_time = 0.0f;
            }
         }

         ImGui::EndChild();

         if (ImGui::BeginChild("##keys", {0.0f, 216.0 * _display_scale},
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

               ImGui::BeginGroup();

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

               if (ImGui::Button("Place", {button_width, 0.0f})) {
                  _animation_editor_context.place = {.active = true};
               }

               ImGui::SetItemTooltip(
                  "Add a new key and place it with the cursor.");

               ImGui::EndDisabled();

               ImGui::EndGroup();

               if (ImGui::IsItemHovered()) {
                  hovered_position_time =
                     _animation_editor_context.new_position_key_time;
               }
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

               ImGui::BeginGroup();

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

               ImGui::EndGroup();

               if (ImGui::IsItemHovered()) {
                  hovered_rotation_time =
                     _animation_editor_context.new_rotation_key_time;
               }
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

               if (ImGui::Button("Auto-Fill Tangents", {ImGui::CalcItemWidth(), 0.0f})) {
                  if (_animation_editor_config.match_tangents) {
                     update_auto_tangents(*selected_animation, selected_key,
                                          _animation_editor_config.auto_tangent_smoothness,
                                          _edit_stack_world, _edit_context, true);
                  }
                  else {
                     const world::position_key& key_back =
                        selected_animation
                           ->position_keys[clamp_position(selected_key - 1, *selected_animation)];
                     const world::position_key& key_forward =
                        selected_animation
                           ->position_keys[clamp_position(selected_key + 1, *selected_animation)];
                     const world::position_key& key_forward_forward =
                        selected_animation
                           ->position_keys[clamp_position(selected_key + 2, *selected_animation)];

                     _edit_stack_world
                        .apply(edits::make_set_vector_value(
                                  &selected_animation->position_keys,
                                  selected_key, &world::position_key::tangent,
                                  _animation_editor_config.auto_tangent_smoothness *
                                     (key_forward.position - key_back.position)),
                               _edit_context, {.transparent = true});
                     _edit_stack_world
                        .apply(edits::make_set_vector_value(
                                  &selected_animation->position_keys,
                                  selected_key, &world::position_key::tangent_next,
                                  _animation_editor_config.auto_tangent_smoothness *
                                     (key_forward_forward.position - key.position)),
                               _edit_context, {.transparent = true});
                  }
               }

               ImGui::SetItemTooltip(
                  "Auto-fill tangents for this key the same way as Convert to "
                  "Smooth Spline would.\n\nIf Match Tangents is enabled then "
                  "neighbouring keys that would use this key's position for "
                  "their tangents will have their tangents auto-filled as "
                  "well.");

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
      world::object_id animated_object_id = world::max_id;

      for (const world::animation_group& group : _world.animation_groups) {
         for (const world::animation_group::entry& entry : group.entries) {
            if (string::iequals(entry.animation, selected_animation->name)) {
               for (const world::object& object : _world.objects) {
                  if (string::iequals(entry.object, object.name)) {
                     base_rotation = object.rotation;
                     base_position = object.position;
                     animated_object_id = object.id;

                     break;
                  }
               }
            }
         }
      }

      std::optional<int32> hovered_background_position_key;
      std::optional<int32> hovered_background_rotation_key;

      if (not _gizmo.want_capture_mouse()) {
         const graphics::camera_ray cursor_ray =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         if (world::raycast_result_keys result =
                world::raycast_position_keys(cursor_ray.origin, cursor_ray.direction,
                                             *selected_animation, base_rotation,
                                             base_position, 1.0f);
             result.hit) {
            hovered_position_key = result.hit;
            hovered_background_position_key = result.background_hit;
         }

         if (world::raycast_result_keys result =
                world::raycast_rotation_keys(cursor_ray.origin, cursor_ray.direction,
                                             *selected_animation, base_rotation,
                                             base_position, 1.0f);
             result.hit) {
            hovered_rotation_key = result.hit;
            hovered_background_rotation_key = result.background_hit;
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
         if (hovered_background_position_key) {
            _animation_editor_context.selected.key = *hovered_background_position_key;
            _animation_editor_context.selected.key_type = animation_key_type::position;
         }
         else if (hovered_background_rotation_key) {
            _animation_editor_context.selected.key = *hovered_background_rotation_key;
            _animation_editor_context.selected.key_type = animation_key_type::rotation;
         }
         else if (hovered_rotation_key) {
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

      if (hovered_position_time) {
         float4x4 key_transform =
            world::evaluate_animation(*selected_animation, base_rotation,
                                      base_position, *hovered_position_time);

         float4x4 transform{};
         transform[3] = key_transform[3];

         _tool_visualizers.add_octahedron_wireframe(transform,
                                                    _settings.graphics.hover_color);
      }

      if (hovered_rotation_time) {
         float4x4 transform =
            world::evaluate_animation(*selected_animation, base_rotation,
                                      base_position, *hovered_rotation_time);

         _tool_visualizers.add_arrow_wireframe(transform,
                                               float4{_settings.graphics.hover_color,
                                                      1.0f});
      }

      if (_animation_editor_context.selected.toggle_playback) {
         if (_animation_editor_context.selected.playback_state ==
             animation_playback_state::play) {
            _animation_editor_context.selected.playback_state =
               animation_playback_state::paused;
         }
         else {
            _animation_editor_context.selected.playback_state =
               animation_playback_state::play;
            _animation_editor_context.selected.playback_tick_start =
               std::chrono::steady_clock::now();
         }

         _animation_editor_context.selected.toggle_playback = false;
      }

      if (_animation_editor_context.selected.stop_playback) {
         _animation_editor_context.selected.playback_state =
            animation_playback_state::stopped;
         _animation_editor_context.selected.playback_time = 0.0f;

         _animation_editor_context.selected.stop_playback = false;
      }

      if (_animation_editor_context.selected.playback_state !=
          animation_playback_state::stopped) {
         if (_animation_editor_context.selected.playback_state ==
             animation_playback_state::play) {
            std::chrono::steady_clock::time_point playback_tick_last =
               _animation_editor_context.selected.playback_tick_start;

            _animation_editor_context.selected.playback_tick_start =
               std::chrono::steady_clock::now();

            _animation_editor_context.selected.playback_time +=
               std::chrono::duration_cast<std::chrono::duration<float>>(
                  _animation_editor_context.selected.playback_tick_start - playback_tick_last)
                  .count();

            if (_animation_editor_context.selected.playback_time >
                selected_animation->runtime) {
               if (selected_animation->loop) {
                  _animation_editor_context.selected.playback_time -=
                     selected_animation->runtime;
               }
               else {
                  _animation_editor_context.selected.playback_state =
                     animation_playback_state::stopped;
                  _animation_editor_context.selected.playback_time = 0.0f;
               }
            }
         }

         if (animated_object_id != world::object_id{world::max_id}) {
            _tool_visualizers.add_ghost_object(
               world::evaluate_animation(*selected_animation, base_rotation, base_position,
                                         _animation_editor_context.selected.playback_time),
               animated_object_id);
         }
         else {
            _tool_visualizers.add_arrow_wireframe(
               world::evaluate_animation(*selected_animation, base_rotation, base_position,
                                         _animation_editor_context.selected.playback_time) *
                  float4x4{{8.0f, 0.0f, 0.0f, 0.0f},
                           {0.0f, 8.0f, 0.0f, 0.0f},
                           {0.0f, 0.0f, 8.0f, 0.0f},
                           {0.0f, 0.0f, 0.0f, 1.0f}},
               {_settings.graphics.creation_color, 1.0f});
         }
      }

      if (_animation_editor_context.place.active) {
         std::optional<int32> previous_key_index;

         for (int32 i = 0; i < std::ssize(selected_animation->position_keys); ++i) {
            if (_animation_editor_context.new_position_key_time >=
                selected_animation->position_keys[i].time) {
               previous_key_index = i;
            }
            else {
               break;
            }
         }

         if (previous_key_index) {
            if (*previous_key_index < std::ssize(selected_animation->position_keys)) {

               _tool_visualizers.add_line(
                  base_position +
                     selected_animation->position_keys[*previous_key_index].position,
                  _cursor_positionWS, 0xff'ff'ff'ffu);
            }

            if (*previous_key_index + 1 < std::ssize(selected_animation->position_keys)) {
               _tool_visualizers.add_line(
                  base_position +
                     selected_animation->position_keys[*previous_key_index + 1].position,
                  _cursor_positionWS, 0xff'ff'ff'ffu);
            }
         }

         float4x4 visualizer_transform;
         visualizer_transform[3] = {_cursor_positionWS, 1.0f};

         _tool_visualizers.add_octahedron_wireframe(visualizer_transform,
                                                    _settings.graphics.creation_color);

         if (_animation_editor_context.place.finish) {
            const int32 insert_before_index =
               previous_key_index ? *previous_key_index + 1 : 0;
            world::position_key new_key = {.time = _animation_editor_context.new_position_key_time,
                                           .position = _cursor_positionWS - base_position};

            if (previous_key_index) {
               new_key.transition =
                  selected_animation->position_keys[*previous_key_index].transition;
            }

            _edit_stack_world
               .apply(edits::make_insert_animation_key(&selected_animation->position_keys,
                                                       insert_before_index, new_key),
                      _edit_context);

            if (_animation_editor_config.auto_tangents) {
               update_auto_tangents(*selected_animation, insert_before_index,
                                    0.5f, _edit_stack_world, _edit_context, true);
            }

            _animation_editor_context.selected.key_type = animation_key_type::position;
            _animation_editor_context.selected.key = insert_before_index;

            _animation_editor_context.place = {};
         }
      }

      if (_animation_editor_context.selected.delete_key) {
         if (_animation_editor_context.selected.key_type == animation_key_type::position and
             selected_key < std::ssize(selected_animation->position_keys)) {
            _edit_stack_world
               .apply(edits::make_delete_animation_key(&selected_animation->position_keys,
                                                       selected_key),
                      _edit_context);

            if (_animation_editor_config.match_tangents and
                _animation_editor_config.auto_tangents) {
               if (selected_key < std::ssize(selected_animation->position_keys)) {
                  update_auto_tangents(*selected_animation, selected_key,
                                       _animation_editor_config.auto_tangent_smoothness,
                                       _edit_stack_world, _edit_context, true);
               }

               if (selected_key - 1 >= 0) {
                  update_auto_tangents(*selected_animation, selected_key - 1,
                                       _animation_editor_config.auto_tangent_smoothness,
                                       _edit_stack_world, _edit_context, true);
               }
            }

            if (selected_key == std::ssize(selected_animation->position_keys)) {
               _animation_editor_context.selected.key -= 1;
            }
         }
         else if (_animation_editor_context.selected.key_type ==
                     animation_key_type::rotation and
                  selected_key < std::ssize(selected_animation->rotation_keys)) {
            _edit_stack_world
               .apply(edits::make_delete_animation_key(&selected_animation->rotation_keys,
                                                       selected_key),
                      _edit_context);

            if (selected_key == std::ssize(selected_animation->rotation_keys)) {
               _animation_editor_context.selected.key -= 1;
            }
         }

         _animation_editor_context.selected.delete_key = false;
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

         ImGui::Text("Toggle Match Tangents");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Animation Editing",
                                   "Toggle Match Tangents")));

         ImGui::Text("Toggle Auto-Tangents");
         ImGui::BulletText(
            get_display_string(_hotkeys.query_binding("Animation Editing",
                                                      "Toggle Auto-Tangents")));

         ImGui::Text("Delete Selected Key");
         ImGui::BulletText(
            get_display_string(_hotkeys.query_binding("Animation Editing",
                                                      "Delete Selected Key")));

         ImGui::Text("Play / Pause");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Animation Editing", "Play / Pause")));

         ImGui::Text("Stop");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Animation Editing", "Stop")));

         ImGui::End();
      }
   }
}

}