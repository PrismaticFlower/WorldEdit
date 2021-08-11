#pragma once

#include "container/enum_array.hpp"
#include "types.hpp"
#include "utility/com_ptr.hpp"
#include "utility/enum_bitflags.hpp"

#include <d3d12.h>

namespace we::graphics::gpu {

enum class blend_modes { none, additive, alpha_blended };

enum class rasterizer_cull_mode { singlesided, doublesided };

enum class depth_test_mode { disabled, read_write, read };

class shader_library;
struct root_signature_library;

enum class material_pipeline_flags : uint8 {
   none = 0b0,
   alpha_cutout = 0b1,
   doublesided = 0b10,
   transparent = 0b100,
   additive = 0b1000,

   count = 0b10000
};

constexpr bool marked_as_enum_bitflag(material_pipeline_flags)
{
   return true;
}

using material_pipelines =
   container::enum_array<utility::com_ptr<ID3D12PipelineState>, material_pipeline_flags>;

struct pipeline_library {
   pipeline_library(ID3D12Device& device, const shader_library& shader_library,
                    const root_signature_library& root_signature_library);

   utility::com_ptr<ID3D12PipelineState> shadow_mesh;

   utility::com_ptr<ID3D12PipelineState> basic_object_mesh;
   utility::com_ptr<ID3D12PipelineState> basic_mesh_lighting;

   utility::com_ptr<ID3D12PipelineState> terrain_basic;
   utility::com_ptr<ID3D12PipelineState> terrain_lighting;
   utility::com_ptr<ID3D12PipelineState> terrain_normal;

   utility::com_ptr<ID3D12PipelineState> meta_object_transparent_mesh;
   utility::com_ptr<ID3D12PipelineState> meta_object_mesh_outlined;
   utility::com_ptr<ID3D12PipelineState> meta_line;

   material_pipelines normal_mesh;
};

}
