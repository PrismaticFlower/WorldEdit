#include "bundle.hpp"

namespace we::edits {

struct bundle : edit<world::edit_context> {
   bundle(bundle_vector edits) : edits{std::move(edits)} {}

   void apply(world::edit_context& target) noexcept
   {
      for (auto& edit : edits) {
         edit->apply(target);
      }
   }

   void revert(world::edit_context& target) noexcept
   {
      for (std::ptrdiff_t i = (std::ssize(edits) - 1); i >= 0; --i) {
         edits[i]->revert(target);
      }
   }

   bool is_coalescable(const edit<world::edit_context>& other_unknown) const noexcept
   {
      const bundle* other = dynamic_cast<const bundle*>(&other_unknown);

      if (not other) return false;
      if (this->edits.size() != other->edits.size()) return false;

      for (std::size_t i = 0; i < this->edits.size(); ++i) {
         if (not this->edits[i]->is_coalescable(*other->edits[i])) {
            return false;
         }
      }

      return true;
   }

   void coalesce(edit<world::edit_context>& other_unknown) noexcept
   {
      bundle& other = dynamic_cast<bundle&>(other_unknown);

      for (std::size_t i = 0; i < this->edits.size(); ++i) {
         this->edits[i]->coalesce(*other.edits[i]);
      }
   }

   bundle_vector edits;
};

auto make_bundle(bundle_vector edits) -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<bundle>(std::move(edits));
}

}
