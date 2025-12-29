#include "pch.h"

#include "assets/option_file.hpp"

#include "munge/builtin/texture_munge/load_texture.hpp"
#include "munge/feedback.hpp"

#include "io/read_file.hpp"

namespace we::munge::tests {

TEST_CASE("texture_munge load_texture 2d", "[Munge]")
{
   const io::path input_path = R"(data\munge\textures\load_2d.tga)";

   load_texture_result result = load_texture(input_path, {});

   CHECK(not result.traits.has_alpha);
   CHECK(not result.traits.has_single_bit_alpha);
   CHECK(not result.traits.is_greyscale);

   texture& texture = result.texture;

   REQUIRE(texture.width() == 4);
   REQUIRE(texture.height() == 4);
   REQUIRE(texture.depth() == 1);
   REQUIRE(texture.array_size() == 1);
   REQUIRE(texture.mip_levels() == 3);

   texture_subresource subresource = texture.subresource({.mip_level = 0});

   CHECK(subresource.at(0, 0, 0) == 0xff55def9);
   CHECK(subresource.at(1, 0, 0) == 0xffa91e78);
   CHECK(subresource.at(2, 0, 0) == 0xff511f2e);
   CHECK(subresource.at(3, 0, 0) == 0xff01ad2d);

   CHECK(subresource.at(0, 1, 0) == 0xfff8bed7);
   CHECK(subresource.at(1, 1, 0) == 0xff1c558a);
   CHECK(subresource.at(2, 1, 0) == 0xffdf1d51);
   CHECK(subresource.at(3, 1, 0) == 0xff2b2aa5);

   CHECK(subresource.at(0, 2, 0) == 0xff270adb);
   CHECK(subresource.at(1, 2, 0) == 0xffafa9ab);
   CHECK(subresource.at(2, 2, 0) == 0xffb43479);
   CHECK(subresource.at(3, 2, 0) == 0xff70209a);

   CHECK(subresource.at(0, 3, 0) == 0xff45466b);
   CHECK(subresource.at(1, 3, 0) == 0xff1a30ec);
   CHECK(subresource.at(2, 3, 0) == 0xff6c2455);
   CHECK(subresource.at(3, 3, 0) == 0xffbeda6a);
}

TEST_CASE("texture_munge load_texture 2d alpha", "[Munge]")
{
   const io::path input_path = R"(data\munge\textures\load_2d_alpha.tga)";

   load_texture_result result = load_texture(input_path, {});

   CHECK(result.traits.has_alpha);
   CHECK(not result.traits.has_single_bit_alpha);
   CHECK(not result.traits.is_greyscale);

   texture& texture = result.texture;

   REQUIRE(texture.width() == 4);
   REQUIRE(texture.height() == 4);
   REQUIRE(texture.depth() == 1);
   REQUIRE(texture.array_size() == 1);
   REQUIRE(texture.mip_levels() == 3);

   texture_subresource subresource = texture.subresource({.mip_level = 0});

   CHECK(subresource.at(0, 0, 0) == 0x3755def9);
   CHECK(subresource.at(1, 0, 0) == 0xffa91e78);
   CHECK(subresource.at(2, 0, 0) == 0x37511f2e);
   CHECK(subresource.at(3, 0, 0) == 0xff01ad2d);

   CHECK(subresource.at(0, 1, 0) == 0xfff8bed7);
   CHECK(subresource.at(1, 1, 0) == 0x371c558a);
   CHECK(subresource.at(2, 1, 0) == 0xffdf1d51);
   CHECK(subresource.at(3, 1, 0) == 0x372b2aa5);

   CHECK(subresource.at(0, 2, 0) == 0x37270adb);
   CHECK(subresource.at(1, 2, 0) == 0xffafa9ab);
   CHECK(subresource.at(2, 2, 0) == 0x37b43479);
   CHECK(subresource.at(3, 2, 0) == 0xff70209a);

   CHECK(subresource.at(0, 3, 0) == 0xff45466b);
   CHECK(subresource.at(1, 3, 0) == 0x371a30ec);
   CHECK(subresource.at(2, 3, 0) == 0xff6c2455);
   CHECK(subresource.at(3, 3, 0) == 0x37beda6a);
}

TEST_CASE("texture_munge load_texture 2d single bit alpha", "[Munge]")
{
   const io::path input_path = R"(data\munge\textures\load_2d_single_bit_alpha.tga)";

   load_texture_result result = load_texture(input_path, {});

   CHECK(result.traits.has_alpha);
   CHECK(result.traits.has_single_bit_alpha);
   CHECK(not result.traits.is_greyscale);

   texture& texture = result.texture;

   REQUIRE(texture.width() == 4);
   REQUIRE(texture.height() == 4);
   REQUIRE(texture.depth() == 1);
   REQUIRE(texture.array_size() == 1);
   REQUIRE(texture.mip_levels() == 3);

   texture_subresource subresource = texture.subresource({.mip_level = 0});

   CHECK(subresource.at(0, 0, 0) == 0x0055def9);
   CHECK(subresource.at(1, 0, 0) == 0xffa91e78);
   CHECK(subresource.at(2, 0, 0) == 0x00511f2e);
   CHECK(subresource.at(3, 0, 0) == 0xff01ad2d);

   CHECK(subresource.at(0, 1, 0) == 0xfff8bed7);
   CHECK(subresource.at(1, 1, 0) == 0x001c558a);
   CHECK(subresource.at(2, 1, 0) == 0xffdf1d51);
   CHECK(subresource.at(3, 1, 0) == 0x002b2aa5);

   CHECK(subresource.at(0, 2, 0) == 0x00270adb);
   CHECK(subresource.at(1, 2, 0) == 0xffafa9ab);
   CHECK(subresource.at(2, 2, 0) == 0x00b43479);
   CHECK(subresource.at(3, 2, 0) == 0xff70209a);

   CHECK(subresource.at(0, 3, 0) == 0xff45466b);
   CHECK(subresource.at(1, 3, 0) == 0x001a30ec);
   CHECK(subresource.at(2, 3, 0) == 0xff6c2455);
   CHECK(subresource.at(3, 3, 0) == 0x00beda6a);
}

TEST_CASE("texture_munge load_texture 2d greyscale", "[Munge]")
{
   const io::path input_path = R"(data\munge\textures\load_2d_greyscale.tga)";

   load_texture_result result = load_texture(input_path, {});

   CHECK(not result.traits.has_alpha);
   CHECK(not result.traits.has_single_bit_alpha);
   CHECK(result.traits.is_greyscale);

   texture& texture = result.texture;

   REQUIRE(texture.width() == 4);
   REQUIRE(texture.height() == 4);
   REQUIRE(texture.depth() == 1);
   REQUIRE(texture.array_size() == 1);
   REQUIRE(texture.mip_levels() == 3);

   texture_subresource subresource = texture.subresource({.mip_level = 0});

   CHECK(subresource.at(0, 0, 0) == 0xffa7a7a7);
   CHECK(subresource.at(1, 0, 0) == 0xff646464);
   CHECK(subresource.at(2, 0, 0) == 0xff383838);
   CHECK(subresource.at(3, 0, 0) == 0xff575757);

   CHECK(subresource.at(0, 1, 0) == 0xffdbdbdb);
   CHECK(subresource.at(1, 1, 0) == 0xff535353);
   CHECK(subresource.at(2, 1, 0) == 0xff7e7e7e);
   CHECK(subresource.at(3, 1, 0) == 0xff686868);

   CHECK(subresource.at(0, 2, 0) == 0xff737373);
   CHECK(subresource.at(1, 2, 0) == 0xffacacac);
   CHECK(subresource.at(2, 2, 0) == 0xff747474);
   CHECK(subresource.at(3, 2, 0) == 0xff5d5d5d);

   CHECK(subresource.at(0, 3, 0) == 0xff585858);
   CHECK(subresource.at(1, 3, 0) == 0xff838383);
   CHECK(subresource.at(2, 3, 0) == 0xff484848);
   CHECK(subresource.at(3, 3, 0) == 0xffa2a2a2);
}

TEST_CASE("texture_munge load_texture 2d greyscale alpha", "[Munge]")
{
   const io::path input_path = R"(data\munge\textures\load_2d_greyscale_alpha.tga)";

   load_texture_result result = load_texture(input_path, {});

   CHECK(result.traits.has_alpha);
   CHECK(not result.traits.has_single_bit_alpha);
   CHECK(result.traits.is_greyscale);

   texture& texture = result.texture;

   REQUIRE(texture.width() == 4);
   REQUIRE(texture.height() == 4);
   REQUIRE(texture.depth() == 1);
   REQUIRE(texture.array_size() == 1);
   REQUIRE(texture.mip_levels() == 3);

   texture_subresource subresource = texture.subresource({.mip_level = 0});

   CHECK(subresource.at(0, 0, 0) == 0x37a7a7a7);
   CHECK(subresource.at(1, 0, 0) == 0xff646464);
   CHECK(subresource.at(2, 0, 0) == 0x37383838);
   CHECK(subresource.at(3, 0, 0) == 0xff575757);

   CHECK(subresource.at(0, 1, 0) == 0xffdbdbdb);
   CHECK(subresource.at(1, 1, 0) == 0x37535353);
   CHECK(subresource.at(2, 1, 0) == 0xff7e7e7e);
   CHECK(subresource.at(3, 1, 0) == 0x37686868);

   CHECK(subresource.at(0, 2, 0) == 0x37737373);
   CHECK(subresource.at(1, 2, 0) == 0xffacacac);
   CHECK(subresource.at(2, 2, 0) == 0x37747474);
   CHECK(subresource.at(3, 2, 0) == 0xff5d5d5d);

   CHECK(subresource.at(0, 3, 0) == 0xff585858);
   CHECK(subresource.at(1, 3, 0) == 0x37838383);
   CHECK(subresource.at(2, 3, 0) == 0xff484848);
   CHECK(subresource.at(3, 3, 0) == 0x37a2a2a2);
}

TEST_CASE("texture_munge load_texture 2d greyscale single bit alpha", "[Munge]")
{
   const io::path input_path =
      R"(data\munge\textures\load_2d_greyscale_single_bit_alpha.tga)";

   load_texture_result result = load_texture(input_path, {});

   CHECK(result.traits.has_alpha);
   CHECK(result.traits.has_single_bit_alpha);
   CHECK(result.traits.is_greyscale);

   texture& texture = result.texture;

   REQUIRE(texture.width() == 4);
   REQUIRE(texture.height() == 4);
   REQUIRE(texture.depth() == 1);
   REQUIRE(texture.array_size() == 1);
   REQUIRE(texture.mip_levels() == 3);

   texture_subresource subresource = texture.subresource({.mip_level = 0});

   CHECK(subresource.at(0, 0, 0) == 0x00a7a7a7);
   CHECK(subresource.at(1, 0, 0) == 0xff646464);
   CHECK(subresource.at(2, 0, 0) == 0x00383838);
   CHECK(subresource.at(3, 0, 0) == 0xff575757);

   CHECK(subresource.at(0, 1, 0) == 0xffdbdbdb);
   CHECK(subresource.at(1, 1, 0) == 0x00535353);
   CHECK(subresource.at(2, 1, 0) == 0xff7e7e7e);
   CHECK(subresource.at(3, 1, 0) == 0x00686868);

   CHECK(subresource.at(0, 2, 0) == 0x00737373);
   CHECK(subresource.at(1, 2, 0) == 0xffacacac);
   CHECK(subresource.at(2, 2, 0) == 0x00747474);
   CHECK(subresource.at(3, 2, 0) == 0xff5d5d5d);

   CHECK(subresource.at(0, 3, 0) == 0xff585858);
   CHECK(subresource.at(1, 3, 0) == 0x00838383);
   CHECK(subresource.at(2, 3, 0) == 0xff484848);
   CHECK(subresource.at(3, 3, 0) == 0x00a2a2a2);
}

TEST_CASE("texture_munge load_texture 2d r8", "[Munge]")
{
   const io::path input_path = R"(data\munge\textures\load_2d_r8.tga)";

   load_texture_result result = load_texture(input_path, {});

   CHECK(not result.traits.has_alpha);
   CHECK(not result.traits.has_single_bit_alpha);
   CHECK(result.traits.is_greyscale);

   texture& texture = result.texture;

   REQUIRE(texture.width() == 4);
   REQUIRE(texture.height() == 4);
   REQUIRE(texture.depth() == 1);
   REQUIRE(texture.array_size() == 1);
   REQUIRE(texture.mip_levels() == 3);

   texture_subresource subresource = texture.subresource({.mip_level = 0});

   CHECK(subresource.at(0, 0, 0) == 0xffa7a7a7);
   CHECK(subresource.at(1, 0, 0) == 0xff646464);
   CHECK(subresource.at(2, 0, 0) == 0xff383838);
   CHECK(subresource.at(3, 0, 0) == 0xff575757);

   CHECK(subresource.at(0, 1, 0) == 0xffdbdbdb);
   CHECK(subresource.at(1, 1, 0) == 0xff535353);
   CHECK(subresource.at(2, 1, 0) == 0xff7e7e7e);
   CHECK(subresource.at(3, 1, 0) == 0xff686868);

   CHECK(subresource.at(0, 2, 0) == 0xff737373);
   CHECK(subresource.at(1, 2, 0) == 0xffacacac);
   CHECK(subresource.at(2, 2, 0) == 0xff747474);
   CHECK(subresource.at(3, 2, 0) == 0xff5d5d5d);

   CHECK(subresource.at(0, 3, 0) == 0xff585858);
   CHECK(subresource.at(1, 3, 0) == 0xff838383);
   CHECK(subresource.at(2, 3, 0) == 0xff484848);
   CHECK(subresource.at(3, 3, 0) == 0xffa2a2a2);
}

TEST_CASE("texture_munge load_texture 2d b5g5r5a1", "[Munge]")
{
   const io::path input_path = R"(data\munge\textures\load_2d_b5g5r5a1.tga)";

   load_texture_result result = load_texture(input_path, {});

   CHECK(not result.traits.has_alpha);
   CHECK(not result.traits.has_single_bit_alpha);
   CHECK(not result.traits.is_greyscale);

   texture& texture = result.texture;

   REQUIRE(texture.width() == 4);
   REQUIRE(texture.height() == 4);
   REQUIRE(texture.depth() == 1);
   REQUIRE(texture.array_size() == 1);
   REQUIRE(texture.mip_levels() == 3);

   texture_subresource subresource = texture.subresource({.mip_level = 0});

   CHECK(subresource.at(0, 0, 0) == 0xff52deff);
   CHECK(subresource.at(1, 0, 0) == 0xffad197b);
   CHECK(subresource.at(2, 0, 0) == 0xff521929);
   CHECK(subresource.at(3, 0, 0) == 0xff00ad29);

   CHECK(subresource.at(0, 1, 0) == 0xffffbdd6);
   CHECK(subresource.at(1, 1, 0) == 0xff19528c);
   CHECK(subresource.at(2, 1, 0) == 0xffde1952);
   CHECK(subresource.at(3, 1, 0) == 0xff2929a5);

   CHECK(subresource.at(0, 2, 0) == 0xff2108de);
   CHECK(subresource.at(1, 2, 0) == 0xffadadad);
   CHECK(subresource.at(2, 2, 0) == 0xffb5317b);
   CHECK(subresource.at(3, 2, 0) == 0xff73219c);

   CHECK(subresource.at(0, 3, 0) == 0xff42426b);
   CHECK(subresource.at(1, 3, 0) == 0xff1931ef);
   CHECK(subresource.at(2, 3, 0) == 0xff6b2152);
   CHECK(subresource.at(3, 3, 0) == 0xffbdde6b);
}

TEST_CASE("texture_munge load_texture 2d b5g5r5a1 single bit alpha", "[Munge]")
{
   const io::path input_path =
      R"(data\munge\textures\load_2d_b5g5r5a1_single_bit_alpha.tga)";

   load_texture_result result = load_texture(input_path, {});

   CHECK(result.traits.has_alpha);
   CHECK(result.traits.has_single_bit_alpha);
   CHECK(not result.traits.is_greyscale);

   texture& texture = result.texture;

   REQUIRE(texture.width() == 4);
   REQUIRE(texture.height() == 4);
   REQUIRE(texture.depth() == 1);
   REQUIRE(texture.array_size() == 1);
   REQUIRE(texture.mip_levels() == 3);

   texture_subresource subresource = texture.subresource({.mip_level = 0});

   CHECK(subresource.at(0, 0, 0) == 0x0052deff);
   CHECK(subresource.at(1, 0, 0) == 0xffad197b);
   CHECK(subresource.at(2, 0, 0) == 0x00521929);
   CHECK(subresource.at(3, 0, 0) == 0xff00ad29);

   CHECK(subresource.at(0, 1, 0) == 0xffffbdd6);
   CHECK(subresource.at(1, 1, 0) == 0x0019528c);
   CHECK(subresource.at(2, 1, 0) == 0xffde1952);
   CHECK(subresource.at(3, 1, 0) == 0x002929a5);

   CHECK(subresource.at(0, 2, 0) == 0x002108de);
   CHECK(subresource.at(1, 2, 0) == 0xffadadad);
   CHECK(subresource.at(2, 2, 0) == 0x00b5317b);
   CHECK(subresource.at(3, 2, 0) == 0xff73219c);

   CHECK(subresource.at(0, 3, 0) == 0xff42426b);
   CHECK(subresource.at(1, 3, 0) == 0x001931ef);
   CHECK(subresource.at(2, 3, 0) == 0xff6b2152);
   CHECK(subresource.at(3, 3, 0) == 0x00bdde6b);
}

TEST_CASE("texture_munge load_texture 2d b5g5r5a1 greyscale", "[Munge]")
{
   const io::path input_path = R"(data\munge\textures\load_2d_b5g5r5a1_greyscale.tga)";

   load_texture_result result = load_texture(input_path, {});

   CHECK(not result.traits.has_alpha);
   CHECK(not result.traits.has_single_bit_alpha);
   CHECK(result.traits.is_greyscale);

   texture& texture = result.texture;

   REQUIRE(texture.width() == 4);
   REQUIRE(texture.height() == 4);
   REQUIRE(texture.depth() == 1);
   REQUIRE(texture.array_size() == 1);
   REQUIRE(texture.mip_levels() == 3);

   texture_subresource subresource = texture.subresource({.mip_level = 0});

   CHECK(subresource.at(0, 0, 0) == 0xffa5a5a5);
   CHECK(subresource.at(1, 0, 0) == 0xff636363);
   CHECK(subresource.at(2, 0, 0) == 0xff3a3a3a);
   CHECK(subresource.at(3, 0, 0) == 0xff525252);

   CHECK(subresource.at(0, 1, 0) == 0xffdedede);
   CHECK(subresource.at(1, 1, 0) == 0xff525252);
   CHECK(subresource.at(2, 1, 0) == 0xff7b7b7b);
   CHECK(subresource.at(3, 1, 0) == 0xff6b6b6b);

   CHECK(subresource.at(0, 2, 0) == 0xff737373);
   CHECK(subresource.at(1, 2, 0) == 0xffadadad);
   CHECK(subresource.at(2, 2, 0) == 0xff737373);
   CHECK(subresource.at(3, 2, 0) == 0xff5a5a5a);

   CHECK(subresource.at(0, 3, 0) == 0xff5a5a5a);
   CHECK(subresource.at(1, 3, 0) == 0xff848484);
   CHECK(subresource.at(2, 3, 0) == 0xff4a4a4a);
   CHECK(subresource.at(3, 3, 0) == 0xffa5a5a5);
}

TEST_CASE("texture_munge load_texture 2d b5g5r5a1 greyscale single bit alpha", "[Munge]")
{
   const io::path input_path =
      R"(data\munge\textures\load_2d_b5g5r5a1_greyscale_single_bit_alpha.tga)";

   load_texture_result result = load_texture(input_path, {});

   CHECK(result.traits.has_alpha);
   CHECK(result.traits.has_single_bit_alpha);
   CHECK(result.traits.is_greyscale);

   texture& texture = result.texture;

   REQUIRE(texture.width() == 4);
   REQUIRE(texture.height() == 4);
   REQUIRE(texture.depth() == 1);
   REQUIRE(texture.array_size() == 1);
   REQUIRE(texture.mip_levels() == 3);

   texture_subresource subresource = texture.subresource({.mip_level = 0});

   CHECK(subresource.at(0, 0, 0) == 0x00a5a5a5);
   CHECK(subresource.at(1, 0, 0) == 0xff636363);
   CHECK(subresource.at(2, 0, 0) == 0x003a3a3a);
   CHECK(subresource.at(3, 0, 0) == 0xff525252);

   CHECK(subresource.at(0, 1, 0) == 0xffdedede);
   CHECK(subresource.at(1, 1, 0) == 0x00525252);
   CHECK(subresource.at(2, 1, 0) == 0xff7b7b7b);
   CHECK(subresource.at(3, 1, 0) == 0x006b6b6b);

   CHECK(subresource.at(0, 2, 0) == 0x00737373);
   CHECK(subresource.at(1, 2, 0) == 0xffadadad);
   CHECK(subresource.at(2, 2, 0) == 0x00737373);
   CHECK(subresource.at(3, 2, 0) == 0xff5a5a5a);

   CHECK(subresource.at(0, 3, 0) == 0xff5a5a5a);
   CHECK(subresource.at(1, 3, 0) == 0x00848484);
   CHECK(subresource.at(2, 3, 0) == 0xff4a4a4a);
   CHECK(subresource.at(3, 3, 0) == 0x00a5a5a5);
}

TEST_CASE("texture_munge load_texture cube", "[Munge]")
{
   const io::path input_path = R"(data\munge\textures\load_cube.tga)";

   load_texture_result result =
      load_texture(input_path, {.type = texture_type::cube});

   CHECK(not result.traits.has_alpha);
   CHECK(not result.traits.has_single_bit_alpha);
   CHECK(not result.traits.is_greyscale);

   texture& texture = result.texture;

   REQUIRE(texture.width() == 4);
   REQUIRE(texture.height() == 4);
   REQUIRE(texture.depth() == 1);
   REQUIRE(texture.array_size() == 6);
   REQUIRE(texture.mip_levels() == 3);

   for (uint32 y = 0; y < 4; ++y) {
      for (uint32 x = 0; x < 4; ++x) {
         texture_subresource subresource =
            texture.subresource({.array_index = 0, .mip_level = 0});

         CHECK(subresource.at(x, y, 0) == 0xff0030ff);
      }
   }

   for (uint32 y = 0; y < 4; ++y) {
      for (uint32 x = 0; x < 4; ++x) {
         texture_subresource subresource =
            texture.subresource({.array_index = 1, .mip_level = 0});

         CHECK(subresource.at(x, y, 0) == 0xff00ff12);
      }
   }

   for (uint32 y = 0; y < 4; ++y) {
      for (uint32 x = 0; x < 4; ++x) {
         texture_subresource subresource =
            texture.subresource({.array_index = 2, .mip_level = 0});

         CHECK(subresource.at(x, y, 0) == 0xffff0000);
      }
   }

   for (uint32 y = 0; y < 4; ++y) {
      for (uint32 x = 0; x < 4; ++x) {
         texture_subresource subresource =
            texture.subresource({.array_index = 3, .mip_level = 0});

         CHECK(subresource.at(x, y, 0) == 0xfffff600);
      }
   }

   for (uint32 y = 0; y < 4; ++y) {
      for (uint32 x = 0; x < 4; ++x) {
         texture_subresource subresource =
            texture.subresource({.array_index = 4, .mip_level = 0});

         CHECK(subresource.at(x, y, 0) == 0xffc000ff);
      }
   }

   for (uint32 y = 0; y < 4; ++y) {
      for (uint32 x = 0; x < 4; ++x) {
         texture_subresource subresource =
            texture.subresource({.array_index = 5, .mip_level = 0});

         CHECK(subresource.at(x, y, 0) == 0xff00fff0);
      }
   }
}

TEST_CASE("texture_munge load_texture volume", "[Munge]")
{
   const io::path input_path = R"(data\munge\textures\load_volume.tga)";

   load_texture_result result =
      load_texture(input_path, {.type = texture_type::volume, .depth = 4});

   CHECK(not result.traits.has_alpha);
   CHECK(not result.traits.has_single_bit_alpha);
   CHECK(not result.traits.is_greyscale);

   texture& texture = result.texture;

   REQUIRE(texture.width() == 4);
   REQUIRE(texture.height() == 4);
   REQUIRE(texture.depth() == 4);
   REQUIRE(texture.array_size() == 1);
   REQUIRE(texture.mip_levels() == 3);

   const std::array<uint32, 4> slice_colors = {0xff0030ff, 0xff00fff0,
                                               0xfff400ff, 0xffff9a00};

   for (uint32 z = 0; z < 4; ++z) {
      for (uint32 y = 0; y < 4; ++y) {
         for (uint32 x = 0; x < 4; ++x) {
            texture_subresource subresource =
               texture.subresource({.array_index = 0, .mip_level = 0});

            CHECK(subresource.at(x, y, z) == slice_colors[z]);
         }
      }
   }
}
}