
#include "output_stream.hpp"

#include <iostream>

namespace sk {

void standard_output_stream::write(const std::string_view string) noexcept
{
   std::cout << string;
}

}
