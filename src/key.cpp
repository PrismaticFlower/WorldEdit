#include "key.hpp"
#include "container/enum_array.hpp"
#include "lowercase_string.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include <Windows.h>

namespace we {

auto translate_virtual_key(const std::uintptr_t vk) noexcept -> key
{
   switch (vk) { // clang-format off
   case VK_TAB: return key::tab;
   case VK_LEFT: return key::left_arrow;
   case VK_RIGHT: return key::right_arrow;
   case VK_UP: return key::up_arrow;
   case VK_DOWN: return key::down_arrow;
   case VK_PRIOR: return key::page_up;
   case VK_NEXT: return key::page_down;
   case VK_HOME: return key::home;
   case VK_END: return key::end;
   case VK_INSERT: return key::insert;
   case VK_DELETE: return key::del;
   case VK_BACK: return key::backspace;
   case VK_SPACE: return key::space;
   case VK_RETURN: return key::enter;
   case VK_ESCAPE: return key::escape;
   case VK_CONTROL: return key::ctrl;
   case VK_SHIFT: return key::shift;
   case VK_APPS: return key::menu;
   case '0': return key::_0;
   case '1': return key::_1;
   case '2': return key::_2;
   case '3': return key::_3;
   case '4': return key::_4;
   case '5': return key::_5;
   case '6': return key::_6;
   case '7': return key::_7;
   case '8': return key::_8;
   case '9': return key::_9;
   case 'A': return key::a;
   case 'B': return key::b;
   case 'C': return key::c;
   case 'D': return key::d;
   case 'E': return key::e;
   case 'F': return key::f;
   case 'G': return key::g;
   case 'H': return key::h;
   case 'I': return key::i;
   case 'J': return key::j;
   case 'K': return key::k;
   case 'L': return key::l;
   case 'M': return key::m;
   case 'N': return key::n;
   case 'O': return key::o;
   case 'P': return key::p;
   case 'Q': return key::q;
   case 'R': return key::r;
   case 'S': return key::s;
   case 'T': return key::t;
   case 'U': return key::u;
   case 'V': return key::v;
   case 'W': return key::w;
   case 'X': return key::x;
   case 'Y': return key::y;
   case 'Z': return key::z;
   case VK_F1: return key::f1;
   case VK_F2: return key::f2;
   case VK_F3: return key::f3;
   case VK_F4: return key::f4;
   case VK_F5: return key::f5;
   case VK_F6: return key::f6;
   case VK_F7: return key::f7;
   case VK_F8: return key::f8;
   case VK_F9: return key::f9;
   case VK_F10: return key::f10;
   case VK_F11: return key::f11;
   case VK_F12: return key::f12;
   case VK_OEM_7: return key::apostrophe;
   case VK_OEM_COMMA: return key::comma;
   case VK_OEM_MINUS: return key::minus;
   case VK_OEM_PERIOD: return key::period;
   case VK_OEM_2: return key::slash;
   case VK_OEM_1: return key::semicolon;
   case VK_OEM_PLUS: return key::equal;
   case VK_OEM_4: return key::left_bracket;
   case VK_OEM_6: return key::right_bracket;
   case VK_OEM_5: return key::backslash;
   case VK_OEM_3: return key::grave_accent;
   case VK_CAPITAL: return key::caps_lock;
   case VK_SCROLL: return key::scroll_lock;
   case VK_NUMLOCK: return key::num_lock;
   case VK_SNAPSHOT: return key::print_screen;
   case VK_PAUSE: return key::pause;
   case VK_NUMPAD0: return key::numpad_0;
   case VK_NUMPAD1: return key::numpad_1;
   case VK_NUMPAD2: return key::numpad_2;
   case VK_NUMPAD3: return key::numpad_3;
   case VK_NUMPAD4: return key::numpad_4;
   case VK_NUMPAD5: return key::numpad_5;
   case VK_NUMPAD6: return key::numpad_6;
   case VK_NUMPAD7: return key::numpad_7;
   case VK_NUMPAD8: return key::numpad_8;
   case VK_NUMPAD9: return key::numpad_9;
   case VK_DECIMAL: return key::numpad_decimal;
   case VK_DIVIDE: return key::numpad_divide;
   case VK_MULTIPLY: return key::numpad_multiply;
   case VK_SUBTRACT: return key::numpad_subtract;
   case VK_ADD: return key::numpad_add;
   case VK_LBUTTON: return key::mouse1;
   case VK_RBUTTON: return key::mouse2;
   case VK_MBUTTON: return key::mouse3;
   case VK_XBUTTON1: return key::mouse4;
   case VK_XBUTTON2: return key::mouse5;
   default:
      return key::void_key;
   }; // clang-format on
}

namespace {

constexpr container::enum_array<const char*, key> key_display_names =
   container::make_enum_array<const char*, key>({
      {key::tab, "Tab"},
      {key::left_arrow, "Left Arrow"},
      {key::right_arrow, "Right Arrow"},
      {key::up_arrow, "Up Arrow"},
      {key::down_arrow, "Down Arrow"},
      {key::page_up, "Page Up"},
      {key::page_down, "Page Down"},
      {key::home, "Home"},
      {key::end, "End"},
      {key::insert, "Insert"},
      {key::del, "Del"},
      {key::backspace, "Backspace"},
      {key::space, "Space"},
      {key::enter, "Enter"},
      {key::escape, "Escape"},
      {key::ctrl, "Ctrl"},
      {key::shift, "Shift"},
      {key::menu, "Menu"},
      {key::_0, "0"},
      {key::_1, "1"},
      {key::_2, "2"},
      {key::_3, "3"},
      {key::_4, "4"},
      {key::_5, "5"},
      {key::_6, "6"},
      {key::_7, "7"},
      {key::_8, "8"},
      {key::_9, "9"},
      {key::a, "A"},
      {key::b, "B"},
      {key::c, "C"},
      {key::d, "D"},
      {key::e, "E"},
      {key::f, "F"},
      {key::g, "G"},
      {key::h, "H"},
      {key::i, "I"},
      {key::j, "J"},
      {key::k, "K"},
      {key::l, "L"},
      {key::m, "M"},
      {key::n, "N"},
      {key::o, "O"},
      {key::p, "P"},
      {key::q, "Q"},
      {key::r, "R"},
      {key::s, "S"},
      {key::t, "T"},
      {key::u, "U"},
      {key::v, "V"},
      {key::w, "W"},
      {key::x, "X"},
      {key::y, "Y"},
      {key::z, "Z"},
      {key::f1, "F1"},
      {key::f2, "F2"},
      {key::f3, "F3"},
      {key::f4, "F4"},
      {key::f5, "F5"},
      {key::f6, "F6"},
      {key::f7, "F7"},
      {key::f8, "F8"},
      {key::f9, "F9"},
      {key::f10, "F10"},
      {key::f11, "F11"},
      {key::f12, "F12"},
      {key::apostrophe, "'"},
      {key::comma, ","},
      {key::minus, "-"},
      {key::period, "."},
      {key::slash, "/"},
      {key::semicolon, ";"},
      {key::equal, "="},
      {key::left_bracket, "["},
      {key::right_bracket, "]"},
      {key::backslash, "\\"},
      {key::grave_accent, "`"},
      {key::caps_lock, "Caps Lock"},
      {key::scroll_lock, "Scroll Lock"},
      {key::num_lock, "Num Lock"},
      {key::print_screen, "Print Screen"},
      {key::pause, "Pause"},
      {key::numpad_0, "Numpad 0"},
      {key::numpad_1, "Numpad 1"},
      {key::numpad_2, "Numpad 2"},
      {key::numpad_3, "Numpad 3"},
      {key::numpad_4, "Numpad 4"},
      {key::numpad_5, "Numpad 5"},
      {key::numpad_6, "Numpad 6"},
      {key::numpad_7, "Numpad 7"},
      {key::numpad_8, "Numpad 8"},
      {key::numpad_9, "Numpad 9"},
      {key::numpad_decimal, "Numpad Decimal"},
      {key::numpad_divide, "Numpad Divide"},
      {key::numpad_multiply, "Numpad Multiply"},
      {key::numpad_subtract, "Numpad Subtract"},
      {key::numpad_add, "Numpad Add"},

      {key::mouse1, "L Mouse"},
      {key::mouse2, "R Mouse"},
      {key::mouse3, "Middle Mouse"},
      {key::mouse4, "Mouse 4"},
      {key::mouse5, "Mouse 5"},

      {key::void_key, "<empty>"},
   });

constexpr container::enum_array<const char*, key> ctrl_key_display_names =
   container::make_enum_array<const char*, key>({
      {key::tab, "Ctrl + Tab"},
      {key::left_arrow, "Ctrl + Left Arrow"},
      {key::right_arrow, "Ctrl + Right Arrow"},
      {key::up_arrow, "Ctrl + Up Arrow"},
      {key::down_arrow, "Ctrl + Down Arrow"},
      {key::page_up, "Ctrl + Page Up"},
      {key::page_down, "Ctrl + Page Down"},
      {key::home, "Ctrl + Home"},
      {key::end, "Ctrl + End"},
      {key::insert, "Ctrl + Insert"},
      {key::del, "Ctrl + Del"},
      {key::backspace, "Ctrl + Backspace"},
      {key::space, "Ctrl + Space"},
      {key::enter, "Ctrl + Enter"},
      {key::escape, "Ctrl + Escape"},
      {key::ctrl, "Ctrl + Ctrl"},
      {key::shift, "Ctrl + Shift"},
      {key::menu, "Ctrl + Menu"},
      {key::_0, "Ctrl + 0"},
      {key::_1, "Ctrl + 1"},
      {key::_2, "Ctrl + 2"},
      {key::_3, "Ctrl + 3"},
      {key::_4, "Ctrl + 4"},
      {key::_5, "Ctrl + 5"},
      {key::_6, "Ctrl + 6"},
      {key::_7, "Ctrl + 7"},
      {key::_8, "Ctrl + 8"},
      {key::_9, "Ctrl + 9"},
      {key::a, "Ctrl + A"},
      {key::b, "Ctrl + B"},
      {key::c, "Ctrl + C"},
      {key::d, "Ctrl + D"},
      {key::e, "Ctrl + E"},
      {key::f, "Ctrl + F"},
      {key::g, "Ctrl + G"},
      {key::h, "Ctrl + H"},
      {key::i, "Ctrl + I"},
      {key::j, "Ctrl + J"},
      {key::k, "Ctrl + K"},
      {key::l, "Ctrl + L"},
      {key::m, "Ctrl + M"},
      {key::n, "Ctrl + N"},
      {key::o, "Ctrl + O"},
      {key::p, "Ctrl + P"},
      {key::q, "Ctrl + Q"},
      {key::r, "Ctrl + R"},
      {key::s, "Ctrl + S"},
      {key::t, "Ctrl + T"},
      {key::u, "Ctrl + U"},
      {key::v, "Ctrl + V"},
      {key::w, "Ctrl + W"},
      {key::x, "Ctrl + X"},
      {key::y, "Ctrl + Y"},
      {key::z, "Ctrl + Z"},
      {key::f1, "Ctrl + F1"},
      {key::f2, "Ctrl + F2"},
      {key::f3, "Ctrl + F3"},
      {key::f4, "Ctrl + F4"},
      {key::f5, "Ctrl + F5"},
      {key::f6, "Ctrl + F6"},
      {key::f7, "Ctrl + F7"},
      {key::f8, "Ctrl + F8"},
      {key::f9, "Ctrl + F9"},
      {key::f10, "Ctrl + F10"},
      {key::f11, "Ctrl + F11"},
      {key::f12, "Ctrl + F12"},
      {key::apostrophe, "Ctrl + '"},
      {key::comma, "Ctrl + ,"},
      {key::minus, "Ctrl + -"},
      {key::period, "Ctrl + ."},
      {key::slash, "Ctrl + /"},
      {key::semicolon, "Ctrl + ;"},
      {key::equal, "Ctrl + ="},
      {key::left_bracket, "Ctrl + ["},
      {key::right_bracket, "Ctrl + ]"},
      {key::backslash, "Ctrl + \\"},
      {key::grave_accent, "Ctrl + `"},
      {key::caps_lock, "Ctrl + Caps Lock"},
      {key::scroll_lock, "Ctrl + Scroll Lock"},
      {key::num_lock, "Ctrl + Num Lock"},
      {key::print_screen, "Ctrl + Print Screen"},
      {key::pause, "Ctrl + Pause"},
      {key::numpad_0, "Ctrl + Numpad 0"},
      {key::numpad_1, "Ctrl + Numpad 1"},
      {key::numpad_2, "Ctrl + Numpad 2"},
      {key::numpad_3, "Ctrl + Numpad 3"},
      {key::numpad_4, "Ctrl + Numpad 4"},
      {key::numpad_5, "Ctrl + Numpad 5"},
      {key::numpad_6, "Ctrl + Numpad 6"},
      {key::numpad_7, "Ctrl + Numpad 7"},
      {key::numpad_8, "Ctrl + Numpad 8"},
      {key::numpad_9, "Ctrl + Numpad 9"},
      {key::numpad_decimal, "Ctrl + Numpad Decimal"},
      {key::numpad_divide, "Ctrl + Numpad Divide"},
      {key::numpad_multiply, "Ctrl + Numpad Multiply"},
      {key::numpad_subtract, "Ctrl + Numpad Subtract"},
      {key::numpad_add, "Ctrl + Numpad Add"},

      {key::mouse1, "Ctrl + L Mouse"},
      {key::mouse2, "Ctrl + R Mouse"},
      {key::mouse3, "Ctrl + Middle Mouse"},
      {key::mouse4, "Ctrl + Mouse 4"},
      {key::mouse5, "Ctrl + Mouse 5"},

      {key::void_key, "Ctrl + <empty>"},
   });

constexpr container::enum_array<const char*, key> shift_key_display_names =
   container::make_enum_array<const char*, key>({
      {key::tab, "Shift + Tab"},
      {key::left_arrow, "Shift + Left Arrow"},
      {key::right_arrow, "Shift + Right Arrow"},
      {key::up_arrow, "Shift + Up Arrow"},
      {key::down_arrow, "Shift + Down Arrow"},
      {key::page_up, "Shift + Page Up"},
      {key::page_down, "Shift + Page Down"},
      {key::home, "Shift + Home"},
      {key::end, "Shift + End"},
      {key::insert, "Shift + Insert"},
      {key::del, "Shift + Del"},
      {key::backspace, "Shift + Backspace"},
      {key::space, "Shift + Space"},
      {key::enter, "Shift + Enter"},
      {key::escape, "Shift + Escape"},
      {key::ctrl, "Shift + Ctrl"},
      {key::shift, "Shift + Shift"},
      {key::menu, "Shift + Menu"},
      {key::_0, "Shift + 0"},
      {key::_1, "Shift + 1"},
      {key::_2, "Shift + 2"},
      {key::_3, "Shift + 3"},
      {key::_4, "Shift + 4"},
      {key::_5, "Shift + 5"},
      {key::_6, "Shift + 6"},
      {key::_7, "Shift + 7"},
      {key::_8, "Shift + 8"},
      {key::_9, "Shift + 9"},
      {key::a, "Shift + A"},
      {key::b, "Shift + B"},
      {key::c, "Shift + C"},
      {key::d, "Shift + D"},
      {key::e, "Shift + E"},
      {key::f, "Shift + F"},
      {key::g, "Shift + G"},
      {key::h, "Shift + H"},
      {key::i, "Shift + I"},
      {key::j, "Shift + J"},
      {key::k, "Shift + K"},
      {key::l, "Shift + L"},
      {key::m, "Shift + M"},
      {key::n, "Shift + N"},
      {key::o, "Shift + O"},
      {key::p, "Shift + P"},
      {key::q, "Shift + Q"},
      {key::r, "Shift + R"},
      {key::s, "Shift + S"},
      {key::t, "Shift + T"},
      {key::u, "Shift + U"},
      {key::v, "Shift + V"},
      {key::w, "Shift + W"},
      {key::x, "Shift + X"},
      {key::y, "Shift + Y"},
      {key::z, "Shift + Z"},
      {key::f1, "Shift + F1"},
      {key::f2, "Shift + F2"},
      {key::f3, "Shift + F3"},
      {key::f4, "Shift + F4"},
      {key::f5, "Shift + F5"},
      {key::f6, "Shift + F6"},
      {key::f7, "Shift + F7"},
      {key::f8, "Shift + F8"},
      {key::f9, "Shift + F9"},
      {key::f10, "Shift + F10"},
      {key::f11, "Shift + F11"},
      {key::f12, "Shift + F12"},
      {key::apostrophe, "Shift + '"},
      {key::comma, "Shift + ,"},
      {key::minus, "Shift + -"},
      {key::period, "Shift + ."},
      {key::slash, "Shift + /"},
      {key::semicolon, "Shift + ;"},
      {key::equal, "Shift + ="},
      {key::left_bracket, "Shift + ["},
      {key::right_bracket, "Shift + ]"},
      {key::backslash, "Shift + \\"},
      {key::grave_accent, "Shift + `"},
      {key::caps_lock, "Shift + Caps Lock"},
      {key::scroll_lock, "Shift + Scroll Lock"},
      {key::num_lock, "Shift + Num Lock"},
      {key::print_screen, "Shift + Print Screen"},
      {key::pause, "Shift + Pause"},
      {key::numpad_0, "Shift + Numpad 0"},
      {key::numpad_1, "Shift + Numpad 1"},
      {key::numpad_2, "Shift + Numpad 2"},
      {key::numpad_3, "Shift + Numpad 3"},
      {key::numpad_4, "Shift + Numpad 4"},
      {key::numpad_5, "Shift + Numpad 5"},
      {key::numpad_6, "Shift + Numpad 6"},
      {key::numpad_7, "Shift + Numpad 7"},
      {key::numpad_8, "Shift + Numpad 8"},
      {key::numpad_9, "Shift + Numpad 9"},
      {key::numpad_decimal, "Shift + Numpad Decimal"},
      {key::numpad_divide, "Shift + Numpad Divide"},
      {key::numpad_multiply, "Shift + Numpad Multiply"},
      {key::numpad_subtract, "Shift + Numpad Subtract"},
      {key::numpad_add, "Shift + Numpad Add"},

      {key::mouse1, "Shift + L Mouse"},
      {key::mouse2, "Shift + R Mouse"},
      {key::mouse3, "Shift + Middle Mouse"},
      {key::mouse4, "Shift + Mouse 4"},
      {key::mouse5, "Shift + Mouse 5"},

      {key::void_key, "Shift + <empty>"},
   });

constexpr container::enum_array<const char*, key> ctrl_shift_key_display_names =
   container::make_enum_array<const char*, key>({
      {key::tab, "Ctrl + Shift + Tab"},
      {key::left_arrow, "Ctrl + Shift + Left Arrow"},
      {key::right_arrow, "Ctrl + Shift + Right Arrow"},
      {key::up_arrow, "Ctrl + Shift + Up Arrow"},
      {key::down_arrow, "Ctrl + Shift + Down Arrow"},
      {key::page_up, "Ctrl + Shift + Page Up"},
      {key::page_down, "Ctrl + Shift + Page Down"},
      {key::home, "Ctrl + Shift + Home"},
      {key::end, "Ctrl + Shift + End"},
      {key::insert, "Ctrl + Shift + Insert"},
      {key::del, "Ctrl + Shift + Del"},
      {key::backspace, "Ctrl + Shift + Backspace"},
      {key::space, "Ctrl + Shift + Space"},
      {key::enter, "Ctrl + Shift + Enter"},
      {key::escape, "Ctrl + Shift + Escape"},
      {key::ctrl, "Ctrl + Shift + Ctrl"},
      {key::shift, "Ctrl + Shift + Shift"},
      {key::menu, "Ctrl + Shift + Menu"},
      {key::_0, "Ctrl + Shift + 0"},
      {key::_1, "Ctrl + Shift + 1"},
      {key::_2, "Ctrl + Shift + 2"},
      {key::_3, "Ctrl + Shift + 3"},
      {key::_4, "Ctrl + Shift + 4"},
      {key::_5, "Ctrl + Shift + 5"},
      {key::_6, "Ctrl + Shift + 6"},
      {key::_7, "Ctrl + Shift + 7"},
      {key::_8, "Ctrl + Shift + 8"},
      {key::_9, "Ctrl + Shift + 9"},
      {key::a, "Ctrl + Shift + A"},
      {key::b, "Ctrl + Shift + B"},
      {key::c, "Ctrl + Shift + C"},
      {key::d, "Ctrl + Shift + D"},
      {key::e, "Ctrl + Shift + E"},
      {key::f, "Ctrl + Shift + F"},
      {key::g, "Ctrl + Shift + G"},
      {key::h, "Ctrl + Shift + H"},
      {key::i, "Ctrl + Shift + I"},
      {key::j, "Ctrl + Shift + J"},
      {key::k, "Ctrl + Shift + K"},
      {key::l, "Ctrl + Shift + L"},
      {key::m, "Ctrl + Shift + M"},
      {key::n, "Ctrl + Shift + N"},
      {key::o, "Ctrl + Shift + O"},
      {key::p, "Ctrl + Shift + P"},
      {key::q, "Ctrl + Shift + Q"},
      {key::r, "Ctrl + Shift + R"},
      {key::s, "Ctrl + Shift + S"},
      {key::t, "Ctrl + Shift + T"},
      {key::u, "Ctrl + Shift + U"},
      {key::v, "Ctrl + Shift + V"},
      {key::w, "Ctrl + Shift + W"},
      {key::x, "Ctrl + Shift + X"},
      {key::y, "Ctrl + Shift + Y"},
      {key::z, "Ctrl + Shift + Z"},
      {key::f1, "Ctrl + Shift + F1"},
      {key::f2, "Ctrl + Shift + F2"},
      {key::f3, "Ctrl + Shift + F3"},
      {key::f4, "Ctrl + Shift + F4"},
      {key::f5, "Ctrl + Shift + F5"},
      {key::f6, "Ctrl + Shift + F6"},
      {key::f7, "Ctrl + Shift + F7"},
      {key::f8, "Ctrl + Shift + F8"},
      {key::f9, "Ctrl + Shift + F9"},
      {key::f10, "Ctrl + Shift + F10"},
      {key::f11, "Ctrl + Shift + F11"},
      {key::f12, "Ctrl + Shift + F12"},
      {key::apostrophe, "Ctrl + Shift + '"},
      {key::comma, "Ctrl + Shift + ,"},
      {key::minus, "Ctrl + Shift + -"},
      {key::period, "Ctrl + Shift + ."},
      {key::slash, "Ctrl + Shift + /"},
      {key::semicolon, "Ctrl + Shift + ;"},
      {key::equal, "Ctrl + Shift + ="},
      {key::left_bracket, "Ctrl + Shift + ["},
      {key::right_bracket, "Ctrl + Shift + ]"},
      {key::backslash, "Ctrl + Shift + \\"},
      {key::grave_accent, "Ctrl + Shift + `"},
      {key::caps_lock, "Ctrl + Shift + Caps Lock"},
      {key::scroll_lock, "Ctrl + Shift + Scroll Lock"},
      {key::num_lock, "Ctrl + Shift + Num Lock"},
      {key::print_screen, "Ctrl + Shift + Print Screen"},
      {key::pause, "Ctrl + Shift + Pause"},
      {key::numpad_0, "Ctrl + Shift + Numpad 0"},
      {key::numpad_1, "Ctrl + Shift + Numpad 1"},
      {key::numpad_2, "Ctrl + Shift + Numpad 2"},
      {key::numpad_3, "Ctrl + Shift + Numpad 3"},
      {key::numpad_4, "Ctrl + Shift + Numpad 4"},
      {key::numpad_5, "Ctrl + Shift + Numpad 5"},
      {key::numpad_6, "Ctrl + Shift + Numpad 6"},
      {key::numpad_7, "Ctrl + Shift + Numpad 7"},
      {key::numpad_8, "Ctrl + Shift + Numpad 8"},
      {key::numpad_9, "Ctrl + Shift + Numpad 9"},
      {key::numpad_decimal, "Ctrl + Shift + Numpad Decimal"},
      {key::numpad_divide, "Ctrl + Shift + Numpad Divide"},
      {key::numpad_multiply, "Ctrl + Shift + Numpad Multiply"},
      {key::numpad_subtract, "Ctrl + Shift + Numpad Subtract"},
      {key::numpad_add, "Ctrl + Shift + Numpad Add"},

      {key::mouse1, "Ctrl + Shift + L Mouse"},
      {key::mouse2, "Ctrl + Shift + R Mouse"},
      {key::mouse3, "Ctrl + Shift + Middle Mouse"},
      {key::mouse4, "Ctrl + Shift + Mouse 4"},
      {key::mouse5, "Ctrl + Shift + Mouse 5"},

      {key::void_key, "Ctrl + Shift + <empty>"},
   });

}

auto get_display_string(const key key, const bool ctrl, const bool shift) -> const char*
{
   if (ctrl and shift) return ctrl_shift_key_display_names[key];
   if (shift) return shift_key_display_names[key];
   if (ctrl) return ctrl_key_display_names[key];

   return key_display_names[key];
}

bool parse_display_string(std::string_view string, key& out_key, bool& out_ctrl,
                          bool& out_shift)
{
   string = string::trim_whitespace(string);

   constexpr std::underlying_type_t<key> first = std::to_underlying(key::tab);
   constexpr std::underlying_type_t<key> count = std::to_underlying(key::count);

   if (string::istarts_with(string, "Ctrl + Shift +")) {
      for (auto i = first; i < count; ++i) {
         if (string::iequals(ctrl_shift_key_display_names[i], string)) {

            out_key = key{i};
            out_ctrl = true;
            out_shift = true;

            return true;
         }
      }
   }
   else if (string::istarts_with(string, "Shift +")) {
      for (auto i = first; i < count; ++i) {
         if (string::iequals(shift_key_display_names[i], string)) {

            out_key = key{i};
            out_ctrl = false;
            out_shift = true;

            return true;
         }
      }
   }
   else if (string::istarts_with(string, "Ctrl +")) {
      for (auto i = first; i < count; ++i) {
         if (string::iequals(ctrl_key_display_names[i], string)) {

            out_key = key{i};
            out_ctrl = true;
            out_shift = false;

            return true;
         }
      }
   }
   else {
      for (auto i = first; i < count; ++i) {
         if (string::iequals(key_display_names[i], string)) {

            out_key = key{i};
            out_ctrl = false;
            out_shift = false;

            return true;
         }
      }
   }

   return false;
}

}