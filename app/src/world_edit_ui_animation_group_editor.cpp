#include "edits/add_animation_group.hpp"
#include "edits/add_animation_group_entry.hpp"
#include "edits/delete_animation_group.hpp"
#include "edits/delete_animation_group_entry.hpp"
#include "edits/imgui_ext.hpp"
#include "edits/set_value.hpp"
#include "math/matrix_funcs.hpp"
#include "math/vector_funcs.hpp"
#include "utility/srgb_conversion.hpp"
#include "utility/string_icompare.hpp"
#include "world/utility/animation.hpp"
#include "world/utility/world_utilities.hpp"
#include "world_edit.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

namespace {

bool is_unique_entry(const world::animation_group& group,
                     std::string_view animation, std::string_view object) noexcept
{
   for (auto& entry : group.entries) {
      if (string::iequals(entry.animation, animation) and
          string::iequals(entry.object, object)) {
         return false;
      }
   }

   return true;
}

}

void world_edit::ui_show_animation_group_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({640.0f * _display_scale, 698.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({640.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   if (ImGui::Begin("Animation Group Editor", &_animation_group_editor_open)) {
      if (ImGui::BeginChild("Animations", {160.0f * _display_scale, 0.0f},
                            ImGuiChildFlags_ResizeX)) {
         ImGui::SeparatorText("Groups");

         if (ImGui::BeginChild("##scroll_region",
                               {0.0f, ImGui::GetContentRegionAvail().y -
                                         110.0f * _display_scale})) {
            for (int32 i = 0; i < std::ssize(_world.animation_groups); ++i) {
               ImGui::PushID(i);

               if (ImGui::Selectable(_world.animation_groups[i].name.c_str(),
                                     _animation_group_editor_context.selected.id ==
                                        _world.animation_groups[i].id)) {
                  _animation_group_editor_context.selected = {
                     .id = _world.animation_groups[i].id};
               }

               ImGui::PopID();
            }
         }

         ImGui::EndChild();

         ImGui::BeginDisabled(_animation_group_editor_context.selected.id ==
                              world::animation_group_id{world::max_id});

         if (ImGui::Button("Delete", {ImGui::GetContentRegionAvail().x, 0.0f})) {
            world::animation_group* selected_group =
               world::find_entity(_world.animation_groups,
                                  _animation_group_editor_context.selected.id);

            if (selected_group) {
               _edit_stack_world
                  .apply(edits::make_delete_animation_group(static_cast<uint32>(
                            selected_group - _world.animation_groups.data())),
                         _edit_context);
            }

            _animation_group_editor_context.selected = {};
         }

         ImGui::EndDisabled();

         ImGui::SeparatorText("New Group");

         ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

         ImGui::InputTextWithHint("##new_name", "i.e door_close",
                                  &_animation_group_editor_config.new_group_name);

         ImGui::BeginDisabled(_animation_group_editor_config.new_group_name.empty());

         if (ImGui::Button("Add", {ImGui::GetContentRegionAvail().x, 0.0f})) {
            if (_world.animation_groups.size() < _world.animation_groups.max_size()) {
               const world::animation_group_id id =
                  _world.next_id.animation_groups.aquire();

               _edit_stack_world.apply(edits::make_add_animation_group(
                                          {.name = world::create_unique_name(
                                              _world.animation_groups,
                                              _animation_group_editor_config.new_group_name),
                                           .id = id}),
                                       _edit_context);
               _animation_group_editor_config.new_group_name.clear();
               _animation_group_editor_context.selected = {.id = id};
            }
            else {
               MessageBoxA(_window,
                           fmt::format("Max Animation Groups ({}) Reached",
                                       _world.animation_groups.max_size())
                              .c_str(),
                           "Limit Reached", MB_OK);
            }
         }

         ImGui::EndDisabled();
      }

      ImGui::EndChild();

      ImGui::SameLine();

      world::animation_group* selected_group =
         world::find_entity(_world.animation_groups,
                            _animation_group_editor_context.selected.id);

      if (ImGui::BeginChild("##selected") and selected_group) {
         ImGui::SeparatorText(selected_group->name.c_str());

         if (ImGui::BeginChild("##properties", {0.0f, 180.0f * _display_scale},
                               ImGuiChildFlags_ResizeY)) {

            ImGui::InputText("Name", &selected_group->name, _edit_stack_world,
                             _edit_context, [this](std::string* new_name) noexcept {
                                *new_name =
                                   world::create_unique_name(_world.animation_groups,
                                                             *new_name);
                             });

            ImGui::Checkbox("Play When Level Begins",
                            &selected_group->play_when_level_begins,
                            _edit_stack_world, _edit_context);
            ImGui::Checkbox("Disable Hierarchies", &selected_group->disable_hierarchies,
                            _edit_stack_world, _edit_context);
            ImGui::Checkbox("Stops When Object Is Controlled",
                            &selected_group->stops_when_object_is_controlled,
                            _edit_stack_world, _edit_context);

            ImGui::SeparatorText("Playback Preview");

            if (ImGui::DragFloat("Playback Time",
                                 &_animation_group_editor_context.selected.playback_time,
                                 0.5f, 0.0f, 1e10f, "%.2f")) {
               _animation_group_editor_context.selected.playback_state =
                  animation_playback_state::paused;
            }

            const float button_width =
               (ImGui::CalcItemWidth() - ImGui::GetStyle().ItemInnerSpacing.x) * 0.5f;

            if (_animation_group_editor_context.selected.playback_state ==
                animation_playback_state::stopped) {
               if (ImGui::Button("Play", {button_width, 0.0f})) {
                  _animation_group_editor_context.selected.playback_state =
                     animation_playback_state::play;
                  _animation_group_editor_context.selected.playback_tick_start =
                     std::chrono::steady_clock::now();
               }
            }
            else if (_animation_group_editor_context.selected.playback_state ==
                     animation_playback_state::paused) {
               if (ImGui::Button("Resume", {button_width, 0.0f})) {
                  _animation_group_editor_context.selected.playback_state =
                     animation_playback_state::play;
                  _animation_group_editor_context.selected.playback_tick_start =
                     std::chrono::steady_clock::now();
               }
            }
            else if (_animation_group_editor_context.selected.playback_state ==
                     animation_playback_state::play) {
               if (ImGui::Button("Pause", {button_width, 0.0f})) {
                  _animation_group_editor_context.selected.playback_state =
                     animation_playback_state::paused;
               }
            }

            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

            if (ImGui::Button("Stop", {button_width, 0.0f})) {
               _animation_group_editor_context.selected.playback_state =
                  animation_playback_state::stopped;
               _animation_group_editor_context.selected.playback_time = 0.0f;
            }
         }

         ImGui::EndChild();

         if (ImGui::BeginChild("##entries")) {
            ImGui::SeparatorText("Entries");

            if (ImGui::BeginChild("##scrolling",
                                  {0.0f, ImGui::GetContentRegionAvail().y -
                                            84.0f * _display_scale})) {
               for (int32 i = 0; i < std::ssize(selected_group->entries); ++i) {
                  if (i != 0) ImGui::Separator();

                  ImGui::PushID(i);

                  world::animation_group::entry& entry = selected_group->entries[i];

                  const float close_width = ImGui::CalcTextSize("X").x +
                                            (ImGui::GetStyle().FramePadding.x * 2.0f);
                  const float input_width =
                     (ImGui::CalcItemWidth() -
                      (ImGui::GetStyle().ItemInnerSpacing.x * 2.0f) - close_width) *
                     0.5f;

                  ImGui::SetNextItemWidth(input_width);

                  if (absl::InlinedVector<char, 256> buffer{entry.animation.begin(),
                                                            entry.animation.end()};
                      ImGui::InputTextAutoComplete("##animation", &buffer, [&]() noexcept {
                         std::array<std::string_view, 6> entries;
                         std::size_t matching_count = 0;

                         for (const world::animation& animation : _world.animations) {
                            if (matching_count == entries.size()) break;

                            if (string::icontains(animation.name, entry.animation)) {
                               entries[matching_count] = animation.name;

                               ++matching_count;
                            }
                         }

                         return entries;
                      })) {
                     _edit_stack_world.apply(edits::make_set_vector_value(
                                                &selected_group->entries, i,
                                                &world::animation_group::entry::animation,
                                                {buffer.data(), buffer.size()}),
                                             _edit_context);
                  }

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }

                  ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

                  ImGui::SetNextItemWidth(input_width);

                  if (absl::InlinedVector<char, 256> buffer{entry.object.begin(),
                                                            entry.object.end()};
                      ImGui::InputTextAutoComplete("##object", &buffer, [&]() noexcept {
                         std::array<std::string_view, 6> entries;
                         std::size_t matching_count = 0;

                         for (const world::object& object : _world.objects) {
                            if (matching_count == entries.size()) break;

                            if (string::icontains(object.name, entry.object)) {
                               entries[matching_count] = object.name;

                               ++matching_count;
                            }
                         }

                         return entries;
                      })) {
                     _edit_stack_world.apply(edits::make_set_vector_value(
                                                &selected_group->entries, i,
                                                &world::animation_group::entry::object,
                                                {buffer.data(), buffer.size()}),
                                             _edit_context);
                  }

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }

                  ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

                  if (ImGui::Button("X", {close_width, 0.0f})) {
                     _edit_stack_world.apply(edits::make_delete_animation_group_entry(
                                                &selected_group->entries, i),
                                             _edit_context);
                  }

                  ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

                  ImGui::Text("Animation-Object");

                  ImGui::PopID();
               }
            }

            ImGui::EndChild();

            if (ImGui::BeginChild("##add_new_entry")) {
               ImGui::SeparatorText("Add New Entry");

               const float item_wdith =
                  (ImGui::CalcItemWidth() - ImGui::GetStyle().ItemInnerSpacing.x) * 0.5f;

               ImGui::SetNextItemWidth(item_wdith);

               if (absl::InlinedVector<char, 256> buffer{
                      _animation_group_editor_config.new_entry_animation_name.begin(),
                      _animation_group_editor_config.new_entry_animation_name.end()};
                   ImGui::InputTextAutoComplete("##animation", &buffer, [&]() noexcept {
                      std::array<std::string_view, 6> entries;
                      std::size_t matching_count = 0;

                      for (const world::animation& animation : _world.animations) {
                         if (matching_count == entries.size()) break;

                         if (string::icontains(animation.name,
                                               _animation_group_editor_config.new_entry_animation_name)) {
                            entries[matching_count] = animation.name;

                            ++matching_count;
                         }
                      }

                      return entries;
                   })) {
                  _animation_group_editor_config
                     .new_entry_animation_name = {buffer.data(), buffer.size()};
               }

               ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

               ImGui::SetNextItemWidth(item_wdith);

               if (absl::InlinedVector<char, 256> buffer{
                      _animation_group_editor_config.new_entry_object_name.begin(),
                      _animation_group_editor_config.new_entry_object_name.end()};
                   ImGui::InputTextAutoComplete("##object", &buffer, [&]() noexcept {
                      std::array<std::string_view, 6> entries;
                      std::size_t matching_count = 0;

                      for (const world::object& object : _world.objects) {
                         if (matching_count == entries.size()) break;

                         if (string::icontains(object.name,
                                               _animation_group_editor_config.new_entry_object_name)) {
                            entries[matching_count] = object.name;

                            ++matching_count;
                         }
                      }

                      return entries;
                   })) {
                  _animation_group_editor_config.new_entry_object_name = {buffer.data(),
                                                                          buffer.size()};
               }

               ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

               ImGui::Text("Animation-Object");

               ImGui::BeginDisabled(
                  _animation_group_editor_config.new_entry_animation_name.empty() or
                  _animation_group_editor_config.new_entry_object_name.empty() or
                  not is_unique_entry(*selected_group,
                                      _animation_group_editor_config.new_entry_animation_name,
                                      _animation_group_editor_config.new_entry_object_name));

               if (ImGui::Button("Add Entry", {item_wdith, 0.0f})) {
                  _edit_stack_world.apply(
                     edits::make_add_animation_group_entry(
                        &selected_group->entries,
                        {.animation = _animation_group_editor_config.new_entry_animation_name,
                         .object = _animation_group_editor_config.new_entry_object_name}),
                     _edit_context);

                  _animation_group_editor_config.new_entry_animation_name.clear();
                  _animation_group_editor_config.new_entry_object_name.clear();
               }

               ImGui::EndDisabled();

               ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

               if (ImGui::Button("Pick Object", {item_wdith, 0.0f})) {
                  _world_draw_mask.objects = true;
                  _world_hit_mask.objects = true;
                  _animation_group_editor_context.pick_object = {.active = true};
               }
            }

            ImGui::EndChild();
         }

         ImGui::EndChild();
      }

      ImGui::EndChild();
   }

   ImGui::End();

   if (not _animation_group_editor_open) return;

   world::animation_group* selected_group =
      world::find_entity(_world.animation_groups,
                         _animation_group_editor_context.selected.id);

   if (selected_group) {
      if (_animation_group_editor_context.selected.playback_state !=
          animation_playback_state::stopped) {
         std::vector<const world::animation*>& playback_animations =
            _animation_group_editor_context.selected.playback_animations;
         std::vector<const world::object*>& playback_objects =
            _animation_group_editor_context.selected.playback_objects;

         playback_animations.clear();
         playback_animations.resize(selected_group->entries.size());

         playback_objects.clear();
         playback_objects.resize(selected_group->entries.size());

         for (const world::animation& animation : _world.animations) {
            for (uint32 i = 0; i < selected_group->entries.size(); ++i) {
               const world::animation_group::entry& entry =
                  selected_group->entries[i];

               if (string::iequals(animation.name, entry.animation)) {
                  playback_animations[i] = &animation;
               }
            }
         }

         for (const world::object& object : _world.objects) {
            for (uint32 i = 0; i < selected_group->entries.size(); ++i) {
               const world::animation_group::entry& entry =
                  selected_group->entries[i];

               if (string::iequals(object.name, entry.object)) {
                  playback_objects[i] = &object;

                  break;
               }
            }
         }

         bool loop = false;
         float runtime = 0.0f;

         for (const world::animation* animation : playback_animations) {
            if (not animation) continue;

            loop |= animation->loop;
            runtime = std::max(animation->runtime, runtime);
         }

         if (_animation_group_editor_context.selected.playback_state ==
             animation_playback_state::play) {
            std::chrono::steady_clock::time_point playback_tick_last =
               _animation_group_editor_context.selected.playback_tick_start;

            _animation_group_editor_context.selected.playback_tick_start =
               std::chrono::steady_clock::now();

            _animation_group_editor_context.selected.playback_time +=
               std::chrono::duration_cast<std::chrono::duration<float>>(
                  _animation_group_editor_context.selected.playback_tick_start -
                  playback_tick_last)
                  .count();

            if (_animation_group_editor_context.selected.playback_time > runtime and
                not loop) {
               _animation_group_editor_context.selected.playback_state =
                  animation_playback_state::stopped;
               _animation_group_editor_context.selected.playback_time = 0.0f;
            }
         }

         for (uint32 i = 0; i < playback_animations.size(); ++i) {
            if (not playback_animations[i]) continue;

            if (playback_objects[i]) {
               _tool_visualizers.add_ghost_object(
                  world::evaluate_animation(
                     *playback_animations[i], playback_objects[i]->rotation,
                     playback_objects[i]->position,
                     _animation_group_editor_context.selected.playback_time),
                  playback_objects[i]->id);
            }
            else {
               _tool_visualizers.add_arrow_wireframe(
                  world::evaluate_animation(
                     *playback_animations[i], quaternion{}, float3{},
                     _animation_group_editor_context.selected.playback_time) *
                     float4x4{{8.0f, 0.0f, 0.0f, 0.0f},
                              {0.0f, 8.0f, 0.0f, 0.0f},
                              {0.0f, 0.0f, 8.0f, 0.0f},
                              {0.0f, 0.0f, 0.0f, 1.0f}},
                  {_settings.graphics.creation_color, 1.0f});
            }
         }
      }
   }

   if (_animation_group_editor_context.pick_object.finish) {
      if (_interaction_targets.hovered_entity and
          std::holds_alternative<world::object_id>(*_interaction_targets.hovered_entity)) {
         const world::object* object =
            world::find_entity(_world.objects,
                               std::get<world::object_id>(
                                  *_interaction_targets.hovered_entity));

         _animation_group_editor_config.new_entry_object_name = object->name;
      }

      _animation_group_editor_context.pick_object = {};
   }
}
}