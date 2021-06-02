
#include "texture_transforms.hpp"

namespace we::assets::texture {

namespace {

void generate_normal_map(const texture_subresource_view& input,
                         texture_subresource_view& output, const float scale)
{
   for (auto y = 0u; y < input.height(); ++y) {
      const uint32 y0 = (y - 1) % input.height();
      const uint32 y1 = (y + 1) % input.height();

      for (auto x = 0u; x < input.width(); ++x) {
         const uint32 x0 = (x - 1) % input.width();
         const uint32 x1 = (x + 1) % input.width();

         const float height0x = input.load({x0, y}).r;
         const float height1x = input.load({x1, y}).r;
         const float height0y = input.load({x, y0}).r;
         const float height1y = input.load({x, y1}).r;

         const float3 normal =
            glm::normalize(float3{(height0x - height1x) * scale / 2.0f,
                                  (height0y - height1y) * scale / 2.0f, 1.0f});

         output.store({x, y}, float4{normal * 0.5f + 0.5f, input.load({x, y}).a});
      }
   }
}

}

auto generate_normal_maps(const texture& input, const float scale) -> texture
{
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
