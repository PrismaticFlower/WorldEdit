#include "temporary_object_classes.hpp"

namespace we::world {

void temporary_object_classes::add(const lowercase_string& name,
                                   object_class_library& object_classes)
{
   _front.push_back(object_classes.acquire(name));
}

void temporary_object_classes::update(object_class_library& object_classes)
{
   for (world::object_class_handle handle : _back) {
      object_classes.free(handle);
   }

   _back.clear();
   _back.swap(_front);
}

void temporary_object_classes::clear()
{
   _back.clear();
   _front.clear();
}

}