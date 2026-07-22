#include "add_tree_line_border_odf.hpp"

#include "world/object_class_library.hpp"

namespace we::edits {

namespace {

struct add_tree_line_border_odf final : edit<world::edit_context> {
   add_tree_line_border_odf(std::vector<world::tree_line_odf>* odfs,
                            world::tree_line_odf new_odf,
                            world::object_class_library& object_class_library)
      : _odfs{odfs},
        _new_odf{std::move(new_odf)},
        _object_class_library{object_class_library}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(_odfs));

      _odfs->push_back(std::move(_new_odf));
      _odfs->back().handle =
         _object_class_library.acquire(lowercase_string{_odfs->back().name});
   }

   void revert(world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(_odfs));

      _object_class_library.free(_odfs->back().handle);
      _new_odf = std::move(_odfs->back());
      _odfs->pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   std::vector<world::tree_line_odf>* _odfs;
   world::tree_line_odf _new_odf;
   world::object_class_library& _object_class_library;
};

}

auto make_add_tree_line_border_odf(std::vector<world::tree_line_odf>* odfs,
                                   world::tree_line_odf new_odf,
                                   world::object_class_library& object_class_library)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_tree_line_border_odf>(odfs, std::move(new_odf),
                                                     object_class_library);
}

}
