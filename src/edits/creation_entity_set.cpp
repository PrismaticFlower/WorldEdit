
#include "creation_entity_set.hpp"

namespace we::edits {

namespace {

struct creation_entity_set final : edit<world::edit_context> {
   creation_entity_set(std::optional<world::creation_entity> new_creation_entity,
                       std::optional<world::creation_entity> old_creation_entity)
      : new_creation_entity{std::move(new_creation_entity)},
        old_creation_entity{std::move(old_creation_entity)}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      context.creation_entity = new_creation_entity;
   }

   void revert(world::edit_context& context) const noexcept override
   {
      context.creation_entity = old_creation_entity;
   }

   const std::optional<world::creation_entity> new_creation_entity;
   const std::optional<world::creation_entity> old_creation_entity;
};

}

auto make_creation_entity_set(std::optional<world::creation_entity> new_creation_entity,
                              std::optional<world::creation_entity> old_creation_entity)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<creation_entity_set>(std::move(new_creation_entity),
                                                std::move(old_creation_entity));
}

}