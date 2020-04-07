
#include "pch.h"

#include "approx_test_helpers.hpp"
#include "assets/texture/texture.hpp"

#include <array>
#include <iterator>

#include <range/v3/algorithm.hpp>

namespace sk::assets::texture::tests {

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

   CHECK_THROWS((void)texture.subresource({.mip_level = 2, .array_index = 0}));
   CHECK_THROWS((void)texture.subresource({.mip_level = 0, .array_index = 1}));
}

TEST_CASE("texture texel load tests", "[Assets][Texture]")
{
   texture::init_params init_params{.width = 1,
                                    .height = 1,
                                    .mip_levels = 1,
                                    .array_size = 1,
                                    .format = texture_format::r8g8b8a8_unorm};

   SECTION("r8g8b8a8_unorm load")
   {
      constexpr std::array<std::byte, 4> texel{std::byte{0x80}, std::byte{0x80},
                                               std::byte{0xff}, std::byte{0xff}};

      init_params.format = texture_format::r8g8b8a8_unorm;

      texture texture{init_params};

      ranges::copy(texel, texture.data());

      CHECK(approx_equals(texture.load({.mip_level = 0}, {0, 0}),
                          {0.50196f, 0.50196f, 1.0f, 1.0f}));
   }

   SECTION("r8g8b8a8_unorm_srgb load")
   {
      constexpr std::array<std::byte, 4> texel{std::byte{0x80}, std::byte{0x80},
                                               std::byte{0xff}, std::byte{0x80}};

      init_params.format = texture_format::r8g8b8a8_unorm_srgb;

      texture texture{init_params};

      ranges::copy(texel, texture.data());

      CHECK(approx_equals(texture.load({.mip_level = 0}, {0, 0}),
                          {0.2158605f, 0.2158605f, 1.0f, 0.50196f}));
   }

   SECTION("b8g8r8a8_unorm load")
   {
      constexpr std::array<std::byte, 4> texel{std::byte{0xff}, std::byte{0x80},
                                               std::byte{0x80}, std::byte{0xff}};

      init_params.format = texture_format::b8g8r8a8_unorm;

      texture texture{init_params};

      ranges::copy(texel, texture.data());

      CHECK(approx_equals(texture.load({.mip_level = 0}, {0, 0}),
                          {0.50196f, 0.50196f, 1.0f, 1.0f}));
   }

   SECTION("b8g8r8a8_unorm_srgb load")
   {
      constexpr std::array<std::byte, 4> texel{std::byte{0xff}, std::byte{0x80},
                                               std::byte{0x80}, std::byte{0x80}};

      init_params.format = texture_format::b8g8r8a8_unorm_srgb;

      texture texture{init_params};

      ranges::copy(texel, texture.data());

      CHECK(approx_equals(texture.load({.mip_level = 0}, {0, 0}),
                          {0.2158605f, 0.2158605f, 1.0f, 0.50196f}));
   }

   SECTION("r16g16b16a16_unorm load")
   {
      constexpr std::array<std::byte, 8> texel{
         std::byte{0x00}, std::byte{0x80}, // red
         std::byte{0x00}, std::byte{0x80}, // green
         std::byte{0xff}, std::byte{0xff}, // blue
         std::byte{0xff}, std::byte{0xff}  // alpha
      };

      init_params.format = texture_format::r16g16b16a16_unorm;

      texture texture{init_params};

      ranges::copy(texel, texture.data());

      CHECK(approx_equals(texture.load({.mip_level = 0}, {0, 0}),
                          {0.500007f, 0.500007f, 1.0f, 1.0f}));
   }

   SECTION("r16g16b16a16_float load")
   {
      constexpr std::array<std::byte, 8> texel{
         std::byte{0x00}, std::byte{0x58}, // red
         std::byte{0x00}, std::byte{0xbc}, // green
         std::byte{0x30}, std::byte{0x50}, // blue
         std::byte{0x00}, std::byte{0x3c}  // alpha
      };

      init_params.format = texture_format::r16g16b16a16_float;

      texture texture{init_params};

      ranges::copy(texel, texture.data());

      CHECK(approx_equals(texture.load({.mip_level = 0}, {0, 0}),
                          {128.0f, -1.0f, 33.5f, 1.0f}));
   }

   SECTION("r32g32b32_float load")
   {
      constexpr std::array<std::byte, 16> texel{
         // clang-format off
         std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x43}, // red
         std::byte{0x00}, std::byte{0x00}, std::byte{0x80}, std::byte{0xbf}, // green
         std::byte{0x00}, std::byte{0x00}, std::byte{0x06}, std::byte{0x42} // blue
         // clang-format on
      };

      init_params.format = texture_format::r32g32b32_float;

      texture texture{init_params};

      ranges::copy(texel, texture.data());

      CHECK(approx_equals(texture.load({.mip_level = 0}, {0, 0}),
                          {128.0f, -1.0f, 33.5f, 1.0f}));
   }

   SECTION("r32g32b32a32_float load")
   {
      constexpr std::array<std::byte, 16> texel{
         // clang-format off
         std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x43}, // red
         std::byte{0x00}, std::byte{0x00}, std::byte{0x80}, std::byte{0xbf}, // green
         std::byte{0x00}, std::byte{0x00}, std::byte{0x06}, std::byte{0x42}, // blue
         std::byte{0x00}, std::byte{0x00}, std::byte{0x80}, std::byte{0x3f}  // alpha
         // clang-format on
      };

      init_params.format = texture_format::r32g32b32a32_float;

      texture texture{init_params};

      ranges::copy(texel, texture.data());

      CHECK(approx_equals(texture.load({.mip_level = 0}, {0, 0}),
                          {128.0f, -1.0f, 33.5f, 1.0f}));
   }

   SECTION("invalid subresource load")
   {
      init_params.format = texture_format::r8g8b8a8_unorm;

      texture texture{init_params};

      CHECK_THROWS(texture.load({.mip_level = 3}, {0, 0}));
   }
}

