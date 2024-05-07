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
            // TODO: Add group.
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

            if (ImGui::BeginChild("##scrolling")) {
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
   }
}
}