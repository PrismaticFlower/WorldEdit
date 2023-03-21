struct input_vertex {
   float4 color : COLOR;
   float4 positionPS : SV_Position;
   nointerpolation float4 flat_positionPS : FLAT_POSITIONPS;
};

struct output_vertex {
   float4 color : COLOR;
   float4 positionPS : SV_Position;
   nointerpolation float4 flat_positionPS[3] : FLAT_POSITIONPS;
};

[maxvertexcount(3)] void main(triangle input_vertex input[3], inout TriangleStream<output_vertex> output) {
   const float4 flat_positionPS[3] = {input[0].flat_positionPS, input[1].flat_positionPS,
                                      input[2].flat_positionPS};

   for (uint i = 0; i < 3; i++) {
      output_vertex vertex;

      vertex.color = input[i].color;
      vertex.positionPS = input[i].positionPS;

      vertex.flat_positionPS = flat_positionPS;
      output.Append(vertex);
   }
}