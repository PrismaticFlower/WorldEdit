
#include "texture_transforms.hpp"
#include "math/vector_funcs.hpp"

#include <bit>
#include <stdexcept>

#include <immintrin.h>

namespace we::assets::texture {

namespace {

auto fast_rsqrt(float v) -> float
{
   _mm_store_ss(&v, _mm_rsqrt_ss(_mm_load_ss(&v)));

   return v;
}

void generate_normal_map(const texture_subresource_view& input,
                         texture_subresource_view& output, const float bump_scale)
{
   assert(format_size(input.format()) == 4);
   assert(std::has_single_bit(input.width()));
   assert(std::has_single_bit(input.height()));

   constexpr float inv_height = 1.0f / 255.0f;
   const float scale = inv_height * bump_scale / 2.0f;

   const uint32 width_mask = input.width() - 1;
   const uint32 height_mask = input.height() - 1;

   for (auto y = 0u; y < input.height(); ++y) {
      const uint32 y0 = (y - 1) & height_mask;
      const uint32 y1 = (y + 1) & height_mask;

      for (auto x = 0u; x < input.width(); ++x) {
         const uint32 x0 = (x - 1) & width_mask;
         const uint32 x1 = (x + 1) & width_mask;

         int32 height0x = 0;
         int32 height1x = 0;
         int32 height0y = 0;
         int32 height1y = 0;

         std::memcpy(&height0x,
                     input.data() + y * input.row_pitch() + (x0 * sizeof(int32)),
                     sizeof(uint32));
         std::memcpy(&height1x,
                     input.data() + y * input.row_pitch() + (x1 * sizeof(int32)),
                     sizeof(uint32));
         std::memcpy(&height0y,
                     input.data() + y0 * input.row_pitch() + (x * sizeof(int32)),
                     sizeof(uint32));
         std::memcpy(&height1y,
                     input.data() + y1 * input.row_pitch() + (x * sizeof(int32)),
                     sizeof(uint32));

         height0x &= 0xff;
         height1x &= 0xff;
         height0y &= 0xff;
         height1y &= 0xff;

         float3 normal = float3{(height0x - height1x) * scale,
                                (height0y - height1y) * scale, 1.0f};

         float length_sq = dot(normal, normal);
         float inv_length = fast_rsqrt(length_sq);

         normal *= inv_length;

         uint32 value = 0;

         std::memcpy(&value,
                     input.data() + y * input.row_pitch() + (x * sizeof(uint32)),
                     sizeof(uint32));

         value &= 0xff'00'00'00u;

         value |= static_cast<uint32>(normal.x * 127.5f + 128.0f) << 0u;
         value |= static_cast<uint32>(normal.y * 127.5f + 128.0f) << 8u;
         value |= static_cast<uint32>(normal.z * 127.5f + 128.0f) << 16u;

         std::memcpy(output.data() + y * output.row_pitch() + (x * sizeof(uint32)),
                     &value, sizeof(value));
      }
   }
}

}

auto generate_normal_maps(const texture& input, const float scale) -> texture
{
   if (format_size(input.format()) != 4) {
      throw std::runtime_error{
         "Normal maps can only be created from R8G8B8A8 or B8G8R8A8 formats."};
   }

   if (not std::has_single_bit(input.width()) or
       not std::has_single_bit(input.height())) {
      throw std::runtime_error{
         "Normal maps can not be created from non-power of 2 textures."};
   }

   texture result = texture::init_params{.width = input.width(),
                                         .height = input.height(),
                                         .mip_levels = input.mip_levels(),
                                         .array_size = input.array_size(),
                                         .format = texture_format::r8g8b8a8_unorm};

   for (auto i = 0u; i < input.subresource_count(); ++i) {
      generate_normal_map(input.subresource(i), result.subresource(i), scale);
   }

   return result;
}

}
