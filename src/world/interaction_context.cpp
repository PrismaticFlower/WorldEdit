#include "interaction_context.hpp"

#include "blocks/find.hpp"

#include "utility/world_utilities.hpp"

namespace we::world {

namespace {

struct address_range {
   template<typename T>
   static auto object(const T& object) -> address_range
   {
      return {.begin = reinterpret_cast<std::uintptr_t>(&object),
              .end = reinterpret_cast<std::uintptr_t>(&object) + sizeof(T)};
   }

   template<typename T>
   static auto container(const T& container) -> address_range
   {
      return {.begin = reinterpret_cast<std::uintptr_t>(container.data()),
              .end = reinterpret_cast<std::uintptr_t>(container.data() +
                                                      container.size())};
   }

   std::uintptr_t begin = 0;
   std::uintptr_t end = 0;
};

}

creation_entity::creation_entity(creation_entity_none_t) {}

creation_entity::creation_entity(creation_entity&& other) noexcept
{
   // clang-format off

   if  (other._active == detail::active_entity::object) new (&_storage.object) object{std::move(other._storage.object)}; 
   else if (other._active == detail::active_entity::light) new (&_storage.light) light{std::move(other._storage.light)}; 
   else if (other._active == detail::active_entity::path) new (&_storage.path) path{std::move(other._storage.path)}; 
   else if (other._active == detail::active_entity::region) new (&_storage.region) region{std::move(other._storage.region)}; 
   else if (other._active == detail::active_entity::sector) new (&_storage.sector) sector{std::move(other._storage.sector)}; 
   else if (other._active == detail::active_entity::portal) new (&_storage.portal) portal{std::move(other._storage.portal)}; 
   else if (other._active == detail::active_entity::barrier) new (&_storage.barrier) barrier{std::move(other._storage.barrier)}; 
   else if (other._active == detail::active_entity::hintnode) new (&_storage.hintnode) hintnode{std::move(other._storage.hintnode)}; 
   else if (other._active == detail::active_entity::planning_hub) new (&_storage.planning_hub) planning_hub{std::move(other._storage.planning_hub)}; 
   else if (other._active == detail::active_entity::planning_connection) new (&_storage.planning_connection) planning_connection{std::move(other._storage.planning_connection)}; 
   else if (other._active == detail::active_entity::boundary) new (&_storage.boundary) boundary{std::move(other._storage.boundary)}; 
   else if (other._active == detail::active_entity::measurement) new (&_storage.measurement) measurement{std::move(other._storage.measurement)};
   else if (other._active == detail::active_entity::entity_group) new (&_storage.entity_group) entity_group{std::move(other._storage.entity_group)};

   // clang-format on

   this->_active = other._active;

   other._storage.destroy(other._active);
   other._active = detail::active_entity::none;
}

auto creation_entity::operator=(creation_entity&& other) noexcept -> creation_entity&
{
   _storage.destroy(_active);

   // clang-format off

   if  (other._active == detail::active_entity::object) new (&_storage.object) object{std::move(other._storage.object)}; 
   else if (other._active == detail::active_entity::light) new (&_storage.light) light{std::move(other._storage.light)}; 
   else if (other._active == detail::active_entity::path) new (&_storage.path) path{std::move(other._storage.path)}; 
   else if (other._active == detail::active_entity::region) new (&_storage.region) region{std::move(other._storage.region)}; 
   else if (other._active == detail::active_entity::sector) new (&_storage.sector) sector{std::move(other._storage.sector)}; 
   else if (other._active == detail::active_entity::portal) new (&_storage.portal) portal{std::move(other._storage.portal)}; 
   else if (other._active == detail::active_entity::barrier) new (&_storage.barrier) barrier{std::move(other._storage.barrier)}; 
   else if (other._active == detail::active_entity::hintnode) new (&_storage.hintnode) hintnode{std::move(other._storage.hintnode)}; 
   else if (other._active == detail::active_entity::planning_hub) new (&_storage.planning_hub) planning_hub{std::move(other._storage.planning_hub)}; 
   else if (other._active == detail::active_entity::planning_connection) new (&_storage.planning_connection) planning_connection{std::move(other._storage.planning_connection)}; 
   else if (other._active == detail::active_entity::boundary) new (&_storage.boundary) boundary{std::move(other._storage.boundary)}; 
   else if (other._active == detail::active_entity::measurement) new (&_storage.measurement) measurement{std::move(other._storage.measurement)};
   else if (other._active == detail::active_entity::entity_group) new (&_storage.entity_group) entity_group{std::move(other._storage.entity_group)};

   // clang-format on

   this->_active = other._active;

   other._storage.destroy(other._active);
   other._active = detail::active_entity::none;

   return *this;
}

creation_entity::~creation_entity()
{
   _storage.destroy(_active);
}

void creation_entity::storage::destroy(detail::active_entity active)
{
   // clang-format off

   if (active == detail::active_entity::object) object.~object();
   else if (active == detail::active_entity::light) light.~light();
   else if (active == detail::active_entity::path) path.~path();
   else if (active == detail::active_entity::region) region.~region();
   else if (active == detail::active_entity::sector) sector.~sector();
   else if (active == detail::active_entity::portal) portal.~portal();
   else if (active == detail::active_entity::barrier) barrier.~barrier();
   else if (active == detail::active_entity::hintnode) hintnode.~hintnode();
   else if (active == detail::active_entity::planning_hub) planning_hub.~planning_hub();
   else if (active == detail::active_entity::planning_connection) planning_connection.~planning_connection();
   else if (active == detail::active_entity::boundary) boundary.~boundary();
   else if (active == detail::active_entity::measurement) measurement.~measurement();
   else if (active == detail::active_entity::entity_group) entity_group.~entity_group();

   // clang-format on
}

auto selection::begin() const noexcept -> std::vector<selected_entity>::const_iterator
{
   return _selection.begin();
}

auto selection::end() const noexcept -> std::vector<selected_entity>::const_iterator
{
   return _selection.end();
}

auto selection::view() const noexcept -> std::span<const selected_entity>
{
   return _selection;
}

auto selection::view_updatable() noexcept -> std::span<selected_entity>
{
   return _selection;
}

auto selection::operator[](const std::size_t i) const noexcept -> selected_entity
{
   assert(i < size());

   return _selection[i];
}

void selection::add(const selected_entity entity) noexcept
{
   if (entity.is<path_id_node_mask>()) {
      for (selected_entity& selected : _selection) {
         if (not selected.is<path_id_node_mask>()) continue;

         const path_id_node_mask& entity_path = entity.get<path_id_node_mask>();
         path_id_node_mask& selected_path = selected.get<path_id_node_mask>();

         if (entity_path.id != selected_path.id) continue;

         selected_path.nodes = selected_path.nodes | entity_path.nodes;

         return;
      }
   }
   else {
      for (selected_entity& selected : _selection) {
         if (selected == entity) {
            return;
         }
      }
   }

   _selection.push_back(entity);
}

void selection::remove(const selected_entity entity) noexcept
{
   if (entity.is<path_id_node_mask>()) {
      for (auto it = _selection.begin(); it != _selection.end(); ++it) {
         if (not it->is<path_id_node_mask>()) continue;

         const path_id_node_mask& entity_path = entity.get<path_id_node_mask>();
         path_id_node_mask& selected_path = it->get<path_id_node_mask>();

         if (entity_path.id != selected_path.id) continue;

         bool remaining_nodes = false;

         for (std::size_t i = 0; i < entity_path.nodes.size(); ++i) {
            if (entity_path.nodes[i]) {
               selected_path.nodes.reset(i);
            }
            else if (selected_path.nodes[i]) {
               remaining_nodes = true;
            }
         }

         if (not remaining_nodes) _selection.erase(it);

         return;
      }
   }
   else {
      for (auto it = _selection.begin(); it != _selection.end(); ++it) {
         if (*it == entity) {
            _selection.erase(it);
            return;
         }
      }
   }
}

void selection::clear() noexcept
{
   _selection.clear();
}

bool selection::empty() const noexcept
{
   return _selection.empty();
}

auto selection::size() const noexcept -> std::size_t
{
   return _selection.size();
}

bool edit_context::is_memory_valid(const void* ptr, std::size_t size) const noexcept
{
   const address_range ranges[] = {
      address_range::object(world),
      address_range::object(creation_entity),
      address_range::object(euler_rotation),
      address_range::object(light_region_euler_rotation),

      address_range::container(world.objects),
      address_range::container(world.lights),
      address_range::container(world.paths),
      address_range::container(world.regions),
      address_range::container(world.sectors),
      address_range::container(world.portals),
      address_range::container(world.hintnodes),
      address_range::container(world.barriers),
      address_range::container(world.planning_hubs),
      address_range::container(world.planning_connections),
      address_range::container(world.boundaries),
      address_range::container(world.measurements),

      address_range::container(world.animations),
      address_range::container(world.animation_groups),
      address_range::container(world.animation_hierarchies),

      address_range::container(world.blocks.boxes.hidden),
      address_range::container(world.blocks.boxes.layer),
      address_range::container(world.blocks.boxes.description),
      address_range::container(world.blocks.ramps.hidden),
      address_range::container(world.blocks.ramps.layer),
      address_range::container(world.blocks.ramps.description),
      address_range::container(world.blocks.cylinders.hidden),
      address_range::container(world.blocks.cylinders.layer),
      address_range::container(world.blocks.cylinders.description),
      address_range::container(world.blocks.quads.hidden),
      address_range::container(world.blocks.quads.layer),
      address_range::container(world.blocks.quads.description),
      address_range::container(world.blocks.stairways.hidden),
      address_range::container(world.blocks.stairways.layer),
      address_range::container(world.blocks.stairways.description),
      address_range::container(world.blocks.cones.hidden),
      address_range::container(world.blocks.cones.layer),
      address_range::container(world.blocks.cones.description),
      address_range::container(world.blocks.hemispheres.hidden),
      address_range::container(world.blocks.hemispheres.layer),
      address_range::container(world.blocks.hemispheres.description),
      address_range::container(world.blocks.pyramids.hidden),
      address_range::container(world.blocks.pyramids.layer),
      address_range::container(world.blocks.pyramids.description),
      address_range::container(world.blocks.materials),
   };

   std::uintptr_t memory_begin = reinterpret_cast<std::uintptr_t>(ptr);
   std::uintptr_t memory_end = memory_begin + size;

   for (const auto& [range_begin, range_end] : ranges) {
      if (memory_begin >= range_begin and //
          memory_begin < range_end and    //
          memory_end > range_begin and    //
          memory_end <= range_end) {
         return true;
      }
   }

   return false;
}

auto make_path_id_node_mask(path_id id, uint32 node_index) noexcept -> path_id_node_mask
{
   assert(node_index < max_path_nodes);

   path_id_node_mask node_id_mask{id};

   if (node_index < max_path_nodes) node_id_mask.nodes.set(node_index);

   return node_id_mask;
}

bool is_selected(const selected_entity entity, const selection& selection) noexcept
{
   for (const auto& selected : selection) {
      if (entity.is<path_id_node_mask>() and selected.is<path_id_node_mask>()) {
         const path_id_node_mask& entity_path = entity.get<path_id_node_mask>();
         const path_id_node_mask& selected_path = selected.get<path_id_node_mask>();

         if (entity_path.id != selected_path.id) continue;

         const path_id_node_mask::node_mask combined_nodes =
            entity_path.nodes & selected_path.nodes;

         if (combined_nodes == entity_path.nodes) return true;
      }
      else if (selected == entity) {
         return true;
      }
   }

   return false;
}

bool is_selected(const path_id entity, const selection& selection) noexcept
{
   for (const auto& selected : selection) {
      if (not selected.is<path_id_node_mask>()) continue;

      if (selected.get<path_id_node_mask>().id == entity) return true;
   }

   return false;
}

bool is_selected(const path_id path, const uint32 node_index,
                 const selection& selection) noexcept
{
   if (node_index >= max_path_nodes) return false;

   for (const auto& selected : selection) {
      if (not selected.is<path_id_node_mask>()) continue;

      const auto& [id, nodes_mask] = selected.get<path_id_node_mask>();

      if (id == path) return nodes_mask[node_index];
   }

   return false;
}

bool is_valid(const interaction_target entity, const world& world) noexcept
{
   if (entity.is<object_id>()) {
      return find_entity(world.objects, entity.get<object_id>()) != nullptr;
   }
   if (entity.is<light_id>()) {
      return find_entity(world.lights, entity.get<light_id>()) != nullptr;
   }
   if (entity.is<path_id_node_mask>()) {
      const auto& [id, nodes_mask] = entity.get<path_id_node_mask>();

      const path* path = find_entity(world.paths, id);

      if (not path) return false;

      for (std::ptrdiff_t i = std::ssize(nodes_mask) - 1;
           i >= std::ssize(path->nodes); --i) {
         if (nodes_mask[i]) return false;
      }

      return true;
   }
   if (entity.is<region_id>()) {
      return find_entity(world.regions, entity.get<region_id>()) != nullptr;
   }
   if (entity.is<sector_id>()) {
      return find_entity(world.sectors, entity.get<sector_id>()) != nullptr;
   }
   if (entity.is<portal_id>()) {
      return find_entity(world.portals, entity.get<portal_id>()) != nullptr;
   }
   if (entity.is<hintnode_id>()) {
      return find_entity(world.hintnodes, entity.get<hintnode_id>()) != nullptr;
   }
   if (entity.is<barrier_id>()) {
      return find_entity(world.barriers, entity.get<barrier_id>()) != nullptr;
   }
   if (entity.is<planning_hub_id>()) {
      return find_entity(world.planning_hubs, entity.get<planning_hub_id>()) != nullptr;
   }
   if (entity.is<planning_connection_id>()) {
      return find_entity(world.planning_connections,
                         entity.get<planning_connection_id>()) != nullptr;
   }
   if (entity.is<boundary_id>()) {
      return find_entity(world.boundaries, entity.get<boundary_id>()) != nullptr;
   }
   if (entity.is<measurement_id>()) {
      return find_entity(world.measurements, entity.get<measurement_id>()) != nullptr;
   }
   if (entity.is<block_id>()) {
      return find_block(world.blocks, entity.get<block_id>()).has_value();
   }

   return false;
}

void path_id_node_mask::node_mask::set(const std::size_t i) noexcept
{
   if (i >= max_path_nodes) return;

   const std::size_t word = i / 32;
   const std::size_t bit = i % 32;

   words[word] |= (1 << bit);
}

void path_id_node_mask::node_mask::reset(const std::size_t i) noexcept
{
   if (i >= max_path_nodes) return;

   const std::size_t word = i / 32;
   const std::size_t bit = i % 32;

   words[word] &= ~(1 << bit);
}

auto operator|(const path_id_node_mask::node_mask& l,
               const path_id_node_mask::node_mask& r) noexcept
   -> path_id_node_mask::node_mask
{
   path_id_node_mask::node_mask result;

   for (std::size_t i = 0; i < std::size(result.words); ++i) {
      result.words[i] = l.words[i] | r.words[i];
   }

   return result;
}

auto operator&(const path_id_node_mask::node_mask& l,
               const path_id_node_mask::node_mask& r) noexcept
   -> path_id_node_mask::node_mask
{
   path_id_node_mask::node_mask result;

   for (std::size_t i = 0; i < std::size(result.words); ++i) {
      result.words[i] = l.words[i] & r.words[i];
   }

   return result;
}

bool operator==(const interaction_target& l, const interaction_target& r) noexcept
{
   if (l._active != r._active) return false;

   if (l._active == detail::active_entity::path) {
      return l._storage.path == r._storage.path;
   }
   else if (l._active == detail::active_entity::block) {
      return l._storage.block == r._storage.block;
   }
   else {
      return memcmp(&l._storage, &r._storage, sizeof(object_id)) == 0;
   }
}

}