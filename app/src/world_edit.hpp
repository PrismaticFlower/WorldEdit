
#include "graphics/camera.hpp"
#include "graphics/renderer.hpp"
#include "input_state.hpp"
#include "world/world.hpp"

#include <chrono>
#include <filesystem>

#include <Windows.h>

namespace sk {

class world_edit {
public:
   explicit world_edit(const HWND window);

   bool update();

   void resized(int width, int height);

   void focused();

   void unfocused();

private:
   void update_camera(const float delta_time, const mouse_state& mouse_state,
                      const keyboard_state& keyboard_state);

   HWND _window{};

   std::chrono::steady_clock::time_point _last_update =
      std::chrono::steady_clock::now();

   // TODO: Decide on project dir handling.
   std::filesystem::path _project_dir = L"D:/BF2_ModTools/data_SPT";

   world::world _world;

   graphics::renderer _renderer;
   graphics::camera _camera;
};

}