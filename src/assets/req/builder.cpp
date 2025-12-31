#include "builder.hpp"

#include "utility/string_icompare.hpp"

namespace we::assets::req {

void add_to(std::vector<std::string>& list, std::string_view entry) noexcept
{
   for (const std::string& existing : list) {
      if (string::iequals(existing, entry)) return;
   }

   list.emplace_back(entry);
}

}