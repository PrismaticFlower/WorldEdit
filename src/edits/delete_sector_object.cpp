#include "delete_entity.hpp"
#include "types.hpp"
#include "utility/string_icompare.hpp"
#include "world/utility/world_utilities.hpp"

#include <vector>

namespace we::edits {

namespace {

struct delete_sector_object final : edit<world::edit_context> {
   delete_sector_object(world::sector_id id, uint32 object_index, std::string object)
      : _id{id}, _object_index{object_index}, _object{std::move(object)}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      std::vector<std::string>& objects =
         world::find_entity(context.world.sectors, _id)->objects;

      objects.erase(objects.begin() + _object_index);
   }

   void revert(world::edit_context& context) const noexcept override
   {
      std::vector<std::string>& objects =
         world::find_entity(context.world.sectors, _id)->objects;

      objects.insert(objects.begin() + _object_index, _object);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::sector_id _id;
   const uint32 _object_index;
   const std::string _object;
};

}

auto make_delete_sector_object(world::sector_id sector_id, const std::size_t object_index,
                               const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const world::sector* sector = world::find_entity(world.sectors, sector_id);

   return std::make_unique<delete_sector_object>(sector_id,
                                                 static_cast<uint32>(object_index),
                                                 sector->objects[object_index]);
}

}
