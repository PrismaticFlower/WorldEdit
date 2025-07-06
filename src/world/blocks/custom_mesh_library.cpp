#include "custom_mesh_library.hpp"
#include "mesh_generate.hpp"

#include "container/pinned_vector.hpp"

#include <cassert>

#include <absl/container/flat_hash_map.h>

namespace we::world {

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

template<typename H>
static H AbslHashValue(H h, const block_custom_mesh_description& desc)
{
   switch (desc.type) {
   case block_custom_mesh_type::stairway:
      return H::combine(std::move(h), desc.type, desc.stairway.size.x,
                        desc.stairway.size.y, desc.stairway.size.z,
                        desc.stairway.step_height, desc.stairway.first_step_offset);
   case block_custom_mesh_type::ring:
      return H::combine(std::move(h), desc.type, desc.ring.inner_radius,
                        desc.ring.outer_radius, desc.ring.height, desc.ring.segments,
                        desc.ring.flat_shading, desc.ring.texture_loops);
   case block_custom_mesh_type::beveled_box:
      return H::combine(std::move(h), desc.type, desc.beveled_box.size.x,
                        desc.beveled_box.size.y, desc.beveled_box.size.z,
                        desc.beveled_box.amount, desc.beveled_box.bevel_top,
                        desc.beveled_box.bevel_sides, desc.beveled_box.bevel_bottom);
   case block_custom_mesh_type::curve:
      return H::combine(std::move(h), desc.type, desc.curve.width, desc.curve.height,
                        desc.curve.segments, desc.curve.texture_loops,
                        desc.curve.p0.x, desc.curve.p0.y, desc.curve.p0.z,
                        desc.curve.p1.x, desc.curve.p1.y, desc.curve.p1.z,
                        desc.curve.p2.x, desc.curve.p2.y, desc.curve.p2.z,
                        desc.curve.p3.x, desc.curve.p3.y, desc.curve.p3.z);
   }

   std::unreachable();
}

namespace {

constexpr uint32 max_custom_meshes = 1'048'576;

auto generate_mesh(const block_custom_mesh_description& mesh) noexcept -> block_custom_mesh
{
   switch (mesh.type) {
   case block_custom_mesh_type::stairway:
      return generate_mesh(mesh.stairway);
   case block_custom_mesh_type::ring:
      return generate_mesh(mesh.ring);
   case block_custom_mesh_type::beveled_box:
      return generate_mesh(mesh.beveled_box);
   case block_custom_mesh_type::curve:
      return generate_mesh(mesh.curve);
   }

   std::unreachable();
}

auto pack_pool_index(const uint32 index) noexcept -> block_custom_mesh_handle
{
   return block_custom_mesh_handle{index + 1};
}

auto unpack_pool_index(const block_custom_mesh_handle handle) noexcept -> uint32
{
   return std::to_underlying(handle) - 1;
}

const block_custom_mesh empty_custom_mesh;

}

struct blocks_custom_mesh_library::impl {
   auto events() const noexcept -> std::span<const event>
   {
      return _events;
   }

   void clear_events() noexcept
   {
      for (const event& event : _events) {
         if (event.type != event_type::mesh_added) continue;

         const uint32 index = unpack_pool_index(event.handle);

         assert(index < _mesh_pool.size());
         assert(_mesh_pool[index].ref_count != 0);

         _mesh_pool[index].ref_count -= 1;
         _mesh_pool[index].in_event_queue = false;

         if (_mesh_pool[index].ref_count == 0) {
            _mesh_index.erase(_mesh_pool[index].desc);
            _mesh_pool[index] = {};
            _mesh_free_list.push_back(index);
         }
      }

      _events.clear();
   }

   void issue_restore_events() noexcept
   {
      clear_events();

      _events.push_back({event_type::cleared});

      for (uint32 index = 0; index < _mesh_pool.size(); ++index) {
         if (_mesh_pool[index].ref_count == 0) continue;

         _mesh_pool[index].ref_count += 1;
         _mesh_pool[index].in_event_queue = true;

         _events.push_back({event_type::mesh_added, pack_pool_index(index)});
      }
   }

   auto operator[](const block_custom_mesh_handle handle) const noexcept
      -> const block_custom_mesh&
   {
      if (handle == null_handle()) return empty_custom_mesh;

      const uint32 index = unpack_pool_index(handle);

      assert(index < _mesh_pool.size());

      return _mesh_pool[index].mesh;
   }

   auto add(const block_custom_mesh_description& mesh) noexcept -> block_custom_mesh_handle
   {
      if (auto existing = _mesh_index.find(mesh); existing != _mesh_index.end()) {
         entry& entry = _mesh_pool[existing->second];

         entry.ref_count += 1;

         assert(entry.ref_count != 0);

         return pack_pool_index(existing->second);
      }

      uint32 index = 0;

      if (not _mesh_free_list.empty()) {
         index = _mesh_free_list.back();
         _mesh_free_list.pop_back();
      }
      else {
         index = static_cast<uint32>(_mesh_pool.size());
         _mesh_pool.push_back({});
      }

      _mesh_pool[index] = {generate_mesh(mesh), 2, true, mesh};

      _mesh_index.emplace(mesh, index);
      _events.push_back({event_type::mesh_added, pack_pool_index(index)});

      return pack_pool_index(index);
   }

