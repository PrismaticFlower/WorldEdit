#pragma once

#include "types.hpp"

#include <string_view>

namespace we {

enum class key : uint8 {
   tab,
   left_arrow,
   right_arrow,
   up_arrow,
   down_arrow,
   page_up,
   page_down,
   home,
   end,
   insert,
   del,
   backspace,
   space,
   enter,
   escape,
   ctrl,
   shift,
   alt,
   menu,
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
   b,
   c,
   d,
   e,
   f,
   g,
   h,
   i,
   j,
   k,
   l,
   m,
   n,
   o,
   p,
   q,
   r,
   s,
   t,
   u,
   v,
   w,
   x,
   y,
   z,
   f1,
   f2,
   f3,
   f4,
   f5,
   f6,
   f7,
   f8,
   f9,
   f10,
   f11,
   f12,
   apostrophe,
   comma,
   minus,
   period,
   slash,
   semicolon,
   equal,
   left_bracket,
   right_bracket,
   backslash,
   grave_accent,
   caps_lock,
   scroll_lock,
   num_lock,
   print_screen,
   pause,
   numpad_0,
   numpad_1,
   numpad_2,
   numpad_3,
   numpad_4,
   numpad_5,
   numpad_6,
   numpad_7,
   numpad_8,
   numpad_9,
   numpad_decimal,
   numpad_divide,
   numpad_multiply,
   numpad_subtract,
   numpad_add,

   mouse1,
   mouse2,
   mouse3,
   mouse4,
   mouse5,

   void_key,

   // size marker
   count
};

/// @brief Translate a windows virtual key to a world edit key.
/// @param vk The virtual key
/// @return The translated key or void_key if the key is not recognized.
auto translate_virtual_key(const std::uintptr_t vk) noexcept -> key;

/// @brief Gets a display string for a key. With optional modifiers.
/// @param key The key.
/// @param ctrl Include Ctrl + in the string.
/// @param shift Include Shift + in the string.
/// @return The C string for passing to ImGui for display.
auto get_display_string(const key key, const bool ctrl, const bool shift,
                        const bool alt) -> const char*;

/// @brief Parse a display string for a key. With optional modifiers.
/// @param out_key The output key.
/// @param out_ctrl Output of if Ctrl + was in the string.
/// @param out_shift Output of if Shift + was in the string.
/// @return True if the string was parsed, false if it wasn't.
bool parse_display_string(std::string_view string, key& out_key, bool& out_ctrl,
                          bool& out_shift, bool& out_alt);

}