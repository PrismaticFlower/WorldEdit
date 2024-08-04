#pragma once

#include "world.hpp"

#include <span>
#include <vector>

namespace we::world {

namespace detail {

enum class active_entity {
   none,
   object,
   light,
   path,
   region,
   sector,
   portal,
   barrier,
   hintnode,
   planning_hub,
   planning_connection,
   boundary,
   measurement,
};

}

/// @brief A reference to a path and a set of it's nodes.
struct path_id_node_mask {
   struct node_mask {
      static auto size() noexcept -> std::size_t
      {
         return max_path_nodes;
      }

      bool operator[](const std::size_t i) const noexcept
      {
         if (i >= max_path_nodes) return false;

         const std::size_t word = i / 32;
         const std::size_t bit = i % 32;

         return words[word] & (1 << bit);
      }

      void set(const std::size_t i) noexcept;

      void reset(const std::size_t i) noexcept;

      bool operator==(const node_mask&) const noexcept = default;

      friend auto operator|(const node_mask& l, const node_mask& r) noexcept -> node_mask;

      friend auto operator&(const node_mask& l, const node_mask& r) noexcept -> node_mask;

   private:
      uint32 words[max_path_nodes / 32] = {};
   };

   path_id id;
   node_mask nodes;

   bool operator==(const path_id_node_mask&) const noexcept = default;
};

/// @brief Represents an entity being interacted with (hovered or selected).
struct interaction_target {
   interaction_target() = default;

   interaction_target(const interaction_target&) = default;

   template<typename T>
   interaction_target(const T& id) noexcept
   {
      if constexpr (std::is_same_v<std::remove_cvref_t<T>, object_id>) {
         _storage.object = id;
         _active = detail::active_entity::object;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, light_id>) {
         _storage.light = id;
         _active = detail::active_entity::light;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, path_id_node_mask>) {
         _storage.path = id;
         _active = detail::active_entity::path;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, region_id>) {
         _storage.region = id;
         _active = detail::active_entity::region;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, sector_id>) {
         _storage.sector = id;
         _active = detail::active_entity::sector;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, portal_id>) {
         _storage.portal = id;
         _active = detail::active_entity::portal;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, barrier_id>) {
         _storage.barrier = id;
         _active = detail::active_entity::barrier;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, hintnode_id>) {
         _storage.hintnode = id;
         _active = detail::active_entity::hintnode;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, planning_hub_id>) {
         _storage.planning_hub = id;
         _active = detail::active_entity::planning_hub;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, planning_connection_id>) {
         _storage.planning_connection = id;
         _active = detail::active_entity::planning_connection;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, boundary_id>) {
         _storage.boundary = id;
         _active = detail::active_entity::boundary;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, measurement_id>) {
         _storage.measurement = id;
         _active = detail::active_entity::measurement;
      }
      else {
         static_assert(not std::is_same_v<std::remove_cvref_t<T>, T>,
                       "T is not an entity ID type");
      }
   }

   template<typename T>
   [[nodiscard]] bool is() const noexcept
   {
      // clang-format off

      if constexpr (std::is_same_v<T, object_id>) return _active == detail::active_entity::object;
      else if constexpr (std::is_same_v<T, light_id>) return _active == detail::active_entity::light;
      else if constexpr (std::is_same_v<T, path_id_node_mask>) return _active == detail::active_entity::path;
      else if constexpr (std::is_same_v<T, region_id>) return _active == detail::active_entity::region;
      else if constexpr (std::is_same_v<T, sector_id>) return _active == detail::active_entity::sector;
      else if constexpr (std::is_same_v<T, portal_id>) return _active == detail::active_entity::portal;
      else if constexpr (std::is_same_v<T, barrier_id>) return _active == detail::active_entity::barrier;
      else if constexpr (std::is_same_v<T, hintnode_id>) return _active == detail::active_entity::hintnode;
      else if constexpr (std::is_same_v<T, planning_hub_id>) return _active == detail::active_entity::planning_hub;
      else if constexpr (std::is_same_v<T, planning_connection_id>) return _active == detail::active_entity::planning_connection;
      else if constexpr (std::is_same_v<T, boundary_id>) return _active == detail::active_entity::boundary;
      else if constexpr (std::is_same_v<T, measurement_id>) return _active == detail::active_entity::measurement;
      else static_assert(not std::is_same_v<T, T>, "T is not an entity ID type");

      // clang-format on
   }

