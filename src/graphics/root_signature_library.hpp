#pragma once

#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "types.hpp"

namespace we::graphics {

struct root_signature_library {
   explicit root_signature_library(gpu::device& device);

   gpu::unique_root_signature_handle mesh_shadow;
   gpu::unique_root_signature_handle mesh_depth_prepass;
   gpu::unique_root_signature_handle mesh;
   gpu::unique_root_signature_handle mesh_wireframe;
   gpu::unique_root_signature_handle terrain;
   gpu::unique_root_signature_handle sky_mesh;

   gpu::unique_root_signature_handle meta_draw;

   gpu::unique_root_signature_handle tile_lights_clear;
   gpu::unique_root_signature_handle tile_lights;

   gpu::unique_root_signature_handle depth_reduce_minmax;

   gpu::unique_root_signature_handle imgui;
};

namespace rs {

namespace mesh_shadow {
constexpr uint32 object_cbv = 0;
constexpr uint32 material_cbv = 1;
constexpr uint32 camera_cbv = 2;
}

namespace mesh_depth_prepass {
constexpr uint32 object_cbv = 0;
constexpr uint32 material_cbv = 1;
constexpr uint32 frame_cbv = 2;
}

namespace mesh {
constexpr uint32 object_cbv = 0;
constexpr uint32 material_cbv = 1;
constexpr uint32 frame_cbv = 2;
constexpr uint32 lights_cbv = 3;
}

namespace mesh_wireframe {
constexpr uint32 object_cbv = 0;
constexpr uint32 wireframe_cbv = 1;
constexpr uint32 frame_cbv = 2;
}

namespace terrain {
constexpr uint32 frame_cbv = 0;
constexpr uint32 lights_cbv = 1;
constexpr uint32 terrain_cbv = 2;
constexpr uint32 terrain_patch_data_srv = 3;
}

namespace sky_mesh {
constexpr uint32 sky_mesh_cbv = 0;
constexpr uint32 material_cbv = 1;
constexpr uint32 frame_cbv = 2;
}

namespace meta_draw {
constexpr uint32 instance_data_srv = 0;
constexpr uint32 frame_cbv = 1;
}

namespace tile_lights_clear {
constexpr uint32 input_cbv = 0;
constexpr uint32 light_tiles_uav = 1;
}

namespace tile_lights {
constexpr uint32 instance_data_srv = 0;
constexpr uint32 light_tiles_uav = 1;
constexpr uint32 cbv = 2;
}

namespace depth_reduce_minmax {
constexpr uint32 input_constants = 0;
constexpr uint32 output_uav = 1;
}

namespace imgui {
constexpr uint32 texture = 0;
constexpr uint32 inv_viewport_size = 1;
}

}

}
