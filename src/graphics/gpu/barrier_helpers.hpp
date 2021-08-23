#pragma once

#include <d3d12.h>

namespace we::graphics::gpu {

[[nodiscard]] inline auto transition_barrier(
   ID3D12Resource& resource, const D3D12_RESOURCE_STATES state_before,
   const D3D12_RESOURCE_STATES state_after,
   const D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
   const uint32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) -> D3D12_RESOURCE_BARRIER
{
   return {.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
           .Flags = flags,
           .Transition = {.pResource = &resource,
                          .Subresource = subresource,
                          .StateBefore = state_before,
                          .StateAfter = state_after}};
}

[[nodiscard]] inline auto uav_barrier(ID3D12Resource& resource,
                                      const D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE)
   -> D3D12_RESOURCE_BARRIER
{
   return {.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
           .Flags = flags,
           .UAV = {.pResource = &resource}};
}

}
