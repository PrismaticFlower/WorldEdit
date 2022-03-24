#pragma once

#include "types.hpp"

#include "container/enum_array.hpp"

#include <functional>
#include <optional>
#include <vector>

namespace we {

enum class key : uint8 {
   _0,
   _1,
   _2,
   _3,
   _4,
   _5,
   _6,
   _7,
   _8,
   _9,
   a,
   accept,
   add,
   b,
   back,
   browser_back,
   browser_favorites,
   browser_forward,
   browser_home,
   browser_refresh,
   browser_search,
   browser_stop,
   c,
   capital,
   clear,
   control,
   convert,
   d,
   decimal,
   delete_,
   divide,
   down,
   e,
   end,
   escape,
   execute,
   f,
   f1,
   f10,
   f11,
   f12,
   f13,
   f14,
   f15,
   f16,
   f17,
   f18,
   f19,
   f2,
   f20,
   f21,
   f22,
   f23,
   f24,
   f3,
   f4,
   f5,
   f6,
   f7,
   f8,
   f9,
   g,
   h,
   help,
   home,
   i,
   insert,
   j,
   k,
   l,
   left,
   m,
   menu,
   modechange,
   multiply,
   n,
   next,
   nonconvert,
   numlock,
   numpad0,
   numpad1,
   numpad2,
   numpad3,
   numpad4,
   numpad5,
   numpad6,
   numpad7,
   numpad8,
   numpad9,
   o,
   oem_1,
   oem_102,
   oem_2,
   oem_3,
   oem_4,
   oem_5,
   oem_6,
   oem_7,
   oem_8,
   oem_ax,
   oem_comma,
   oem_fj_jisho,
   oem_fj_loya,
   oem_fj_masshou,
   oem_fj_roya,
   oem_fj_touroku,
   oem_minus,
   oem_period,
   oem_plus,
   p,
   pause,
   print,
   prior,
   q,
   r,
   return_,
   right,
   s,
   scroll,
   select,
   separator,
   shift,
   sleep,
   snapshot,
   space,
   subtract,
   t,
   tab,
   u,
   up,
   v,
   w,
   x,
   y,
   z,

   mouse1,
   mouse2,
   mouse3,
   mouse4,
   mouse5,

   void_key,

   // size marker
   count
};

struct keyboard_modifiers {
   bool ctrl = false;
   bool shift = false;
};

struct bind_key {
   key key;
   keyboard_modifiers modifiers;
};

struct bind_config {
   /// @brief If true the callback is invoked once when the key combination is
   /// pressed down and once when it is released.
   bool toggle = false;
};

/// @brief Simple key input manager. Takes key events and invokes callbacks bound to those keys.
class key_input_manager {
public:
   /// @brief Notify the manager of a key being pressed.
   /// @param key The key.
   void notify_key_down(const key key) noexcept;

   /// @brief Notify the manager of a key being released.
   /// @param key The key.
   void notify_key_up(const key key) noexcept;

   /// @brief Bind a callback to a key combination.
   /// @param binding The key combination.
   /// @param bind_config The config for the binding.
   /// @param callback The callback.
   void bind(const bind_key binding, const bind_config bind_config,
             std::function<void()> callback) noexcept;

   /// @brief Unbind a previously bound key, releasing the associated callback.
   void unbind(const bind_key binding) noexcept;

   /// @brief Process the event queue for the manager and invoke callbacks for bindings.
   /// @param ignore_unmodified_bindings Ignore bindings that do not use Ctrl or Shift modifiers.
   void update(const bool ignore_unmodified_bindings) noexcept;

   /// @brief Call and clear the state for all toggle bindings.
   void release_toggles() noexcept;

   /// @brief Call and clear the state for any toggle bindings that do not use Ctrl or Shift modifiers.
   void release_unmodified_toggles() noexcept;

private:
   enum class key_state : bool { up, down };

   struct key_event {
      key key;
      key_state new_state;
   };

   struct key_binding {
      bool toggle = false;
      bool toggle_active = false;
      std::function<void()> callback;
   };

   struct key_bindings {
      key_state state = key_state::up;

      std::optional<key_binding> binding;
      std::optional<key_binding> ctrl_binding;
      std::optional<key_binding> ctrl_shift_binding;
      std::optional<key_binding> shift_binding;
   };

   void process_new_key_state(const key key, const key_state new_state,
                              const bool ignore_unmodified_bindings) noexcept;

   bool is_key_down(const key key) const noexcept;

   [[nodiscard]] auto select_key_binding(const bind_key binding) noexcept
      -> std::optional<key_binding>&;

   std::vector<key_event> _key_events;
   container::enum_array<key_bindings, key> _keys{};
};

/// @brief Translate a windows virtual key to a world edit key.
/// @param vk The virtual key
/// @return The translated key or void_key if the key is not recognized.
auto translate_virtual_key(const std::uintptr_t vk) noexcept -> key;

}
