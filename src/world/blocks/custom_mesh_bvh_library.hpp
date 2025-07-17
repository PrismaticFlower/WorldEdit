#pragma once

#include "custom_mesh_library.hpp"

#include "utility/implementation_storage.hpp"

namespace we::async {

class thread_pool;

}

namespace we::world {

struct block_bvh;

/// @brief Separate library for BVHs which are too expensive to build on the main thread.
struct blocks_custom_mesh_bvh_library {
   /// @brief Update the BVH library from the main library. The main library's events mustn't have been cleared before calling this.
   /// @param library The main custom mesh library to mirror.
   /// @param thread_pool Thread pool to use to build the BVHs.
   void update(const blocks_custom_mesh_library& library,
               async::thread_pool& thread_pool) noexcept;

   /// @brief Gets a BVH from a handle.
   /// @param handle The handle to the mesh.
   /// @return A const reference to the mesh's BVH or a reference to an empty BVH.
   auto operator[](const block_custom_mesh_handle handle) const noexcept
      -> const block_bvh&;

   blocks_custom_mesh_bvh_library();

   blocks_custom_mesh_bvh_library(blocks_custom_mesh_bvh_library&&) noexcept;
   auto operator=(blocks_custom_mesh_bvh_library&&) noexcept
      -> blocks_custom_mesh_bvh_library&;

   ~blocks_custom_mesh_bvh_library();

private:
   struct impl;

   implementation_storage<impl, 64> _impl;
};

}