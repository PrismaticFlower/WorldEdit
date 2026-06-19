#include "add_sector_object.hpp"

namespace we::edits {

namespace {

struct add_sector_object final : edit<world::edit_context> {
   add_sector_object(std::vector<uint32>* objects, uint32 object_index)
      : _objects{objects}, _object_index{object_index}
   {
   }

   void apply([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(_objects));

      _objects->push_back(_object_index);
   }

   void revert([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(_objects));

      _objects->pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   std::vector<uint32>* _objects;
   uint32 _object_index;
};

}

auto make_add_sector_object(std::vector<uint32>* objects, uint32 object_index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_sector_object>(objects, object_index);
}

}