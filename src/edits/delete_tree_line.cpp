#include "delete_tree_line.hpp"

#include "world/object_class_library.hpp"

namespace we::edits {

namespace {

struct delete_tree_line final : edit<world::edit_context> {
   delete_tree_line(uint32 index, world::object_class_library& object_class_library)
      : _index{index}, _object_class_library{object_class_library}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      assert(_index < context.world.tree_lines.size());

      for (world::tree_line_odf& odf : context.world.tree_lines[_index].border_odfs) {
         _object_class_library.free(odf.handle);
      }

      _tree_line = std::move(context.world.tree_lines[_index]);

      context.world.tree_lines.erase(context.world.tree_lines.begin() + _index);
   }

   void revert(world::edit_context& context) noexcept override
   {
      assert(_index <= context.world.tree_lines.size());

      context.world.tree_lines.insert(context.world.tree_lines.begin() + _index,
                                      std::move(_tree_line));

      for (world::tree_line_odf& odf : context.world.tree_lines[_index].border_odfs) {
         odf.handle = _object_class_library.acquire(lowercase_string{odf.name});
      }
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   uint32 _index;
   world::tree_line _tree_line;
   world::object_class_library& _object_class_library;
};

}

auto make_delete_tree_line(uint32 index, world::object_class_library& object_class_library)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<delete_tree_line>(index, object_class_library);
}

}