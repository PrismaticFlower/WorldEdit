
#include "hotkeys.hpp"
#include "hotkeys_io.hpp"
#include "imgui/imgui.h"
#include "utility/overload.hpp"
#include "utility/string_ops.hpp"

#include <algorithm>

namespace we {

namespace {

const std::wstring_view save_path = L".bindings";

constexpr bool is_mouse_key(const key key) noexcept
{
   switch (key) {
   case key::mouse1:
   case key::mouse2:
   case key::mouse3:
   case key::mouse4:
   case key::mouse5:
      return true;
   default:
      return false;
   }
}

constexpr bool is_keyboard_key(const key key) noexcept
{
   return not is_mouse_key(key);
}

}

hotkeys::hotkeys(commands& commands, output_stream& error_output_stream) noexcept
   : _commands{commands},
     _error_output_stream{error_output_stream},
     _saved_bindings{load_bindings(save_path)},
     _saved_used_bindings{[this] {
        absl::flat_hash_map<std::string, absl::flat_hash_set<hotkey_bind>> saved_used_bindings;
        saved_used_bindings.reserve(_saved_bindings.size());

        for (const auto& [set_name, saved] : _saved_bindings) {
           absl::flat_hash_set<hotkey_bind>& used = saved_used_bindings[set_name];

           used.reserve(saved.size());

           for (const auto& [name, binding] : saved) {
              used.insert(binding);
           }
        }

        return saved_used_bindings;
     }()}
{
   _hotkey_sets.reserve(16);
   _key_events.reserve(256);
}

void hotkeys::add_set(std::string set_name, std::function<bool()> activated,
                      std::initializer_list<hotkey_default> default_hotkeys)
{
   absl::flat_hash_set<std::string> hotkey_set;
   absl::flat_hash_map<hotkey_bind, hotkey> bindings;
   absl::flat_hash_map<std::string, hotkey_bind> query_bindings;
   std::vector<hotkey> unbound_hotkeys;

   hotkey_set.reserve(default_hotkeys.size());
   bindings.reserve(default_hotkeys.size());
   query_bindings.reserve(default_hotkeys.size());
   unbound_hotkeys.reserve(default_hotkeys.size());

   for (auto& default_hotkey : default_hotkeys) {
      validate_command(default_hotkey.command);

      if (not hotkey_set.emplace(default_hotkey.name).second) {
         std::terminate(); // Duplicate hotkey name!
      }

      const bool has_saved_binding =
         _saved_bindings[set_name].contains(default_hotkey.name);
      const bool has_saved_used_set = _saved_used_bindings.contains(set_name);

      const hotkey_bind binding =
         has_saved_binding ? _saved_bindings[set_name].at(default_hotkey.name)
                           : default_hotkey.binding;

      const bool binding_free_for_use =
         not has_saved_used_set or
         not _saved_used_bindings.at(set_name).contains(binding);

      if (has_saved_binding or binding_free_for_use) {
         bindings[binding] =
            hotkey{.command = std::string{default_hotkey.command},
                   .toggle = default_hotkey.bind_config.toggle,
                   .ignore_imgui_focus = default_hotkey.bind_config.ignore_imgui_focus,
                   .name = std::string{default_hotkey.name}};
         query_bindings[default_hotkey.name] = binding;
      }
      else {
         unbound_hotkeys.push_back(
            hotkey{.command = std::string{default_hotkey.command},
                   .toggle = default_hotkey.bind_config.toggle,
                   .ignore_imgui_focus = default_hotkey.bind_config.ignore_imgui_focus,
                   .name = std::string{default_hotkey.name}});
      }
   }

   _hotkey_sets.emplace_back(set_name, std::move(activated), std::move(bindings),
                             std::move(query_bindings), std::move(unbound_hotkeys));
}

void hotkeys::notify_key_down(const key key) noexcept
{
   _key_events.push_back({.key = key, .new_state = key_state::down});
}

void hotkeys::notify_key_up(const key key) noexcept
{
   _key_events.push_back({.key = key, .new_state = key_state::up});
}

void hotkeys::release_toggles() noexcept
{
   for (auto& active : _active_toggles) {
      hotkey& hotkey = _hotkey_sets[active.set_index].bindings[active.bind];

      if (std::exchange(hotkey.toggle_active, false)) {
         _commands.execute(hotkey.command);
      }
   }

   _active_toggles.clear();
}

void hotkeys::update(const bool imgui_has_mouse, const bool imgui_has_keyboard) noexcept
{
   release_stale_toggles(imgui_has_mouse, imgui_has_keyboard);

   for (const auto& event : _key_events) {
      process_new_key_state(event.key, event.new_state, imgui_has_mouse,
                            imgui_has_keyboard);
   }

   if (_user_inputting_new_binding and not _key_events.empty()) {
      _user_editing_last_key_event = _key_events.back();
   }

   _key_events.clear();
}

auto hotkeys::query_binding(std::string_view set_name, std::string_view command) const noexcept
   -> std::optional<hotkey_bind>
{
   for (const auto& set : _hotkey_sets) {
      if (set.name != set_name) continue;

      if (auto it = set.query_bindings.find(command); it != set.query_bindings.end()) {
         const auto& [hotkey_command, hotkey_bind] = *it;

         if (hotkey_command == command) return hotkey_bind;
      }
   }

   return std::nullopt;
}

void hotkeys::process_new_key_state(const key key, const key_state new_state,
                                    const bool imgui_has_mouse,
                                    const bool imgui_has_keyboard)
{
   const key_state old_state = std::exchange(_keys[key], new_state);

   if (old_state == new_state) return;

   if (key == key::ctrl or key == key::shift or key == key::alt) {
      release_modified_toggles(key == key::ctrl, key == key::shift, key == key::alt);
   }

   for (std::ptrdiff_t i = std::ssize(_hotkey_sets) - 1; i >= 0; --i) {
      hotkey_set& set = _hotkey_sets[i];

      if (not set.activated_predicate()) continue;

      const auto handle_hotkey = [&](const hotkey_bind bind, hotkey& hotkey) {
         if (imgui_has_mouse and is_mouse_key(bind.key) and
             not hotkey.ignore_imgui_focus) {
            return;
         }

         if (imgui_has_keyboard and is_keyboard_key(bind.key) and
             not hotkey.ignore_imgui_focus) {
            return;
         }

         if (hotkey.toggle and (new_state == key_state::down or hotkey.toggle_active)) {
            hotkey.toggle_active = not hotkey.toggle_active;

            try_execute_command(hotkey.command);

            if (hotkey.toggle_active) {
               _active_toggles.emplace(i, bind);
            }
            else {
               _active_toggles.erase({i, bind});
            }
         }
         else if (new_state == key_state::down) {
            // If this was true then we know this is a transition from up to down
            // as we've already ruled out a transition to the same state above.

            try_execute_command(hotkey.command);
         }
      };

      absl::flat_hash_map<hotkey_bind, hotkey>& bindings = set.bindings;

      const bool ctrl_down = is_key_down(key::ctrl);
      const bool shift_down = is_key_down(key::shift);
      const bool alt_down = is_key_down(key::alt);

      if (ctrl_down and shift_down and alt_down) {
         auto bind_hotkey = bindings.find(
            {.key = key, .modifiers = {.ctrl = true, .shift = true, .alt = true}});

         if (bind_hotkey == bindings.end()) continue;

         handle_hotkey(bind_hotkey->first, bind_hotkey->second);
         break;
      }
      else if (ctrl_down and shift_down) {
         auto bind_hotkey =
            bindings.find({.key = key, .modifiers = {.ctrl = true, .shift = true}});

         if (bind_hotkey == bindings.end()) continue;

         handle_hotkey(bind_hotkey->first, bind_hotkey->second);
         break;
      }
      else if (ctrl_down and alt_down) {
         auto bind_hotkey =
            bindings.find({.key = key, .modifiers = {.ctrl = true, .alt = true}});

         if (bind_hotkey == bindings.end()) continue;

         handle_hotkey(bind_hotkey->first, bind_hotkey->second);
         break;
      }
      else if (shift_down and alt_down) {
         auto bind_hotkey =
            bindings.find({.key = key, .modifiers = {.shift = true, .alt = true}});

         if (bind_hotkey == bindings.end()) continue;

         handle_hotkey(bind_hotkey->first, bind_hotkey->second);
         break;
      }
      else if (ctrl_down) {
         auto bind_hotkey = bindings.find({.key = key, .modifiers = {.ctrl = true}});

         if (bind_hotkey == bindings.end()) continue;

         handle_hotkey(bind_hotkey->first, bind_hotkey->second);
         break;
      }
      else if (shift_down) {
         auto bind_hotkey =
            bindings.find({.key = key, .modifiers = {.shift = true}});

         if (bind_hotkey == bindings.end()) continue;

         handle_hotkey(bind_hotkey->first, bind_hotkey->second);
         break;
      }
      else if (alt_down) {
         auto bind_hotkey = bindings.find({.key = key, .modifiers = {.alt = true}});

         if (bind_hotkey == bindings.end()) continue;

         handle_hotkey(bind_hotkey->first, bind_hotkey->second);
         break;
      }
      else {
         auto bind_hotkey = bindings.find({.key = key});

         if (bind_hotkey == bindings.end()) continue;

         handle_hotkey(bind_hotkey->first, bind_hotkey->second);
         break;
      }
   }
}

void hotkeys::validate_command(const std::string_view command)
{
   auto [command_name, command_args] = string::split_first_of_exclusive(command, " ");

   if (not _commands.has_command(command_name)) {
      throw unknown_command{fmt::format("Unknown command '{}'!", command_name)};
   }
}

void hotkeys::release_stale_toggles(const bool imgui_has_mouse,
                                    const bool imgui_has_keyboard) noexcept
{
   for (auto it = _active_toggles.begin(); it != _active_toggles.end();) {
      auto active = it++;

      hotkey_set& set = _hotkey_sets[active->set_index];
      hotkey& hotkey = set.bindings[active->bind];

      bool release_toggle = false;

      if (imgui_has_mouse and not hotkey.ignore_imgui_focus and
          is_mouse_key(active->bind.key)) {
         release_toggle = true;
      }
      else if (imgui_has_keyboard and not hotkey.ignore_imgui_focus and
               is_keyboard_key(active->bind.key)) {
         release_toggle = true;
      }
      else if (not set.activated_predicate()) {
         release_toggle = true;
      }

      if (not release_toggle) continue;

      if (std::exchange(hotkey.toggle_active, false)) {
         _commands.execute(hotkey.command);
         _active_toggles.erase(active);
      }
   }
}

void hotkeys::release_modified_toggles(const bool ctrl, const bool shift,
                                       const bool alt) noexcept
{
   for (auto it = _active_toggles.begin(); it != _active_toggles.end();) {
      auto active_toggle = it++;

      const bool release = active_toggle->bind.modifiers.ctrl == ctrl or
                           active_toggle->bind.modifiers.shift == shift or
                           active_toggle->bind.modifiers.alt == alt;

      if (not release) continue;

      hotkey& hotkey =
         _hotkey_sets[active_toggle->set_index].bindings[active_toggle->bind];

      if (std::exchange(hotkey.toggle_active, false)) {
         _commands.execute(hotkey.command);
      }

      _active_toggles.erase(active_toggle);
   }
}

bool hotkeys::is_key_down(const key key) const noexcept
{
   return _keys[key] == key_state::down;
}

void hotkeys::try_execute_command(const std::string_view command) const noexcept
{
   try {
      _commands.execute(command);
   }
   catch (invalid_command_argument& e) {
      _error_output_stream
         .write("Failed to execute command '{}'\n   Error: {}\n", command, e.what());
   }
}

void hotkeys::show_imgui(bool& window_open, const float display_scale) noexcept
{
   ImGui::SetNextWindowSize({960.0f * display_scale, 540.0f * display_scale},
                            ImGuiCond_Once);

   if (ImGui::Begin("Hotkeys Editor", &window_open)) {
      for (int set_index = 0; set_index < std::ssize(_hotkey_sets); ++set_index) {
         hotkey_set& set = _hotkey_sets[set_index];

         ImGui::PushID(set_index);

         if (not ImGui::CollapsingHeader(set.name.c_str())) {
            ImGui::PopID();
            continue;
         }

         ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});

         fill_sorted_info(set, _sorted_hotkey_set);

         for (int bind_index = 0; bind_index < std::ssize(_sorted_hotkey_set);
              ++bind_index) {
            ImGui::PushID(bind_index);

            std::visit(overload{[&](const std::pair<const hotkey_bind, hotkey>* hotkey_bind_hotkey) {
                                   const auto& [hotkey_bind, hotkey] =
                                      *hotkey_bind_hotkey;

                                   ImGui::LabelText("", hotkey.name.c_str());
                                   ImGui::SameLine();

                                   if (ImGui::Selectable(get_display_string(hotkey_bind))) {
                                      _user_inputting_new_binding = true;
                                      _user_editing_bind_set = set_index;
                                      _user_editing_bind = hotkey_bind;
                                      _user_editing_hotkey = hotkey;
                                   }
                                },
                                [&](const hotkey* hotkey) {
                                   ImGui::LabelText("", hotkey->name.c_str());
                                   ImGui::SameLine();

                                   if (ImGui::Selectable("<unbound>")) {
                                      _user_inputting_new_binding = true;
                                      _user_editing_bind_set = set_index;
                                      _user_editing_bind = std::nullopt;
                                      _user_editing_hotkey = *hotkey;
                                   }
                                }},
                       _sorted_hotkey_set[bind_index]);

            ImGui::PopID();
         }

         ImGui::PopStyleVar();
         ImGui::PopID();
      }
   }

   ImGui::End();

   if (_user_inputting_new_binding) {
      ImGui::OpenPopup("Press New Binding##Hotkeys");
   }

   if (ImGui::BeginPopupModal("Press New Binding##Hotkeys", &_user_inputting_new_binding,
                              ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("Press Escape to cancel and go back.");

      const std::optional<key_event> last_key_event =
         std::exchange(_user_editing_last_key_event, std::nullopt);

      if (last_key_event) {
         if (last_key_event->key == key::escape and
             last_key_event->new_state == key_state::up) {
            _user_inputting_new_binding = false;
         }
         else if (last_key_event->new_state == key_state::up and
                  last_key_event->key != key::ctrl and
                  last_key_event->key != key::shift and
                  last_key_event->key != key::alt) {
            hotkey_set& set = _hotkey_sets[_user_editing_bind_set];

            if (_user_editing_bind) {
               set.bindings.erase(*_user_editing_bind);
            }

            const hotkey_bind new_bind{.key = last_key_event->key,
                                       .modifiers = {.ctrl = is_key_down(key::ctrl),
                                                     .shift = is_key_down(key::shift),
                                                     .alt = is_key_down(key::alt)}};

            if (set.bindings.contains(new_bind)) {
               set.unbound_hotkeys.push_back(set.bindings.at(new_bind));
               set.query_bindings.erase(set.unbound_hotkeys.back().name);

               _saved_bindings[set.name].erase(set.unbound_hotkeys.back().name);
            }

            set.bindings[new_bind] = _user_editing_hotkey;
            set.query_bindings[_user_editing_hotkey.name] = new_bind;

            std::erase_if(set.unbound_hotkeys, [&](const hotkey& hotkey) {
               return hotkey == _user_editing_hotkey;
            });

            _user_inputting_new_binding = false;

            _saved_bindings[set.name][_user_editing_hotkey.name] = new_bind;

            save_bindings(save_path, _saved_bindings);
         }
      }

      ImGui::EndPopup();
   }

   if (not window_open) {
      _user_inputting_new_binding = false;
   }
}

