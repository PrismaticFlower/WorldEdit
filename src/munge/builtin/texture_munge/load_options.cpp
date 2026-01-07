#include "load_options.hpp"
#include "error.hpp"

#include "assets/option_file.hpp"

#include "io/error.hpp"
#include "io/read_file.hpp"

#include "utility/string_icompare.hpp"

#include <charconv>
#include <optional>

using we::string::iequals;

namespace we::munge {

namespace {

auto open_options(const io::path& input_file_path) -> assets::options
{
   const io::path options_path =
      io::compose_path(input_file_path.parent_path(),
                       input_file_path.filename(), ".option");

   try {
      return assets::parse_options(io::read_file_to_string(options_path));
   }
   catch (io::error&) {
      return {};
   }
}

auto parse_border_color(std::string_view arg) noexcept -> std::optional<uint32>
{
   if (string::istarts_with(arg, "0x")) arg.remove_prefix(2);
   if (string::istarts_with(arg, "#")) arg.remove_prefix(1);

   if (arg.size() != 6 and arg.size() != 8) return std::nullopt;

   uint32 color = 0;

   if (std::from_chars_result result =
          std::from_chars(arg.data(), arg.data() + arg.size(), color, 0x10);
       result.ptr != arg.data() + arg.size() or result.ec != std::errc{}) {
      return std::nullopt;
   }

   return color | 0xff'00'00'00u;
}

auto read_format(std::string_view arg) noexcept -> texture_format
{
   if (iequals(arg, "detail")) {
      return texture_format::meta_detail;
   }
   else if (iequals(arg, "terrain_detail")) {
      return texture_format::meta_detail;
   }
   else if (iequals(arg, "bump")) {
      return texture_format::meta_bump;
   }
   else if (iequals(arg, "terrain_bump")) {
      return texture_format::meta_bump;
   }
   else if (iequals(arg, "bump_alpha")) {
      return texture_format::meta_bump_alpha;
   }
   else if (iequals(arg, "terrain_bump_alpha")) {
      return texture_format::meta_bump_alpha;
   }
   else if (iequals(arg, "compressed")) {
      return texture_format::meta_compressed;
   }
   else if (iequals(arg, "compressed_alpha")) {
      return texture_format::meta_compressed_alpha;
   }
   else if (iequals(arg, "dxt1")) {
      return texture_format::dxt1;
   }
   else if (iequals(arg, "dxt3")) {
      return texture_format::dxt3;
   }
   else if (iequals(arg, "dxt5")) {
      return texture_format::dxt5;
   }
   else if (iequals(arg, "a8r8g8b8")) {
      return texture_format::a8r8g8b8;
   }
   else if (iequals(arg, "x8r8g8b8")) {
      return texture_format::x8r8g8b8;
   }
   else if (iequals(arg, "a4r4g4b4")) {
      return texture_format::a4r4g4b4;
   }
   else if (iequals(arg, "a1r5g5b5")) {
      return texture_format::a1r5g5b5;
   }
   else if (iequals(arg, "r5g6b5")) {
      return texture_format::r5g6b5;
   }
   else if (iequals(arg, "x1r5g5b5")) {
      return texture_format::x1r5g5b5;
   }
   else if (iequals(arg, "a8l8")) {
      return texture_format::a8l8;
   }
   else if (iequals(arg, "a8")) {
      return texture_format::a8;
   }
   else if (iequals(arg, "l8")) {
      return texture_format::l8;
   }
   else if (iequals(arg, "a4l4")) {
      return texture_format::a4l4;
   }
   else if (iequals(arg, "v8u8")) {
      return texture_format::v8u8;
   }

   // If adding to this also update the OPTION_LOAD_BAD_FORMAT error message and
   // the return of get_unknown_format_warning().

   return texture_format::unknown;
}

void read_option(const assets::option& option, texture_munge_options& out)
{
   if (iequals(option.name, "-maps")) {
      if (option.arguments.empty() or
          std::from_chars(option.arguments[0].data(),
                          option.arguments[0].data() + option.arguments[0].size(),
                          out.mip_levels)
                .ec != std::errc{}) {
         throw texture_error{"Invalid -maps option.", texture_ec::option_load_bad_maps};
      }
   }
   else if (iequals(option.name, "-bordercolor")) {
      if (option.arguments.empty()) {
         throw texture_error{"Invalid -bordercolor option.",
                             texture_ec::option_load_bad_border_color};
      }

      const std::optional<uint32> border_color =
         parse_border_color(option.arguments[0]);

      if (not border_color) {
         throw texture_error{"Invalid -bordercolor option.",
                             texture_ec::option_load_bad_border_color};
      }

      out.u_border = true;
      out.v_border = true;

      out.u_border_color = *border_color;
      out.v_border_color = *border_color;
   }
   else if (iequals(option.name, "-ubordercolor")) {
      if (option.arguments.empty()) {
         throw texture_error{"Invalid -ubordercolor option.",
                             texture_ec::option_load_bad_border_color};
      }

      const std::optional<uint32> border_color =
         parse_border_color(option.arguments[0]);

      if (not border_color) {
         throw texture_error{"Invalid -ubordercolor option.",
                             texture_ec::option_load_bad_border_color};
      }

      out.u_border = true;
      out.u_border_color = *border_color;
   }
   else if (iequals(option.name, "-vbordercolor")) {
      if (option.arguments.empty()) {
         throw texture_error{"Invalid -vbordercolor option.",
                             texture_ec::option_load_bad_border_color};
      }

      const std::optional<uint32> border_color =
         parse_border_color(option.arguments[0]);

      if (not border_color) {
         throw texture_error{"Invalid -vbordercolor option.",
                             texture_ec::option_load_bad_border_color};
      }

      out.v_border = true;
      out.v_border_color = *border_color;
   }
   else if (iequals(option.name, "-borderalpha")) {
      uint32 border_alpha = 0xff;

      if (option.arguments.empty() or
          std::from_chars(option.arguments[0].data(),
                          option.arguments[0].data() + option.arguments[0].size(),
                          border_alpha, 0x10)
                .ec != std::errc{}) {
         throw texture_error{"Invalid -borderalpha option.",
                             texture_ec::option_load_bad_border_alpha};
      }

      out.u_border_alpha = true;
      out.v_border_alpha = true;
      out.u_border_alpha_value = border_alpha;
      out.v_border_alpha_value = border_alpha;
   }
   else if (iequals(option.name, "-uborderalpha")) {
      uint32 border_alpha = 0xff;

      if (option.arguments.empty() or
          std::from_chars(option.arguments[0].data(),
                          option.arguments[0].data() + option.arguments[0].size(),
                          border_alpha, 0x10)
                .ec != std::errc{}) {
         throw texture_error{"Invalid -uborderalpha option.",
                             texture_ec::option_load_bad_border_alpha};
      }

      out.u_border_alpha = true;
      out.u_border_alpha_value = border_alpha;
   }
   else if (iequals(option.name, "-vborderalpha")) {
      uint32 border_alpha = 0xff;

      if (option.arguments.empty() or
          std::from_chars(option.arguments[0].data(),
                          option.arguments[0].data() + option.arguments[0].size(),
                          border_alpha, 0x10)
                .ec != std::errc{}) {
         throw texture_error{"Invalid -vborderalpha option.",
                             texture_ec::option_load_bad_border_alpha};
      }

      out.v_border_alpha = true;
      out.v_border_alpha_value = border_alpha;
   }
   else if (iequals(option.name, "-saturation")) {
      float saturation = 0.5f;

      if (option.arguments.empty() or
          std::from_chars(option.arguments[0].data(),
                          option.arguments[0].data() + option.arguments[0].size(), saturation)
                .ec != std::errc{}) {
         throw texture_error{"Invalid -saturation option.",
                             texture_ec::option_load_bad_saturation};
      }

      out.saturation = std::min(std::max(saturation, 0.0f), 1.0f);
   }
   else if (iequals(option.name, "-cubemap")) {
      if (out.type == texture_type::volume) {
         throw texture_error{"Supplying both -cubemap and -volume options.",
                             texture_ec::option_load_ambiguous_type};
      }

      out.type = texture_type::cube;
   }
   else if (iequals(option.name, "-volume")) {
      if (out.type == texture_type::volume) {
         throw texture_error{"Supplying both -cubemap and -volume options.",
                             texture_ec::option_load_ambiguous_type};
      }

      out.type = texture_type::volume;
   }
   else if (iequals(option.name, "-depth")) {
      if (option.arguments.empty() or
          std::from_chars(option.arguments[0].data(),
                          option.arguments[0].data() + option.arguments[0].size(),
                          out.depth)
                .ec != std::errc{}) {
         throw texture_error{"Invalid -depth option.", texture_ec::option_load_bad_depth};
      }
   }
   else if (iequals(option.name, "-bumpmap")) {
      out.bump_map = bump_map::normal;
   }
   else if (iequals(option.name, "-hiqbumpmap")) {
      out.bump_map = bump_map::highq;
   }
   else if (iequals(option.name, "-overridenzto1")) {
      out.override_Z_to_1 = true;
   }
   else if (iequals(option.name, "-bumpscale")) {
      if (option.arguments.empty() or
          std::from_chars(option.arguments[0].data(),
                          option.arguments[0].data() + option.arguments[0].size(),
                          out.bump_scale)
                .ec != std::errc{}) {
         throw texture_error{"Invalid -bumpscale option.",
                             texture_ec::option_load_bad_bumpscale};
      }
   }
   else if (iequals(option.name, "-format") or
            iequals(option.name, "-forceformat")) {
      if (option.arguments.empty()) {
         throw texture_error{"Invalid -format|-forceformat option.",
                             texture_ec::option_load_bad_format};
      }

      out.format = read_format(option.arguments[0]);
   }
   else if (iequals(option.name, "-detailbias")) {
      if (option.arguments.empty() or
          std::from_chars(option.arguments[0].data(),
                          option.arguments[0].data() + option.arguments[0].size(),
                          out.detail_bias)
                .ec != std::errc{}) {
         throw texture_error{"Invalid -detailbias option.",
                             texture_ec::option_load_bad_detailbias};
      }
   }
}

auto read_options(const std::vector<assets::option>& file_options,
                  const std::vector<assets::option>& folder_options) -> texture_munge_options
{
   texture_munge_options out;

   for (const std::span<const assets::option>& options :
        {std::span<const assets::option>{folder_options},
         std::span<const assets::option>{file_options}}) {
      for (const assets::option& option : options) {
         read_option(option, out);
      }
   }

   return out;
}
}

auto load_texture_options(const io::path& texture_path,
                          const std::vector<assets::option>& folder_options)
   -> texture_munge_options
{
   return read_options(open_options(texture_path), folder_options);
}

}