   [[nodiscard]] bool holds_entity_id() const noexcept
   {
      return _active != detail::active_entity::none;
   }

   template<typename T>
   [[nodiscard]] auto get(this auto&& self) noexcept -> auto&
   {
      // clang-format off

      if constexpr (std::is_same_v<std::remove_cvref_t<T>, object_id>) { if (self._active != detail::active_entity::object) std::terminate(); return self._storage.object; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, light_id>) { if (self._active != detail::active_entity::light) std::terminate(); return self._storage.light; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, path_id_node_mask>) { if (self._active != detail::active_entity::path) std::terminate(); return self._storage.path; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, region_id>) { if (self._active != detail::active_entity::region) std::terminate(); return self._storage.region; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, sector_id>) { if (self._active != detail::active_entity::sector) std::terminate(); return self._storage.sector; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, portal_id>) { if (self._active != detail::active_entity::portal) std::terminate(); return self._storage.portal; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, barrier_id>) { if (self._active != detail::active_entity::barrier) std::terminate(); return self._storage.barrier; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, hintnode_id>) { if (self._active != detail::active_entity::hintnode) std::terminate(); return self._storage.hintnode; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, planning_hub_id>) { if (self._active != detail::active_entity::planning_hub) std::terminate(); return self._storage.planning_hub; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, planning_connection_id>) { if (self._active != detail::active_entity::planning_connection) std::terminate(); return self._storage.planning_connection; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, boundary_id>) { if (self._active != detail::active_entity::boundary) std::terminate(); return self._storage.boundary; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, measurement_id>) { if (self._active != detail::active_entity::measurement) std::terminate(); return self._storage.measurement; }
      else static_assert(not std::is_same_v<std::remove_cvref_t<T>, T>, "T is not an entity ID type");

      // clang-format on
   }

   friend bool operator==(const interaction_target& l,
                          const interaction_target& r) noexcept;

private:
   detail::active_entity _active = detail::active_entity::none;

   union storage {
      void destroy(detail::active_entity active);

      char none = '\0';
      object_id object;
      light_id light;
      path_id_node_mask path;
      region_id region;
      sector_id sector;
      portal_id portal;
      barrier_id barrier;
      hintnode_id hintnode;
      planning_hub_id planning_hub;
      planning_connection_id planning_connection;
      boundary_id boundary;
      measurement_id measurement;
   } _storage;
};

/// @brief Represents an entity being hovered over.
using hovered_entity = interaction_target;

/// @brief Represents an entity that is currently selected.
using selected_entity = interaction_target;

struct creation_entity_none_t {};

inline constexpr creation_entity_none_t creation_entity_none;

/// @brief A variant class for holding the creation entity. Guarantees address stability of the held entities to in order to enable a simpler undo system.
struct creation_entity {
   creation_entity() = default;

   creation_entity(creation_entity_none_t);

   template<typename T>
   creation_entity(const T& entity) noexcept
   {
      if constexpr (std::is_same_v<std::remove_cvref_t<T>, object>) {
         new (&_storage.object) object{entity};
         _active = detail::active_entity::object;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, light>) {
         new (&_storage.light) light{entity};
         _active = detail::active_entity::light;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, path>) {
         new (&_storage.path) path{entity};
         _active = detail::active_entity::path;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, region>) {
         new (&_storage.region) region{entity};
         _active = detail::active_entity::region;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, sector>) {
         new (&_storage.sector) sector{entity};
         _active = detail::active_entity::sector;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, portal>) {
         new (&_storage.portal) portal{entity};
         _active = detail::active_entity::portal;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, barrier>) {
         new (&_storage.barrier) barrier{entity};
         _active = detail::active_entity::barrier;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, hintnode>) {
         new (&_storage.hintnode) hintnode{entity};
         _active = detail::active_entity::hintnode;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, planning_hub>) {
         new (&_storage.planning_hub) planning_hub{entity};
         _active = detail::active_entity::planning_hub;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, planning_connection>) {
         new (&_storage.planning_connection) planning_connection{entity};
         _active = detail::active_entity::planning_connection;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, boundary>) {
         new (&_storage.boundary) boundary{entity};
         _active = detail::active_entity::boundary;
      }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, measurement>) {
         new (&_storage.measurement) measurement{entity};
         _active = detail::active_entity::measurement;
      }
      else {
         static_assert(not std::is_same_v<std::remove_cvref_t<T>, T>,
                       "T is not an entity type");
      }
   }

