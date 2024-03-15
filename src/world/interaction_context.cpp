#include "interaction_context.hpp"
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
   if (std::holds_alternative<path_id_node_mask>(entity)) {
      for (selected_entity& selected : _selection) {
         if (not std::holds_alternative<path_id_node_mask>(selected)) continue;

         const path_id_node_mask& entity_path = std::get<path_id_node_mask>(entity);
         path_id_node_mask& selected_path = std::get<path_id_node_mask>(selected);

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
   if (std::holds_alternative<path_id_node_mask>(entity)) {
      for (auto it = _selection.begin(); it != _selection.end(); ++it) {
         if (not std::holds_alternative<path_id_node_mask>(*it)) continue;

         const path_id_node_mask& entity_path = std::get<path_id_node_mask>(entity);
         path_id_node_mask& selected_path = std::get<path_id_node_mask>(*it);

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
      if (std::holds_alternative<path_id_node_mask>(entity) and
          std::holds_alternative<path_id_node_mask>(selected)) {
         const path_id_node_mask& entity_path = std::get<path_id_node_mask>(entity);
         const path_id_node_mask& selected_path =
            std::get<path_id_node_mask>(selected);

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
      if (not std::holds_alternative<path_id_node_mask>(selected)) continue;

      if (std::get<path_id_node_mask>(selected).id == entity) return true;
   }

   return false;
}

bool is_selected(const path_id path, const uint32 node_index,
                 const selection& selection) noexcept
{
   if (node_index >= max_path_nodes) return false;

   for (const auto& selected : selection) {
      if (not std::holds_alternative<path_id_node_mask>(selected)) continue;

      const auto& [id, nodes_mask] = std::get<path_id_node_mask>(selected);

      if (id == path) return nodes_mask[node_index];
   }

   return false;
}

bool is_valid(const interaction_target entity, const world& world) noexcept
{
   if (std::holds_alternative<object_id>(entity)) {
      return find_entity(world.objects, std::get<object_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<light_id>(entity)) {
      return find_entity(world.lights, std::get<light_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<path_id_node_mask>(entity)) {
      const auto& [id, nodes_mask] = std::get<path_id_node_mask>(entity);

      const path* path = find_entity(world.paths, id);

      if (not path) return false;

      for (std::ptrdiff_t i = std::ssize(nodes_mask) - 1;
           i >= std::ssize(path->nodes); --i) {
         if (nodes_mask[i]) return false;
      }

      return true;
   }
   if (std::holds_alternative<region_id>(entity)) {
      return find_entity(world.regions, std::get<region_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<sector_id>(entity)) {
      return find_entity(world.sectors, std::get<sector_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<portal_id>(entity)) {
      return find_entity(world.portals, std::get<portal_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<hintnode_id>(entity)) {
      return find_entity(world.hintnodes, std::get<hintnode_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<barrier_id>(entity)) {
      return find_entity(world.barriers, std::get<barrier_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<planning_hub_id>(entity)) {
      return find_entity(world.planning_hubs, std::get<planning_hub_id>(entity)) !=
             nullptr;
   }
   if (std::holds_alternative<planning_connection_id>(entity)) {
      return find_entity(world.planning_connections,
                         std::get<planning_connection_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<boundary_id>(entity)) {
      return find_entity(world.boundaries, std::get<boundary_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<measurement_id>(entity)) {
      return find_entity(world.measurements, std::get<measurement_id>(entity)) != nullptr;
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

}