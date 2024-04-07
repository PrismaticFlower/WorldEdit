#pragma once

#include "lowercase_string.hpp"
#include "object.hpp"
#include "utility/implementation_storage.hpp"

#include <span>

namespace we::assets {

struct libraries_manager;

}

namespace we::world {

struct object_class;

struct object_class_library {
   explicit object_class_library(assets::libraries_manager& asset_libraries) noexcept;

   ~object_class_library();

   object_class_library(const object_class_library&) noexcept = delete;
   auto operator=(const object_class_library&) noexcept
      -> object_class_library& = delete;

   void update(std::span<std::span<const object>> object_spans) noexcept;

   void clear() noexcept;

   [[nodiscard]] auto acquire(const lowercase_string& name) noexcept
      -> object_class_handle;

   void free(const object_class_handle handle) noexcept;

   auto operator[](const object_class_handle handle) const noexcept
      -> const object_class&;

private:
   struct impl;

   implementation_storage<impl, 320> _impl;
};

}