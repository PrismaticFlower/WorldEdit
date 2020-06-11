#pragma once

#include "utility/com_ptr.hpp"

#include <d3d12.h>

namespace sk::graphics::gpu {

struct root_signature_library {
   explicit root_signature_library(ID3D12Device& device);

   utility::com_ptr<ID3D12RootSignature> object_mesh;
   utility::com_ptr<ID3D12RootSignature> meta_object_mesh;
   utility::com_ptr<ID3D12RootSignature> meta_line;
};

}
