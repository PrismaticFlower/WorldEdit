#pragma once

#include "async/thread_pool.hpp"
#include "container/enum_array.hpp"
#include "types.hpp"
#include "utility/com_ptr.hpp"
#include "utility/enum_bitflags.hpp"

#include <vector>

#include <d3d12.h>

namespace we::graphics {

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
   pipeline_library(ID3D12Device10& device, const shader_library& shader_library,
                    const root_signature_library& root_signature_library);

   utility::com_ptr<ID3D12PipelineState> mesh_depth_prepass;
   utility::com_ptr<ID3D12PipelineState> mesh_depth_prepass_doublesided;
   utility::com_ptr<ID3D12PipelineState> mesh_depth_prepass_alpha_cutout;
   utility::com_ptr<ID3D12PipelineState> mesh_depth_prepass_alpha_cutout_doublesided;
   utility::com_ptr<ID3D12PipelineState> mesh_shadow;
   utility::com_ptr<ID3D12PipelineState> mesh_basic;
   utility::com_ptr<ID3D12PipelineState> mesh_basic_lighting;
   material_pipelines mesh_normal;
   utility::com_ptr<ID3D12PipelineState> mesh_wireframe;

   utility::com_ptr<ID3D12PipelineState> terrain_depth_prepass;
   utility::com_ptr<ID3D12PipelineState> terrain_basic;
   utility::com_ptr<ID3D12PipelineState> terrain_lighting;
   utility::com_ptr<ID3D12PipelineState> terrain_normal;

   utility::com_ptr<ID3D12PipelineState> meta_mesh;
   utility::com_ptr<ID3D12PipelineState> meta_mesh_outlined;
   utility::com_ptr<ID3D12PipelineState> meta_mesh_wireframe;
   utility::com_ptr<ID3D12PipelineState> meta_line;

   utility::com_ptr<ID3D12PipelineState> tile_lights_clear;
   utility::com_ptr<ID3D12PipelineState> tile_lights_spheres;
};

}
