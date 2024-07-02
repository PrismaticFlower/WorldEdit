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
   case VK_MENU: return key::alt;
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
      {key::alt, "Alt"},
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
      {key::mouse_wheel_forward, "Mouse Wheel Forward"},
      {key::mouse_wheel_back, "Mouse Wheel Back"},

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
      {key::alt, "Ctrl + Alt"},
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
      {key::mouse_wheel_forward, "Ctrl + Mouse Wheel Forward"},
      {key::mouse_wheel_back, "Ctrl + Mouse Wheel Back"},

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
      {key::alt, "Shift + Alt"},
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
      {key::mouse_wheel_forward, "Shift + Mouse Wheel Forward"},
      {key::mouse_wheel_back, "Shift + Mouse Wheel Back"},

      {key::void_key, "Shift + <empty>"},
   });

constexpr container::enum_array<const char*, key> alt_key_display_names =
   container::make_enum_array<const char*, key>({
      {key::tab, "Alt + Tab"},
      {key::left_arrow, "Alt + Left Arrow"},
      {key::right_arrow, "Alt + Right Arrow"},
      {key::up_arrow, "Alt + Up Arrow"},
      {key::down_arrow, "Alt + Down Arrow"},
      {key::page_up, "Alt + Page Up"},
      {key::page_down, "Alt + Page Down"},
      {key::home, "Alt + Home"},
      {key::end, "Alt + End"},
      {key::insert, "Alt + Insert"},
      {key::del, "Alt + Del"},
      {key::backspace, "Alt + Backspace"},
      {key::space, "Alt + Space"},
      {key::enter, "Alt + Enter"},
      {key::escape, "Alt + Escape"},
      {key::ctrl, "Alt + Ctrl"},
      {key::shift, "Alt + Shift"},
      {key::alt, "Alt + Alt"},
      {key::menu, "Alt + Menu"},
      {key::_0, "Alt + 0"},
      {key::_1, "Alt + 1"},
      {key::_2, "Alt + 2"},
      {key::_3, "Alt + 3"},
      {key::_4, "Alt + 4"},
      {key::_5, "Alt + 5"},
      {key::_6, "Alt + 6"},
      {key::_7, "Alt + 7"},
      {key::_8, "Alt + 8"},
      {key::_9, "Alt + 9"},
      {key::a, "Alt + A"},
      {key::b, "Alt + B"},
      {key::c, "Alt + C"},
      {key::d, "Alt + D"},
      {key::e, "Alt + E"},
      {key::f, "Alt + F"},
      {key::g, "Alt + G"},
      {key::h, "Alt + H"},
      {key::i, "Alt + I"},
      {key::j, "Alt + J"},
      {key::k, "Alt + K"},
      {key::l, "Alt + L"},
      {key::m, "Alt + M"},
      {key::n, "Alt + N"},
      {key::o, "Alt + O"},
      {key::p, "Alt + P"},
      {key::q, "Alt + Q"},
      {key::r, "Alt + R"},
      {key::s, "Alt + S"},
      {key::t, "Alt + T"},
      {key::u, "Alt + U"},
      {key::v, "Alt + V"},
      {key::w, "Alt + W"},
      {key::x, "Alt + X"},
      {key::y, "Alt + Y"},
      {key::z, "Alt + Z"},
      {key::f1, "Alt + F1"},
      {key::f2, "Alt + F2"},
      {key::f3, "Alt + F3"},
      {key::f4, "Alt + F4"},
      {key::f5, "Alt + F5"},
      {key::f6, "Alt + F6"},
      {key::f7, "Alt + F7"},
      {key::f8, "Alt + F8"},
      {key::f9, "Alt + F9"},
      {key::f10, "Alt + F10"},
      {key::f11, "Alt + F11"},
      {key::f12, "Alt + F12"},
      {key::apostrophe, "Alt + '"},
      {key::comma, "Alt + ,"},
      {key::minus, "Alt + -"},
      {key::period, "Alt + ."},
      {key::slash, "Alt + /"},
      {key::semicolon, "Alt + ;"},
      {key::equal, "Alt + ="},
      {key::left_bracket, "Alt + ["},
      {key::right_bracket, "Alt + ]"},
      {key::backslash, "Alt + \\"},
      {key::grave_accent, "Alt + `"},
      {key::caps_lock, "Alt + Caps Lock"},
      {key::scroll_lock, "Alt + Scroll Lock"},
      {key::num_lock, "Alt + Num Lock"},
      {key::print_screen, "Alt + Print Screen"},
      {key::pause, "Alt + Pause"},
      {key::numpad_0, "Alt + Numpad 0"},
      {key::numpad_1, "Alt + Numpad 1"},
      {key::numpad_2, "Alt + Numpad 2"},
      {key::numpad_3, "Alt + Numpad 3"},
      {key::numpad_4, "Alt + Numpad 4"},
      {key::numpad_5, "Alt + Numpad 5"},
      {key::numpad_6, "Alt + Numpad 6"},
      {key::numpad_7, "Alt + Numpad 7"},
      {key::numpad_8, "Alt + Numpad 8"},
      {key::numpad_9, "Alt + Numpad 9"},
      {key::numpad_decimal, "Alt + Numpad Decimal"},
      {key::numpad_divide, "Alt + Numpad Divide"},
      {key::numpad_multiply, "Alt + Numpad Multiply"},
      {key::numpad_subtract, "Alt + Numpad Subtract"},
      {key::numpad_add, "Alt + Numpad Add"},

      {key::mouse1, "Alt + L Mouse"},
      {key::mouse2, "Alt + R Mouse"},
      {key::mouse3, "Alt + Middle Mouse"},
      {key::mouse4, "Alt + Mouse 4"},
      {key::mouse5, "Alt + Mouse 5"},
      {key::mouse_wheel_forward, "Alt + Mouse Wheel Forward"},
      {key::mouse_wheel_back, "Alt + Mouse Wheel Back"},

      {key::void_key, "Alt + <empty>"},
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
      {key::alt, "Ctrl + Shift + Alt"},
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
      {key::mouse_wheel_forward, "Ctrl + Shift + Mouse Wheel Forward"},
      {key::mouse_wheel_back, "Ctrl + Shift + Mouse Wheel Back"},

      {key::void_key, "Ctrl + Shift + <empty>"},
   });

