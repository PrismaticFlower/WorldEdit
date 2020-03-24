struct input_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMALWS;
   float2 texcoords : TEXCOORD;
};

float4 main(input_vertex input_vertex) : SV_TARGET
{
   const float3 light_normalWS =
      normalize(float3(-159.264923, 300.331013, -66.727310));

   return float4(dot(normalize(input_vertex.normalWS), light_normalWS) * 0.6.xxx, 1.0f);
}