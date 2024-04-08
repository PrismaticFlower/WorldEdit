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

   void update() noexcept;

   /// @brief Clear the library. All handles become invalid after this and there is no need to call `free` for them.
   void clear() noexcept;

   auto operator[](const object_class_handle handle) const noexcept
      -> const object_class&;

   /// @brief Acquire a class handle for the class name or a handle to the default class if the max class count has been reached.
   /// @param name The name of the class to get a handle for.
   /// @return The handle. This must always be passed to free when you are done with it.
   [[nodiscard]] auto acquire(const lowercase_string& name) noexcept
      -> object_class_handle;

   /// @brief Free a class handle.
   /// @param handle The handle to free.
   void free(const object_class_handle handle) noexcept;

   /// @brief Gets the number of references to a class. For testing and debugging.
   /// @return The ref count.
   auto debug_ref_count(const lowercase_string& name) const noexcept -> uint32;

private:
   struct impl;

   implementation_storage<impl, 320> _impl;
};

}