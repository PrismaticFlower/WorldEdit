
#include "hotkeys.hpp"
#include "utility/string_ops.hpp"

namespace we {

namespace {

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
   : _commands{commands}, _error_output_stream{error_output_stream}
{
   _hotkey_sets.reserve(16);
   _key_events.reserve(256);
}

void hotkeys::add_set(std::string set_name, std::function<bool()> activated,
                      std::initializer_list<hotkey_default> default_hotkeys)
{
   absl::flat_hash_map<hotkey_bind, hotkey> bindings;
   absl::flat_hash_map<std::string, hotkey_bind> query_bindings;

   bindings.reserve(default_hotkeys.size());
   query_bindings.reserve(default_hotkeys.size());

   for (auto& default_hotkey : default_hotkeys) {
      validate_command(default_hotkey.command);

      bindings.emplace(default_hotkey.binding,
                       hotkey{.command = std::string{default_hotkey.command},
                              .toggle = default_hotkey.bind_config.toggle,
                              .ignore_imgui_focus =
                                 default_hotkey.bind_config.ignore_imgui_focus});
      query_bindings.emplace(default_hotkey.command, default_hotkey.binding);
   }

   _hotkey_sets.emplace_back(set_name, std::move(activated),
                             std::move(bindings), std::move(query_bindings));
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
   if (key == key::ctrl) return;
   if (key == key::shift) return;

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
               _active_toggles.emplace(static_cast<std::size_t>(i), bind);
            }
            else {
               _active_toggles.erase({static_cast<std::size_t>(i), bind});
            }
         }
         else if (new_state == key_state::down) {
            // If this was true then we know this is a transition from up to down
            // as we've already ruled out a transition to the same state above.

            try_execute_command(hotkey.command);
         }
      };

      absl::flat_hash_map<hotkey_bind, hotkey>& bindings = set.bindings;

      if (auto bind_hotkey =
             bindings.find({.key = key, .modifiers = {.ctrl = true, .shift = true}});
          bind_hotkey != bindings.end() and is_key_down(key::ctrl) and
          is_key_down(key::shift)) {
         handle_hotkey(bind_hotkey->first, bind_hotkey->second);
         break;
      }
      else if (bind_hotkey = bindings.find({.key = key, .modifiers = {.ctrl = true}});
               bind_hotkey != bindings.end() and is_key_down(key::ctrl)) {
         handle_hotkey(bind_hotkey->first, bind_hotkey->second);
         break;
      }
      else if (bind_hotkey = bindings.find({.key = key, .modifiers = {.shift = true}});
               bind_hotkey != bindings.end() and is_key_down(key::shift)) {
         handle_hotkey(bind_hotkey->first, bind_hotkey->second);
         break;
      }
      else if (bind_hotkey = bindings.find({.key = key});
               bind_hotkey != bindings.end()) {
         handle_hotkey(bind_hotkey->first, bind_hotkey->second);
         break;
      }
   }
}

void hotkeys::validate_command(const std::string_view command)
{
   auto [command_name, command_args] =
      utility::string::split_first_of_exclusive(command, " ");

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

}