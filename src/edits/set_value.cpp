#include "set_value.hpp"

namespace we::edits {

namespace {

struct set_sector_position final : edit<world::edit_context> {
   set_sector_position(std::vector<float2>* points_address, const uint32 point_index,
                       float2 new_point_position, float* base_address, float new_base)
      : set_point{points_address, point_index, 0, new_point_position},
        set_base{base_address, new_base}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      set_point.apply(context);
      set_base.apply(context);
   }

   void revert(world::edit_context& context) noexcept override
   {
      set_point.revert(context);
      set_base.revert(context);
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_sector_position* other =
         dynamic_cast<const set_sector_position*>(&other_unknown);

      if (not other) return false;

      return other->set_point.is_coalescable(this->set_point) and
             other->set_base.is_coalescable(this->set_base);
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_sector_position& other = dynamic_cast<set_sector_position&>(other_unknown);

      set_point.coalesce(other.set_point);
      set_base.coalesce(other.set_base);
   }

   set_vector_value<float2, float2> set_point;
   set_memory_value<float> set_base;
};

struct set_path_node_property_value final : edit<world::edit_context> {
   set_path_node_property_value(std::vector<world::path::node>* nodes,
                                const we::uint32 node_index,
                                const we::uint32 property_index, std::string new_value)
      : nodes{nodes},
        node_index{node_index},
        property_index{property_index},
        value{std::move(new_value)}
   {
   }

   void apply([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(nodes));
      assert(node_index < nodes->size());

      std::swap((*nodes)[node_index].properties[property_index].value, value);
   }

   void revert([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(nodes));
      assert(node_index < nodes->size());

      std::swap((*nodes)[node_index].properties[property_index].value, value);
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_path_node_property_value* other =
         dynamic_cast<const set_path_node_property_value*>(&other_unknown);

      if (not other) return false;

      return this->nodes == other->nodes and this->node_index == other->node_index and
             this->property_index == other->property_index;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_path_node_property_value& other =
         dynamic_cast<set_path_node_property_value&>(other_unknown);

      value = std::move(other.value);
   }

private:
   std::vector<world::path::node>* nodes;
   we::uint32 node_index;
   we::uint32 property_index;
   std::string value;
};

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

auto make_set_sector_position(std::vector<float2>* points_address,
                              const uint32 point_index, float2 new_point_position,
                              float* base_address, float new_base)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_sector_position>(points_address, point_index,
                                                new_point_position,
                                                base_address, new_base);
}

auto make_set_path_node_property_value(std::vector<world::path::node>* nodes,
                                       const we::uint32 node_index,
                                       const we::uint32 property_index,
                                       std::string new_value)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_path_node_property_value>(nodes, node_index,
                                                         property_index,
                                                         std::move(new_value));
}

auto make_set_creation_path_node_location(quaternion new_rotation, float3 new_position,
                                          float3 new_euler_rotation)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_creation_path_node_location>(new_rotation, new_position,
                                                            new_euler_rotation);
}

}