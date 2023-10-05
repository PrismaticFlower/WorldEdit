struct output_vertex {
   float2 texcoords : TEXCOORDS;
   float4 positionPS : SV_Position;
};

output_vertex main(uint vertex_index : SV_VertexID)
{
   output_vertex output;
   
   output.texcoords = 0.0;
   output.positionPS = 0.0;
   
   switch (vertex_index) {
      case 0:
         output.texcoords = float2(0.0, 1.0);
         output.positionPS = float4(-1.f, -1.f, 0.0, 1.0);
         break;
      case 1:
         output.texcoords = float2(2.0, 1.0);
         output.positionPS = float4(3.f, -1.f, 0.0, 1.0);
         break;
      case 2:
         output.texcoords = float2(0.0, -1.0);
         output.positionPS = float4(-1.f, 3.f, 0.0, 1.0);
         break;
   }
   
   return output;
}