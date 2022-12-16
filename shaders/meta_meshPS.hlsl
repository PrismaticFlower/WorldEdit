#include "bindings.hlsli"

struct color_constants {
   float4 color;
};

ConstantBuffer<color_constants> cb_color_constants : register(META_MESH_CB_REGISTER);

float4 main() : SV_TARGET
{
   return float4(cb_color_constants.color.rgb * cb_color_constants.color.a, cb_color_constants.color.a);
}