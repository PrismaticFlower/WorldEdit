
#include "input_state.hpp"

#include <array>

#include <Windows.h>

namespace sk {

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