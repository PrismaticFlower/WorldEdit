#pragma once

namespace we::edits {

/// @brief Represents an edit.
/// @tparam T The type that the edit targets.
template<typename T>
struct edit {
   using edit_target = T;

   virtual ~edit() = default;

   /// @brief Apply the edit to the target.
   /// @param target The target of the edit.
   virtual void apply(edit_target& target) noexcept = 0;

   /// @brief Revert the edit from the target.
   /// @param world The target of the edit.
   virtual void revert(edit_target& target) noexcept = 0;

   /// @brief Determines if an edit can be coalesced with another.
   /// @param other The new edit to check.
   /// @return True if the edit can be coalesced, false if it can not.
   virtual bool is_coalescable(const edit& other) const noexcept = 0;

   /// @brief Coalesce other into this edit. When this edit is reverted the changed
   /// values should be the original's from this and when it is applied the new values should be the same as the new values from other. If is_coalescable(other) is false the a crash should be expected from the call.
   /// @param other The edit to coalesce into this. Maybe left in an invalid state after call.
   virtual void coalesce(edit& other) noexcept = 0;

   /// @brief Checks if the edit is marked as being closed and shouldn't be coalesced by the edit stack.
   /// @return If the edit is closed.
   bool is_closed() const noexcept
   {
      return _closed;
   }

   /// @brief Marks the edit as closed. Preventing the edit stack from attempting to coalesce it.
   void close() noexcept
   {
      _closed = true;
   }

   /// @brief Checks if the edit is marked as transparent. Intended for edits not
   /// directly caused by user actions. When revert/reapply is called transparent edits on top of the stack are processed in addition to the first non-transparent edit.
   /// @return If the edit is transparent.
   bool is_transparent() const noexcept
   {
      return _transparent;
   }

   /// @brief Marks the edit as transparent.
   void mark_transparent() noexcept
   {
      _transparent = true;
   }

private:
   bool _closed = false;
   bool _transparent = false;
};

}
