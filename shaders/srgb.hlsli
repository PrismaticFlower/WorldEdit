#pragma once

float srgb_to_linear(float color)
{
   return (color < 0.04045) ? color / 12.92 : pow(abs((color + 0.055)) / 1.055, 2.4);
}

float3 srgb_to_linear(float3 color)
{
   return float3(srgb_to_linear(color.r), srgb_to_linear(color.g), srgb_to_linear(color.b));
}

float4 srgb_to_linear(float4 color)
{
   return float4(srgb_to_linear(color.rgb), color.a);
}

float4 unpack_bgra_srgb(uint packed)
{
   uint b = (packed >> 0) & 0xff;
   uint g = (packed >> 8) & 0xff;
   uint r = (packed >> 16) & 0xff;
   uint a = (packed >> 24) & 0xff;

   return float4(srgb_to_linear(float3(r / 255.0, g / 255.0, b / 255.0)), a / 255.0);
}
