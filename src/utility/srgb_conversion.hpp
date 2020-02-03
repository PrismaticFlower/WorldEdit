#pragma once

#include "types.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace sk::utility {

template<typename Float>
inline auto decompress_srgb(const Float v) -> Float
{
   return (v < Float{0.04045})
             ? v / Float{12.92}
             : std::pow(std::abs((v + Float{0.055})) / Float{1.055}, Float{2.4});
}

inline auto decompress_srgb(const vec3 color) -> vec3
{
   return {decompress_srgb(color.r), decompress_srgb(color.g),
           decompress_srgb(color.b)};
}

inline auto decompress_srgb(const vec4 color) -> vec4
{
   return {decompress_srgb(color.r), decompress_srgb(color.g),
           decompress_srgb(color.b), color.a};
}

template<typename Float>
inline auto compress_srgb(const Float v)
{
   return (v < Float{0.0031308})
             ? v * Float{12.92}
             : Float{1.055} * std::pow(std::abs(v), Float{1.0} / Float{2.4}) -
                  Float{0.055};
}

inline auto compress_srgb(const vec3 color) -> vec3
{
   return {compress_srgb(color.r), compress_srgb(color.g), compress_srgb(color.b)};
}

inline auto compress_srgb(const vec4 color) -> vec4
{
   return {compress_srgb(color.r), compress_srgb(color.g),
           compress_srgb(color.b), color.a};
}

inline constexpr std::array srgb_unorm_to_float_lut = {
   0.0f,          0.000303527f,  0.000607054f,  0.000910581f, 0.001214108f,
   0.001517635f,  0.001821162f,  0.0021246888f, 0.002428216f, 0.0027317428f,
   0.00303527f,   0.0033465358f, 0.0036765074f, 0.004024717f, 0.004391442f,
   0.0047769533f, 0.0051815165f, 0.0056053917f, 0.006048833f, 0.0065120906f,
   0.00699541f,   0.007499032f,  0.008023193f,  0.008568126f, 0.009134059f,
   0.009721218f,  0.010329823f,  0.010960094f,  0.011612245f, 0.012286488f,
   0.0129830325f, 0.013702083f,  0.014443844f,  0.015208514f, 0.015996294f,
   0.016807375f,  0.017641954f,  0.01850022f,   0.019382361f, 0.020288562f,
   0.02121901f,   0.022173885f,  0.023153367f,  0.024157632f, 0.02518686f,
   0.026241222f,  0.027320892f,  0.02842604f,   0.029556835f, 0.030713445f,
   0.031896032f,  0.033104766f,  0.034339808f,  0.035601314f, 0.03688945f,
   0.038204372f,  0.039546236f,  0.0409152f,    0.04231141f,  0.04373503f,
   0.045186203f,  0.046665087f,  0.048171826f,  0.049706567f, 0.051269457f,
   0.052860647f,  0.054480277f,  0.05612849f,   0.05780543f,  0.059511237f,
   0.061246052f,  0.063010015f,  0.064803265f,  0.06662594f,  0.06847817f,
   0.070360094f,  0.07227185f,   0.07421357f,   0.07618538f,  0.07818742f,
   0.08021982f,   0.08228271f,   0.08437621f,   0.08650046f,  0.08865558f,
   0.09084171f,   0.093058966f,  0.09530747f,   0.09758735f,  0.099898726f,
   0.10224173f,   0.104616486f,  0.107023105f,  0.10946171f,  0.11193243f,
   0.114435375f,  0.116970666f,  0.11953843f,   0.122138776f, 0.12477182f,
   0.12743768f,   0.13013647f,   0.13286832f,   0.13563333f,  0.13843161f,
   0.14126329f,   0.14412847f,   0.14702727f,   0.14995979f,  0.15292615f,
   0.15592647f,   0.15896083f,   0.16202937f,   0.1651322f,   0.1682694f,
   0.17144111f,   0.1746474f,    0.17788842f,   0.18116425f,  0.18447499f,
   0.18782078f,   0.19120169f,   0.19461784f,   0.19806932f,  0.20155625f,
   0.20507874f,   0.20863687f,   0.21223076f,   0.2158605f,   0.2195262f,
   0.22322796f,   0.22696587f,   0.23074006f,   0.23455058f,  0.23839757f,
   0.24228112f,   0.24620132f,   0.25015828f,   0.2541521f,   0.25818285f,
   0.26225066f,   0.2663556f,    0.2704978f,    0.2746773f,   0.27889428f,
   0.28314874f,   0.28744084f,   0.29177064f,   0.29613826f,  0.30054379f,
   0.3049873f,    0.30946892f,   0.31398872f,   0.31854677f,  0.3231432f,
   0.3277781f,    0.33245152f,   0.33716363f,   0.34191442f,  0.34670407f,
   0.3515326f,    0.35640013f,   0.3613068f,    0.3662526f,   0.3712377f,
   0.37626213f,   0.38132602f,   0.38642943f,   0.39157248f,  0.39675522f,
   0.40197778f,   0.4072402f,    0.4125426f,    0.41788507f,  0.42326766f,
   0.4286905f,    0.43415365f,   0.43965718f,   0.4452012f,   0.4507858f,
   0.45641103f,   0.462077f,     0.4677838f,    0.47353148f,  0.47932017f,
   0.48514995f,   0.49102086f,   0.49693298f,   0.5028865f,   0.50888133f,
   0.5149177f,    0.52099556f,   0.5271151f,    0.5332764f,   0.5394795f,
   0.54572445f,   0.55201143f,   0.5583404f,    0.5647115f,   0.57112485f,
   0.57758045f,   0.58407843f,   0.59061885f,   0.59720176f,  0.60382736f,
   0.61049557f,   0.6172066f,    0.6239604f,    0.63075715f,  0.63759685f,
   0.6444797f,    0.65140563f,   0.65837485f,   0.6653873f,   0.67244315f,
   0.6795425f,    0.6866853f,    0.69387174f,   0.7011019f,   0.70837575f,
   0.7156935f,    0.7230551f,    0.73046076f,   0.7379104f,   0.7454042f,
   0.7529422f,    0.7605245f,    0.76815116f,   0.7758222f,   0.7835378f,
   0.7912979f,    0.7991027f,    0.80695224f,   0.8148466f,   0.82278574f,
   0.8307699f,    0.838799f,     0.8468732f,    0.8549926f,   0.8631572f,
   0.8713671f,    0.8796224f,    0.8879231f,    0.8962694f,   0.9046612f,
   0.91309863f,   0.92158186f,   0.9301109f,    0.9386857f,   0.9473065f,
   0.9559733f,    0.9646863f,    0.9734453f,    0.9822506f,   0.9911021f,
   1.0f,
};

inline auto unpack_srgb_bgra(const uint32 bgra) -> vec4
{
   return {srgb_unorm_to_float_lut[(bgra >> 16) & 0xff],
           srgb_unorm_to_float_lut[(bgra >> 8) & 0xff],
           srgb_unorm_to_float_lut[bgra & 0xff], ((bgra >> 24) & 0xff) / 255.0f};
}

inline auto pack_srgb_bgra(const vec4 color) -> uint32
{
   const auto pack_unorm = [](const float v) -> uint32 {
      return static_cast<uint32>(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f);
   };

   uint32 packed = 0;

   packed |= pack_unorm(compress_srgb(color.b));
   packed |= pack_unorm(compress_srgb(color.g)) << 8;
   packed |= pack_unorm(compress_srgb(color.r)) << 16;
   packed |= pack_unorm(compress_srgb(color.a)) << 24;

   return packed;
}

}