constexpr container::enum_array<const char*, key> ctrl_alt_key_display_names =
   container::make_enum_array<const char*, key>({
      {key::tab, "Ctrl + Alt + Tab"},
      {key::left_arrow, "Ctrl + Alt + Left Arrow"},
      {key::right_arrow, "Ctrl + Alt + Right Arrow"},
      {key::up_arrow, "Ctrl + Alt + Up Arrow"},
      {key::down_arrow, "Ctrl + Alt + Down Arrow"},
      {key::page_up, "Ctrl + Alt + Page Up"},
      {key::page_down, "Ctrl + Alt + Page Down"},
      {key::home, "Ctrl + Alt + Home"},
      {key::end, "Ctrl + Alt + End"},
      {key::insert, "Ctrl + Alt + Insert"},
      {key::del, "Ctrl + Alt + Del"},
      {key::backspace, "Ctrl + Alt + Backspace"},
      {key::space, "Ctrl + Alt + Space"},
      {key::enter, "Ctrl + Alt + Enter"},
      {key::escape, "Ctrl + Alt + Escape"},
      {key::ctrl, "Ctrl + Alt + Ctrl"},
      {key::shift, "Ctrl + Alt + Shift"},
      {key::alt, "Ctrl + Alt + Alt"},
      {key::menu, "Ctrl + Alt + Menu"},
      {key::_0, "Ctrl + Alt + 0"},
      {key::_1, "Ctrl + Alt + 1"},
      {key::_2, "Ctrl + Alt + 2"},
      {key::_3, "Ctrl + Alt + 3"},
      {key::_4, "Ctrl + Alt + 4"},
      {key::_5, "Ctrl + Alt + 5"},
      {key::_6, "Ctrl + Alt + 6"},
      {key::_7, "Ctrl + Alt + 7"},
      {key::_8, "Ctrl + Alt + 8"},
      {key::_9, "Ctrl + Alt + 9"},
      {key::a, "Ctrl + Alt + A"},
      {key::b, "Ctrl + Alt + B"},
      {key::c, "Ctrl + Alt + C"},
      {key::d, "Ctrl + Alt + D"},
      {key::e, "Ctrl + Alt + E"},
      {key::f, "Ctrl + Alt + F"},
      {key::g, "Ctrl + Alt + G"},
      {key::h, "Ctrl + Alt + H"},
      {key::i, "Ctrl + Alt + I"},
      {key::j, "Ctrl + Alt + J"},
      {key::k, "Ctrl + Alt + K"},
      {key::l, "Ctrl + Alt + L"},
      {key::m, "Ctrl + Alt + M"},
      {key::n, "Ctrl + Alt + N"},
      {key::o, "Ctrl + Alt + O"},
      {key::p, "Ctrl + Alt + P"},
      {key::q, "Ctrl + Alt + Q"},
      {key::r, "Ctrl + Alt + R"},
      {key::s, "Ctrl + Alt + S"},
      {key::t, "Ctrl + Alt + T"},
      {key::u, "Ctrl + Alt + U"},
      {key::v, "Ctrl + Alt + V"},
      {key::w, "Ctrl + Alt + W"},
      {key::x, "Ctrl + Alt + X"},
      {key::y, "Ctrl + Alt + Y"},
      {key::z, "Ctrl + Alt + Z"},
      {key::f1, "Ctrl + Alt + F1"},
      {key::f2, "Ctrl + Alt + F2"},
      {key::f3, "Ctrl + Alt + F3"},
      {key::f4, "Ctrl + Alt + F4"},
      {key::f5, "Ctrl + Alt + F5"},
      {key::f6, "Ctrl + Alt + F6"},
      {key::f7, "Ctrl + Alt + F7"},
      {key::f8, "Ctrl + Alt + F8"},
      {key::f9, "Ctrl + Alt + F9"},
      {key::f10, "Ctrl + Alt + F10"},
      {key::f11, "Ctrl + Alt + F11"},
      {key::f12, "Ctrl + Alt + F12"},
      {key::apostrophe, "Ctrl + Alt + '"},
      {key::comma, "Ctrl + Alt + ,"},
      {key::minus, "Ctrl + Alt + -"},
      {key::period, "Ctrl + Alt + ."},
      {key::slash, "Ctrl + Alt + /"},
      {key::semicolon, "Ctrl + Alt + ;"},
      {key::equal, "Ctrl + Alt + ="},
      {key::left_bracket, "Ctrl + Alt + ["},
      {key::right_bracket, "Ctrl + Alt + ]"},
      {key::backslash, "Ctrl + Alt + \\"},
      {key::grave_accent, "Ctrl + Alt + `"},
      {key::caps_lock, "Ctrl + Alt + Caps Lock"},
      {key::scroll_lock, "Ctrl + Alt + Scroll Lock"},
      {key::num_lock, "Ctrl + Alt + Num Lock"},
      {key::print_screen, "Ctrl + Alt + Print Screen"},
      {key::pause, "Ctrl + Alt + Pause"},
      {key::numpad_0, "Ctrl + Alt + Numpad 0"},
      {key::numpad_1, "Ctrl + Alt + Numpad 1"},
      {key::numpad_2, "Ctrl + Alt + Numpad 2"},
      {key::numpad_3, "Ctrl + Alt + Numpad 3"},
      {key::numpad_4, "Ctrl + Alt + Numpad 4"},
      {key::numpad_5, "Ctrl + Alt + Numpad 5"},
      {key::numpad_6, "Ctrl + Alt + Numpad 6"},
      {key::numpad_7, "Ctrl + Alt + Numpad 7"},
      {key::numpad_8, "Ctrl + Alt + Numpad 8"},
      {key::numpad_9, "Ctrl + Alt + Numpad 9"},
      {key::numpad_decimal, "Ctrl + Alt + Numpad Decimal"},
      {key::numpad_divide, "Ctrl + Alt + Numpad Divide"},
      {key::numpad_multiply, "Ctrl + Alt + Numpad Multiply"},
      {key::numpad_subtract, "Ctrl + Alt + Numpad Subtract"},
      {key::numpad_add, "Ctrl + Alt + Numpad Add"},

      {key::mouse1, "Ctrl + Alt + L Mouse"},
      {key::mouse2, "Ctrl + Alt + R Mouse"},
      {key::mouse3, "Ctrl + Alt + Middle Mouse"},
      {key::mouse4, "Ctrl + Alt + Mouse 4"},
      {key::mouse5, "Ctrl + Alt + Mouse 5"},
      {key::mouse_wheel_forward, "Ctrl + Alt + Mouse Wheel Forward"},
      {key::mouse_wheel_back, "Ctrl + Alt + Mouse Wheel Back"},

      {key::void_key, "Ctrl + Alt + <empty>"},
   });

