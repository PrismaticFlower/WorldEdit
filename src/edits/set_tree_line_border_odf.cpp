#include "set_tree_line_border_odf.hpp"

#include "world/object_class_library.hpp"

namespace we::edits {

namespace {

struct set_tree_line_border_odf final : edit<world::edit_context> {
   set_tree_line_border_odf(std::vector<world::tree_line_odf>* odfs,
                            uint32 index, world::tree_line_odf new_odf,
                            world::object_class_library& object_class_library)
      : _odfs{odfs},
        _index{index},
        _odf{std::move(new_odf)},
        _object_class_library{object_class_library}
   {
   }

   void apply([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(_odfs));
      assert(_index < _odfs->size());

      _object_class_library.free((*_odfs)[_index].handle);

      std::swap((*_odfs)[_index], _odf);

      (*_odfs)[_index].handle =
         _object_class_library.acquire(lowercase_string{(*_odfs)[_index].name});
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
   std::vector<world::tree_line_odf>* _odfs;
   uint32 _index;
   world::tree_line_odf _odf;
   world::object_class_library& _object_class_library;
};

}

auto make_set_tree_line_border_odf(std::vector<world::tree_line_odf>* odfs,
                                   uint32 index, world::tree_line_odf new_odf,
                                   world::object_class_library& object_class_library)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_tree_line_border_odf>(odfs, index, std::move(new_odf),
                                                     object_class_library);
}

}