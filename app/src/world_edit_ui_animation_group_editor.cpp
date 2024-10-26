#include "edits/add_animation_group.hpp"
#include "edits/add_animation_group_entry.hpp"
#include "edits/delete_animation_group.hpp"
#include "edits/delete_animation_group_entry.hpp"
#include "edits/imgui_ext.hpp"
#include "edits/set_value.hpp"
#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"
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
                  _animation_group_editor_context.selected.playback_tick_timer.restart();
               }
            }
            else if (_animation_group_editor_context.selected.playback_state ==
                     animation_playback_state::paused) {
               if (ImGui::Button("Resume", {button_width, 0.0f})) {
                  _animation_group_editor_context.selected.playback_state =
                     animation_playback_state::play;
                  _animation_group_editor_context.selected.playback_tick_timer.restart();
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
               _animation_group_editor_context.selected.playback_cache.clear();
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

                  if (ImGui::IsItemHovered() and
                      ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_MouseLeft)) {
                     const world::animation* animation =
                        world::find_entity(_world.animations,
                                           selected_group->entries[i].animation);

                     if (animation) {
                        _animation_editor_open = true;
                        _animation_editor_context = {
                           .selected = {.id = animation->id}};

                        ImGui::SetWindowFocus("Animation Editor");
                     }
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

                  if (ImGui::IsItemHovered() and
                      ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_MouseLeft)) {
                     const world::object* object =
                        world::find_entity(_world.objects,
                                           selected_group->entries[i].object);

                     if (object) {
                        _interaction_targets.selection.add(object->id);

                        ImGui::SetWindowFocus("Selection");
                     }
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
         using playback_cache_entry =
            animation_group_editor_context::selected::playback_cache_entry;

         std::vector<playback_cache_entry>& playback_cache =
            _animation_group_editor_context.selected.playback_cache;

         bool playback_cache_dirty = false;

         if (playback_cache.size() != selected_group->entries.size()) {
            playback_cache_dirty = true;
         }

         if (playback_cache.size() == selected_group->entries.size()) {
            for (uint32 i = 0; i < playback_cache.size(); ++i) {
               const playback_cache_entry& cache_entry = playback_cache[i];
               const world::animation_group::entry& entry =
                  selected_group->entries[i];

               if (cache_entry.animation_name != entry.animation or
                   cache_entry.object_name != entry.object) {
                  playback_cache_dirty = true;
               }

               if (cache_entry.animation_hierarchy !=
                   world::animation_hierarchy_id{world::max_id}) {
                  const world::animation_hierarchy* hierarchy =
                     world::find_entity(_world.animation_hierarchies,
                                        cache_entry.animation_hierarchy);

                  if (hierarchy) {
                     if (not string::iequals(cache_entry.object_name,
                                             hierarchy->root_object)) {
                        playback_cache_dirty = true;
                     }

                     if (hierarchy->objects.size() != cache_entry.children.size()) {
                        playback_cache_dirty = true;
                     }

                     if (hierarchy->objects.size() == cache_entry.children.size()) {
                        for (uint32 child_index = 0;
                             child_index < hierarchy->objects.size(); ++child_index) {
                           if (hierarchy->objects[child_index] !=
                               cache_entry.children[child_index].name) {
                              playback_cache_dirty = true;
                           }
                        }
                     }
                  }
               }
            }
         }

         if (playback_cache_dirty) {
            playback_cache.clear();
            playback_cache.resize(selected_group->entries.size());

            for (const world::object& object : _world.objects) {
               for (uint32 i = 0; i < selected_group->entries.size(); ++i) {
                  const world::animation_group::entry& entry =
                     selected_group->entries[i];

                  if (string::iequals(object.name, entry.object)) {
                     playback_cache[i].object_name = entry.object;
                     playback_cache[i].object = object.id;
                     playback_cache[i].base_position = object.position;
                     playback_cache[i].base_rotation = object.rotation;

                     const quaternion inverse_rotation = conjugate(object.rotation);

                     playback_cache[i].inverse_base_rotation = inverse_rotation;
                     playback_cache[i].inverse_base_position =
                        inverse_rotation * -object.position;

                     break;
                  }
               }
            }

            for (const world::animation& animation : _world.animations) {
               for (uint32 i = 0; i < selected_group->entries.size(); ++i) {
                  const world::animation_group::entry& entry =
                     selected_group->entries[i];

                  if (string::iequals(animation.name, entry.animation)) {
                     playback_cache[i].animation_name = entry.animation;
                     playback_cache[i].animation = animation.id;
                     playback_cache[i].animation_runtime = animation.runtime;
                     playback_cache[i].animation_loops = animation.loop;
                  }
               }
            }

            for (const world::animation_hierarchy& hierarchy :
                 _world.animation_hierarchies) {
               for (playback_cache_entry& entry : playback_cache) {
                  if (string::iequals(hierarchy.root_object, entry.object_name)) {
                     entry.children.reserve(hierarchy.objects.size());

                     for (const auto& child_name : hierarchy.objects) {
                        for (const world::object& object : _world.objects) {
                           if (not string::iequals(child_name, object.name)) {
                              continue;
                           }

                           float4x4 transform =
                              to_matrix(object.rotation * entry.inverse_base_rotation);
                           transform[3] = {entry.inverse_base_rotation * object.position +
                                              entry.inverse_base_position,
                                           1.0f};

                           entry.children.emplace_back(child_name, transform,
                                                       object.id);
                        }
                     }

                     entry.animation_hierarchy = hierarchy.id;

                     break;
                  }
               }
            }
         }

         bool loop = false;
         float runtime = 0.0f;

         for (const playback_cache_entry& entry : playback_cache) {
            if (entry.animation == world::animation_id{world::max_id}) continue;

            loop |= entry.animation_loops;
            runtime = std::max(entry.animation_runtime, runtime);
         }

         if (_animation_group_editor_context.selected.playback_state ==
             animation_playback_state::play) {
            _animation_group_editor_context.selected.playback_time +=
               _animation_group_editor_context.selected.playback_tick_timer.elapsed();

            _animation_group_editor_context.selected.playback_tick_timer.restart();

            if (_animation_group_editor_context.selected.playback_time > runtime and
                not loop) {
               _animation_group_editor_context.selected.playback_state =
                  animation_playback_state::stopped;
               _animation_group_editor_context.selected.playback_time = 0.0f;
               _animation_group_editor_context.selected.playback_cache.clear();
            }
         }

         for (const playback_cache_entry& entry : playback_cache) {
            if (entry.animation == world::animation_id{world::max_id}) continue;

            const world::animation* animation =
               world::find_entity(_world.animations, entry.animation);

            if (not animation) continue;

            const world::object* object =
               entry.object != world::object_id{world::max_id}
                  ? world::find_entity(_world.objects, entry.object)
                  : nullptr;

            _animation_solver.init(*animation, entry.base_rotation, entry.base_position);

            const float4x4 transform =
               _animation_solver.evaluate(*animation, _animation_group_editor_context
                                                         .selected.playback_time);

            if (object) {
               _tool_visualizers.add_ghost_object(transform, entry.object);

               if (not selected_group->disable_hierarchies) {
                  for (const auto& [child_name, child_transform, child_id] :
                       entry.children) {
                     _tool_visualizers.add_ghost_object(transform * child_transform,
                                                        child_id);
                  }
               }
            }
            else {
               _tool_visualizers.add_arrow_wireframe(
                  transform * float4x4{{8.0f, 0.0f, 0.0f, 0.0f},
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
          _interaction_targets.hovered_entity->is<world::object_id>()) {
         const world::object* object =
            world::find_entity(_world.objects,
                               _interaction_targets.hovered_entity->get<world::object_id>());

         if (object) {
            _animation_group_editor_config.new_entry_object_name = object->name;
         }
      }

      _animation_group_editor_context.pick_object = {};
   }
}
}