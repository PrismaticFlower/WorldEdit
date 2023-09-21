#include "thumbnail_manager.hpp"

#include <imgui.h>

namespace we::graphics {

struct thumbnail_manager::impl {
   auto request_object_class_thumbnail(const std::string_view name)
      -> object_class_thumbnail
   {
      (void)name;

      return {.imgui_texture_id = ImGui::GetFont()->ContainerAtlas->TexID,
              .uv_left = 0.0f,
              .uv_top = 0.0f,
              .uv_right = 1.0f,
              .uv_bottom = 1.0f};
   }

private:
};

auto thumbnail_manager::request_object_class_thumbnail(const std::string_view name)
-> object_class_thumbnail
{
   return _impl->request_object_class_thumbnail(name);
}

thumbnail_manager::thumbnail_manager() = default;

thumbnail_manager::~thumbnail_manager() = default;

}