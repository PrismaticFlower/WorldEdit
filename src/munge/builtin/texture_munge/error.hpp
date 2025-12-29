#pragma once

#include <stdexcept>

namespace we::munge {

enum class texture_ec {
   tga_load_fail,
   tga_load_unexpected_format,
   tga_load_too_large_2d,
   tga_load_too_large_cube,
   tga_load_too_large_volume,
   tga_load_bad_cube_source_aspect_ratio,
   tga_load_bad_volume_height,
   tga_load_bad_volume_depth,

   option_load_bad_maps,
   option_load_bad_border_color,
   option_load_bad_border_alpha,
   option_load_ambiguous_type,
   option_load_bad_saturation,
   option_load_bad_depth,
   option_load_bad_bumpscale,
   option_load_bad_format,
   option_load_bad_detailbias,

   generate_mipmaps_volume_non_pow2,
   generate_normal_maps_volume,

   format_select_compressed_non_pow2,
   format_select_compressed_alpha_non_pow2,
   format_select_dxt1_non_pow2,
   format_select_dxt3_non_pow2,
   format_select_dxt5_non_pow2,

   format_convert_not_enough_memory,

   write_io_open_error,
   write_io_generic_error,
};

/// @brief Indicates an error munging a texture.
class texture_error : public std::runtime_error {
public:
   /// @brief Construct an texture_error from a message and error code.
   /// @param message The message.
   /// @param code The error code.
   texture_error(const std::string& message, texture_ec ec) noexcept
      : std::runtime_error{message}, _code{ec} {};

   /// @brief Construct an texture_error from a message and error code.
   /// @param message The message.
   /// @param code The error code.
   texture_error(const char* message, texture_ec ec) noexcept
      : std::runtime_error{message}, _code{ec} {};

   /// @brief Gets the error code for the error.
   /// @return The error code.
   auto code() const noexcept -> texture_ec
   {
      return _code;
   }

private:
   texture_ec _code;
};

auto get_descriptive_message(const texture_error& e) noexcept -> std::string;

auto get_unknown_format_warning() noexcept -> std::string_view;

}