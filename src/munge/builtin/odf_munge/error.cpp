#include "error.hpp"

#include <fmt/format.h>

namespace we::munge {

namespace {

auto get_description(odf_ec ec) noexcept -> std::string_view
{
   switch (ec) {
   case odf_ec::odf_load_io_error:
      return R"(ODF_LOAD_IO_ERROR

An error occured while opening the .odf file. It may be in use by another app or you may not have permission to open it.)";
   case odf_ec::odf_load_parse_error:
      return R"(ODF_LOAD_PARSE_ERROR

Failed to parse the ODF! This is most likely because of a syntax error in the ODF. The Exception Message below will have more information.)";

   case odf_ec::req_write_io_open_error:
      return R"(REQ_WRITE_IO_OPEN_ERROR

Failed to open .class.req file for output! Make sure the munge out directory is accessible and that nothing currently has the output .class.req file open.)";
   case odf_ec::req_write_io_generic_error:
      return R"(REQ_WRITE_IO_GENERIC_ERROR

An unexpected IO error has occured while writing the ODF's .class.req file. The Exception Message below may have more information and context.)";
   case odf_ec::write_io_open_error:
      return R"(WRITE_IO_OPEN_ERROR

Failed to open .class file for output! Make sure the munge out directory is accessible and that nothing currently has the output .class file open.)";
   case odf_ec::write_io_generic_error:
      return R"(WRITE_IO_GENERIC_ERROR

An unexpected IO error has occured while writing the munged ODF. The Exception Message below may have more information and context.)";
   }

   return "Unknown Error";
}

}

auto get_descriptive_message(const odf_error& e) noexcept -> std::string
{
   return fmt::format("{}\n\nException Message: {}", get_description(e.code()),
                      e.what());
}

}