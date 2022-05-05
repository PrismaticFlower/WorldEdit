#pragma once

#include "action.hpp"

#include <memory>
#include <vector>

namespace we::actions {

struct stack_init {
   /// @brief Reverse size for the applied and reverted stacks.
   const std::size_t reserve_size = 8192;
};

/// @brief Encapsulates and manages Undo and Redo stacks.
class stack {
public:
   stack() : stack{stack_init{}} {};

   stack(const stack_init params) noexcept;

   /// @brief Apply an action and push it onto the stack. Clears anything in the reverted stack.
   /// @param action The action.
   /// @param world The world used with the action.
   void apply(std::unique_ptr<action> action, world::world& world) noexcept;

   /// @brief Revert an action. Does nothing if there is no action to revert.
   /// @param world The world used with the action.
   void revert(world::world& world) noexcept;

   /// @brief Reapply a reverted action. Does nothing if there is no action to reapply.
   /// @param world The world used with the action.
   void reapply(world::world& world) noexcept;

   /// @brief Reverts a number of actions. Does nothing if there is no action to revert.
   /// @param count The number of actions to revert.
   /// @param world The world used with the actions.
   void revert(const std::size_t count, world::world& world) noexcept;

   /// @brief Reapplies a number of actions. Does nothing if there is no action to reapply.
   /// @param count The number of actions to revert.
   /// @param world The world used with the actions.
   void reapply(const std::size_t count, world::world& world) noexcept;

   /// @brief Revert all actions.
   /// @param world The world used with the action.
   void revert_all(world::world& world) noexcept;

   /// @brief Reapply all actions.
   /// @param world The world used with the action.
   void reapply_all(world::world& world) noexcept;

   /// @brief Number of actions in the applied stack.
   auto applied_size() const noexcept -> std::size_t;

   /// @brief Number of actions in the reverted stack.
   auto reverted_size() const noexcept -> std::size_t;

   /// @brief Check if the applied stack is empty.
   bool applied_empty() const noexcept;

   /// @brief Check if the reverted stack is empty.
   bool reverted_empty() const noexcept;

   /// @brief Gets the pointer to the action at the top of the applied stack. If changes are made to the action drop_reverted should be called.
   /// @return The pointer to the action or nullptr if no action is on the stack.
   auto applied_top() noexcept -> action*;

private:
   std::vector<std::unique_ptr<action>> _applied;
   std::vector<std::unique_ptr<action>> _reverted;
};

}