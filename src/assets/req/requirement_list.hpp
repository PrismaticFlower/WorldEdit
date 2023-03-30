#pragma once

#include <string>
#include <vector>

namespace we::assets::req {

enum class platform { all, pc, xbox, ps2 };

struct requirement_list {
   std::string file_type;
   platform platform = platform::all;
   int alignment = 0;

   std::vector<std::string> entries;
};

}
