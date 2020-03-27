#pragma once

#include "container/enum_array.hpp"

#include <array>
#include <string_view>

#include <Windows.h>

namespace sk {

struct mouse_state {
   int x = 0;
   int y = 0;
   int x_movement = 0;
   int y_movement = 0;

   bool left_button = false;
   bool middle_button = false;
   bool right_button = false;

   std::array<bool, 2> extra_buttons{false, false};

   bool over_window = false;
};

enum class keyboard_keys {
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
   oem_nec_equal,
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

   // size marker
   count
};

inline constexpr container::enum_array<int, keyboard_keys> keyboard_keys_vk_mapping =
   container::make_enum_array<int, keyboard_keys>(
      {{keyboard_keys::_0, 0x30},
       {keyboard_keys::_1, 0x31},
       {keyboard_keys::_2, 0x32},
       {keyboard_keys::_3, 0x33},
       {keyboard_keys::_4, 0x34},
       {keyboard_keys::_5, 0x35},
       {keyboard_keys::_6, 0x36},
       {keyboard_keys::_7, 0x37},
       {keyboard_keys::_8, 0x38},
       {keyboard_keys::_9, 0x39},
       {keyboard_keys::a, 0x41},
       {keyboard_keys::accept, 0x1e},
       {keyboard_keys::add, 0x6b},
       {keyboard_keys::b, 0x42},
       {keyboard_keys::back, 0x08},
       {keyboard_keys::browser_back, 0xa6},
       {keyboard_keys::browser_favorites, 0xab},
       {keyboard_keys::browser_forward, 0xa7},
       {keyboard_keys::browser_home, 0xac},
       {keyboard_keys::browser_refresh, 0xa8},
       {keyboard_keys::browser_search, 0xaa},
       {keyboard_keys::browser_stop, 0xa9},
       {keyboard_keys::c, 0x43},
       {keyboard_keys::capital, 0x14},
       {keyboard_keys::clear, 0x0c},
       {keyboard_keys::control, 0x11},
       {keyboard_keys::convert, 0x1c},
       {keyboard_keys::d, 0x44},
       {keyboard_keys::decimal, 0x6e},
       {keyboard_keys::delete_, 0x2e},
       {keyboard_keys::divide, 0x6f},
       {keyboard_keys::down, 0x28},
       {keyboard_keys::e, 0x45},
       {keyboard_keys::end, 0x23},
       {keyboard_keys::escape, 0x1b},
       {keyboard_keys::execute, 0x2b},
       {keyboard_keys::f, 0x46},
       {keyboard_keys::f1, 0x70},
       {keyboard_keys::f10, 0x79},
       {keyboard_keys::f11, 0x7a},
       {keyboard_keys::f12, 0x7b},
       {keyboard_keys::f13, 0x7c},
       {keyboard_keys::f14, 0x7d},
       {keyboard_keys::f15, 0x7e},
       {keyboard_keys::f16, 0x7f},
       {keyboard_keys::f17, 0x80},
       {keyboard_keys::f18, 0x81},
       {keyboard_keys::f19, 0x82},
       {keyboard_keys::f2, 0x71},
       {keyboard_keys::f20, 0x83},
       {keyboard_keys::f21, 0x84},
       {keyboard_keys::f22, 0x85},
       {keyboard_keys::f23, 0x86},
       {keyboard_keys::f24, 0x87},
       {keyboard_keys::f3, 0x72},
       {keyboard_keys::f4, 0x73},
       {keyboard_keys::f5, 0x74},
       {keyboard_keys::f6, 0x75},
       {keyboard_keys::f7, 0x76},
       {keyboard_keys::f8, 0x77},
       {keyboard_keys::f9, 0x78},
       {keyboard_keys::g, 0x47},
       {keyboard_keys::h, 0x48},
       {keyboard_keys::help, 0x2f},
       {keyboard_keys::home, 0x24},
       {keyboard_keys::i, 0x49},
       {keyboard_keys::insert, 0x2d},
       {keyboard_keys::j, 0x4a},
       {keyboard_keys::k, 0x4b},
       {keyboard_keys::l, 0x4c},
       {keyboard_keys::left, 0x25},
       {keyboard_keys::m, 0x4d},
       {keyboard_keys::menu, 0x12},
       {keyboard_keys::modechange, 0x1f},
       {keyboard_keys::multiply, 0x6a},
       {keyboard_keys::n, 0x4e},
       {keyboard_keys::next, 0x22},
       {keyboard_keys::nonconvert, 0x1d},
       {keyboard_keys::numlock, 0x90},
       {keyboard_keys::numpad0, 0x60},
       {keyboard_keys::numpad1, 0x61},
       {keyboard_keys::numpad2, 0x62},
       {keyboard_keys::numpad3, 0x63},
       {keyboard_keys::numpad4, 0x64},
       {keyboard_keys::numpad5, 0x65},
       {keyboard_keys::numpad6, 0x66},
       {keyboard_keys::numpad7, 0x67},
       {keyboard_keys::numpad8, 0x68},
       {keyboard_keys::numpad9, 0x69},
       {keyboard_keys::o, 0x4f},
       {keyboard_keys::oem_1, 0xba},
       {keyboard_keys::oem_102, 0xe2},
       {keyboard_keys::oem_2, 0xbf},
       {keyboard_keys::oem_3, 0xc0},
       {keyboard_keys::oem_4, 0xdb},
       {keyboard_keys::oem_5, 0xdc},
       {keyboard_keys::oem_6, 0xdd},
       {keyboard_keys::oem_7, 0xde},
       {keyboard_keys::oem_8, 0xdf},
       {keyboard_keys::oem_ax, 0xe1},
       {keyboard_keys::oem_comma, 0xbc},
       {keyboard_keys::oem_fj_jisho, 0x92},
       {keyboard_keys::oem_fj_loya, 0x95},
       {keyboard_keys::oem_fj_masshou, 0x93},
       {keyboard_keys::oem_fj_roya, 0x96},
       {keyboard_keys::oem_fj_touroku, 0x94},
       {keyboard_keys::oem_minus, 0xbd},
       {keyboard_keys::oem_nec_equal, 0x92},
       {keyboard_keys::oem_period, 0xbe},
       {keyboard_keys::oem_plus, 0xbb},
       {keyboard_keys::p, 0x50},
       {keyboard_keys::pause, 0x13},
       {keyboard_keys::print, 0x2a},
       {keyboard_keys::prior, 0x21},
       {keyboard_keys::q, 0x51},
       {keyboard_keys::r, 0x52},
       {keyboard_keys::return_, 0x0d},
       {keyboard_keys::right, 0x27},
       {keyboard_keys::s, 0x53},
       {keyboard_keys::scroll, 0x91},
       {keyboard_keys::select, 0x29},
       {keyboard_keys::separator, 0x6c},
       {keyboard_keys::shift, 0x10},
       {keyboard_keys::sleep, 0x5f},
       {keyboard_keys::snapshot, 0x2c},
       {keyboard_keys::space, 0x20},
       {keyboard_keys::subtract, 0x6d},
       {keyboard_keys::t, 0x54},
       {keyboard_keys::tab, 0x09},
       {keyboard_keys::u, 0x55},
       {keyboard_keys::up, 0x26},
       {keyboard_keys::v, 0x56},
       {keyboard_keys::w, 0x57},
       {keyboard_keys::x, 0x58},
       {keyboard_keys::y, 0x59},
       {keyboard_keys::z, 0x5a}});

inline constexpr container::enum_array<std::string_view, keyboard_keys> keyboard_keys_names_mapping =
   container::make_enum_array<std::string_view, keyboard_keys>({
      {keyboard_keys::_0, "0"},
      {keyboard_keys::_1, "1"},
      {keyboard_keys::_2, "2"},
      {keyboard_keys::_3, "3"},
      {keyboard_keys::_4, "4"},
      {keyboard_keys::_5, "5"},
      {keyboard_keys::_6, "6"},
      {keyboard_keys::_7, "7"},
      {keyboard_keys::_8, "8"},
      {keyboard_keys::_9, "9"},
      {keyboard_keys::a, "A"},
      {keyboard_keys::accept, "Accept"},
      {keyboard_keys::add, "Add"},
      {keyboard_keys::b, "B"},
      {keyboard_keys::back, "Back"},
      {keyboard_keys::browser_back, "Browser Back"},
      {keyboard_keys::browser_favorites, "Browser Favorites"},
      {keyboard_keys::browser_forward, "Browser Forward"},
      {keyboard_keys::browser_home, "Browser Home"},
      {keyboard_keys::browser_refresh, "Browser Refresh"},
      {keyboard_keys::browser_search, "Browser Search"},
      {keyboard_keys::browser_stop, "Browser Stop"},
      {keyboard_keys::c, "C"},
      {keyboard_keys::capital, "Capital"},
      {keyboard_keys::clear, "Clear"},
      {keyboard_keys::control, "CTRL"},
      {keyboard_keys::convert, "Convert"},
      {keyboard_keys::d, "D"},
      {keyboard_keys::decimal, "Decimal"},
      {keyboard_keys::delete_, "Delete"},
      {keyboard_keys::divide, "Divide"},
      {keyboard_keys::down, "Down"},
      {keyboard_keys::e, "E"},
      {keyboard_keys::end, "End"},
      {keyboard_keys::escape, "Escape"},
      {keyboard_keys::execute, "Execute"},
      {keyboard_keys::f, "F"},
      {keyboard_keys::f1, "F1"},
      {keyboard_keys::f10, "F10"},
      {keyboard_keys::f11, "F11"},
      {keyboard_keys::f12, "F12"},
      {keyboard_keys::f13, "F13"},
      {keyboard_keys::f14, "F14"},
      {keyboard_keys::f15, "F15"},
      {keyboard_keys::f16, "F16"},
      {keyboard_keys::f17, "F17"},
      {keyboard_keys::f18, "F18"},
      {keyboard_keys::f19, "F19"},
      {keyboard_keys::f2, "F2"},
      {keyboard_keys::f20, "F20"},
      {keyboard_keys::f21, "F21"},
      {keyboard_keys::f22, "F22"},
      {keyboard_keys::f23, "F23"},
      {keyboard_keys::f24, "F24"},
      {keyboard_keys::f3, "F3"},
      {keyboard_keys::f4, "F4"},
      {keyboard_keys::f5, "F5"},
      {keyboard_keys::f6, "F6"},
      {keyboard_keys::f7, "F7"},
      {keyboard_keys::f8, "F8"},
      {keyboard_keys::f9, "F9"},
      {keyboard_keys::g, "G"},
      {keyboard_keys::h, "H"},
      {keyboard_keys::help, "Help"},
      {keyboard_keys::home, "Home"},
      {keyboard_keys::i, "I"},
      {keyboard_keys::insert, "Insert"},
      {keyboard_keys::j, "J"},
      {keyboard_keys::k, "K"},
      {keyboard_keys::l, "L"},
      {keyboard_keys::left, "Left"},
      {keyboard_keys::m, "M"},
      {keyboard_keys::menu, "ALT"},
      {keyboard_keys::modechange, "Modechange"},
      {keyboard_keys::multiply, "Multiply"},
      {keyboard_keys::n, "N"},
      {keyboard_keys::next, "Page Down"},
      {keyboard_keys::nonconvert, "IME nonconvert"},
      {keyboard_keys::numlock, "numlock"},
      {keyboard_keys::numpad0, "Numpad 0"},
      {keyboard_keys::numpad1, "Numpad 1"},
      {keyboard_keys::numpad2, "Numpad 2"},
      {keyboard_keys::numpad3, "Numpad 3"},
      {keyboard_keys::numpad4, "Numpad 4"},
      {keyboard_keys::numpad5, "Numpad 5"},
      {keyboard_keys::numpad6, "Numpad 6"},
      {keyboard_keys::numpad7, "Numpad 7"},
      {keyboard_keys::numpad8, "Numpad 8"},
      {keyboard_keys::numpad9, "Numpad 9"},
      {keyboard_keys::o, "O"},
      {keyboard_keys::oem_1, ";"},
      {keyboard_keys::oem_102, "<>"},
      {keyboard_keys::oem_2, "/"},
      {keyboard_keys::oem_3, "`"},
      {keyboard_keys::oem_4, "["},
      {keyboard_keys::oem_5, "\\"},
      {keyboard_keys::oem_6, "]"},
      {keyboard_keys::oem_7, "\""},
      {keyboard_keys::oem_8, "OEM 8 (Misc)"},
      {keyboard_keys::oem_ax, "AX"},
      {keyboard_keys::oem_comma, ","},
      {keyboard_keys::oem_fj_jisho, "Dictionary"},
      {keyboard_keys::oem_fj_loya, "Left Oyayubi"},
      {keyboard_keys::oem_fj_masshou, "oem_fj_masshou"},
      {keyboard_keys::oem_fj_roya, "Right Oyayubi"},
      {keyboard_keys::oem_fj_touroku, "Register Word"},
      {keyboard_keys::oem_minus, "-"},
      {keyboard_keys::oem_nec_equal, "="},
      {keyboard_keys::oem_period, "."},
      {keyboard_keys::oem_plus, "+"},
      {keyboard_keys::p, "P"},
      {keyboard_keys::pause, "Pause"},
      {keyboard_keys::print, "Print"},
      {keyboard_keys::prior, "Page Up"},
      {keyboard_keys::q, "Q"},
      {keyboard_keys::r, "R"},
      {keyboard_keys::return_, "Return"},
      {keyboard_keys::right, "Right"},
      {keyboard_keys::s, "S"},
      {keyboard_keys::scroll, "Scroll"},
      {keyboard_keys::select, "Select"},
      {keyboard_keys::separator, "Separator"},
      {keyboard_keys::shift, "Shift"},
      {keyboard_keys::sleep, "Sleep"},
      {keyboard_keys::snapshot, "Snapshot"},
      {keyboard_keys::space, "Space"},
      {keyboard_keys::subtract, "Subtract"},
      {keyboard_keys::t, "T"},
      {keyboard_keys::tab, "Tab"},
      {keyboard_keys::u, "U"},
      {keyboard_keys::up, "Up"},
      {keyboard_keys::v, "V"},
      {keyboard_keys::w, "W"},
      {keyboard_keys::x, "X"},
      {keyboard_keys::y, "Y"},
      {keyboard_keys::z, "Z"},
   });

using keyboard_state = container::enum_array<bool, keyboard_keys>;

auto get_mouse_state(const HWND window) -> mouse_state; // NOT thread safe!

auto get_keyboard_state() -> keyboard_state;

}