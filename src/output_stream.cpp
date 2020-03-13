
#include "output_stream.hpp"

#include <iostream>

namespace sk {

void standard_output_stream::write(const std::string_view string) noexcept
{
   std::cout << string;
}

auto null_output_stream::get_static_instance() noexcept -> null_output_stream&
{
   static null_output_stream null_stream;

   return null_stream;
}

}
