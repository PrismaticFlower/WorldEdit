float4 main(uint vertex_index : SV_VertexID) : SV_Position
{
   switch (vertex_index) {
      case 0:
         return float4(-1.f, -1.f, 0.0, 1.0);
      case 1:
         return float4(3.f, -1.f, 0.0, 1.0);
      case 2:
         return float4(-1.f, 3.f, 0.0, 1.0);
   }
   
   return float4(0.0, 0.0, 0.0, 0.0);
}