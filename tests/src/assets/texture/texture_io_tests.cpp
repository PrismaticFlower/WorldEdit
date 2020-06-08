
#include "pch.h"

#include "approx_test_helpers.hpp"
#include "assets/texture/texture_io.hpp"
#include "utility/srgb_conversion.hpp"

#include <string_view>

using namespace std::literals;
using namespace Catch::literals;

namespace sk::assets::texture::tests {

TEST_CASE(".tga texture loading", "[Assets][Texture]")
{
   auto texture = load_texture("data/noise.tga");

   constexpr float epsilon = 0.001f;

   CHECK(approx_equals(texture.load({.mip_level = 0}, {0, 0}),
                       utility::decompress_srgb({0.835f, 0.49f, 0.812f, 1.0f}),
                       epsilon));
   CHECK(approx_equals(texture.load({.mip_level = 0}, {3, 0}),
                       utility::decompress_srgb({0.937f, 0.298f, 0.824f, 1.0f}),
                       epsilon));
   CHECK(approx_equals(texture.load({.mip_level = 0}, {0, 3}),
                       utility::decompress_srgb({0.486f, 0.8f, 0.529f, 1.0f}), epsilon));
   CHECK(approx_equals(texture.load({.mip_level = 0}, {3, 3}),
                       utility::decompress_srgb({0.808f, 0.922f, 0.227f, 1.0f}),
                       epsilon));

   CHECK(approx_equals(texture.load({.mip_level = 1}, {0, 0}),
                       utility::decompress_srgb({0.718f, 0.663f, 0.718f, 1.0f}),
                       epsilon));
   CHECK(approx_equals(texture.load({.mip_level = 1}, {1, 0}),
                       utility::decompress_srgb({0.616f, 0.439f, 0.78f, 1.0f}),
                       epsilon));
   CHECK(approx_equals(texture.load({.mip_level = 1}, {0, 1}),
                       utility::decompress_srgb({0.518f, 0.624f, 0.325f, 1.0f}),
                       epsilon));
   CHECK(approx_equals(texture.load({.mip_level = 1}, {1, 1}),
                       utility::decompress_srgb({0.62f, 0.533f, 0.588f, 1.0f}),
                       epsilon));

   CHECK(approx_equals(texture.load({.mip_level = 2}, {0, 0}),
                       utility::decompress_srgb({0.624f, 0.573f, 0.635f, 1.0f}),
                       epsilon));
}

}
