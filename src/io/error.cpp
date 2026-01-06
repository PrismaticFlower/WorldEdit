#include "error.hpp"

#include <winerror.h>

namespace we::io {

auto map_os_open_error_code(unsigned int os_code) noexcept -> open_error_code
{
   switch (os_code) {
   case ERROR_FILE_NOT_FOUND:
   case ERROR_PATH_NOT_FOUND:
      return open_error_code::file_not_found;
   case ERROR_SHARING_VIOLATION:
      return open_error_code::sharing_violation;
   default:
      return open_error_code::generic;
   }
}

}