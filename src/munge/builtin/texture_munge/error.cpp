#include "error.hpp"

#include <fmt/format.h>

namespace we::munge {

namespace {

auto get_description(texture_ec ec) noexcept -> std::string_view
{
   switch (ec) {
   case texture_ec::tga_load_fail:
      return R"(TGA_LOAD_FAIL

Failed to load .tga file. This could mean the file is corrupted, is saved in a niche format or in a non-standard format.

Use an external tool (like an image editor) to validate the texture and make sure it is saved in a supported data type.

Supported .tga Data Types are:

1   -  Color-mapped images.
2   -  RGB(A) images.
3   -  Uncompressed greyscale images.
9   -  Runlength encoded color-mapped images.
10  -  Runlength encoded RGB images.
11  -  Runlength encoded greyscale images.)";
   case texture_ec::tga_load_unexpected_format:
      return R"(TGA_LOAD_UNEXPECTED_FORMAT

The .tga file was loaded successfully but it's format is not understood by WorldEdit. 

Use an external tool (like an image editor) to resave the .tga file.)";
   case texture_ec::tga_load_too_large_2d:
      return R"(TGA_LOAD_TOO_LARGE_2D

The loaded .tga file was too large to use as a 2D texture.

Use an image editor to downsample and save a lower resolution copy of the texture. Then try munging again.)";
   case texture_ec::tga_load_too_large_cube:
      return R"(TGA_LOAD_TOO_LARGE_CUBE

The loaded .tga file is to be used as a cubemap but is too large.

Use an image editor to downsample and save a lower resolution copy of the texture. Then try munging again.)";
   case texture_ec::tga_load_too_large_volume:
      return R"(TGA_LOAD_TOO_LARGE_VOLUME

The loaded .tga file is to be used as a volume texture but is too large.

Use an image editor to downsample and save a lower resolution copy of the texture. Adjust the "-depth" option in the .tga.option file as needed as well. Then try munging again.)";
   case texture_ec::tga_load_bad_cube_source_aspect_ratio:
      return R"(TGA_LOAD_BAD_CUBE_SOURCE_ASPECT_RATIO

The loaded .tga file is to be used as a cubemap but has the incorrect aspect ratio. Cubemaps must have a 4:3 aspect ratio with the following layout.

__|+Y|__|__
-X|+Z|+X|-Z
--|-Y|--|--)";
   case texture_ec::tga_load_bad_volume_height:
      return R"(TGA_LOAD_BAD_VOLUME_HEIGHT

The loaded .tga file is to be used as a volume texture but it's height is not divisible by the depth set in the .tga.option file.)";

   case texture_ec::tga_load_bad_volume_depth:
      return R"(TGA_LOAD_BAD_VOLUME_DEPTH

The requested depth for a volume texture is invalid. Either too large or 0.)";
   case texture_ec::option_load_bad_maps:
      return R"(OPTION_LOAD_BAD_MAPS

Bad "-maps" option in .tga.option file. "-maps" must be followed by an unsigned integer setting the maximum number of mipmaps to export.

i.e "-maps 2")";
   case texture_ec::option_load_bad_border_color:
      return R"(option_load_bad_border_color

Bad border color option in .tga.option file. The option must followed by a color in HTML notation or a hexadecimal number representing the color.

Examples:

"-bordercolor ff0000" to set the border to red
"-ubordercolor #00ff00" to set the left and right edges to green
"-bordercolor 0x0000ff" to set the border to blue)";
   case texture_ec::option_load_bad_border_alpha:
      return R"(OPTION_LOAD_BAD_BORDER_ALPHA

Bad border alpha option in .tga.option file. The option must followed by a hexadecimal number representing the alpha.

i.e "-borderalpha ff")";
   case texture_ec::option_load_ambiguous_type:
      return R"(OPTION_LOAD_AMBIGUOUS_TYPE

Both "-cubemap" and "-volume" have been set in the .tga.option file. Only one can be used at a time.)";
   case texture_ec::option_load_bad_saturation:
      return R"(OPTION_LOAD_BAD_SATURATION

Bad "-saturation" option in .tga.option file. "-saturation" must be followed by a number setting the saturation adjustment.

i.e "-saturation 0.75"))";
   case texture_ec::option_load_bad_depth:
      return R"(OPTION_LOAD_BAD_DEPTH

Bad "-depth" option in .tga.option file. "-depth" must be followed by an unsigned integer setting the depth of the volume map.

