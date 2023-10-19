struct output_vertex {
   float2 location : LOCATION;
   uint face_index : SV_RenderTargetArrayIndex;
   float4 positionPS : SV_Position;
};

output_vertex main(uint vertex_id : SV_VertexID)
{
   output_vertex output;
   
   output.location = 0.0;
   output.face_index = vertex_id / 3;
   output.positionPS = 0.0;
   
   switch (vertex_id % 3) {
      case 0:
         output.location = float2(0.0, 1.0);
         output.positionPS = float4(-1.f, -1.f, 0.0, 1.0);
         break;
      case 1:
         output.location = float2(2.0, 1.0);
         output.positionPS = float4(3.f, -1.f, 0.0, 1.0);
         break;
      case 2:
         output.location = float2(0.0, -1.0);
         output.positionPS = float4(-1.f, 3.f, 0.0, 1.0);
         break;
   }
   
   return output;
}