   template<typename T>
   creation_entity(T&& entity) noexcept
   {
      using entity_type = std::remove_cvref_t<T>;

      if constexpr (std::is_same_v<entity_type, object>) {
         new (&_storage.object) object{std::forward<T>(entity)};
         _active = detail::active_entity::object;
      }
      else if constexpr (std::is_same_v<entity_type, light>) {
         new (&_storage.light) light{std::forward<T>(entity)};
         _active = detail::active_entity::light;
      }
      else if constexpr (std::is_same_v<entity_type, path>) {
         new (&_storage.path) path{std::forward<T>(entity)};
         _active = detail::active_entity::path;
      }
      else if constexpr (std::is_same_v<entity_type, region>) {
         new (&_storage.region) region{std::forward<T>(entity)};
         _active = detail::active_entity::region;
      }
      else if constexpr (std::is_same_v<entity_type, sector>) {
         new (&_storage.sector) sector{std::forward<T>(entity)};
         _active = detail::active_entity::sector;
      }
      else if constexpr (std::is_same_v<entity_type, portal>) {
         new (&_storage.portal) portal{std::forward<T>(entity)};
         _active = detail::active_entity::portal;
      }
      else if constexpr (std::is_same_v<entity_type, barrier>) {
         new (&_storage.barrier) barrier{std::forward<T>(entity)};
         _active = detail::active_entity::barrier;
      }
      else if constexpr (std::is_same_v<entity_type, hintnode>) {
         new (&_storage.hintnode) hintnode{std::forward<T>(entity)};
         _active = detail::active_entity::hintnode;
      }
      else if constexpr (std::is_same_v<entity_type, planning_hub>) {
         new (&_storage.planning_hub) planning_hub{std::forward<T>(entity)};
         _active = detail::active_entity::planning_hub;
      }
      else if constexpr (std::is_same_v<entity_type, planning_connection>) {
         new (&_storage.planning_connection)
            planning_connection{std::forward<T>(entity)};
         _active = detail::active_entity::planning_connection;
      }
      else if constexpr (std::is_same_v<entity_type, boundary>) {
         new (&_storage.boundary) boundary{std::forward<T>(entity)};
         _active = detail::active_entity::boundary;
      }
      else if constexpr (std::is_same_v<entity_type, measurement>) {
         new (&_storage.measurement) measurement{std::forward<T>(entity)};
         _active = detail::active_entity::measurement;
      }
      else {
         static_assert(not std::is_same_v<entity_type, T>,
                       "T is not an entity type");
      }
   }

   creation_entity(creation_entity&& other) noexcept;

   auto operator=(creation_entity&& other) noexcept -> creation_entity&;

   ~creation_entity();

