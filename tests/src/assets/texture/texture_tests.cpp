
#include "pch.h"

#include "approx_test_helpers.hpp"
#include "assets/texture/texture.hpp"

#include <algorithm>
#include <array>
#include <iterator>

namespace we::assets::texture::tests {

TEST_CASE("texture subresource view creation tests", "[Assets][Texture]")
{
   std::array<std::byte, 512> texels{};

   texture_subresource_view texture_view =
      texture_subresource_view::init_params{.data = texels,
                                            .offset = 0,
                                            .row_pitch = 256,
                                            .width = 4,
                                            .height = 2,
                                            .format = texture_format::r8g8b8a8_unorm};

   CHECK(texture_view.data() == texels.data());
   CHECK(texture_view.size() == texels.size());
   CHECK(texture_view.offset() == 0);
   CHECK(texture_view.row_pitch() == 256);
   CHECK(texture_view.width() == 4);
   CHECK(texture_view.height() == 2);
   CHECK(texture_view.format() == texture_format::r8g8b8a8_unorm);
   CHECK(texture_view.dxgi_format() == DXGI_FORMAT_R8G8B8A8_UNORM);
}

TEST_CASE("texture creation tests", "[Assets][Texture]")
{
   texture texture = texture::init_params{.width = 65,
                                          .height = 32,
                                          .mip_levels = 2,
                                          .array_size = 1,
                                          .format = texture_format::r8g8b8a8_unorm};

   CHECK(texture.subresource_count() == 2);
   CHECK(texture.data() != nullptr);
   CHECK(texture.size() == (512 * 32 + 256 * 16));
   CHECK(texture.width() == 65);
   CHECK(texture.height() == 32);
   CHECK(texture.mip_levels() == 2);
   CHECK(texture.array_size() == 1);
   CHECK(texture.format() == texture_format::r8g8b8a8_unorm);
   CHECK(texture.dxgi_format() == DXGI_FORMAT_R8G8B8A8_UNORM);

   CHECK(texture.subresource({.mip_level = 0, .array_index = 0}).data() != nullptr);
   CHECK(texture.subresource({.mip_level = 0, .array_index = 0}).size() == (512 * 32));
   CHECK(
      texture.subresource({.mip_level = 0, .array_index = 0}).offset() ==
      static_cast<std::size_t>(
         std::distance(texture.data(),
                       texture.subresource({.mip_level = 0, .array_index = 0}).data())));
   CHECK(texture.subresource({.mip_level = 0, .array_index = 0}).row_pitch() == 512);
   CHECK(texture.subresource({.mip_level = 0, .array_index = 0}).width() == 65);
   CHECK(texture.subresource({.mip_level = 0, .array_index = 0}).height() == 32);
   CHECK(texture.subresource({.mip_level = 0, .array_index = 0}).format() ==
         texture.format());
   CHECK(texture.subresource({.mip_level = 0, .array_index = 0}).dxgi_format() ==
         texture.dxgi_format());

   CHECK(texture.subresource({.mip_level = 1, .array_index = 0}).data() != nullptr);
   CHECK(texture.subresource({.mip_level = 1, .array_index = 0}).size() == (256 * 16));
   CHECK(
      texture.subresource({.mip_level = 1, .array_index = 0}).offset() ==
      static_cast<std::size_t>(
         std::distance(texture.data(),
                       texture.subresource({.mip_level = 1, .array_index = 0}).data())));
   CHECK(texture.subresource({.mip_level = 1, .array_index = 0}).row_pitch() == 256);
   CHECK(texture.subresource({.mip_level = 1, .array_index = 0}).width() == 32);
   CHECK(texture.subresource({.mip_level = 1, .array_index = 0}).height() == 16);
   CHECK(texture.subresource({.mip_level = 1, .array_index = 0}).format() ==
         texture.format());
   CHECK(texture.subresource({.mip_level = 1, .array_index = 0}).dxgi_format() ==
         texture.dxgi_format());

   CHECK(&texture.subresource({.mip_level = 0, .array_index = 0}) ==
         &texture.subresource(0));
   CHECK(&texture.subresource({.mip_level = 1, .array_index = 0}) ==
         &texture.subresource(1));

   CHECK_THROWS((void)texture.subresource({.mip_level = 2, .array_index = 0}));
   CHECK_THROWS((void)texture.subresource({.mip_level = 0, .array_index = 1}));
   CHECK_THROWS((void)texture.subresource(2));
}

}
