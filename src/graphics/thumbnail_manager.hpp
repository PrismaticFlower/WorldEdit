#pragma once

#include "object_class_thumbnail.hpp"
#include "utility/implementation_storage.hpp"

#include <string_view>

namespace we::graphics {

struct thumbnail_manager {
   thumbnail_manager();

   ~thumbnail_manager();

   thumbnail_manager(const thumbnail_manager& other) noexcept = delete;
   auto operator=(const thumbnail_manager& other) noexcept
      -> thumbnail_manager& = delete;

   thumbnail_manager(thumbnail_manager&& other) noexcept = delete;
   auto operator=(thumbnail_manager&& other) noexcept -> thumbnail_manager& = delete;


   auto request_object_class_thumbnail(const std::string_view name)
      -> object_class_thumbnail;

private:
   struct impl;

   implementation_storage<impl, 1024> _impl;
};

}