   template<typename T>
   [[nodiscard]] bool is() const noexcept
   {
      // clang-format off

      if constexpr (std::is_same_v<std::remove_cvref_t<T>, object>) return _active == detail::active_entity::object;
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, light>) return _active == detail::active_entity::light;
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, path>) return _active == detail::active_entity::path;
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, region>) return _active == detail::active_entity::region;
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, sector>) return _active == detail::active_entity::sector;
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, portal>) return _active == detail::active_entity::portal;
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, barrier>) return _active == detail::active_entity::barrier;
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, hintnode>) return _active == detail::active_entity::hintnode;
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, planning_hub>) return _active == detail::active_entity::planning_hub;
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, planning_connection>) return _active == detail::active_entity::planning_connection;
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, boundary>) return _active == detail::active_entity::boundary;
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, measurement>) return _active == detail::active_entity::measurement;
      else static_assert(not std::is_same_v<std::remove_cvref_t<T>, T>, "T is not an entity type");

      // clang-format on
   }

   [[nodiscard]] bool holds_entity() const noexcept
   {
      return _active != detail::active_entity::none;
   }

   template<typename T>
   [[nodiscard]] auto get(this auto&& self) noexcept -> auto&
   {
      // clang-format off

      if constexpr (std::is_same_v<std::remove_cvref_t<T>, object>) { if (self._active != detail::active_entity::object) std::terminate(); return self._storage.object; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, light>) { if (self._active != detail::active_entity::light) std::terminate(); return self._storage.light; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, path>) { if (self._active != detail::active_entity::path) std::terminate(); return self._storage.path; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, region>) { if (self._active != detail::active_entity::region) std::terminate(); return self._storage.region; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, sector>) { if (self._active != detail::active_entity::sector) std::terminate(); return self._storage.sector; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, portal>) { if (self._active != detail::active_entity::portal) std::terminate(); return self._storage.portal; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, barrier>) { if (self._active != detail::active_entity::barrier) std::terminate(); return self._storage.barrier; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, hintnode>) { if (self._active != detail::active_entity::hintnode) std::terminate(); return self._storage.hintnode; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, planning_hub>) { if (self._active != detail::active_entity::planning_hub) std::terminate(); return self._storage.planning_hub; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, planning_connection>) { if (self._active != detail::active_entity::planning_connection) std::terminate(); return self._storage.planning_connection; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, boundary>) { if (self._active != detail::active_entity::boundary) std::terminate(); return self._storage.boundary; }
      else if constexpr (std::is_same_v<std::remove_cvref_t<T>, measurement>) { if (self._active != detail::active_entity::measurement) std::terminate(); return self._storage.measurement; }
      else static_assert(not std::is_same_v<std::remove_cvref_t<T>, T>, "T is not an entity type");

      // clang-format on
   }

private:
   detail::active_entity _active = detail::active_entity::none;

   union storage {
      ~storage(){};

      void destroy(detail::active_entity active);

      char none = '\0';
      object object;
      light light;
      path path;
      region region;
      sector sector;
      portal portal;
      barrier barrier;
      hintnode hintnode;
      planning_hub planning_hub;
      planning_connection planning_connection;
      boundary boundary;
      measurement measurement;
   } _storage;
};

/// @brief Holds a selection and ensures only each entity is present at most once.
struct selection {
   auto begin() const noexcept -> std::vector<selected_entity>::const_iterator;

   auto end() const noexcept -> std::vector<selected_entity>::const_iterator;

   auto view() const noexcept -> std::span<const selected_entity>;

   auto view_updatable() noexcept -> std::span<selected_entity>;

   auto operator[](const std::size_t i) const noexcept -> selected_entity;

   void add(const selected_entity entity) noexcept;

   void remove(const selected_entity entity) noexcept;

   void clear() noexcept;

   bool empty() const noexcept;

   auto size() const noexcept -> std::size_t;

private:
   std::vector<selected_entity> _selection;
};

/// @brief Stores references to the entities currently being interacted with.
struct interaction_targets {
   std::optional<hovered_entity> hovered_entity;
   selection selection;

   creation_entity creation_entity;
};

/// @brief References to data that is managed by an edit stack.
struct edit_context {
   world& world;
   creation_entity& creation_entity;
   float3 euler_rotation;
   float3 light_region_euler_rotation;

   /// @brief Check if an address is within the edit context and won't move due to container reallocations.
   bool is_memory_valid(const void* ptr, std::size_t size) const noexcept;

   template<typename T>
   bool is_memory_valid(const T* ptr) const noexcept
   {
      return is_memory_valid(ptr, sizeof(T));
   }
};

auto make_path_id_node_mask(path_id id, uint32 node_index) noexcept -> path_id_node_mask;

bool is_selected(const path_id path, const selection& selection) noexcept;

bool is_selected(const path_id path, const uint32 node_index,
                 const selection& selection) noexcept;

bool is_selected(const interaction_target entity, const selection& selection) noexcept;

bool is_valid(const interaction_target entity, const world& world) noexcept;

}
