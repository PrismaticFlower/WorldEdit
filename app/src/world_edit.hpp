
#include "assets/asset_libraries.hpp"
#include "graphics/camera.hpp"
#include "graphics/renderer.hpp"
#include "input_state.hpp"
#include "output_stream.hpp"
#include "world/object_class.hpp"
#include "world/world.hpp"

#include <chrono>
#include <filesystem>
#include <unordered_map>

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
   void update_object_classes();

   void update_camera(const float delta_time, const mouse_state& mouse_state,
                      const keyboard_state& keyboard_state);

   standard_output_stream _stream;
   HWND _window{};

   std::chrono::steady_clock::time_point _last_update =
      std::chrono::steady_clock::now();

   // TODO: Decide on project dir handling.
   std::filesystem::path _project_dir = L"D:/BF2_ModTools/data_SPT";

   assets::libraries_manager _asset_libraries{_stream};
   std::unordered_map<std::string, world::object_class> _object_classes;
   world::world _world;

   graphics::renderer _renderer;
   graphics::camera _camera;
};

}