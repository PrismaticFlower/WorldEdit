uint Hash(uint s)
{
   s ^= 2747636419u;
   s *= 2654435769u;
   s ^= s >> 16;
   s *= 2654435769u;
   s ^= s >> 16;
   s *= 2654435769u;
   return s;
}

float Random(uint seed)
{
   return float(Hash(seed)) / 4294967295.0; // 2^32-1
}

float4 main(uint id : SV_PrimitiveID) : SV_TARGET
{
   float3 col = {Random(id), Random(id), Random(id)};

   return float4(col, 1.0f);
}