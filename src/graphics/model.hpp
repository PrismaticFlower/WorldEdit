#pragma once

#include "bounding_box.hpp"
#include "types.hpp"

#include <boost/container/small_vector.hpp>

namespace sk::graphics {

struct model_material {
};

struct model_part {
   uint32 index_count;
   uint32 start_index;
   uint32 start_vertex;
};

struct model {
   bounding_box bbox;

   boost::container::small_vector<model_part, 4> parts;
};

}
