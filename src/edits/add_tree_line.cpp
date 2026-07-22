#include "add_tree_line.hpp"

#include "world/object_class_library.hpp"

namespace we::edits {

namespace {

struct add_tree_line final : edit<world::edit_context> {
   add_tree_line(world::tree_line tree_line,
                 world::object_class_library& object_class_library)
      : _tree_line{std::move(tree_line)}, _object_class_library{object_class_library}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      context.world.tree_lines.push_back(std::move(_tree_line));

      for (world::tree_line_odf& odf : context.world.tree_lines.back().border_odfs) {
         odf.handle = _object_class_library.acquire(lowercase_string{odf.name});
      }
   }

   void revert(world::edit_context& context) noexcept override
   {
      for (world::tree_line_odf& odf : context.world.tree_lines.back().border_odfs) {
         _object_class_library.free(odf.handle);
      }

      _tree_line = std::move(context.world.tree_lines.back());

      context.world.tree_lines.pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   world::tree_line _tree_line;
   world::object_class_library& _object_class_library;
};

}

auto make_add_tree_line(world::tree_line tree_line,
                        world::object_class_library& object_class_library)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_tree_line>(std::move(tree_line), object_class_library);
}

}
