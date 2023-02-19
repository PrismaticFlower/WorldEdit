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
   virtual void apply(edit_target& target) const noexcept = 0;

   /// @brief Revert the edit from the target.
   /// @param world The target of the edit.
   virtual void revert(edit_target& target) const noexcept = 0;
};

}
