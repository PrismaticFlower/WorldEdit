#include "custom_mesh_library.hpp"
#include "mesh_generate.hpp"

#include "container/pinned_vector.hpp"

#include <cassert>

#include <absl/container/flat_hash_map.h>

namespace we::world {

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace {

constexpr uint32 max_custom_meshes = 1'048'576;

enum class mesh_type {
   stairway,
};

struct mesh_desc {
   mesh_desc() = default;

   mesh_desc(const block_custom_mesh_stairway_desc& desc)
      : type{mesh_type::stairway}, stairway{desc}
   {
   }

   mesh_type type = mesh_type::stairway;

   union {
      block_custom_mesh_stairway_desc stairway = {};
   };

   template<typename H>
   friend H AbslHashValue(H h, const mesh_desc& desc)
   {
      switch (desc.type) {
      case mesh_type::stairway:
         return H::combine(std::move(h), desc.type, desc.stairway.size.x,
                           desc.stairway.size.y, desc.stairway.size.z,
                           desc.stairway.step_height, desc.stairway.first_step_offset);
      }

      std::unreachable();
   }
};

bool operator==(const mesh_desc& left, const mesh_desc& right) noexcept
{
   if (left.type != right.type) return false;

   switch (left.type) {
   case mesh_type::stairway:
      return left.stairway == right.stairway;
   }

   std::unreachable();
}

auto pack_pool_index(const uint32 index) noexcept -> block_custom_mesh_handle
{
   return block_custom_mesh_handle{index};
}

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
      const uint32 index = unpack_pool_index(handle);

      assert(index < _mesh_pool.size());

      return _mesh_pool[index].mesh;
   }

   auto add(const block_custom_mesh_stairway_desc& stairway) noexcept -> block_custom_mesh_handle
   {
      if (auto existing = _mesh_index.find(stairway); existing != _mesh_index.end()) {
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

      _mesh_pool[index] = {generate_mesh(stairway), 2, true, stairway};
      _mesh_index.emplace(stairway, index);
      _events.push_back({event_type::mesh_added, pack_pool_index(index)});

      return pack_pool_index(index);
   }

   void remove(const block_custom_mesh_handle handle) noexcept
   {
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

   auto debug_ref_count(const block_custom_mesh_stairway_desc& stairway,
                        bool exclude_event_queue_ref) const noexcept -> uint32
   {
      if (auto existing = _mesh_index.find(stairway); existing != _mesh_index.end()) {
         const uint32 ref_count = _mesh_pool[existing->second].ref_count;

         if (exclude_event_queue_ref and _mesh_pool[existing->second].in_event_queue) {
            return ref_count - 1;
         }

         return ref_count;
      }

      return 0;
   }

   auto debug_query_handle(const block_custom_mesh_stairway_desc& stairway) const noexcept
      -> block_custom_mesh_handle
   {
      if (auto existing = _mesh_index.find(stairway); existing != _mesh_index.end()) {
         return pack_pool_index(existing->second);
      }

      return block_custom_mesh_handle{0xff'ff'ff'ff};
   }

private:
   struct entry {
      block_custom_mesh mesh;
      uint32 ref_count = 0;
      bool in_event_queue = false;
      mesh_desc desc;
   };

   pinned_vector<entry> _mesh_pool =
      pinned_vector_init{.max_size = max_custom_meshes, .initial_capacity = 1024};
   std::vector<uint32> _mesh_free_list;
   absl::flat_hash_map<mesh_desc, uint32> _mesh_index;

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

auto blocks_custom_mesh_library::add(const block_custom_mesh_stairway_desc& stairway) noexcept
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
   return std::to_underlying(handle);
}

auto blocks_custom_mesh_library::debug_ref_count(
   const block_custom_mesh_stairway_desc& stairway,
   bool exclude_event_queue_ref) const noexcept -> uint32
{
   return _impl->debug_ref_count(stairway, exclude_event_queue_ref);
}
auto blocks_custom_mesh_library::debug_query_handle(
   const block_custom_mesh_stairway_desc& stairway) const noexcept -> block_custom_mesh_handle
{
   return _impl->debug_query_handle(stairway);
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