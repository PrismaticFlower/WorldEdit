#pragma once

namespace we::world {
struct world;
}

namespace we::actions {

/// @brief Represents an action on world state.
struct action {
   virtual ~action() = default;

   /// @brief Apply the action to the world.
   /// @param world The world.
   virtual void apply(world::world& world) noexcept = 0;

   /// @brief Revert the action from the world.
   /// @param world The world.
   virtual void revert(world::world& world) noexcept = 0;
};

}
