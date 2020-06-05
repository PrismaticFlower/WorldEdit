#pragma once

#include "frustrum.hpp"
#include "gpu/buffer.hpp"
#include "gpu/command_list.hpp"
#include "gpu/descriptor_heap.hpp"
#include "gpu/device.hpp"
#include "gpu/dynamic_buffer_allocator.hpp"
#include "world/world.hpp"

#include <array>
#include <vector>

namespace sk::graphics {

class light_clusters {
public:
   light_clusters(gpu::device& gpu_device);

   void update_lights(const frustrum& view_frustrum, const world::world& world,
                      gpu::command_list& command_list,
                      gpu::dynamic_buffer_allocator& dynamic_buffer_allocator);

   auto light_descriptors() const noexcept -> gpu::descriptor_range;

private:
   gsl::not_null<gpu::device*> _gpu_device;

   gpu::buffer _lights_constant_buffer;
   gpu::buffer _regional_lights_buffer;

   gpu::descriptor_range _descriptors;
};

}
