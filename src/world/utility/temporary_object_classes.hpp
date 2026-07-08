#pragma once

#include "../object_class_library.hpp"

#include <vector>

namespace we::world {

struct temporary_object_classes {
   /// @brief Add an object class to the temporary list. A handle to it will be held as long as this is called each frame.
   /// @param name The name of the object class
   /// @param object_classes The object class library.
   void add(const lowercase_string& name, object_class_library& object_classes);

   /// @brief Update the temporary object classes store. Freeing unused classes.
   /// @param object_classes The object class library.
   void update(object_class_library& object_classes);

   /// @brief Clear the temporary object classes store. Doesn't call free on the handles.
   void clear();

private:
   std::vector<object_class_handle> _front;
   std::vector<object_class_handle> _back;
};

}
