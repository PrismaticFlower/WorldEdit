
#include "texture_format.hpp"
#include "utility/srgb_conversion.hpp"

#include <cassert>
#include <stdexcept>

#include <glm/gtc/packing.hpp>

namespace sk::assets::texture {

namespace {

template<typename Byte>
auto get_texel_bytes(const texture_format format, const uint32 x, const uint32 y,
                     const gsl::span<Byte> data, const uint32 row_pitch)
   -> gsl::span<Byte>
{
   assert(y * row_pitch <= data.size());

   const auto texel_size = format_size(format);
   const auto texel_offset = x * texel_size + (y * row_pitch);

   return data.subspan(texel_offset, texel_size);
}

auto clamp_unorm(const float4 value) noexcept -> float4
{
   return {glm::isnan(value.r) ? 0.0 : glm::clamp(value.r, 0.0f, 1.0f),
           glm::isnan(value.g) ? 0.0 : glm::clamp(value.g, 0.0f, 1.0f),
           glm::isnan(value.b) ? 0.0 : glm::clamp(value.b, 0.0f, 1.0f),
           glm::isnan(value.a) ? 0.0 : glm::clamp(value.a, 0.0f, 1.0f)};
}

auto load_r8g8b8a8_unorm(const gsl::span<const std::byte> texel_bytes) noexcept -> float4
{
   assert(texel_bytes.size() >= sizeof(uint32));

   uint32 packed{};

   std::memcpy(&packed, texel_bytes.data(), sizeof(uint32));

   return {(packed & 0xff) / 255.f,         //
           ((packed >> 8) & 0xff) / 255.f,  //
           ((packed >> 16) & 0xff) / 255.f, //
           ((packed >> 24) & 0xff) / 255.f};
}

auto load_r8g8b8a8_unorm_srgb(const gsl::span<const std::byte> texel_bytes) noexcept
   -> float4
{
   return utility::decompress_srgb(load_r8g8b8a8_unorm(texel_bytes));
}

auto load_b8g8r8a8_unorm(const gsl::span<const std::byte> texel_bytes) noexcept -> float4
{
   const float4 rgba = load_r8g8b8a8_unorm(texel_bytes);

   return {rgba.b, rgba.g, rgba.r, rgba.a};
}

auto load_b8g8r8a8_unorm_srgb(const gsl::span<const std::byte> texel_bytes) noexcept
   -> float4
{
   return utility::decompress_srgb(load_b8g8r8a8_unorm(texel_bytes));
}

auto load_r16g16b16a16_unorm(const gsl::span<const std::byte> texel_bytes) noexcept -> float4
{
   assert(texel_bytes.size() >= sizeof(uint64));

   uint64 packed{};

   std::memcpy(&packed, texel_bytes.data(), sizeof(uint64));

   return {(packed & 0xffff) / 65535.f,         //
           ((packed >> 16) & 0xffff) / 65535.f, //
           ((packed >> 32) & 0xffff) / 65535.f, //
           ((packed >> 48) & 0xffff) / 65535.f};
}

auto load_r16g16b16a16_float(const gsl::span<const std::byte> texel_bytes) noexcept -> float4
{
   assert(texel_bytes.size() >= sizeof(uint64));

   uint64 packed{};

   std::memcpy(&packed, texel_bytes.data(), sizeof(uint64));

   return {glm::unpackHalf1x16(packed & 0xffff),         //
           glm::unpackHalf1x16((packed >> 16) & 0xffff), //
           glm::unpackHalf1x16((packed >> 32) & 0xffff), //
           glm::unpackHalf1x16((packed >> 48) & 0xffff)};
}

auto load_r32g32b32_float(const gsl::span<const std::byte> texel_bytes) noexcept -> float4
{
   assert(texel_bytes.size() >= sizeof(float3));

   float3 value{};

   std::memcpy(&value, texel_bytes.data(), sizeof(float3));

   return {value, 1.0f};
}

auto load_r32g32b32a32_float(const gsl::span<const std::byte> texel_bytes) noexcept -> float4
{
   assert(texel_bytes.size() >= sizeof(float4));

   float4 value{};

   std::memcpy(&value, texel_bytes.data(), sizeof(float4));

   return value;
}

void store_r8g8b8a8_unorm(const float4 value, const gsl::span<std::byte> texel_bytes) noexcept
{
   assert(texel_bytes.size() >= sizeof(uint32));

   const float4 clamped = clamp_unorm(value);

   uint32 packed{};

   packed |= (static_cast<uint32>(clamped.r * 255.f + 0.5f));
   packed |= (static_cast<uint32>(clamped.g * 255.f + 0.5f) << 8);
   packed |= (static_cast<uint32>(clamped.b * 255.f + 0.5f) << 16);
   packed |= (static_cast<uint32>(clamped.a * 255.f + 0.5f) << 24);

   std::memcpy(texel_bytes.data(), &packed, sizeof(uint32));
}

void store_r8g8b8a8_unorm_srgb(const float4 value,
                               const gsl::span<std::byte> texel_bytes) noexcept
{
   store_r8g8b8a8_unorm(utility::compress_srgb(value), texel_bytes);
}

void store_b8g8r8a8_unorm(const float4 value, const gsl::span<std::byte> texel_bytes) noexcept
{
   store_r8g8b8a8_unorm({value.b, value.g, value.r, value.a}, texel_bytes);
}

void store_b8g8r8a8_unorm_srgb(const float4 value,
                               const gsl::span<std::byte> texel_bytes) noexcept
{
   store_b8g8r8a8_unorm(utility::compress_srgb(value), texel_bytes);
}

void store_r16g16b16a16_unorm(const float4 value,
                              const gsl::span<std::byte> texel_bytes) noexcept
{
   assert(texel_bytes.size() >= sizeof(uint64));

   const float4 clamped = clamp_unorm(value);

   uint64 packed{};

   packed |= (static_cast<uint64>(clamped.r * 65535.f + 0.5f));
   packed |= (static_cast<uint64>(clamped.g * 65535.f + 0.5f) << 16);
   packed |= (static_cast<uint64>(clamped.b * 65535.f + 0.5f) << 32);
   packed |= (static_cast<uint64>(clamped.a * 65535.f + 0.5f) << 48);

   std::memcpy(texel_bytes.data(), &packed, sizeof(uint64));
}

void store_r16g16b16a16_float(const float4 value,
                              const gsl::span<std::byte> texel_bytes) noexcept
{
   assert(texel_bytes.size() >= sizeof(uint64));

   uint64 packed{};

   packed |= (uint64{glm::packHalf1x16(value.r)});
   packed |= (uint64{glm::packHalf1x16(value.g)} << 16);
   packed |= (uint64{glm::packHalf1x16(value.b)} << 32);
   packed |= (uint64{glm::packHalf1x16(value.a)} << 48);

   std::memcpy(texel_bytes.data(), &packed, sizeof(uint64));
}

void store_r32g32b32_float(const float4 value, const gsl::span<std::byte> texel_bytes) noexcept

{
   assert(texel_bytes.size() >= sizeof(float3));

   std::memcpy(texel_bytes.data(), &value, sizeof(float3));
}

void store_r32g32b32a32_float(const float4 value,
                              const gsl::span<std::byte> texel_bytes) noexcept

{
   assert(texel_bytes.size() >= sizeof(float4));

   std::memcpy(texel_bytes.data(), &value, sizeof(float4));
}

}

auto load_texel(const texture_format format, const uint32 x, const uint32 y,
                const gsl::span<const std::byte> data, const uint32 width,
                const uint32 height, const uint32 row_pitch) -> float4
{
   (void)width, (void)height;
   assert(x < width and y < height);
   assert(y * row_pitch <= data.size());
   assert(format_size(format) * width <= row_pitch);

   const auto texel_bytes = get_texel_bytes(format, x, y, data, row_pitch);

   switch (format) {
   case texture_format::r8g8b8a8_unorm:
      return load_r8g8b8a8_unorm(texel_bytes);
   case texture_format::r8g8b8a8_unorm_srgb:
      return load_r8g8b8a8_unorm_srgb(texel_bytes);
   case texture_format::b8g8r8a8_unorm:
      return load_b8g8r8a8_unorm(texel_bytes);
   case texture_format::b8g8r8a8_unorm_srgb:
      return load_b8g8r8a8_unorm_srgb(texel_bytes);
   case texture_format::r16g16b16a16_unorm:
      return load_r16g16b16a16_unorm(texel_bytes);
   case texture_format::r16g16b16a16_float:
      return load_r16g16b16a16_float(texel_bytes);
   case texture_format::r32g32b32_float:
      return load_r32g32b32_float(texel_bytes);
   case texture_format::r32g32b32a32_float:
      return load_r32g32b32a32_float(texel_bytes);
   }

   std::terminate();
}

void store_texel(const float4 value, const texture_format format,
                 const uint32 x, const uint32 y, const gsl::span<std::byte> data,
                 const uint32 width, const uint32 height, const uint32 row_pitch)
{
   (void)width, (void)height;
   assert(x < width and y < height);
   assert(y * row_pitch <= data.size());
   assert(format_size(format) * width <= row_pitch);

   const auto texel_bytes = get_texel_bytes(format, x, y, data, row_pitch);

   switch (format) {
   case texture_format::r8g8b8a8_unorm:
      store_r8g8b8a8_unorm(value, texel_bytes);
      return;
   case texture_format::r8g8b8a8_unorm_srgb:
      store_r8g8b8a8_unorm_srgb(value, texel_bytes);
      return;
   case texture_format::b8g8r8a8_unorm:
      store_b8g8r8a8_unorm(value, texel_bytes);
      return;
   case texture_format::b8g8r8a8_unorm_srgb:
      store_b8g8r8a8_unorm_srgb(value, texel_bytes);
      return;
   case texture_format::r16g16b16a16_unorm:
      store_r16g16b16a16_unorm(value, texel_bytes);
      return;
   case texture_format::r16g16b16a16_float:
      store_r16g16b16a16_float(value, texel_bytes);
      return;
   case texture_format::r32g32b32_float:
      store_r32g32b32_float(value, texel_bytes);
      return;
   case texture_format::r32g32b32a32_float:
      store_r32g32b32a32_float(value, texel_bytes);
      return;
   }

   std::terminate();
}

}