constexpr container::enum_array<const char*, key> shift_alt_key_display_names =
   container::make_enum_array<const char*, key>({
      {key::tab, "Shift + Alt + Tab"},
      {key::left_arrow, "Shift + Alt + Left Arrow"},
      {key::right_arrow, "Shift + Alt + Right Arrow"},
      {key::up_arrow, "Shift + Alt + Up Arrow"},
      {key::down_arrow, "Shift + Alt + Down Arrow"},
      {key::page_up, "Shift + Alt + Page Up"},
      {key::page_down, "Shift + Alt + Page Down"},
      {key::home, "Shift + Alt + Home"},
      {key::end, "Shift + Alt + End"},
      {key::insert, "Shift + Alt + Insert"},
      {key::del, "Shift + Alt + Del"},
      {key::backspace, "Shift + Alt + Backspace"},
      {key::space, "Shift + Alt + Space"},
      {key::enter, "Shift + Alt + Enter"},
      {key::escape, "Shift + Alt + Escape"},
      {key::ctrl, "Shift + Alt + Ctrl"},
      {key::shift, "Shift + Alt + Shift"},
      {key::alt, "Shift + Alt + Alt"},
      {key::menu, "Shift + Alt + Menu"},
      {key::_0, "Shift + Alt + 0"},
      {key::_1, "Shift + Alt + 1"},
      {key::_2, "Shift + Alt + 2"},
      {key::_3, "Shift + Alt + 3"},
      {key::_4, "Shift + Alt + 4"},
      {key::_5, "Shift + Alt + 5"},
      {key::_6, "Shift + Alt + 6"},
      {key::_7, "Shift + Alt + 7"},
      {key::_8, "Shift + Alt + 8"},
      {key::_9, "Shift + Alt + 9"},
      {key::a, "Shift + Alt + A"},
      {key::b, "Shift + Alt + B"},
      {key::c, "Shift + Alt + C"},
      {key::d, "Shift + Alt + D"},
      {key::e, "Shift + Alt + E"},
      {key::f, "Shift + Alt + F"},
      {key::g, "Shift + Alt + G"},
      {key::h, "Shift + Alt + H"},
      {key::i, "Shift + Alt + I"},
      {key::j, "Shift + Alt + J"},
      {key::k, "Shift + Alt + K"},
      {key::l, "Shift + Alt + L"},
      {key::m, "Shift + Alt + M"},
      {key::n, "Shift + Alt + N"},
      {key::o, "Shift + Alt + O"},
      {key::p, "Shift + Alt + P"},
      {key::q, "Shift + Alt + Q"},
      {key::r, "Shift + Alt + R"},
      {key::s, "Shift + Alt + S"},
      {key::t, "Shift + Alt + T"},
      {key::u, "Shift + Alt + U"},
      {key::v, "Shift + Alt + V"},
      {key::w, "Shift + Alt + W"},
      {key::x, "Shift + Alt + X"},
      {key::y, "Shift + Alt + Y"},
      {key::z, "Shift + Alt + Z"},
      {key::f1, "Shift + Alt + F1"},
      {key::f2, "Shift + Alt + F2"},
      {key::f3, "Shift + Alt + F3"},
      {key::f4, "Shift + Alt + F4"},
      {key::f5, "Shift + Alt + F5"},
      {key::f6, "Shift + Alt + F6"},
      {key::f7, "Shift + Alt + F7"},
      {key::f8, "Shift + Alt + F8"},
      {key::f9, "Shift + Alt + F9"},
      {key::f10, "Shift + Alt + F10"},
      {key::f11, "Shift + Alt + F11"},
      {key::f12, "Shift + Alt + F12"},
      {key::apostrophe, "Shift + Alt + '"},
      {key::comma, "Shift + Alt + ,"},
      {key::minus, "Shift + Alt + -"},
      {key::period, "Shift + Alt + ."},
      {key::slash, "Shift + Alt + /"},
      {key::semicolon, "Shift + Alt + ;"},
      {key::equal, "Shift + Alt + ="},
      {key::left_bracket, "Shift + Alt + ["},
      {key::right_bracket, "Shift + Alt + ]"},
      {key::backslash, "Shift + Alt + \\"},
      {key::grave_accent, "Shift + Alt + `"},
      {key::caps_lock, "Shift + Alt + Caps Lock"},
      {key::scroll_lock, "Shift + Alt + Scroll Lock"},
      {key::num_lock, "Shift + Alt + Num Lock"},
      {key::print_screen, "Shift + Alt + Print Screen"},
      {key::pause, "Shift + Alt + Pause"},
      {key::numpad_0, "Shift + Alt + Numpad 0"},
      {key::numpad_1, "Shift + Alt + Numpad 1"},
      {key::numpad_2, "Shift + Alt + Numpad 2"},
      {key::numpad_3, "Shift + Alt + Numpad 3"},
      {key::numpad_4, "Shift + Alt + Numpad 4"},
      {key::numpad_5, "Shift + Alt + Numpad 5"},
      {key::numpad_6, "Shift + Alt + Numpad 6"},
      {key::numpad_7, "Shift + Alt + Numpad 7"},
      {key::numpad_8, "Shift + Alt + Numpad 8"},
      {key::numpad_9, "Shift + Alt + Numpad 9"},
      {key::numpad_decimal, "Shift + Alt + Numpad Decimal"},
      {key::numpad_divide, "Shift + Alt + Numpad Divide"},
      {key::numpad_multiply, "Shift + Alt + Numpad Multiply"},
      {key::numpad_subtract, "Shift + Alt + Numpad Subtract"},
      {key::numpad_add, "Shift + Alt + Numpad Add"},

      {key::mouse1, "Shift + Alt + L Mouse"},
      {key::mouse2, "Shift + Alt + R Mouse"},
      {key::mouse3, "Shift + Alt + Middle Mouse"},
      {key::mouse4, "Shift + Alt + Mouse 4"},
      {key::mouse5, "Shift + Alt + Mouse 5"},
      {key::mouse_wheel_forward, "Shift + Alt + Mouse Wheel Forward"},
      {key::mouse_wheel_back, "Shift + Alt + Mouse Wheel Back"},

      {key::void_key, "Shift + Alt + <empty>"},
   });

