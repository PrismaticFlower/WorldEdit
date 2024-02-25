#pragma once

#include "commands.hpp"
#include "key.hpp"
#include "output_stream.hpp"
#include "scale_factor.hpp"
#include "utility/implementation_storage.hpp"

#include <initializer_list>
#include <optional>
#include <string_view>

struct ImFont;

namespace we {

struct hotkey_modifiers {
   bool ctrl = false;
   bool shift = false;
   bool alt = false;

   constexpr bool operator==(const hotkey_modifiers&) const noexcept = default;
};

struct hotkey_bind {
   key key;
   hotkey_modifiers modifiers;

   template<typename H>
   friend H AbslHashValue(H h, const hotkey_bind& bind_key)
   {
      return H::combine(std::move(h), bind_key.key, bind_key.modifiers.ctrl,
                        bind_key.modifiers.shift, bind_key.modifiers.alt);
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

struct hotkey_set_desc {
   std::string name;
   std::string description;
   std::move_only_function<bool()> activated;
   std::initializer_list<hotkey_default> default_hotkeys;
   const bool hidden = false;
};

struct hotkeys {
   hotkeys(commands& commands, output_stream& error_output_stream) noexcept;

   hotkeys(const hotkeys&) = delete;
   hotkeys(hotkeys&&) = delete;

   ~hotkeys();

   /// @brief Adds a set of hotkeys.
   /// @param set_name The name of the set.
   /// @param activated Predicate to call to test if the set is active or not.
   /// @param bindings_set
   void add_set(hotkey_set_desc desc);

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

   /// @brief Call and clear the state for all toggle bindings. Reset all keys to up state.
   void clear_state() noexcept;

   /// @brief Query for the key bound to a hotkey in a set.
   /// @param set_name The name of the set.
   /// @param command The name of the hotkey.
   /// @return The key(s) bound to the hotkey or nullopt if the command is unbound in the set.
   auto query_binding(std::string_view set_name, std::string_view name) const noexcept

      -> std::optional<hotkey_bind>;

   /// @brief Show an ImGui window for editing bindings.
   /// @param window_open If the ImGui window is still open or not.
   void show_imgui(bool& window_open, const scale_factor display_scale) noexcept;

private:
   struct impl;

   implementation_storage<impl, 456> _impl;
};

auto get_display_string(const std::optional<hotkey_bind> binding) -> const char*;

}