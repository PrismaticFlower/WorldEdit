#pragma once

#include "edit.hpp"

#include <memory>
#include <vector>

namespace we::edits {

struct stack_init {
   /// @brief Reverse size for the applied and reverted stacks.
   const std::size_t reserve_size = 8192;
};

struct apply_flags {
   /// @brief Mark the edit as closed. (Not coalescable.)
   bool closed = false;

   /// @brief Mark the edit as transparent. (Not directly performed by the user.)
   bool transparent = false;
};

/// @brief Encapsulates and manages Undo and Redo stacks.
/// @tparam T The type that the edits targets.
template<typename T>
struct stack {
   using edit_target = T;
   using edit_type = edit<T>;

   stack() : stack{stack_init{}} {}

   stack(const stack_init params) noexcept
   {
      _applied.reserve(params.reserve_size);
      _reverted.reserve(params.reserve_size);
   }

   /// @brief Apply an edit and push it onto the stack. Clears anything in the reverted stack.
   /// @param edit The edit.
   /// @param target The target of the edit.
   /// @param flags The flags for the edit
   void apply(std::unique_ptr<edit_type> edit, edit_target& target,
              const apply_flags flags = {}) noexcept
   {
      if (not _applied.empty() and                   //
          not _applied.back()->is_closed() and       //
          _applied.back()->is_coalescable(*edit) and //
          not edit->is_closed()) {
         _applied.back()->revert(target);
         _applied.back()->coalesce(*edit);
         _applied.back()->apply(target);
      }
      else {
         edit->apply(target);

         _applied.push_back(std::move(edit));
      }

      _reverted.clear();

      if (flags.closed) _applied.back()->close();
      if (flags.transparent) _applied.back()->mark_transparent();
   }

   /// @brief Revert an edit. Does nothing if there is no edit to revert
   /// @param target The target of the edit.
   void revert(edit_target& target) noexcept
   {
      revert(1, target);
   }

   /// @brief Reapply a reverted edit. Does nothing if there is no edit to reapply.
   /// @param target The target of the edit.
   void reapply(edit_target& target) noexcept
   {
      reapply(1, target);
   }

   /// @brief Reverts a number of edits. Does nothing if there is no edit to revert.
   /// @param count The number of edits to revert.
   /// @param target The target of the edits.
   void revert(const std::size_t count, edit_target& target) noexcept
   {
      for (std::size_t i = 0; i < count; ++i) {
         while (not _applied.empty() and _applied.back()->is_transparent()) {
            std::unique_ptr<edit_type>& transparent_edit = _applied.back();

            transparent_edit->revert(target);

            _reverted.push_back(std::move(transparent_edit));
            _applied.pop_back();
         }

         if (_applied.empty()) break;

         std::unique_ptr<edit_type>& edit = _applied.back();

         edit->revert(target);

         _reverted.push_back(std::move(edit));
         _applied.pop_back();
      }

      if (not _applied.empty()) _applied.back()->close();
   }

   /// @brief Reapplies a number of edits. Does nothing if there is no edit to reapply.
   /// @param count The number of edits to revert.
   /// @param target The target of the edits.
   void reapply(const std::size_t count, edit_target& target) noexcept
   {
      for (std::size_t i = 0; i < count; ++i) {
         if (_reverted.empty()) break;

         std::unique_ptr<edit_type>& edit = _reverted.back();

         edit->apply(target);

         _applied.push_back(std::move(edit));
         _reverted.pop_back();

         while (not _reverted.empty() and _reverted.back()->is_transparent()) {
            std::unique_ptr<edit_type>& transparent_edit = _reverted.back();

            transparent_edit->apply(target);

            _applied.push_back(std::move(transparent_edit));
            _reverted.pop_back();
         }
      }
   }

   /// @brief Revert all edits.
   /// @param target The target of the edits.
   void revert_all(edit_target& target) noexcept
   {
      revert(_applied.size(), target);
   }

   /// @brief Reapply all edits.
   /// @param target The target of the edits.
   void reapply_all(edit_target& target) noexcept
   {
      reapply(_reverted.size(), target);
   }

   /// @brief Number of edits in the applied stack.
   auto applied_size() const noexcept -> std::size_t
   {
      return _applied.size();
   }

   /// @brief Number of edits in the reverted stack.
   auto reverted_size() const noexcept -> std::size_t
   {
      return _reverted.size();
   }

   /// @brief Check if the applied stack is empty.
   bool applied_empty() const noexcept
   {
      return _applied.empty();
   }

   /// @brief Check if the reverted stack is empty.
   bool reverted_empty() const noexcept
   {
      return _reverted.empty();
   }

   /// @brief Call close() on the edit at the top of the applied stack, if there is one. Else does nothing.
   void close_last() noexcept
   {
      if (not _applied.empty()) _applied.back()->close();
   }

   /// @brief Clear both the applied and reverted stacks while keeping their allocated memory.
   void clear() noexcept
   {
      _applied.clear();
      _reverted.clear();
   }

private:
   std::vector<std::unique_ptr<edit_type>> _applied;
   std::vector<std::unique_ptr<edit_type>> _reverted;
};

}