constexpr container::enum_array<const char*, key> ctrl_shift_alt_key_display_names =
   container::make_enum_array<const char*, key>({
      {key::tab, "Ctrl + Shift + Alt + Tab"},
      {key::left_arrow, "Ctrl + Shift + Alt + Left Arrow"},
      {key::right_arrow, "Ctrl + Shift + Alt + Right Arrow"},
      {key::up_arrow, "Ctrl + Shift + Alt + Up Arrow"},
      {key::down_arrow, "Ctrl + Shift + Alt + Down Arrow"},
      {key::page_up, "Ctrl + Shift + Alt + Page Up"},
      {key::page_down, "Ctrl + Shift + Alt + Page Down"},
      {key::home, "Ctrl + Shift + Alt + Home"},
      {key::end, "Ctrl + Shift + Alt + End"},
      {key::insert, "Ctrl + Shift + Alt + Insert"},
      {key::del, "Ctrl + Shift + Alt + Del"},
      {key::backspace, "Ctrl + Shift + Alt + Backspace"},
      {key::space, "Ctrl + Shift + Alt + Space"},
      {key::enter, "Ctrl + Shift + Alt + Enter"},
      {key::escape, "Ctrl + Shift + Alt + Escape"},
      {key::ctrl, "Ctrl + Shift + Alt + Ctrl"},
      {key::shift, "Ctrl + Shift + Alt + Shift"},
      {key::alt, "Ctrl + Shift + Alt + Alt"},
      {key::menu, "Ctrl + Shift + Alt + Menu"},
      {key::_0, "Ctrl + Shift + Alt + 0"},
      {key::_1, "Ctrl + Shift + Alt + 1"},
      {key::_2, "Ctrl + Shift + Alt + 2"},
      {key::_3, "Ctrl + Shift + Alt + 3"},
      {key::_4, "Ctrl + Shift + Alt + 4"},
      {key::_5, "Ctrl + Shift + Alt + 5"},
      {key::_6, "Ctrl + Shift + Alt + 6"},
      {key::_7, "Ctrl + Shift + Alt + 7"},
      {key::_8, "Ctrl + Shift + Alt + 8"},
      {key::_9, "Ctrl + Shift + Alt + 9"},
      {key::a, "Ctrl + Shift + Alt + A"},
      {key::b, "Ctrl + Shift + Alt + B"},
      {key::c, "Ctrl + Shift + Alt + C"},
      {key::d, "Ctrl + Shift + Alt + D"},
      {key::e, "Ctrl + Shift + Alt + E"},
      {key::f, "Ctrl + Shift + Alt + F"},
      {key::g, "Ctrl + Shift + Alt + G"},
      {key::h, "Ctrl + Shift + Alt + H"},
      {key::i, "Ctrl + Shift + Alt + I"},
      {key::j, "Ctrl + Shift + Alt + J"},
      {key::k, "Ctrl + Shift + Alt + K"},
      {key::l, "Ctrl + Shift + Alt + L"},
      {key::m, "Ctrl + Shift + Alt + M"},
      {key::n, "Ctrl + Shift + Alt + N"},
      {key::o, "Ctrl + Shift + Alt + O"},
      {key::p, "Ctrl + Shift + Alt + P"},
      {key::q, "Ctrl + Shift + Alt + Q"},
      {key::r, "Ctrl + Shift + Alt + R"},
      {key::s, "Ctrl + Shift + Alt + S"},
      {key::t, "Ctrl + Shift + Alt + T"},
      {key::u, "Ctrl + Shift + Alt + U"},
      {key::v, "Ctrl + Shift + Alt + V"},
      {key::w, "Ctrl + Shift + Alt + W"},
      {key::x, "Ctrl + Shift + Alt + X"},
      {key::y, "Ctrl + Shift + Alt + Y"},
      {key::z, "Ctrl + Shift + Alt + Z"},
      {key::f1, "Ctrl + Shift + Alt + F1"},
      {key::f2, "Ctrl + Shift + Alt + F2"},
      {key::f3, "Ctrl + Shift + Alt + F3"},
      {key::f4, "Ctrl + Shift + Alt + F4"},
      {key::f5, "Ctrl + Shift + Alt + F5"},
      {key::f6, "Ctrl + Shift + Alt + F6"},
      {key::f7, "Ctrl + Shift + Alt + F7"},
      {key::f8, "Ctrl + Shift + Alt + F8"},
      {key::f9, "Ctrl + Shift + Alt + F9"},
      {key::f10, "Ctrl + Shift + Alt + F10"},
      {key::f11, "Ctrl + Shift + Alt + F11"},
      {key::f12, "Ctrl + Shift + Alt + F12"},
      {key::apostrophe, "Ctrl + Shift + Alt + '"},
      {key::comma, "Ctrl + Shift + Alt + ,"},
      {key::minus, "Ctrl + Shift + Alt + -"},
      {key::period, "Ctrl + Shift + Alt + ."},
      {key::slash, "Ctrl + Shift + Alt + /"},
      {key::semicolon, "Ctrl + Shift + Alt + ;"},
      {key::equal, "Ctrl + Shift + Alt + ="},
      {key::left_bracket, "Ctrl + Shift + Alt + ["},
      {key::right_bracket, "Ctrl + Shift + Alt + ]"},
      {key::backslash, "Ctrl + Shift + Alt + \\"},
      {key::grave_accent, "Ctrl + Shift + Alt + `"},
      {key::caps_lock, "Ctrl + Shift + Alt + Caps Lock"},
      {key::scroll_lock, "Ctrl + Shift + Alt + Scroll Lock"},
      {key::num_lock, "Ctrl + Shift + Alt + Num Lock"},
      {key::print_screen, "Ctrl + Shift + Alt + Print Screen"},
      {key::pause, "Ctrl + Shift + Alt + Pause"},
      {key::numpad_0, "Ctrl + Shift + Alt + Numpad 0"},
      {key::numpad_1, "Ctrl + Shift + Alt + Numpad 1"},
      {key::numpad_2, "Ctrl + Shift + Alt + Numpad 2"},
      {key::numpad_3, "Ctrl + Shift + Alt + Numpad 3"},
      {key::numpad_4, "Ctrl + Shift + Alt + Numpad 4"},
      {key::numpad_5, "Ctrl + Shift + Alt + Numpad 5"},
      {key::numpad_6, "Ctrl + Shift + Alt + Numpad 6"},
      {key::numpad_7, "Ctrl + Shift + Alt + Numpad 7"},
      {key::numpad_8, "Ctrl + Shift + Alt + Numpad 8"},
      {key::numpad_9, "Ctrl + Shift + Alt + Numpad 9"},
      {key::numpad_decimal, "Ctrl + Shift + Alt + Numpad Decimal"},
      {key::numpad_divide, "Ctrl + Shift + Alt + Numpad Divide"},
      {key::numpad_multiply, "Ctrl + Shift + Alt + Numpad Multiply"},
      {key::numpad_subtract, "Ctrl + Shift + Alt + Numpad Subtract"},
      {key::numpad_add, "Ctrl + Shift + Alt + Numpad Add"},

      {key::mouse1, "Ctrl + Shift + Alt + L Mouse"},
      {key::mouse2, "Ctrl + Shift + Alt + R Mouse"},
      {key::mouse3, "Ctrl + Shift + Alt + Middle Mouse"},
      {key::mouse4, "Ctrl + Shift + Alt + Mouse 4"},
      {key::mouse5, "Ctrl + Shift + Alt + Mouse 5"},
      {key::mouse_wheel_forward, "Ctrl + Shift + Alt + Mouse Wheel Forward"},
      {key::mouse_wheel_back, "Ctrl + Shift + Alt + Mouse Wheel Back"},

      {key::void_key, "Ctrl + Shift + Alt + <empty>"},
   });

