#pragma once

const static uint sampler_bilinear_wrap_index = 0;
const static uint sampler_trilinear_wrap_index = 1;
const static uint sampler_anisotropic_wrap_index = 2;

const static uint sampler_bilinear_clamp_index = 3;
const static uint sampler_trilinear_clamp_index = 4;
const static uint sampler_anisotropic_clamp_index = 5;

const static uint sampler_shadow_index = 6;

static SamplerState sampler_bilinear_wrap = SamplerDescriptorHeap[sampler_bilinear_wrap_index];
static SamplerState sampler_trilinear_wrap = SamplerDescriptorHeap[sampler_trilinear_wrap_index];
static SamplerState sampler_anisotropic_wrap = SamplerDescriptorHeap[sampler_anisotropic_wrap_index];

static SamplerState sampler_bilinear_clamp = SamplerDescriptorHeap[sampler_bilinear_clamp_index];
static SamplerState sampler_trilinear_clamp = SamplerDescriptorHeap[sampler_trilinear_clamp_index];
static SamplerState sampler_anisotropic_clamp = SamplerDescriptorHeap[sampler_anisotropic_clamp_index];

static SamplerComparisonState sampler_shadow = SamplerDescriptorHeap[sampler_shadow_index];
