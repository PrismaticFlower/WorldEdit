
#include "pch.h"

#include "assets/texture/texture_io.hpp"

#include <string_view>

using namespace std::literals;
using namespace Catch::literals;

namespace we::assets::texture::tests {

namespace {

bool test_texel(texture_subresource_view& view, uint32 x, uint32 y, uint32 expected)
{
   if (x >= view.width()) return false;
   if (y >= view.height()) return false;

   const uint32 texel =
      reinterpret_cast<uint32*>(view.data() + y * view.row_pitch())[x];

   return texel == expected;
}

}

TEST_CASE(".tga texture loading", "[Assets][Texture]")
{
   auto texture = load_texture("data/noise.tga");

   REQUIRE(texture.width() == 4);
   REQUIRE(texture.height() == 4);
   REQUIRE(texture.mip_levels() == 3);

   CHECK(test_texel(texture.subresource({.mip_level = 0}), 0, 0, 0xffcf7dd5));
   CHECK(test_texel(texture.subresource({.mip_level = 0}), 3, 0, 0xffd24cef));
   CHECK(test_texel(texture.subresource({.mip_level = 0}), 0, 3, 0xff87cc7c));
   CHECK(test_texel(texture.subresource({.mip_level = 0}), 3, 3, 0xff3aebce));

   CHECK(test_texel(texture.subresource({.mip_level = 1}), 0, 0, 0xffb7a9b7));
   CHECK(test_texel(texture.subresource({.mip_level = 1}), 1, 0, 0xffc7709d));
   CHECK(test_texel(texture.subresource({.mip_level = 1}), 0, 1, 0xff539f84));
   CHECK(test_texel(texture.subresource({.mip_level = 1}), 1, 1, 0xff96889e));

   CHECK(test_texel(texture.subresource({.mip_level = 2}), 0, 0, 0xffa2929f));
}

}
