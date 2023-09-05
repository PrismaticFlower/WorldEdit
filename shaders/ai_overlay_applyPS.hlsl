struct apply_inputs {
   float4 color;
};

ConstantBuffer<apply_inputs> input : register(b0);

float4 main(float4 positionSS : SV_Position) : SV_TARGET
{
   return input.color;
}