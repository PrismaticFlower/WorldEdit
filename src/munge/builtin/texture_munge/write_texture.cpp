#include "write_texture.hpp"
#include "error.hpp"

#include "io/error.hpp"

#include "ucfb/writer.hpp"

using namespace we::ucfb::literals;

namespace we::munge {

namespace {

enum class munged_texture_type : std::uint32_t {
   _2d = 1,
   cube = 2,
   volume = 3
};

auto get_texture_type(const write_texture_options& options) noexcept -> munged_texture_type
{
   if (options.type == texture_type::cube) return munged_texture_type::cube;
   if (options.type == texture_type::volume) return munged_texture_type::volume;

   return munged_texture_type::_2d;
}

auto pack_detail_bias_type(const write_texture_options& options)
{
   return ((options.detail_bias & 0xff) << 8u) |
          static_cast<uint32>(get_texture_type(options));
}

void write_lvl(ucfb::writer& lvl, const texture_transmuted_view_subresource& texture,
               const uint32 mip_level)
{
   // INFO
   {
      ucfb::writer info = lvl.write_child("INFO"_id);

      info.write(mip_level);
      info.write(static_cast<uint32>(texture.as_bytes().size()));
   }

   // BODY
   {
      ucfb::writer body = lvl.write_child("BODY"_id);

      body.write(texture.as_bytes());
   }
}

void write_fmt(ucfb::writer& fmt, const texture_transmuted_view& texture,
               const write_texture_options& options)
{
   // INFO
   {
      ucfb::writer info = fmt.write_child("INFO"_id);

      info.write(to_d3dformat(texture.format()));            // Format
      info.write(static_cast<uint16>(texture.width()));      // Width
      info.write(static_cast<uint16>(texture.height()));     // Height
      info.write(static_cast<uint16>(texture.depth()));      // Depth
      info.write(static_cast<uint16>(texture.mip_levels())); // Mip Levels
      info.write(pack_detail_bias_type(options)); // ([0,7] Type, [8, 15] Bias)
   }

   const uint32 face_count = options.type == texture_type::cube
                                ? texture.array_size()
                                : std::min(texture.array_size(), 1u);

   for (uint32 face_index = 0; face_index < face_count; ++face_index) {
      ucfb::writer face = fmt.write_child("FACE"_id);

      for (uint32 mip_level = 0; mip_level < texture.mip_levels(); ++mip_level) {
         ucfb::writer lvl = face.write_child("LVL_"_id);

         write_lvl(lvl,
                   texture.subresource({.array_index = face_index, .mip_level = mip_level}),
                   mip_level);
      }
   }
}

void write_tex(ucfb::writer& tex, std::string_view name,
               const texture_transmuted_view& texture,
               const write_texture_options& options)
{
   // NAME
   {
      tex.write_child("NAME"_id).write(name);
   }

   // INFO
   {
      ucfb::writer info = tex.write_child("INFO"_id);

      info.write(uint32{1});                      // Format Count
      info.write(to_d3dformat(texture.format())); // Format
   }

   // We currently only save one format. Barring any other info it
   // seems reasonable to assume that all texture formats the game supports are
   // now supported by almost every GPU in use.
   //
   // The luminance formats might be a wild card but the functionality needed to
   // implement them is required by D3D12 and Vulkan. So it seems very unlikely
   // to me that a D3D9 driver from the past 10-15 years wouldn't support them.
   //
   // Hopefully this doesn't come back around to bite me or anyone else.
   //
   // FMT_
   {
      ucfb::writer fmt = tex.write_child("FMT_"_id);

      write_fmt(fmt, texture, options);
   }
}

}

void write_texture(const io::path& output_file_path,
                   const texture_transmuted_view& texture,
                   const write_texture_options& options)
{
   try {
      io::output_file out{output_file_path};

      ucfb::writer ucfb{"ucfb"_id, out, {}};

      // tex_
      {
         ucfb::writer tex = ucfb.write_child("tex_"_id);

         write_tex(tex, output_file_path.stem(), texture, options);
      }
   }
   catch (io::open_error& error) {
      throw texture_error{error.what(), texture_ec::write_io_open_error};
   }
   catch (io::error& error) {
      throw texture_error{error.what(), texture_ec::write_io_generic_error};
   }
}

}