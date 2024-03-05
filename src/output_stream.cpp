
#include "output_stream.hpp"

#include <cstdio>

namespace we {

void standard_output_stream::write(const std::string_view string) noexcept
{
   std::fwrite(string.data(), string.size(), 1, stdout);
}

}