constexpr container::enum_array<const char*, key> key_abbreviated_names =
   container::make_enum_array<const char*, key>({
      {key::tab, "Tab"},
      {key::left_arrow, "Left"},
      {key::right_arrow, "Rig-\n-ht"},
      {key::up_arrow, "Up"},
      {key::down_arrow, "Do-\n-wn"},
      {key::page_up, "Page\nUp"},
      {key::page_down, "Page\nDwn"},
      {key::home, "Ho-\n-me"},
      {key::end, "End"},
      {key::insert, "Ins"},
      {key::del, "Del"},
      {key::backspace, "Backspace"},
      {key::space, "Space"},
      {key::enter, "Enter"},
      {key::escape, "Esc"},
      {key::ctrl, "Ctrl"},
      {key::shift, "Shift"},
      {key::alt, "Alt"},
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
      {key::caps_lock, "Caps\nLock"},
      {key::scroll_lock, "Scr\nLk"},
      {key::num_lock, "Num\nLock"},
      {key::print_screen, "Prt\nSc"},
      {key::pause, "Pau-\n-se"},
      {key::numpad_0, "0"},
      {key::numpad_1, "1"},
      {key::numpad_2, "2"},
      {key::numpad_3, "3"},
      {key::numpad_4, "4"},
      {key::numpad_5, "5"},
      {key::numpad_6, "6"},
      {key::numpad_7, "7"},
      {key::numpad_8, "8"},
      {key::numpad_9, "9"},
      {key::numpad_decimal, "."},
      {key::numpad_divide, "/"},
      {key::numpad_multiply, "*"},
      {key::numpad_subtract, "-"},
      {key::numpad_add, "+"},

      {key::mouse1, "L Mouse"},
      {key::mouse2, "R Mouse"},
      {key::mouse3, "Middle Mouse"},
      {key::mouse4, "Mouse 4"},
      {key::mouse5, "Mouse 5"},
      {key::mouse_wheel_forward, "Wheel Forward"},
      {key::mouse_wheel_back, "Wheel Back"},

      {key::void_key, ""},
   });

}

