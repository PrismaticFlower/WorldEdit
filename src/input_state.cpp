
#include "input_state.hpp"

#include <cassert>

#include <imgui/imgui.h>
#include <range/v3/algorithm/copy.hpp>

namespace we {

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

void imgui_keymap_init(ImGuiIO& io) noexcept
{
   io.KeyMap[ImGuiKey_Tab] = static_cast<int>(keyboard_keys::tab);
   io.KeyMap[ImGuiKey_LeftArrow] = static_cast<int>(keyboard_keys::left);
   io.KeyMap[ImGuiKey_RightArrow] = static_cast<int>(keyboard_keys::right);
   io.KeyMap[ImGuiKey_UpArrow] = static_cast<int>(keyboard_keys::up);
   io.KeyMap[ImGuiKey_DownArrow] = static_cast<int>(keyboard_keys::down);
   io.KeyMap[ImGuiKey_PageUp] = static_cast<int>(keyboard_keys::prior);
   io.KeyMap[ImGuiKey_PageDown] = static_cast<int>(keyboard_keys::next);
   io.KeyMap[ImGuiKey_Home] = static_cast<int>(keyboard_keys::home);
   io.KeyMap[ImGuiKey_End] = static_cast<int>(keyboard_keys::end);
   io.KeyMap[ImGuiKey_Insert] = static_cast<int>(keyboard_keys::insert);
   io.KeyMap[ImGuiKey_Delete] = static_cast<int>(keyboard_keys::delete_);
   io.KeyMap[ImGuiKey_Backspace] = static_cast<int>(keyboard_keys::back);
   io.KeyMap[ImGuiKey_Space] = static_cast<int>(keyboard_keys::space);
   io.KeyMap[ImGuiKey_Enter] = static_cast<int>(keyboard_keys::return_);
   io.KeyMap[ImGuiKey_Escape] = static_cast<int>(keyboard_keys::escape);
   io.KeyMap[ImGuiKey_KeyPadEnter] = static_cast<int>(keyboard_keys::return_);
   io.KeyMap[ImGuiKey_A] = static_cast<int>(keyboard_keys::a);
   io.KeyMap[ImGuiKey_C] = static_cast<int>(keyboard_keys::c);
   io.KeyMap[ImGuiKey_V] = static_cast<int>(keyboard_keys::v);
   io.KeyMap[ImGuiKey_X] = static_cast<int>(keyboard_keys::x);
   io.KeyMap[ImGuiKey_Y] = static_cast<int>(keyboard_keys::y);
   io.KeyMap[ImGuiKey_Z] = static_cast<int>(keyboard_keys::z);
}

void imgui_update_io(const mouse_state& mouse_state,
                     const keyboard_state& keyboard_state, ImGuiIO& io) noexcept
{
   io.MousePos = {static_cast<float>(mouse_state.x),
                  static_cast<float>(mouse_state.y)};
   io.MouseDown[0] = mouse_state.left_button;
   io.MouseDown[1] = mouse_state.right_button;
   io.MouseDown[2] = mouse_state.middle_button;
   io.MouseDown[3] = mouse_state.extra_buttons[0];
   io.MouseDown[4] = mouse_state.extra_buttons[1];

   io.KeyCtrl = keyboard_state[keyboard_keys::control];
   io.KeyShift = keyboard_state[keyboard_keys::shift];
   io.KeyAlt = keyboard_state[keyboard_keys::menu];

   ranges::copy(keyboard_state, io.KeysDown);
}

}
