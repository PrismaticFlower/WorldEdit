#pragma once

#include "custom_mesh_description.hpp"

#include "types.hpp"

#include "utility/implementation_storage.hpp"

#include <span>

namespace we::world {

// Forward declaration for block_custom_mesh to avoid pulling custom_mesh.hpp into world.hpp
struct block_custom_mesh;

enum class block_custom_mesh_handle : uint32 {};

struct blocks_custom_mesh_library {
   enum class event_type {
      /// @brief The library has been cleared.
      cleared,
      /// @brief A mesh has been added to the library.
      mesh_added,
      /// @brief A mesh has been removed from the library.
      mesh_removed,
   };

   struct event {
      /// @brief The type of the event.
      event_type type;

      /// @brief Handle of the added mesh for event_type::new_mesh and event_type::mesh_removed.
      /// For event_type::mesh_removed the handle is no longer valid.
      block_custom_mesh_handle handle = {};

      bool operator==(const event&) const noexcept = default;
   };

   /// @brief Gets events cataloging changes to the library.
   /// @return The events for the current frame.
   auto events() const noexcept -> std::span<const event>;

   /// @brief Clear the event queue.
   void clear_events() noexcept;

   /// @brief Issue events needed to restore the GPU copy of the library.
   void issue_restore_events() noexcept;

   /// @brief Gets a mesh from a handle.
   /// @param handle The handle to the mesh.
   /// @return A const reference to the mesh or a reference to an empty mesh.
   auto operator[](const block_custom_mesh_handle handle) const noexcept
      -> const block_custom_mesh&;

   /// @brief Create and add a stairway mesh to the library.
   /// @param mesh The description of the custom mesh to add.
   /// @return The mesh handle. (Must be passed to blocks_custom_mesh_library::remove when you are done with it.)
   auto add(const block_custom_mesh_description& mesh) noexcept
      -> block_custom_mesh_handle;

   /// @brief Remove a mesh from the library.
   /// @param handle A handle to the mesh to remove.
   void remove(const block_custom_mesh_handle handle) noexcept;

   /// @brief Gets the current size of the library's pool. Used to duplicate the library for the GPU.
   /// @return The current size of the pool.
   auto pool_size() const noexcept -> uint32;

   /// @brief Converts a handle to it's pool index.
   /// @return The index of the handle.
   static auto unpack_pool_index(const block_custom_mesh_handle handle) noexcept
      -> uint32;

   /// @brief Returns the null handle. When passed to operator[] this will return an empty mesh. Passing the null handle to remove is a no-op.
   /// @return The null handle.
   static auto null_handle() noexcept -> block_custom_mesh_handle;

   /// @brief Gets the number of references to custom mesh. For testing and debugging.
   /// @return The ref count.
   auto debug_ref_count(const block_custom_mesh_description& mesh,
                        bool exclude_event_queue_ref = true) const noexcept -> uint32;

   /// @brief Gets a handle to a mesh without increasing it's ref count. For testing and debugging.
   /// @return The handle or the null handle.
   auto debug_query_handle(const block_custom_mesh_description& mesh) const noexcept
      -> block_custom_mesh_handle;

   blocks_custom_mesh_library();

   blocks_custom_mesh_library(const blocks_custom_mesh_library&);
   auto operator=(const blocks_custom_mesh_library&) noexcept
      -> blocks_custom_mesh_library&;

   blocks_custom_mesh_library(blocks_custom_mesh_library&&) noexcept;
   auto operator=(blocks_custom_mesh_library&&) noexcept
      -> blocks_custom_mesh_library&;

   ~blocks_custom_mesh_library();

private:
   struct impl;

   implementation_storage<impl, 128> _impl;
};

}