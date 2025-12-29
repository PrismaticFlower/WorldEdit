#include "texture_convert.hpp"
#include "error.hpp"

#include <BC.h> // BC.h from DirectXTex

#include <array>

namespace we::munge {

namespace {

template<typename T>
void write_texel(const uint32 width, std::span<std::byte> output, uint32 x,
                 uint32 y, const T& texel) noexcept
{
   const std::size_t offset = y * width * sizeof(T) + x * sizeof(T);

   assert(offset + sizeof(T) <= output.size());

   std::memcpy(&output[offset], &texel, sizeof(T));
}

void convert_slice_dxt1(texture_slice input, std::span<std::byte> output)
{
   const uint32 blocks_width = (input.width() + 3) / 4;
   const uint32 blocks_height = (input.height() + 3) / 4;

   if (blocks_width * blocks_height * sizeof(uint8[8]) > output.size()) {
      throw texture_error{"Not enough memory to convert format.",
                          texture_ec::format_convert_not_enough_memory};
   }

   for (uint32 block_y = 0; block_y < blocks_height; ++block_y) {
      for (uint32 block_x = 0; block_x < blocks_width; ++block_x) {
         std::array<uint8, 8> compressed_block = {};
         std::array<DirectX::XMVECTOR, 16> src_block = {};

         for (uint32 y = 0; y < 4; ++y) {
            for (uint32 x = 0; x < 4; ++x) {
               DirectX::PackedVector::XMCOLOR color{
                  input.at(block_x * 4 + x, block_y * 4 + y)};

               src_block[y * 4 + x] = DirectX::PackedVector::XMLoadColor(&color);
            }
         }

         DirectX::D3DXEncodeBC1(compressed_block.data(), src_block.data(), 0.5f, 0);

         write_texel(blocks_width, output, block_x, block_y, compressed_block);
      }
   }
}

void convert_slice_dxt3(texture_slice input, std::span<std::byte> output)
{
   const uint32 blocks_width = (input.width() + 3) / 4;
   const uint32 blocks_height = (input.height() + 3) / 4;

   if (blocks_width * blocks_height * sizeof(uint8[16]) > output.size()) {
      throw texture_error{"Not enough memory to convert format.",
                          texture_ec::format_convert_not_enough_memory};
   }

   for (uint32 block_y = 0; block_y < blocks_height; ++block_y) {
      for (uint32 block_x = 0; block_x < blocks_width; ++block_x) {
         std::array<uint8, 16> compressed_block = {};
         std::array<DirectX::XMVECTOR, 16> src_block = {};

         for (uint32 y = 0; y < 4; ++y) {
            for (uint32 x = 0; x < 4; ++x) {
               DirectX::PackedVector::XMCOLOR color{
                  input.at(block_x * 4 + x, block_y * 4 + y)};

               src_block[y * 4 + x] = DirectX::PackedVector::XMLoadColor(&color);
            }
         }

         DirectX::D3DXEncodeBC2(compressed_block.data(), src_block.data(), 0);

         write_texel(blocks_width, output, block_x, block_y, compressed_block);
      }
   }
}

void convert_slice_dxt5(texture_slice input, std::span<std::byte> output)
{
   const uint32 blocks_width = (input.width() + 3) / 4;
   const uint32 blocks_height = (input.height() + 3) / 4;

   if (blocks_width * blocks_height * sizeof(uint8[16]) > output.size()) {
      throw texture_error{"Not enough memory to convert format.",
                          texture_ec::format_convert_not_enough_memory};
   }

   for (uint32 block_y = 0; block_y < blocks_height; ++block_y) {
      for (uint32 block_x = 0; block_x < blocks_width; ++block_x) {
         std::array<uint8, 16> compressed_block = {};
         std::array<DirectX::XMVECTOR, 16> src_block = {};

         for (uint32 y = 0; y < 4; ++y) {
            for (uint32 x = 0; x < 4; ++x) {
               DirectX::PackedVector::XMCOLOR color{
                  input.at(block_x * 4 + x, block_y * 4 + y)};

               src_block[y * 4 + x] = DirectX::PackedVector::XMLoadColor(&color);
            }
         }

         DirectX::D3DXEncodeBC3(compressed_block.data(), src_block.data(), 0);

         write_texel(blocks_width, output, block_x, block_y, compressed_block);
      }
   }
}

void convert_slice_a8r8g8b8([[maybe_unused]] texture_slice input,
                            [[maybe_unused]] std::span<std::byte> output)
{
}

void convert_slice_a4r4g4b4(texture_slice input, std::span<std::byte> output)
{
   if (input.width() * input.height() * sizeof(uint16) > output.size()) {
      throw texture_error{"Not enough memory to convert format.",
                          texture_ec::format_convert_not_enough_memory};
   }

   for (uint32 y = 0; y < input.height(); ++y) {
      for (uint32 x = 0; x < input.width(); ++x) {
         const uint32 color_32 = input.at(x, y);

         const uint32 red = ((color_32 >> 16) & 0xff) >> 4;
         const uint32 green = ((color_32 >> 8) & 0xff) >> 4;
         const uint32 blue = (color_32 & 0xff) >> 4;
         const uint32 alpha = ((color_32 >> 24) & 0xff) >> 4;

         uint32 color_16 = blue;

         color_16 |= red << 8;
         color_16 |= green << 4;
         color_16 |= alpha << 12;

         write_texel(input.width(), output, x, y, static_cast<uint16>(color_16));
      }
   }
}

void convert_slice_a1r5g5b5(texture_slice input, std::span<std::byte> output)
{
   if (input.width() * input.height() * sizeof(uint16) > output.size()) {
      throw texture_error{"Not enough memory to convert format.",
                          texture_ec::format_convert_not_enough_memory};
   }

   for (uint32 y = 0; y < input.height(); ++y) {
      for (uint32 x = 0; x < input.width(); ++x) {
         const uint32 color_32 = input.at(x, y);

         const uint32 red = ((color_32 >> 16) & 0xff) >> 3;
         const uint32 green = ((color_32 >> 8) & 0xff) >> 3;
         const uint32 blue = (color_32 & 0xff) >> 3;
         const uint32 alpha = ((color_32 >> 24) & 0xff) >> 7;

         uint32 color_16 = blue;

         color_16 |= red << 10;
         color_16 |= green << 5;
         color_16 |= alpha << 15;

         write_texel(input.width(), output, x, y, static_cast<uint16>(color_16));
      }
   }
}

void convert_slice_r5g6b5(texture_slice input, std::span<std::byte> output)
{
   if (input.width() * input.height() * sizeof(uint16) > output.size()) {
      throw texture_error{"Not enough memory to convert format.",
                          texture_ec::format_convert_not_enough_memory};
   }

   for (uint32 y = 0; y < input.height(); ++y) {
      for (uint32 x = 0; x < input.width(); ++x) {
         const uint32 color_32 = input.at(x, y);

         const uint32 red = ((color_32 >> 16) & 0xff) >> 3;
         const uint32 green = ((color_32 >> 8) & 0xff) >> 2;
         const uint32 blue = (color_32 & 0xff) >> 3;

         uint32 color_16 = blue;

         color_16 |= red << 11;
         color_16 |= green << 5;

         write_texel(input.width(), output, x, y, static_cast<uint16>(color_16));
      }
   }
}

void convert_slice_a8l8(texture_slice input, std::span<std::byte> output)
{
   if (input.width() * input.height() * sizeof(uint16) > output.size()) {
      throw texture_error{"Not enough memory to convert format.",
                          texture_ec::format_convert_not_enough_memory};
   }

   for (uint32 y = 0; y < input.height(); ++y) {
      for (uint32 x = 0; x < input.width(); ++x) {
         const uint32 color_32 = input.at(x, y);

         const uint32 luma = color_32 & 0xff;
         const uint32 alpha = (color_32 >> 24) & 0xff;

         uint32 color_16 = luma;

         color_16 |= alpha << 8;

         write_texel(input.width(), output, x, y, static_cast<uint16>(color_16));
      }
   }
}

void convert_slice_a8(texture_slice input, std::span<std::byte> output)
{
   if (input.width() * input.height() > output.size()) {
      throw texture_error{"Not enough memory to convert format.",
                          texture_ec::format_convert_not_enough_memory};
   }

   for (uint32 y = 0; y < input.height(); ++y) {
      for (uint32 x = 0; x < input.width(); ++x) {
         const uint32 alpha = (input.at(x, y) >> 24) & 0xff;

         write_texel(input.width(), output, x, y, static_cast<uint8>(alpha));
      }
   }
}

void convert_slice_l8(texture_slice input, std::span<std::byte> output)
{
   if (input.width() * input.height() > output.size()) {
      throw texture_error{"Not enough memory to convert format.",
                          texture_ec::format_convert_not_enough_memory};
   }
   assert(input.width() * input.height() <= output.size());

   for (uint32 y = 0; y < input.height(); ++y) {
      for (uint32 x = 0; x < input.width(); ++x) {
         write_texel(input.width(), output, x, y,
                     static_cast<uint8>(input.at(x, y) & 0xffu));
      }
   }
}

void convert_slice_a4l4(texture_slice input, std::span<std::byte> output)
{
   if (input.width() * input.height() > output.size()) {
      throw texture_error{"Not enough memory to convert format.",
                          texture_ec::format_convert_not_enough_memory};
   }

   for (uint32 y = 0; y < input.height(); ++y) {
      for (uint32 x = 0; x < input.width(); ++x) {
         const uint32 color_32 = input.at(x, y);

         const uint32 luma = (color_32 & 0xff) >> 4;
         const uint32 alpha = ((color_32 >> 24) & 0xff) >> 4;

         uint32 color_8 = luma;

         color_8 |= alpha << 4;

         write_texel(input.width(), output, x, y, static_cast<uint8>(color_8));
      }
   }
}

void convert_slice_v8u8(texture_slice input, std::span<std::byte> output)
{
   if (input.width() * input.height() * sizeof(uint16) > output.size()) {
      throw texture_error{"Not enough memory to convert format.",
                          texture_ec::format_convert_not_enough_memory};
   }

   for (uint32 y = 0; y < input.height(); ++y) {
      for (uint32 x = 0; x < input.width(); ++x) {
         const uint32 color_32 = input.at(x, y);

         const uint32 red =
            static_cast<uint8>(static_cast<int>((color_32 >> 16) & 0xff) - 128);
         const uint32 green =
            static_cast<uint8>(static_cast<int>((color_32 >> 8) & 0xff) - 128);

         uint32 color_16 = red;

         color_16 |= green << 8;

         write_texel(input.width(), output, x, y, static_cast<uint16>(color_16));
      }
   }
}

auto convert_texture(texture& input, texture_write_format format,
                     void (*convert_slice)(texture_slice input,
                                           std::span<std::byte> output)) -> texture_transmuted_view
{
   texture_transmuted_view output{input, format};

   for (uint32 array_index = 0; array_index < input.array_size(); ++array_index) {
      for (uint32 mip_level = 0; mip_level < input.mip_levels(); ++mip_level) {
         texture_subresource input_subresource =
            input.subresource({.array_index = array_index, .mip_level = mip_level});
         texture_transmuted_view_subresource out_subresource =
            output.subresource({.array_index = array_index, .mip_level = mip_level});

         for (uint32 z = 0; z < input_subresource.depth(); ++z) {
            convert_slice(input_subresource.slice(z), out_subresource.slice(z));
         }
      }
   }

   return output;
}

}

auto convert_texture(texture& texture, const texture_write_format format)
   -> texture_transmuted_view
{
   switch (format) {
   case texture_write_format::dxt1:
      return convert_texture(texture, format, convert_slice_dxt1);
   case texture_write_format::dxt3:
      return convert_texture(texture, format, convert_slice_dxt3);
   case texture_write_format::dxt5:
      return convert_texture(texture, format, convert_slice_dxt5);
   case texture_write_format::a8r8g8b8:
      return convert_texture(texture, format, convert_slice_a8r8g8b8);
   case texture_write_format::a4r4g4b4:
      return convert_texture(texture, format, convert_slice_a4r4g4b4);
   case texture_write_format::a1r5g5b5:
      return convert_texture(texture, format, convert_slice_a1r5g5b5);
   case texture_write_format::r5g6b5:
      return convert_texture(texture, format, convert_slice_r5g6b5);
   case texture_write_format::a8l8:
      return convert_texture(texture, format, convert_slice_a8l8);
   case texture_write_format::a8:
      return convert_texture(texture, format, convert_slice_a8);
   case texture_write_format::l8:
      return convert_texture(texture, format, convert_slice_l8);
   case texture_write_format::a4l4:
      return convert_texture(texture, format, convert_slice_a4l4);
   case texture_write_format::v8u8:
      return convert_texture(texture, format, convert_slice_v8u8);
   }

   std::unreachable();
}

}