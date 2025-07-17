#include "custom_mesh_bvh_library.hpp"
#include "bvh.hpp"
#include "custom_mesh.hpp"

#include "async/thread_pool.hpp"
#include "container/pinned_vector.hpp"

namespace we::world {

namespace {

const block_bvh empty_bvh;

}

struct blocks_custom_mesh_bvh_library::impl {
   void update(const blocks_custom_mesh_library& library,
               async::thread_pool& thread_pool) noexcept
   {
      if (_bvh_pool.size() < library.pool_size()) {
         _bvh_pool.resize(library.pool_size());
      }

      for (const world::blocks_custom_mesh_library::event event : library.events()) {
         using event_type = blocks_custom_mesh_library::event_type;

         switch (event.type) {
         case event_type::cleared: {
            for (entry& entry : _bvh_pool) {
               if (entry.build_task.valid()) entry.build_task.cancel_no_wait();

               entry = {};
            }

            _build_list.clear();
         } break;
         case event_type::mesh_added: {
            const uint32 mesh_index =
               blocks_custom_mesh_library::unpack_pool_index(event.handle);

            const world::block_custom_mesh& mesh = library[event.handle];

            std::vector<float3> vertices;
            std::vector<std::array<uint16, 3>> triangles = mesh.triangles;

            vertices.resize(mesh.vertices.size());

            for (std::size_t i = 0; i < vertices.size(); ++i) {
               vertices[i] = mesh.vertices[i].position;
            }

            _bvh_pool[mesh_index] =
               entry{.build_task = thread_pool.exec(
                        [vertices = std::move(vertices),
                         triangles = std::move(triangles)]() -> block_bvh {
                           return {std::move(triangles), std::move(vertices)};
                        })};
            _build_list.push_back(mesh_index);
         } break;
         case event_type::mesh_removed: {
            const uint32 mesh_index =
               blocks_custom_mesh_library::unpack_pool_index(event.handle);

            if (mesh_index >= _bvh_pool.size()) continue;

            entry& entry = _bvh_pool[mesh_index];

            if (entry.build_task.valid()) {
               entry.build_task.cancel_no_wait();

               erase(_build_list, mesh_index);
            }

            entry = {};
         } break;
         }
      }

      for (auto build_it = _build_list.begin(); build_it != _build_list.end();) {
         const uint32 mesh_index = *build_it;

         if (_bvh_pool[mesh_index].build_task.ready()) {
            _bvh_pool[mesh_index].bvh = _bvh_pool[mesh_index].build_task.get();
            _bvh_pool[mesh_index].build_task = {};

            build_it = _build_list.erase(build_it);
         }
         else {
            ++build_it;
         }
      }
   }

   auto operator[](const block_custom_mesh_handle handle) const noexcept
      -> const block_bvh&
   {
      if (handle == blocks_custom_mesh_library::null_handle()) return empty_bvh;

      const uint32 index = blocks_custom_mesh_library::unpack_pool_index(handle);

      if (index >= _bvh_pool.size()) return empty_bvh;

      return _bvh_pool[index].bvh;
   }

private:
   struct entry {
      block_bvh bvh;
      async::task<block_bvh> build_task;
   };

   pinned_vector<entry> _bvh_pool =
      pinned_vector_init{.max_size = blocks_custom_mesh_library::max_custom_meshes,
                         .initial_capacity = 1024};

   pinned_vector<uint32> _build_list =
      pinned_vector_init{.max_size = blocks_custom_mesh_library::max_custom_meshes,
                         .initial_capacity = 1024};
};

void blocks_custom_mesh_bvh_library::update(const blocks_custom_mesh_library& library,
                                            async::thread_pool& thread_pool) noexcept
{
   return _impl->update(library, thread_pool);
}

auto blocks_custom_mesh_bvh_library::operator[](const block_custom_mesh_handle handle) const noexcept
   -> const block_bvh&
{
   return _impl->operator[](handle);
}

blocks_custom_mesh_bvh_library::blocks_custom_mesh_bvh_library() = default;

blocks_custom_mesh_bvh_library::blocks_custom_mesh_bvh_library(
   blocks_custom_mesh_bvh_library&&) noexcept = default;

auto blocks_custom_mesh_bvh_library::operator=(blocks_custom_mesh_bvh_library&&) noexcept
   -> blocks_custom_mesh_bvh_library& = default;

blocks_custom_mesh_bvh_library::~blocks_custom_mesh_bvh_library() = default;
}