void hotkeys::fill_sorted_info(const hotkey_set& set,
                               std::vector<sorted_hotkey_variant>& sorted_hotkey_set)
{
   sorted_hotkey_set.clear();
   sorted_hotkey_set.reserve(set.bindings.size());

   for (const auto& bind_hotkey : set.bindings) {
      sorted_hotkey_set.emplace_back(&bind_hotkey);
   }

   for (const auto& unbound : set.unbound_hotkeys) {
      sorted_hotkey_set.emplace_back(&unbound);
   }

   std::sort(sorted_hotkey_set.begin(), sorted_hotkey_set.end(),
             [](const sorted_hotkey_variant& left, const sorted_hotkey_variant& right) {
                const std::string& left_command =
                   std::holds_alternative<const std::pair<const hotkey_bind, hotkey>*>(left)
                      ? std::get<const std::pair<const hotkey_bind, hotkey>*>(left)
                           ->second.command
                      : std::get<const hotkey*>(left)->command;
                const std::string& right_command =
                   std::holds_alternative<const std::pair<const hotkey_bind, hotkey>*>(right)
                      ? std::get<const std::pair<const hotkey_bind, hotkey>*>(right)
                           ->second.command
                      : std::get<const hotkey*>(right)->command;

                return left_command < right_command;
             });
}

auto get_display_string(const std::optional<hotkey_bind> binding) -> const char*
{
   if (not binding) return "<UNBOUND>";

   return get_display_string(binding->key, binding->modifiers.ctrl,
                             binding->modifiers.shift, binding->modifiers.alt);
}
}