   void remove(const block_custom_mesh_handle handle) noexcept
   {
      if (handle == null_handle()) return;

      const uint32 index = unpack_pool_index(handle);

      assert(index < _mesh_pool.size());

      entry& entry = _mesh_pool[index];

      assert(entry.ref_count != 0);

      entry.ref_count -= 1;

      if (entry.ref_count == 0) {
         _mesh_index.erase(entry.desc);
         _mesh_pool[index] = {};
         _mesh_free_list.push_back(index);
      }

      if (entry.ref_count == 0 or (entry.in_event_queue and entry.ref_count == 1)) {
         _events.push_back({event_type::mesh_removed, pack_pool_index(index)});
      }
   }

   auto pool_size() const noexcept -> uint32
   {
      return static_cast<uint32>(_mesh_pool.size());
   }

   auto debug_ref_count(const block_custom_mesh_description& mesh,
                        bool exclude_event_queue_ref) const noexcept -> uint32
   {
      if (auto existing = _mesh_index.find(mesh); existing != _mesh_index.end()) {
         const uint32 ref_count = _mesh_pool[existing->second].ref_count;

         if (exclude_event_queue_ref and _mesh_pool[existing->second].in_event_queue) {
            return ref_count - 1;
         }

         return ref_count;
      }

      return 0;
   }

   auto debug_query_handle(const block_custom_mesh_description& mesh) const noexcept
      -> block_custom_mesh_handle
   {
      if (auto existing = _mesh_index.find(mesh); existing != _mesh_index.end()) {
         return pack_pool_index(existing->second);
      }

      return null_handle();
   }

private:
   struct entry {
      block_custom_mesh mesh;
      uint32 ref_count = 0;
      bool in_event_queue = false;
      block_custom_mesh_description desc;
   };

   pinned_vector<entry> _mesh_pool =
      pinned_vector_init{.max_size = max_custom_meshes, .initial_capacity = 1024};
   std::vector<uint32> _mesh_free_list;
   absl::flat_hash_map<block_custom_mesh_description, uint32> _mesh_index;

   std::vector<event> _events;
};

auto blocks_custom_mesh_library::events() const noexcept -> std::span<const event>
{
   return _impl->events();
}

void blocks_custom_mesh_library::clear_events() noexcept
{
   return _impl->clear_events();
}

void blocks_custom_mesh_library::issue_restore_events() noexcept
{
   return _impl->issue_restore_events();
}

auto blocks_custom_mesh_library::operator[](const block_custom_mesh_handle handle) const noexcept
   -> const block_custom_mesh&
{
   return _impl.get()[handle];
}

auto blocks_custom_mesh_library::add(const block_custom_mesh_description& stairway) noexcept
   -> block_custom_mesh_handle
{
   return _impl->add(stairway);
}

void blocks_custom_mesh_library::remove(const block_custom_mesh_handle handle) noexcept
{
   return _impl->remove(handle);
}

auto blocks_custom_mesh_library::pool_size() const noexcept -> uint32
{
   return _impl->pool_size();
}

auto blocks_custom_mesh_library::unpack_pool_index(const block_custom_mesh_handle handle) noexcept
   -> uint32
{
   return world::unpack_pool_index(handle);
}

auto blocks_custom_mesh_library::null_handle() noexcept -> block_custom_mesh_handle
{
   return {};
}

auto blocks_custom_mesh_library::debug_ref_count(const block_custom_mesh_description& mesh,
                                                 bool exclude_event_queue_ref) const noexcept
   -> uint32
{
   return _impl->debug_ref_count(mesh, exclude_event_queue_ref);
}

auto blocks_custom_mesh_library::debug_query_handle(
   const block_custom_mesh_description& mesh) const noexcept -> block_custom_mesh_handle
{
   return _impl->debug_query_handle(mesh);
}

blocks_custom_mesh_library::blocks_custom_mesh_library() = default;

blocks_custom_mesh_library::blocks_custom_mesh_library(const blocks_custom_mesh_library&) = default;

auto blocks_custom_mesh_library::operator=(const blocks_custom_mesh_library&) noexcept
   -> blocks_custom_mesh_library& = default;

blocks_custom_mesh_library::blocks_custom_mesh_library(
   blocks_custom_mesh_library&&) noexcept = default;

auto blocks_custom_mesh_library::operator=(blocks_custom_mesh_library&&) noexcept
   -> blocks_custom_mesh_library& = default;

blocks_custom_mesh_library::~blocks_custom_mesh_library() = default;

}