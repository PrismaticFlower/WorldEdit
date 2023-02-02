#pragma once

#include "commands.hpp"
#include "container/enum_array.hpp"
#include "key.hpp"
#include "output_stream.hpp"

#include <initializer_list>
#include <string_view>

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

private:
   enum class key_state : bool { up, down };

   void validate_command(const std::string_view command);

   void release_stale_toggles(const bool imgui_has_mouse,
                              const bool imgui_has_keyboard) noexcept;

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
   };

   struct hotkey_set {
      std::string name;
      std::function<bool()> activated_predicate;
      absl::flat_hash_map<hotkey_bind, hotkey> bindings;
   };

   std::vector<hotkey_set> _hotkey_sets;

   struct active_toggle {
      std::size_t set_index;
      hotkey_bind bind;

      template<typename H>
      friend H AbslHashValue(H h, const active_toggle& active_toggle)
      {
         return H::combine(std::move(h), active_toggle.set_index, active_toggle.bind);
      }

      constexpr bool operator==(const active_toggle&) const noexcept = default;
   };

   absl::flat_hash_set<active_toggle> _active_toggles;

   key_state _shift_state = key_state::up;
   key_state _ctrl_state = key_state::up;

   container::enum_array<key_state, key> _keys{};
};

}