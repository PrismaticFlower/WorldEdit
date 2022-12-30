

float4 main(float4 color : COLOR) : SV_TARGET
{
   return float4(color.rgb * color.a, color.a);
}