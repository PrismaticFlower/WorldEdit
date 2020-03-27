#pragma once

#include "utility/com_ptr.hpp"

#include <d3d12.h>

namespace sk::graphics::gpu {

enum class blend_modes { none, additive, alpha_blended };

enum class rasterizer_cull_mode { singlesided, doublesided };

enum class depth_test_mode { disabled, read_write, read };

struct root_signature_library;

struct pipeline_library {
   explicit pipeline_library(ID3D12Device& device,
                             const root_signature_library& root_signature_library);

   utility::com_ptr<ID3D12PipelineState> basic_test;
};

}