#pragma once

#include "commands.hpp"
#include "container/enum_array.hpp"
#include "key.hpp"
#include "output_stream.hpp"

#include <initializer_list>
#include <optional>
#include <string_view>
#include <variant>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

namespace we {

struct hotkey_modifiers {
   bool ctrl = false;
   bool shift = false;

   constexpr bool operator==(const hotkey_modifiers&) const noexcept = default;
};

struct hotkey_bind {
   key key;
   hotkey_modifiers modifiers;

   template<typename H>
   friend H AbslHashValue(H h, const hotkey_bind& bind_key)
   {
      return H::combine(std::move(h), bind_key.key, bind_key.modifiers.ctrl,
                        bind_key.modifiers.shift);
   }

   constexpr bool operator==(const hotkey_bind&) const noexcept = default;
};

struct hotkey_bind_config {
   /// @brief If true the callback is invoked once when the key combination is
   /// pressed down and once when it is released.
   bool toggle = false;

   /// @brief The hotkey will work even when ImGui has focus.
   bool ignore_imgui_focus = false;
};

struct hotkey_default {
   std::string_view name;
   std::string_view command;
   hotkey_bind binding;
   hotkey_bind_config bind_config;
};

struct hotkeys {
   hotkeys(commands& commands, output_stream& error_output_stream) noexcept;

   /// @brief Adds a set of hotkeys.
   /// @param set_name The name of the set.
   /// @param activated Predicate to call to test if the set is active or not.
   /// @param bindings_set
   void add_set(std::string set_name, std::function<bool()> activated,
                std::initializer_list<hotkey_default> default_hotkeys);

   /// @brief Notify the manager of a key being pressed.
   /// @param key The key.
   void notify_key_down(const key key) noexcept;

   /// @brief Notify the manager of a key being released.
   /// @param key The key.
   void notify_key_up(const key key) noexcept;

   /// @brief Process the event queue for the manager and invoke callbacks for bindings.
   /// @param imgui_has_mouse ImGui::GetIO().WantCaptureMouse passed through.
   /// @param imgui_has_keyboard ImGui::GetIO().WantCaptureKeyboard passed through.
   void update(const bool imgui_has_mouse, const bool imgui_has_keyboard) noexcept;

   /// @brief Call and clear the state for all toggle bindings.
   void release_toggles() noexcept;

   /// @brief Query for the key bound to a hotkey in a set.
   /// @param set_name The name of the set.
   /// @param command The name of the hotkey.
   /// @return The key(s) bound to the hotkey or nullopt if the command is unbound in the set.
   auto query_binding(std::string_view set_name, std::string_view name) const noexcept

      -> std::optional<hotkey_bind>;

   /// @brief Show an ImGui window for editing bindings.
   /// @param window_open If the ImGui window is still open or not.
   void show_imgui(bool& window_open, const float display_scale) noexcept;

private:
   enum class key_state : bool { up, down };

   void validate_command(const std::string_view command);

   void release_stale_toggles(const bool imgui_has_mouse,
                              const bool imgui_has_keyboard) noexcept;

   void release_modified_toggles(const bool ctrl, const bool shift) noexcept;

   void process_new_key_state(const key key, const key_state new_state,
                              const bool imgui_has_mouse,
                              const bool imgui_has_keyboard);

   bool is_key_down(const key key) const noexcept;

   void try_execute_command(const std::string_view command) const noexcept;

   commands& _commands;
   output_stream& _error_output_stream;

   struct key_event {
      key key;
      key_state new_state;
   };

   std::vector<key_event> _key_events;

   struct hotkey {
      std::string command;

      bool toggle = false;
      bool toggle_active = false;
      bool ignore_imgui_focus = false;

      std::string name;

      bool operator==(const hotkey&) const noexcept = default;
   };

   struct hotkey_set {
      std::string name;
      std::function<bool()> activated_predicate;
      absl::flat_hash_map<hotkey_bind, hotkey> bindings;
      absl::flat_hash_map<std::string, hotkey_bind> query_bindings;
      std::vector<hotkey> unbound_hotkeys;
   };

   std::vector<hotkey_set> _hotkey_sets;

   struct active_toggle {
      std::ptrdiff_t set_index;
      hotkey_bind bind;

      template<typename H>
      friend H AbslHashValue(H h, const active_toggle& active_toggle)
      {
         return H::combine(std::move(h), active_toggle.set_index, active_toggle.bind);
      }

      constexpr bool operator==(const active_toggle&) const noexcept = default;
   };

   using activated_hotkey = active_toggle;

   absl::flat_hash_set<active_toggle> _active_toggles;

   bool _user_inputting_new_binding = false;
   std::optional<key_event> _user_editing_last_key_event;
   std::ptrdiff_t _user_editing_bind_set = 0;
   std::optional<hotkey_bind> _user_editing_bind;
   hotkey _user_editing_hotkey;

   container::enum_array<key_state, key> _keys{};

   using sorted_hotkey_variant =
      std::variant<const std::pair<const hotkey_bind, hotkey>*, const hotkey*>;

   std::vector<sorted_hotkey_variant> _sorted_hotkey_set;

   absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, hotkey_bind>> _saved_bindings;

   static void fill_sorted_info(const hotkey_set& set,
                                std::vector<sorted_hotkey_variant>& sorted_hotkey_set);
};

auto get_display_string(const std::optional<hotkey_bind> binding) -> const char*;

}