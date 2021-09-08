#pragma once

#include "gpu/device.hpp"
#include "types.hpp"
#include "utility/com_ptr.hpp"

#include <d3d12.h>

namespace we::graphics {

struct root_signature_library {
   explicit root_signature_library(gpu::device& device);

   utility::com_ptr<ID3D12RootSignature> mesh_shadow;
   utility::com_ptr<ID3D12RootSignature> mesh_depth_prepass;
   utility::com_ptr<ID3D12RootSignature> mesh;
   utility::com_ptr<ID3D12RootSignature> terrain;
   utility::com_ptr<ID3D12RootSignature> meta_mesh;
   utility::com_ptr<ID3D12RootSignature> meta_line;

   utility::com_ptr<ID3D12RootSignature> tile_lights;
};

namespace rs {

namespace mesh_shadow {
constexpr uint32 object_cbv = 0;
constexpr uint32 camera_cbv = 1;
}

namespace mesh_depth_prepass {
constexpr uint32 object_cbv = 0;
constexpr uint32 material_descriptor_table = 1;
constexpr uint32 camera_descriptor_table = 2;
}

namespace mesh {
constexpr uint32 object_cbv = 0;
constexpr uint32 material_descriptor_table = 1;
constexpr uint32 camera_descriptor_table = 2;
constexpr uint32 lights_descriptor_table = 3;
}

namespace terrain {
constexpr uint32 camera_descriptor_table = 0;
constexpr uint32 lights_descriptor_table = 1;
constexpr uint32 terrain_descriptor_table = 2;
constexpr uint32 terrain_patch_data_srv = 3;
constexpr uint32 material_descriptor_table = 4;
}

namespace meta_mesh {
constexpr uint32 object_cbv = 0;
constexpr uint32 color_cbv = 1;
constexpr uint32 camera_descriptor_table = 2;
}

namespace meta_line {
constexpr uint32 color_cbv = 0;
constexpr uint32 camera_descriptor_table = 1;
}

namespace tile_lights {
constexpr uint32 descriptor_table = 0;
}

}

}
