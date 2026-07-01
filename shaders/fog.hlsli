#include "frame_constants.hlsli"

float calculate_fog(float3 positionWS, float4 positionPS)
{
	const float view_fog = positionPS.w * cb_frame.fog_mul_add.x + cb_frame.fog_mul_add.y;
	const float world_fog = positionWS.y * cb_frame.world_fog_mul_add.x + cb_frame.world_fog_mul_add.y;

	return saturate(min(view_fog, world_fog));
}

float4 apply_fog(float4 color, float fog)
{
	return float4(lerp(cb_frame.fog_color, color.rgb, fog), color.a);
}