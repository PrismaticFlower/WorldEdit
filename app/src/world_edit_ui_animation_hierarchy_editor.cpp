#include "world_edit.hpp"

#include "edits/add_animation_hierarchy.hpp"
#include "edits/add_animation_hierarchy_child.hpp"
#include "edits/delete_animation_hierarchy.hpp"
#include "edits/delete_animation_hierarchy_child.hpp"
#include "edits/set_value.hpp"
#include "imgui_ext.hpp"
#include "utility/string_icompare.hpp"
#include "world/utility/world_utilities.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

namespace {

bool has_child(const world::animation_hierarchy& hierarchy, std::string_view name) noexcept
{
   for (const std::string& child : hierarchy.objects) {
      if (string::iequals(child, name)) return true;
   }

   return false;
}

}

void world_edit::ui_show_animation_hierarchy_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({640.0f * _display_scale, 698.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({640.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   if (ImGui::Begin("Animation Hierarchy Editor", &_animation_hierarchy_editor_open)) {
      if (ImGui::BeginChild("Hierarchies", {160.0f * _display_scale, 0.0f},
                            ImGuiChildFlags_ResizeX)) {
         ImGui::SeparatorText("Hierarchies");

         if (ImGui::BeginChild("##scroll_region",
                               {0.0f, ImGui::GetContentRegionAvail().y -
                                         130.0f * _display_scale})) {
            for (int32 i = 0; i < std::ssize(_world.animation_hierarchies); ++i) {
               ImGui::PushID(i);

               if (ImGui::Selectable(
                      _world.animation_hierarchies[i].root_object.c_str(),
                      _animation_hierarchy_editor_context.selected.id ==
                         _world.animation_hierarchies[i].id)) {
                  _animation_hierarchy_editor_context.selected = {
                     .id = _world.animation_hierarchies[i].id};
               }

               if (ImGui::IsItemHovered()) {
                  const world::object* object =
                     world::find_entity(_world.objects,
                                        _world.animation_hierarchies[i].root_object);

                  if (object) _interaction_targets.hovered_entity = object->id;

                  if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_MouseLeft) and
                      object) {
                     _interaction_targets.selection.add(object->id);

                     ImGui::SetWindowFocus("Selection");
                  }
               }

               ImGui::PopID();
            }
         }

         ImGui::EndChild();

         ImGui::BeginDisabled(_animation_hierarchy_editor_context.selected.id ==
                              world::animation_hierarchy_id{world::max_id});

         if (ImGui::Button("Delete", {ImGui::GetContentRegionAvail().x, 0.0f})) {
            world::animation_hierarchy* hierarchy =
               world::find_entity(_world.animation_hierarchies,
                                  _animation_hierarchy_editor_context.selected.id);

            if (hierarchy) {
               _edit_stack_world
                  .apply(edits::make_delete_animation_hierarchy(static_cast<uint32>(
                            hierarchy - _world.animation_hierarchies.data())),
                         _edit_context);
            }

            _animation_hierarchy_editor_context.selected = {};
         }

         ImGui::EndDisabled();

         ImGui::SeparatorText("New Hierarchy");

         ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

         if (absl::InlinedVector<char, 256> buffer{
                _animation_hierarchy_editor_config.new_root_object_name.begin(),
                _animation_hierarchy_editor_config.new_root_object_name.end()};
             ImGui::InputTextAutoComplete("##new_name", &buffer, [&]() noexcept {
                std::array<std::string_view, 6> entries;
                std::size_t matching_count = 0;

                for (const world::object& object : _world.objects) {
                   if (matching_count == entries.size()) break;

                   if (string::icontains(object.name,
                                         _animation_hierarchy_editor_config.new_root_object_name)) {
                      entries[matching_count] = object.name;

                      ++matching_count;
                   }
                }

                return entries;
             })) {
            _animation_hierarchy_editor_config.new_root_object_name = {buffer.data(),
                                                                       buffer.size()};
         }

         ImGui::BeginDisabled(
            _animation_hierarchy_editor_config.new_root_object_name.empty());

         if (ImGui::Button("Add", {ImGui::GetContentRegionAvail().x, 0.0f})) {
            if (_world.animation_hierarchies.size() <
                _world.animation_hierarchies.max_size()) {
               world::animation_hierarchy_id id =
                  _world.next_id.animation_hierarchies.aquire();

               _edit_stack_world.apply(edits::make_add_animation_hierarchy(
                                          {.root_object =
                                              _animation_hierarchy_editor_config.new_root_object_name,
                                           .id = id}),
                                       _edit_context);

               _animation_hierarchy_editor_context.selected.id = id;
            }
            else {
               MessageBoxA(_window,
                           fmt::format("Max Animation Hierarchies ({}) Reached",
                                       _world.animation_hierarchies.max_size())
                              .c_str(),
                           "Limit Reached", MB_OK);
            }

            _animation_hierarchy_editor_config.new_root_object_name.clear();
         }

         ImGui::EndDisabled();

         if (ImGui::Button("Pick Object", {ImGui::GetContentRegionAvail().x, 0.0f})) {
            _animation_hierarchy_editor_context.pick_object =
               {.target = animation_hierarchy_editor_context::pick_object::target::root,
                .active = true};
         }
      }

      ImGui::EndChild();

      ImGui::SameLine();

      world::animation_hierarchy* selected_hierarchy =
         world::find_entity(_world.animation_hierarchies,
                            _animation_hierarchy_editor_context.selected.id);

      if (ImGui::BeginChild("##selected") and selected_hierarchy) {
         ImGui::SeparatorText(selected_hierarchy->root_object.c_str());

         if (ImGui::BeginChild("##children",
                               {0.0f, ImGui::GetContentRegionAvail().y -
                                         104.0f * _display_scale})) {
            for (int32 i = 0; i < std::ssize(selected_hierarchy->objects); ++i) {
               ImGui::PushID(i);

               const float close_width = ImGui::CalcTextSize("X").x +
                                         (ImGui::GetStyle().FramePadding.x * 2.0f);
               const float input_width = ImGui::CalcItemWidth() -
                                         ImGui::GetStyle().ItemInnerSpacing.x -
                                         close_width;

               ImGui::SetNextItemWidth(input_width);

               if (absl::InlinedVector<char, 256>
                      buffer{selected_hierarchy->objects[i].begin(),
                             selected_hierarchy->objects[i].end()};
                   ImGui::InputTextAutoComplete("##object", &buffer, [&]() noexcept {
                      std::array<std::string_view, 6> entries;
                      std::size_t matching_count = 0;

                      for (const world::object& object : _world.objects) {
                         if (matching_count == entries.size()) break;

                         if (string::icontains(object.name,
                                               selected_hierarchy->objects[i])) {
                            entries[matching_count] = object.name;

                            ++matching_count;
                         }
                      }

                      return entries;
                   })) {
                  _edit_stack_world.apply(
                     edits::make_set_vector_value(&selected_hierarchy->objects, i,
                                                  {buffer.data(), buffer.size()}),
                     _edit_context);
               }

               if (ImGui::IsItemDeactivatedAfterEdit()) {
                  _edit_stack_world.close_last();
               }

               if (ImGui::IsItemHovered()) {
                  const world::object* object =
                     world::find_entity(_world.objects,
                                        selected_hierarchy->objects[i]);

                  if (object) _interaction_targets.hovered_entity = object->id;

                  if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_MouseLeft) and
                      object) {
                     _interaction_targets.selection.add(object->id);

                     ImGui::SetWindowFocus("Selection");
                  }
               }

               ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

               if (ImGui::Button("X", {close_width, 0.0f})) {
                  _edit_stack_world.apply(edits::make_delete_animation_hierarchy_child(
                                             &selected_hierarchy->objects, i),
                                          _edit_context);
               }

               ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

               ImGui::Text("Object");

               ImGui::PopID();
            }
         }

         ImGui::EndChild();

         if (ImGui::BeginChild("##add_new_child")) {
            ImGui::SeparatorText("Add New Child");

            if (absl::InlinedVector<char, 256> buffer{
                   _animation_hierarchy_editor_config.new_child_object_name.begin(),
                   _animation_hierarchy_editor_config.new_child_object_name.end()};
                ImGui::InputTextAutoComplete("Object", &buffer, [&]() noexcept {
                   std::array<std::string_view, 6> entries;
                   std::size_t matching_count = 0;

                   for (const world::object& object : _world.objects) {
                      if (matching_count == entries.size()) break;

                      if (string::icontains(object.name,
                                            _animation_hierarchy_editor_config.new_child_object_name)) {
                         entries[matching_count] = object.name;

                         ++matching_count;
                      }
                   }

                   return entries;
                })) {
               _animation_hierarchy_editor_config
                  .new_child_object_name = {buffer.data(), buffer.size()};
            }

            ImGui::BeginDisabled(
               _animation_hierarchy_editor_config.new_child_object_name.empty() or
               has_child(*selected_hierarchy,
                         _animation_hierarchy_editor_config.new_child_object_name));

            const float button_width =
               (ImGui::CalcItemWidth() - ImGui::GetStyle().ItemInnerSpacing.x) * 0.5f;

            if (ImGui::Button("Add Entry", {button_width, 0.0f})) {
               _edit_stack_world.apply(edits::make_add_animation_hierarchy_child(
                                          &selected_hierarchy->objects,
                                          _animation_hierarchy_editor_config.new_child_object_name),
                                       _edit_context);

               _animation_hierarchy_editor_config.new_child_object_name.clear();
            }

            ImGui::EndDisabled();

            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

            if (ImGui::Button("Pick Object", {button_width, 0.0f})) {
               _world_draw_mask.objects = true;
               _world_hit_mask.objects = true;
               _animation_hierarchy_editor_context.pick_object =
                  {.target = animation_hierarchy_editor_context::pick_object::target::child,
                   .active = true};
            }
         }

         ImGui::EndChild();
      }

      ImGui::EndChild();
   }

   ImGui::End();

   if (not _animation_hierarchy_editor_open) return;

   if (_animation_hierarchy_editor_context.pick_object.finish) {
      if (_interaction_targets.hovered_entity.is<world::object_id>()) {
         const world::object* object =
            world::find_entity(_world.objects,
                               _interaction_targets.hovered_entity.get<world::object_id>());
         if (object and not object->name.empty()) {
            if (_animation_hierarchy_editor_context.pick_object.target ==
                animation_hierarchy_editor_context::pick_object::target::root) {
               if (_world.animation_hierarchies.size() <
                   _world.animation_hierarchies.max_size()) {
                  world::animation_hierarchy_id id =
                     _world.next_id.animation_hierarchies.aquire();

                  _edit_stack_world.apply(edits::make_add_animation_hierarchy(
                                             {.root_object = object->name, .id = id}),
                                          _edit_context);

                  _animation_hierarchy_editor_context.selected.id = id;
               }
               else {
                  MessageBoxA(
                     _window,
                     fmt::format("Max Animation Hierarchies ({}) Reached",
                                 _world.animation_hierarchies.max_size())
                        .c_str(),
                     "Limit Reached", MB_OK);
               }

               _animation_hierarchy_editor_config.new_root_object_name.clear();
            }
            else if (_animation_hierarchy_editor_context.pick_object.target ==
                     animation_hierarchy_editor_context::pick_object::target::child) {
               world::animation_hierarchy* selected_hierarchy =
                  world::find_entity(_world.animation_hierarchies,
                                     _animation_hierarchy_editor_context
                                        .selected.id);

               if (selected_hierarchy and
                   not has_child(*selected_hierarchy, object->name)) {
                  _edit_stack_world.apply(edits::make_add_animation_hierarchy_child(
                                             &selected_hierarchy->objects,
                                             object->name),
                                          _edit_context);
               }

               _animation_hierarchy_editor_config.new_child_object_name.clear();
            }
         }
      }

      _animation_hierarchy_editor_context.pick_object = {};
   }
}
}