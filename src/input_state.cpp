
#include "input_state.hpp"

#include <cassert>

namespace sk {

auto get_mouse_state(const HWND window) -> mouse_state
{
   assert(window);

   POINT point{};

   if (not GetCursorPos(&point)) return {};
   if (not ScreenToClient(window, &point)) return {};

   static mouse_state previous_mouse_state{};

   mouse_state new_mouse_state{.x = point.x,
                               .y = point.y,
                               .x_movement = previous_mouse_state.x - point.x,
                               .y_movement = previous_mouse_state.y - point.y};

   new_mouse_state.left_button = (GetKeyState(VK_LBUTTON) & 0x8000) != 0;
   new_mouse_state.right_button = (GetKeyState(VK_RBUTTON) & 0x8000) != 0;
   new_mouse_state.middle_button = (GetKeyState(VK_MBUTTON) & 0x8000) != 0;
   new_mouse_state.extra_buttons[0] = (GetKeyState(VK_XBUTTON1) & 0x8000) != 0;
   new_mouse_state.extra_buttons[1] = (GetKeyState(VK_XBUTTON2) & 0x8000) != 0;

   RECT client_rect{};
   if (not GetClientRect(window, &client_rect)) return {};

   new_mouse_state.over_window = PtInRect(&client_rect, point);

   previous_mouse_state = new_mouse_state;

   return new_mouse_state;
}

auto get_keyboard_state() -> keyboard_state
{
   std::array<BYTE, 256> win_state{};

   if (GetKeyboardState(win_state.data()) == 0) return {};

   keyboard_state state;

   for (std::size_t i = 0; i < state.size(); ++i) {
      state[i] = (win_state[keyboard_keys_vk_mapping[i]] & 0x80) == 0x80;
   }

   return state;
}

}