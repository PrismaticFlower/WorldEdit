
#include "creation_entity_set.hpp"

namespace we::edits {

namespace {

struct creation_entity_set final : edit<world::edit_context> {
   creation_entity_set(world::creation_entity new_creation_entity)
      : creation_entity{std::move(new_creation_entity)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      std::swap(context.creation_entity, creation_entity);
   }

   void revert(world::edit_context& context) noexcept override
   {
      std::swap(context.creation_entity, creation_entity);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   world::creation_entity creation_entity;
};

}

auto make_creation_entity_set(world::creation_entity new_creation_entity)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<creation_entity_set>(std::move(new_creation_entity));
}

}