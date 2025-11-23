#include "io.hpp"

#include "io/output_file.hpp"
#include "utility/string_ops.hpp"

#include <charconv>
#include <stdexcept>

#include <fmt/core.h>

using namespace std::literals;

namespace we::assets::req {

namespace {

constexpr auto count_reqn(std::string_view str) noexcept -> std::size_t
{
   std::size_t count = 0;
   std::size_t position = 0;

   while (true) {

      position = str.find("REQN"sv);

      if (position == std::string_view::npos) return count;

      str.remove_prefix(position + "REQN"sv.size());

      ++count;
   }
}

[[noreturn]] void throw_parse_error(const int line, std::string_view message)
{
   throw std::runtime_error{fmt::format("Error in .req on line #{}! {}", line, message)};
}

static_assert(count_reqn("REQN REQN REQN") == 3);
static_assert(count_reqn("NQEN") == 0);

}

auto read(std::string_view req_string) -> std::vector<requirement_list>
{
   std::vector<requirement_list> list;

   list.reserve(count_reqn(req_string));

   bool ucft_declared = false;
   bool in_ucft = false;
   bool section_declared = false;
   bool in_section = false;

   requirement_list current_section;

   for (const string::line line : string::lines_iterator{req_string}) {
      std::string_view str =
         string::split_first_of_exclusive(line.string, "//"sv)[0];
      str = string::trim_whitespace(str);

      if (str.empty()) continue;

      if (in_section) {
         if (str == "}"sv) {
            if (not current_section.file_type.empty()) {
               list.emplace_back(std::move(current_section));
            }

            current_section = {};

            in_section = false;

            continue;
         }

         std::optional quoted_str = string::quoted_read(str).and_then(
            [](std::optional<std::array<std::string_view, 2>> quoted) {
               return quoted ? std::optional{(*quoted)[0]} : std::nullopt;
            });

         if (not quoted_str) {
            throw_parse_error(line.number, "Expected entry!");
         }

         if (quoted_str->empty()) continue;

         str = *quoted_str;

         auto [prop, value] = string::split_first_of_exclusive(str, "="sv);

         if (not value.empty()) {
            if (prop == "platform"sv) {
               if (value == "pc"sv) {
                  current_section.platform = platform::pc;
               }
               else if (value == "xbox"sv) {
                  current_section.platform = platform::xbox;
               }
               else if (value == "ps2"sv) {
                  current_section.platform = platform::ps2;
               }
               else {
                  throw_parse_error(line.number,
                                    fmt::format("Unknown platform '{}'!", value));
               }
            }
            else if (prop == "align"sv) {
               if (std::from_chars(value.data(), value.data() + value.size(),
                                   current_section.alignment)
                      .ec != std::errc{}) {
                  throw_parse_error(
                     line.number,
                     fmt::format("Expected number for align but got '{}'!", value));
               }
            }
            else {
               throw_parse_error(
                  line.number,
                  fmt::format("Unknown property. Expected 'platform' or "
                              "'align' but got '{}'!",
                              value));
            }
         }
         else {
            if (current_section.file_type.empty()) {
               current_section.file_type = str;
            }
            else {
               current_section.entries.emplace_back(str);
            }
         }
      }
      else if (section_declared) {
         if (str == "{"sv) {
            in_section = true;
            section_declared = false;
         }
         else {
            throw_parse_error(
               line.number, fmt::format("Expected '{{' after 'REQN' got '{}'!", str));
         }
      }
      else if (in_ucft) {
         if (str == "REQN"sv) {
            section_declared = true;
         }
         else if (str == "}"sv) {
            in_ucft = false;

            break;
         }
         else {
            throw_parse_error(line.number,
                              fmt::format("Expected '}}' or 'REQN' got '{}'!", str));
         }
      }
      else if (ucft_declared) {
         if (str == "{"sv) {
            in_ucft = true;
            ucft_declared = false;
         }
         else {
            throw_parse_error(
               line.number, fmt::format("Expected '{{' after 'ucft' got '{}'!", str));
         }
      }
      else if (str == "ucft"sv) {
         ucft_declared = true;
      }
   }

   if (in_ucft) {
      throw std::runtime_error("Expected } to close 'ucft' section!");
   }
   if (in_section) {
      throw std::runtime_error("Expected } to close last 'REQN' section!");
   }

   return list;
}

void save(const io::path& path, const std::span<const requirement_list> requirements)
{
   io::output_file out{path};

   out.write_ln("ucft");
   out.write_ln("{");

   for (const auto& list : requirements) {
      out.write_ln("\tREQN");
      out.write_ln("\t{");

      out.write_ln("\t\t\"{}\"", list.file_type);

      // clang-format off
      if (list.platform == platform::pc) out.write_ln("\t\t\"platform=pc\"");
      if (list.platform == platform::xbox) out.write_ln("\t\t\"platform=xbox\"");
      if (list.platform == platform::ps2) out.write_ln("\t\t\"platform=ps2\"");
      // clang-format on

      if (list.alignment != 0) out.write_ln("\t\t\"align={}\"", list.alignment);

      for (auto& entry : list.entries) out.write_ln("\t\t\"{}\"", entry);

      out.write_ln("\t}");
   }

   out.write_ln("}");
}

}