TEST_CASE("texture texel store tests", "[Assets][Texture]")
{
   texture::init_params init_params{.width = 1,
                                    .height = 1,
                                    .mip_levels = 1,
                                    .array_size = 1,
                                    .format = texture_format::r8g8b8a8_unorm};

   SECTION("r8g8b8a8_unorm store")
   {
      constexpr std::array<std::byte, 4> expected_texel{std::byte{0x80},
                                                        std::byte{0x80},
                                                        std::byte{0xff},
                                                        std::byte{0xff}};

      init_params.format = texture_format::r8g8b8a8_unorm;

      texture texture{init_params};

      texture.store({.mip_level = 0}, {0, 0}, float4{0.5f, 0.5f, 1.0f, 1.0f});

      CHECK(ranges::equal(expected_texel,
                          gsl::span{texture.data(), expected_texel.size()}));
   }

   SECTION("r8g8b8a8_unorm_srgb store")
   {
      constexpr std::array<std::byte, 4> expected_texel{std::byte{0x80},
                                                        std::byte{0x80},
                                                        std::byte{0xff},
                                                        std::byte{0x80}};

      init_params.format = texture_format::r8g8b8a8_unorm_srgb;

      texture texture{init_params};

      texture.store({.mip_level = 0}, {0, 0}, {0.2158605f, 0.2158605f, 1.0f, 0.5f});

      CHECK(ranges::equal(expected_texel,
                          gsl::span{texture.data(), expected_texel.size()}));
   }

   SECTION("b8g8r8a8_unorm store")
   {
      constexpr std::array<std::byte, 4> expected_texel{std::byte{0xff},
                                                        std::byte{0x80},
                                                        std::byte{0x80},
                                                        std::byte{0xff}};

      init_params.format = texture_format::b8g8r8a8_unorm;

      texture texture{init_params};

      texture.store({.mip_level = 0}, {0, 0}, {0.5f, 0.5f, 1.0f, 1.0f});

      CHECK(ranges::equal(expected_texel,
                          gsl::span{texture.data(), expected_texel.size()}));
   }

   SECTION("b8g8r8a8_unorm_srgb store")
   {
      constexpr std::array<std::byte, 4> expected_texel{std::byte{0xff},
                                                        std::byte{0x80},
                                                        std::byte{0x80},
                                                        std::byte{0x80}};

      init_params.format = texture_format::b8g8r8a8_unorm_srgb;

      texture texture{init_params};

      texture.store({.mip_level = 0}, {0, 0}, {0.2158605f, 0.2158605f, 1.0f, 0.5f});

      CHECK(ranges::equal(expected_texel,
                          gsl::span{texture.data(), expected_texel.size()}));
   }

   SECTION("r16g16b16a16_unorm store")
   {
      constexpr std::array<std::byte, 8> expected_texel{
         std::byte{0x00}, std::byte{0x80}, // red
         std::byte{0x00}, std::byte{0x80}, // green
         std::byte{0xff}, std::byte{0xff}, // blue
         std::byte{0xff}, std::byte{0xff}  // alpha
      };

      init_params.format = texture_format::r16g16b16a16_unorm;

      texture texture{init_params};

      texture.store({.mip_level = 0}, {0, 0}, {0.5f, 0.5f, 1.0f, 1.0f});

      CHECK(ranges::equal(expected_texel,
                          gsl::span{texture.data(), expected_texel.size()}));
   }

   SECTION("r16g16b16a16_float store")
   {
      constexpr std::array<std::byte, 8> expected_texel{
         std::byte{0x00}, std::byte{0x58}, // red
         std::byte{0x00}, std::byte{0xbc}, // green
         std::byte{0x30}, std::byte{0x50}, // blue
         std::byte{0x00}, std::byte{0x3c}  // alpha
      };

      init_params.format = texture_format::r16g16b16a16_float;

      texture texture{init_params};

      texture.store({.mip_level = 0}, {0, 0}, {128.0f, -1.0f, 33.5f, 1.0f});

      CHECK(ranges::equal(expected_texel,
                          gsl::span{texture.data(), expected_texel.size()}));
   }

   SECTION("r32g32b32_float store")
   {
      constexpr std::array<std::byte, 16> expected_texel{
         // clang-format off
         std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x43}, // red
         std::byte{0x00}, std::byte{0x00}, std::byte{0x80}, std::byte{0xbf}, // green
         std::byte{0x00}, std::byte{0x00}, std::byte{0x06}, std::byte{0x42} // blue
         // clang-format on
      };

      init_params.format = texture_format::r32g32b32_float;

      texture texture{init_params};

      texture.store({.mip_level = 0}, {0, 0}, {128.0f, -1.0f, 33.5f, 1.0f});

      CHECK(ranges::equal(expected_texel,
                          gsl::span{texture.data(), expected_texel.size()}));
   }

   SECTION("r32g32b32a32_float store")
   {
      constexpr std::array<std::byte, 16> expected_texel{
         // clang-format off
         std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x43}, // red
         std::byte{0x00}, std::byte{0x00}, std::byte{0x80}, std::byte{0xbf}, // green
         std::byte{0x00}, std::byte{0x00}, std::byte{0x06}, std::byte{0x42}, // blue
         std::byte{0x00}, std::byte{0x00}, std::byte{0x80}, std::byte{0x3f}  // alpha
         // clang-format on
      };

      init_params.format = texture_format::r32g32b32a32_float;

      texture texture{init_params};

      texture.store({.mip_level = 0}, {0, 0}, {128.0f, -1.0f, 33.5f, 1.0f});

      CHECK(ranges::equal(expected_texel,
                          gsl::span{texture.data(), expected_texel.size()}));
   }

   SECTION("invalid subresource store")
   {
      constexpr std::array<std::byte, 4> expected_texel{std::byte{0x80},
                                                        std::byte{0x80},
                                                        std::byte{0xff},
                                                        std::byte{0xff}};

      init_params.format = texture_format::r8g8b8a8_unorm;

      texture texture{init_params};

      CHECK_THROWS(texture.store({.mip_level = 3}, {0, 0},
                                 float4{0.5f, 0.5f, 1.0f, 1.0f}));
   }
}

}