i.e "-depth 16")";
   case texture_ec::option_load_bad_bumpscale:
      return R"(OPTION_LOAD_BAD_BUMPSCALE

Bad "-bumpscale" option in .tga.option file. "-bumpscale" must be followed by a number setting the bump scale to use when generating the normal map.

i.e "-bumpscale 4.0")";
   case texture_ec::option_load_bad_format:
      return R"(OPTION_LOAD_BAD_FORMAT

Bad "-format" or "-forceformat" option in .tga.option file. "-format" must be followed by the name of the format to use.

i.e "-format compressed"

Valid Meta Formats:

detail
terrain_detail
bump
terrain_bump
bump_alpha
terrain_bump_alpha
compressed
compressed_alpha

Valid Explicit Formats:

dxt1
dxt3
dxt5
a8r8g8b8
x8r8g8b8
a4r4g4b4
a1r5g5b5
r5g6b5
x1r5g5b5
a8l8
a8
l8
a4l4
v8u8)";
   case texture_ec::option_load_bad_detailbias:
      return R"(OPTION_LOAD_BAD_MAPS

Bad "-detailbias" option in .tga.option file. "-detailbias" must be followed by an unsigned integer setting the detail bias of the munged texture.

i.e "-detailbias 2")";
   case texture_ec::generate_mipmaps_volume_non_pow2:
      return R"(GENERATE_MIPMAPS_VOLUME_NON_POW2

WorldEdit can not generate mipmaps non-power of 2 volume textures. Either resize the texture or add "-maps 1" to the .tga.option file.)";
   case texture_ec::generate_normal_maps_volume:
      return R"(GENERATE_NORMAL_MAPS_VOLUME

Can not generate normal maps for volume textures. This would be meaningless to the game. Remove both of "-bumpmap" and "-hiqbumpmap" from the .tga.option file.)";
   case texture_ec::format_select_compressed_non_pow2:
      return R"(FORMAT_SELECT_COMPRESSED_NON_POW2

"-format compressed" has been used on a non-power of 2 texture. This is invalid either resize the texture or use an uncompressed format.)";
   case texture_ec::format_select_compressed_alpha_non_pow2:
      return R"(FORMAT_SELECT_COMPRESSED_ALPHA_NON_POW2

"-format compressed_alpha" has been used on a non-power of 2 texture. This is invalid either resize the texture or use an uncompressed format.)";
   case texture_ec::format_select_dxt1_non_pow2:
      return R"(FORMAT_SELECT_DXT1_NON_POW2

"-format dxt1" has been used on a non-power of 2 texture. This is invalid either resize the texture or use an uncompressed format.)";
   case texture_ec::format_select_dxt3_non_pow2:
      return R"(FORMAT_SELECT_DXT3_NON_POW2

"-format dxt3" has been used on a non-power of 2 texture. This is invalid either resize the texture or use an uncompressed format.)";
   case texture_ec::format_select_dxt5_non_pow2:
      return R"(FORMAT_SELECT_DXT5_NON_POW2

"-format dxt5" has been used on a non-power of 2 texture. This is invalid either resize the texture or use an uncompressed format.)";
   case texture_ec::format_convert_not_enough_memory:
      return R"(FORMAT_CONVERT_NOT_ENOUGH_MEMORY

There was not enough memory to convert the texture's format. This is a bug in WorldEdit, please report it. Include the texture and it's .tga.option file)";
   case texture_ec::write_io_open_error:
      return R"(WRITE_IO_OPEN_ERROR

Failed to open .texture file for output! Make sure the munge out directory is accessible and that nothing currently has the output .texture file open.)";
   case texture_ec::write_io_generic_error:
      return R"(WRITE_IO_GENERIC_ERROR

An unexpected IO error has occured while writing the texture. The Exception Message below may have more information and context.)";
   }

   return "Unknown Error";
}

}

auto get_descriptive_message(const texture_error& e) noexcept -> std::string
{
   return fmt::format("{}\n\nException Message: {}", get_description(e.code()),
                      e.what());
}

auto get_unknown_format_warning() noexcept -> std::string_view
{
   return R"(Unknown format passed in through -format|-forceformat. Default will be used.

Valid Meta Formats:

detail
terrain_detail
bump
terrain_bump
bump_alpha
terrain_bump_alpha
compressed
compressed_alpha

Valid Explicit Formats:

dxt1
dxt3
dxt5
a8r8g8b8
x8r8g8b8
a4r4g4b4
a1r5g5b5
r5g6b5
x1r5g5b5
a8l8
a8
l8
a4l4
v8u8)";
}

}