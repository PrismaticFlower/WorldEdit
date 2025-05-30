
#include "creation_entity_set.hpp"
#include "world/object_class_library.hpp"

namespace we::edits {

namespace {

struct creation_entity_set final : edit<world::edit_context> {
   creation_entity_set(world::creation_entity new_creation_entity,
                       world::object_class_library& object_class_library)
      : creation_entity{std::move(new_creation_entity)},
        object_class_library{object_class_library}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      if (context.creation_entity.is<world::object>()) {
         world::object& object = context.creation_entity.get<world::object>();

         object_class_library.free(object.class_handle);
      }
      else if (context.creation_entity.is<world::entity_group>()) {
         world::entity_group& entity_group =
            context.creation_entity.get<world::entity_group>();

         for (world::object& object : entity_group.objects) {
            object_class_library.free(object.class_handle);
         }

         for (std::size_t i = 0;
              i < entity_group.blocks.stairways.description.size(); ++i) {
            context.world.blocks.custom_meshes.remove(
               entity_group.blocks.stairways.mesh[i]);
         }
      }

      std::swap(context.creation_entity, creation_entity);

      if (context.creation_entity.is<world::object>()) {
         world::object& object = context.creation_entity.get<world::object>();

         object.class_handle = object_class_library.acquire(object.class_name);
      }
      else if (context.creation_entity.is<world::entity_group>()) {
         world::entity_group& entity_group =
            context.creation_entity.get<world::entity_group>();

         for (world::object& object : entity_group.objects) {
            object.class_handle = object_class_library.acquire(object.class_name);
         }

         for (std::size_t i = 0;
              i < entity_group.blocks.stairways.description.size(); ++i) {
            entity_group.blocks.stairways.mesh[i] =
               context.world.blocks.custom_meshes.add(
                  entity_group.blocks.stairways.description[i].custom_mesh_desc());
         }
      }
   }

   void revert(world::edit_context& context) noexcept override
   {
      apply(context);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   world::creation_entity creation_entity;
   world::object_class_library& object_class_library;
};

}

auto make_creation_entity_set(world::creation_entity new_creation_entity,
                              world::object_class_library& object_class_library)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<creation_entity_set>(std::move(new_creation_entity),
                                                object_class_library);
}

}