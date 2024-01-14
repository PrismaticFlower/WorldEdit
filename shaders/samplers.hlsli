#pragma once

SamplerState sampler_bilinear_wrap : register(s0);
SamplerState sampler_trilinear_wrap  : register(s1);
SamplerState sampler_anisotropic_wrap : register(s2);

SamplerState sampler_bilinear_clamp : register(s3);
SamplerState sampler_trilinear_clamp : register(s4);
SamplerState sampler_anisotropic_clamp : register(s5);

SamplerComparisonState sampler_shadow : register(s6);
