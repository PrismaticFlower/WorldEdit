#include "set_value.hpp"

namespace we::edits {

namespace {

struct set_creation_path_node_location final : edit<world::edit_context> {
   set_creation_path_node_location(quaternion new_rotation, quaternion original_rotation,
                                   float3 new_position, float3 original_position,
                                   float3 new_euler_rotation,
                                   float3 original_euler_rotation)
      : new_rotation{new_rotation},
        new_position{new_position},
        new_euler_rotation{new_euler_rotation},
        original_rotation{original_rotation},
        original_position{original_position},
        original_euler_rotation{original_euler_rotation}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      context.creation_entity.get<world::path>().nodes[0].rotation = new_rotation;
      context.creation_entity.get<world::path>().nodes[0].position = new_position;
      context.euler_rotation = new_euler_rotation;
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.creation_entity.get<world::path>().nodes[0].rotation = original_rotation;
      context.creation_entity.get<world::path>().nodes[0].position = original_position;
      context.euler_rotation = original_euler_rotation;
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

      new_rotation = other.new_rotation;
      new_position = other.new_position;
      new_euler_rotation = other.new_euler_rotation;
   }

   quaternion new_rotation;
   float3 new_position;
   float3 new_euler_rotation;

   quaternion original_rotation;
   float3 original_position;
   float3 original_euler_rotation;
};

struct set_creation_region_metrics final : edit<world::edit_context> {
   set_creation_region_metrics(quaternion new_rotation, quaternion original_rotation,
                               float3 new_position, float3 original_position,
                               float3 new_size, float3 original_size)
      : new_rotation{new_rotation},
        new_position{new_position},
        new_size{new_size},
        original_rotation{original_rotation},
        original_position{original_position},
        original_size{original_size}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      context.creation_entity.get<world::region>().rotation = new_rotation;
      context.creation_entity.get<world::region>().position = new_position;
      context.creation_entity.get<world::region>().size = new_size;
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.creation_entity.get<world::region>().rotation = original_rotation;
      context.creation_entity.get<world::region>().position = original_position;
      context.creation_entity.get<world::region>().size = original_size;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_creation_region_metrics* other =
         dynamic_cast<const set_creation_region_metrics*>(&other_unknown);

      return other != nullptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_creation_region_metrics& other =
         dynamic_cast<set_creation_region_metrics&>(other_unknown);

      new_rotation = other.new_rotation;
      new_position = other.new_position;
      new_size = other.new_size;
   }

   quaternion new_rotation;
   float3 new_position;
   float3 new_size;

   quaternion original_rotation;
   float3 original_position;
   float3 original_size;
};

struct set_creation_measurement_points final : edit<world::edit_context> {
   set_creation_measurement_points(float3 new_start, float3 original_start,
                                   float3 new_end, float3 original_end)
      : new_start{new_start},
        original_start{original_start},
        new_end{new_end},
        original_end{original_end}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      context.creation_entity.get<world::measurement>().start = new_start;
      context.creation_entity.get<world::measurement>().end = new_end;
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.creation_entity.get<world::measurement>().start = original_start;
      context.creation_entity.get<world::measurement>().end = original_end;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_creation_measurement_points* other =
         dynamic_cast<const set_creation_measurement_points*>(&other_unknown);

      return other != nullptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_creation_measurement_points& other =
         dynamic_cast<set_creation_measurement_points&>(other_unknown);

      new_start = other.new_start;
      new_end = other.new_end;
   }

   float3 new_start;
   float3 original_start;
   float3 new_end;
   float3 original_end;
};

}

auto make_set_creation_path_node_location(quaternion new_rotation,
                                          quaternion original_rotation,
                                          float3 new_position, float3 original_position,
                                          float3 new_euler_rotation,
                                          float3 original_euler_rotation)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_creation_path_node_location>(new_rotation,
                                                            original_rotation,
                                                            new_position,
                                                            original_position,
                                                            new_euler_rotation,
                                                            original_euler_rotation);
}

auto make_set_creation_region_metrics(quaternion new_rotation,
                                      quaternion original_rotation,
                                      float3 new_position, float3 original_position,
                                      float3 new_size, float3 original_size)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_creation_region_metrics>(new_rotation, original_rotation,
                                                        new_position, original_position,
                                                        new_size, original_size);
}

auto make_set_creation_measurement_points(float3 new_start, float3 original_start,
                                          float3 new_end, float3 original_end)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_creation_measurement_points>(new_start, original_start,
                                                            new_end, original_end);
}

}