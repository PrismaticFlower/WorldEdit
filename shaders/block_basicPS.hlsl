struct input_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMALWS;
   float2 texcoords : TEXCOORD;
   nointerpolation uint material_index : MATERIAL;

   float4 positionSS : SV_Position;
};

float4 main(input_vertex input) : SV_TARGET
{
   const float3 light_normalWS = normalize(float3(-159.264923, 300.331013, -66.727310));

   float3 light = saturate(dot(normalize(input.normalWS), light_normalWS));

   return float4(light * 0.6 + (1.0 - light) * 0.05.xxx, 1.0f);
}