#include "set_class_name.hpp"
#include "world/object_class_library.hpp"

namespace we::edits {

namespace {

struct set_class_name final : edit<world::edit_context> {
   set_class_name(world::object* object, lowercase_string new_class_name,
                  world::object_class_library& object_class_library)
      : object{object},
        class_name{std::move(new_class_name)},
        object_class_library{object_class_library}
   {
   }

   void apply([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(object));

      object_class_library.free(object->class_handle);

      std::swap(object->class_name, class_name);

      object->class_handle = object_class_library.acquire(object->class_name);
   }

   void revert([[maybe_unused]] world::edit_context& context) noexcept override
   {
      apply(context);
   }

   bool is_coalescable([[maybe_unused]] const edit& other_unknown) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other_unknown) noexcept override {}

private:
   world::object* object = nullptr;
   lowercase_string class_name;
   world::object_class_library& object_class_library;
};

}

auto make_set_class_name(world::object* object, lowercase_string new_class_name,
                         world::object_class_library& object_class_library)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_class_name>(object, std::move(new_class_name),
                                           object_class_library);
}

}