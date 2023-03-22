#pragma once

#include "container/paged_stack.hpp"
#include "edit.hpp"

#include <memory>

namespace we::edits {

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

   /// @brief Apply an edit and push it onto the stack. Clears anything in the reverted stack.
   /// @param edit The edit.
   /// @param target The target of the edit.
   /// @param flags The flags for the edit
   void apply(std::unique_ptr<edit_type> edit, edit_target& target,
              const apply_flags flags = {}) noexcept
   {
      if (not _applied.empty() and                  //
          not _applied.top()->is_closed() and       //
          _applied.top()->is_coalescable(*edit) and //
          not edit->is_closed()) {
         _applied.top()->revert(target);
         _applied.top()->coalesce(*edit);
         _applied.top()->apply(target);
      }
      else {
         edit->apply(target);

         _applied.push(std::move(edit));
      }

      _reverted.clear();

      if (flags.closed) _applied.top()->close();
      if (flags.transparent) _applied.top()->mark_transparent();
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
         while (not _applied.empty() and _applied.top()->is_transparent()) {
            std::unique_ptr<edit_type>& transparent_edit = _applied.top();

            transparent_edit->revert(target);

            _reverted.push(std::move(transparent_edit));
            _applied.pop();
         }

         if (_applied.empty()) break;

         std::unique_ptr<edit_type>& edit = _applied.top();

         edit->revert(target);

         _reverted.push(std::move(edit));
         _applied.pop();
      }

      if (not _applied.empty()) _applied.top()->close();
   }

   /// @brief Reapplies a number of edits. Does nothing if there is no edit to reapply.
   /// @param count The number of edits to revert.
   /// @param target The target of the edits.
   void reapply(const std::size_t count, edit_target& target) noexcept
   {
      for (std::size_t i = 0; i < count; ++i) {
         if (_reverted.empty()) break;

         std::unique_ptr<edit_type>& edit = _reverted.top();

         edit->apply(target);

         _applied.push(std::move(edit));
         _reverted.pop();

         while (not _reverted.empty() and _reverted.top()->is_transparent()) {
            std::unique_ptr<edit_type>& transparent_edit = _reverted.top();

            transparent_edit->apply(target);

            _applied.push(std::move(transparent_edit));
            _reverted.pop();
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
      if (not _applied.empty()) _applied.top()->close();
   }

   /// @brief Clear both the applied and reverted stacks while keeping their allocated memory.
   void clear() noexcept
   {
      _applied.clear();
      _reverted.clear();
   }

private:
   container::paged_stack<std::unique_ptr<edit_type>, 8192> _applied;
   container::paged_stack<std::unique_ptr<edit_type>, 8192> _reverted;
};

}