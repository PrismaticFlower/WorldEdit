#include "set_value.hpp"

namespace we::edits {

namespace {

struct set_creation_path_node_location final : edit<world::edit_context> {
   set_creation_path_node_location(quaternion new_rotation, float3 new_position,
                                   float3 new_euler_rotation)
      : rotation{new_rotation}, position{new_position}, euler_rotation{new_euler_rotation}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      assert(context.creation_entity.is<world::path>());
      assert(context.creation_entity.get<world::path>().nodes.size() >= 1);

      std::swap(context.creation_entity.get<world::path>().nodes[0].rotation, rotation);
      std::swap(context.creation_entity.get<world::path>().nodes[0].position, position);
      std::swap(context.euler_rotation, euler_rotation);
   }

   void revert(world::edit_context& context) noexcept override
   {
      assert(context.creation_entity.is<world::path>());
      assert(context.creation_entity.get<world::path>().nodes.size() >= 1);

      std::swap(context.creation_entity.get<world::path>().nodes[0].rotation, rotation);
      std::swap(context.creation_entity.get<world::path>().nodes[0].position, position);
      std::swap(context.euler_rotation, euler_rotation);
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_creation_path_node_location* other =
         dynamic_cast<const set_creation_path_node_location*>(&other_unknown);

      return other != nullptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_creation_path_node_location& other =
         dynamic_cast<set_creation_path_node_location&>(other_unknown);

      rotation = other.rotation;
      position = other.position;
      euler_rotation = other.euler_rotation;
   }

   quaternion rotation;
   float3 position;
   float3 euler_rotation;
};

}

auto make_set_creation_path_node_location(quaternion new_rotation, float3 new_position,
                                          float3 new_euler_rotation)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_creation_path_node_location>(new_rotation, new_position,
                                                            new_euler_rotation);
}

}