auto get_display_string(const key key, const bool ctrl, const bool shift,
                        const bool alt) -> const char*
{
   if (ctrl and shift and alt) return ctrl_shift_alt_key_display_names[key];
   if (ctrl and shift) return ctrl_shift_key_display_names[key];
   if (ctrl and alt) return ctrl_alt_key_display_names[key];
   if (shift and alt) return shift_alt_key_display_names[key];
   if (shift) return shift_key_display_names[key];
   if (ctrl) return ctrl_key_display_names[key];
   if (alt) return alt_key_display_names[key];

   return key_display_names[key];
}

bool parse_display_string(std::string_view string, key& out_key, bool& out_ctrl,
                          bool& out_shift, bool& out_alt)
{
   string = string::trim_whitespace(string);

   constexpr std::underlying_type_t<key> first = std::to_underlying(key::tab);
   constexpr std::underlying_type_t<key> count = std::to_underlying(key::COUNT);

   if (string::istarts_with(string, "Ctrl + Shift + Alt")) {
      for (auto i = first; i < count; ++i) {
         if (string::iequals(ctrl_shift_alt_key_display_names[i], string)) {

            out_key = key{i};
            out_ctrl = true;
            out_shift = true;
            out_alt = true;

            return true;
         }
      }
   }
   else if (string::istarts_with(string, "Ctrl + Shift +")) {
      for (auto i = first; i < count; ++i) {
         if (string::iequals(ctrl_shift_key_display_names[i], string)) {

            out_key = key{i};
            out_ctrl = true;
            out_shift = true;
            out_alt = false;

            return true;
         }
      }
   }
   else if (string::istarts_with(string, "Ctrl + Alt +")) {
      for (auto i = first; i < count; ++i) {
         if (string::iequals(ctrl_alt_key_display_names[i], string)) {

            out_key = key{i};
            out_ctrl = true;
            out_shift = false;
            out_alt = true;

            return true;
         }
      }
   }
   else if (string::istarts_with(string, "Shift + Alt +")) {
      for (auto i = first; i < count; ++i) {
         if (string::iequals(shift_alt_key_display_names[i], string)) {

            out_key = key{i};
            out_ctrl = false;
            out_shift = true;
            out_alt = true;

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
            out_alt = false;

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
            out_alt = false;

            return true;
         }
      }
   }
   else if (string::istarts_with(string, "Alt +")) {
      for (auto i = first; i < count; ++i) {
         if (string::iequals(alt_key_display_names[i], string)) {

            out_key = key{i};
            out_ctrl = false;
            out_shift = false;
            out_alt = true;

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
            out_alt = false;

            return true;
         }
      }
   }

   return false;
}

auto get_key_name(const key key) -> const char*
{
   return key_abbreviated_names[key];
}

}