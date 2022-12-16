#pragma once

#include "rhi.hpp"

namespace we::graphics::gpu {

[[nodiscard]] inline auto transition_barrier(
   gpu::resource_handle resource, const gpu::resource_state state_before,
   const gpu::resource_state state_after,
   const barrier_split split = barrier_split::none,
   const uint32 subresource = gpu::resource_barrier_all_subresources) -> gpu::resource_barrier
{
   return {.type = barrier_type::transition,
           .split = split,
           .transition = {.resource = resource,
                          .subresource = subresource,
                          .state_before = state_before,
                          .state_after = state_after}};
}

[[nodiscard]] inline auto uav_barrier(gpu::resource_handle resource,
                                      const barrier_split split = barrier_split::none)
   -> gpu::resource_barrier
{
   return {.type = barrier_type::uav, .split = split, .uav = {.resource = resource}};
}

}
