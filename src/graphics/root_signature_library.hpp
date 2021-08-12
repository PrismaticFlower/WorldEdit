#pragma once

#include "gpu/device.hpp"
#include "utility/com_ptr.hpp"

#include <d3d12.h>

namespace we::graphics {

struct root_signature_library {
   explicit root_signature_library(gpu::device& device);

   utility::com_ptr<ID3D12RootSignature> depth_only_mesh;
   utility::com_ptr<ID3D12RootSignature> object_mesh;
   utility::com_ptr<ID3D12RootSignature> terrain;
   utility::com_ptr<ID3D12RootSignature> meta_object_mesh;
   utility::com_ptr<ID3D12RootSignature> meta_line;
};

}
