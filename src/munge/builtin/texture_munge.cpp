#include "texture_munge.hpp"
#include "texture_munge/error.hpp"
#include "texture_munge/load_options.hpp"
#include "texture_munge/load_texture.hpp"
#include "texture_munge/texture_convert.hpp"
#include "texture_munge/texture_format.hpp"
#include "texture_munge/texture_ops.hpp"
#include "texture_munge/write_texture.hpp"

#include "executor.hpp"

#include "types.hpp"

#include "io/read_file.hpp"

#include <fmt/format.h>

namespace we::munge {

void execute_texture_munge(const tool_context& context) noexcept
{
   execute_builtin_munge({.input_extension = "tga",
                          .output_extension = "texture",
                          .tool_name = "TextureMunge",
                          .execute_munge = execute_texture_munge},
                         context);
}

void execute_texture_munge(const io::path& input_file_path,
                           const std::vector<assets::option>& folder_options,
                           const tool_context& context)
{
   context.feedback.print_output(fmt::format("Munging {}", input_file_path.filename()));

   const texture_munge_options options =
      load_texture_options(input_file_path, folder_options);

   if (options.format == texture_format::unknown) {
      context.feedback.add_warning(
         {.file = input_file_path,
          .tool = "TextureMunge",
          .message = std::string{get_unknown_format_warning()}});
   }

   load_texture_result load_result =
      load_texture(input_file_path, {
                                       .type = options.type,
                                       .mip_levels = options.mip_levels,
                                       .depth = options.depth,
                                    });

   texture& texture = load_result.texture;

   if (options.format == texture_format::meta_detail) {
      convert_to_detail_map(texture, load_result.traits.has_alpha);
   }

   if (load_result.traits.has_alpha and
       (options.format == texture_format::meta_bump or
        options.format == texture_format::meta_compressed or
        options.format == texture_format::x8r8g8b8 or
        options.format == texture_format::x1r5g5b5)) {
      make_opaque(texture);

      load_result.traits.has_alpha = false;
      load_result.traits.has_single_bit_alpha = false;
   }

   if (options.saturation != 0.5f) {
      adjust_saturation(texture, options.saturation);
   }

   if (options.mip_levels != 1) generate_mipmaps(texture);

   if (options.bump_map == bump_map::normal) {
      generate_normal_maps(texture, options.bump_scale);
   }
   else if (options.bump_map == bump_map::highq) {
      generate_normal_maps_highq(texture, options.bump_scale);
   }
   else if (options.format == texture_format::meta_bump or
            options.format == texture_format::meta_bump_alpha) {
      normalize_maps(texture);
   }

   if (options.override_Z_to_1) override_z_to_one(texture);

   if (options.u_border or options.u_border_alpha) {
      uint32 border_and_mask = 0;
      uint32 border_or_mask = 0;

      if (options.u_border) {
         border_or_mask |= options.u_border_color;
         border_and_mask |= 0xff'00'00'00;
      }

      if (options.u_border_alpha) {
         border_or_mask |= options.u_border_alpha_value << 24;
         border_and_mask |= 0x00'ff'ff'ff;
      }

      apply_u_border(texture, border_and_mask, border_or_mask);
   }

   if (options.v_border or options.v_border_alpha) {
      uint32 border_and_mask = 0;
      uint32 border_or_mask = 0;

      if (options.v_border) {
         border_or_mask |= options.v_border_color;
         border_and_mask |= 0xff'00'00'00;
      }

      if (options.v_border_alpha) {
         border_or_mask |= options.v_border_alpha_value << 24;
         border_and_mask |= 0x00'ff'ff'ff;
      }

      apply_v_border(texture, border_and_mask, border_or_mask);
   }

   const texture_write_format format =
      get_write_format(texture, options.format, load_result.traits);

   write_texture(io::compose_path(context.output_path, input_file_path.stem(), ".texture"),
                 convert_texture(texture, format),
                 {.type = options.type, .detail_bias = options.detail_bias});
}

}