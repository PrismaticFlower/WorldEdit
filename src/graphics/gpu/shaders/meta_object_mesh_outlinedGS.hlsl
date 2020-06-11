struct input_vertex {
   float4 positionPS : SV_Position;
};

struct output_vertex {
   float4 positionPS : SV_Position;

   nointerpolation float4 tri_positionPS[3] : POSITIONPS;
};

[maxvertexcount(3)] void main(triangle input_vertex input[3],
                              inout TriangleStream<output_vertex> output) {
   const float4 tri_positionPS[3] = {input[0].positionPS, input[1].positionPS,
                                     input[2].positionPS};

   for (uint i = 0; i < 3; i++) {
      output_vertex vertex;
      vertex.positionPS = tri_positionPS[i];
      vertex.tri_positionPS = tri_positionPS;
      output.Append(vertex);
   }
}