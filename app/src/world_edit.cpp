
#include "world_edit.hpp"
#include "hresult_error.hpp"

#include <stdexcept>
#include <type_traits>

namespace sk {

world_edit::world_edit(const HWND window) : _window{window}, _renderer{window}
{
}

bool world_edit::update()
{
   _renderer.draw_frame();

   return true;
}

void world_edit::resized(int width, int height)
{
   _renderer.window_resized(width, height);
}

void world_edit::focused() {}

void world_edit::unfocused() {}

}