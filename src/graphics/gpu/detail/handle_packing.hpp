#pragma once

#include "../rhi.hpp"

#include <bit>
#include <type_traits>

#include <d3d12.h>

namespace we::graphics::gpu::detail::handle_packing {

auto pack_resource_handle(ID3D12Resource2* resource) -> resource_handle
{
   static_assert(std::is_same_v<std::underlying_type_t<resource_handle>, std::uintptr_t>,
                 "resource_handle's underlying type is incorrect");

   return std::bit_cast<resource_handle>(resource);
}

auto unpack_resource_handle(resource_handle resource) -> ID3D12Resource2*
{
   static_assert(std::is_same_v<std::underlying_type_t<resource_handle>, std::uintptr_t>,
                 "resource_handle's underlying type is incorrect");

   return std::bit_cast<ID3D12Resource2*>(resource);
}

auto pack_rtv_handle(D3D12_CPU_DESCRIPTOR_HANDLE descriptor) -> rtv_handle
{
   static_assert(std::is_same_v<std::underlying_type_t<rtv_handle>, std::uintptr_t>,
                 "rtv_handle's underlying type is incorrect");
   static_assert(sizeof(rtv_handle) == sizeof(D3D12_CPU_DESCRIPTOR_HANDLE),
                 "rtv_handle's size is incorrect");

   return rtv_handle{descriptor.ptr};
}

auto unpack_rtv_handle(rtv_handle descriptor) -> D3D12_CPU_DESCRIPTOR_HANDLE
{
   static_assert(std::is_same_v<std::underlying_type_t<rtv_handle>, std::uintptr_t>,
                 "rtv_handle's underlying type is incorrect");
   static_assert(sizeof(dsv_handle) == sizeof(D3D12_CPU_DESCRIPTOR_HANDLE),
                 "rtv_handle's size is incorrect");

   return D3D12_CPU_DESCRIPTOR_HANDLE{.ptr = static_cast<std::uintptr_t>(descriptor)};
}

auto pack_dsv_handle(D3D12_CPU_DESCRIPTOR_HANDLE descriptor) -> dsv_handle
{
   static_assert(std::is_same_v<std::underlying_type_t<dsv_handle>, std::uintptr_t>,
                 "dsv_handle's underlying type is incorrect");
   static_assert(sizeof(dsv_handle) == sizeof(D3D12_CPU_DESCRIPTOR_HANDLE),
                 "dsv_handle's size is incorrect");

   return dsv_handle{descriptor.ptr};
}

auto unpack_dsv_handle(dsv_handle descriptor) -> D3D12_CPU_DESCRIPTOR_HANDLE
{
   static_assert(std::is_same_v<std::underlying_type_t<resource_handle>, std::uintptr_t>,
                 "dsv_handle's underlying type is incorrect");
   static_assert(sizeof(dsv_handle) == sizeof(D3D12_CPU_DESCRIPTOR_HANDLE),
                 "dsv_handle's size is incorrect");

   return D3D12_CPU_DESCRIPTOR_HANDLE{.ptr = static_cast<std::uintptr_t>(descriptor)};
}

auto pack_root_signature_handle(ID3D12RootSignature* root_signature) -> root_signature_handle
{
   static_assert(std::is_same_v<std::underlying_type_t<root_signature_handle>, std::uintptr_t>,
                 "root_signature_handle's underlying type is incorrect");

   return std::bit_cast<root_signature_handle>(root_signature);
}

auto unpack_root_signature_handle(root_signature_handle root_signature)
   -> ID3D12RootSignature*
{
   static_assert(std::is_same_v<std::underlying_type_t<root_signature_handle>, std::uintptr_t>,
                 "root_signature_handle's underlying type is incorrect");

   return std::bit_cast<ID3D12RootSignature*>(root_signature);
}

auto pack_pipeline_handle(ID3D12PipelineState* pipeline) -> pipeline_handle
{
   static_assert(std::is_same_v<std::underlying_type_t<pipeline_handle>, std::uintptr_t>,
                 "pipeline_handle's underlying type is incorrect");

   return std::bit_cast<pipeline_handle>(pipeline);
}

auto unpack_pipeline_handle(pipeline_handle pipeline) -> ID3D12PipelineState*
{
   static_assert(std::is_same_v<std::underlying_type_t<pipeline_handle>, std::uintptr_t>,
                 "pipeline_handle's underlying type is incorrect");

   return std::bit_cast<ID3D12PipelineState*>(pipeline);
}

auto pack_sampler_heap_handle(ID3D12DescriptorHeap* heap) -> sampler_heap_handle
{
   static_assert(std::is_same_v<std::underlying_type_t<sampler_heap_handle>, std::uintptr_t>,
                 "sampler_heap_handle's underlying type is incorrect");

   return std::bit_cast<sampler_heap_handle>(heap);
}

auto unpack_sampler_heap_handle(sampler_heap_handle heap) -> ID3D12DescriptorHeap*
{
   static_assert(std::is_same_v<std::underlying_type_t<sampler_heap_handle>, std::uintptr_t>,
                 "sampler_heap_handle's underlying type is incorrect");

   return std::bit_cast<ID3D12DescriptorHeap*>(heap);
}

}