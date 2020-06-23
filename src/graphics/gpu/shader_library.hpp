#pragma once

#include <d3d12.h>

namespace sk::graphics::gpu::shader_library {

extern const D3D12_SHADER_BYTECODE basic_object_mesh_vs;
extern const D3D12_SHADER_BYTECODE basic_object_mesh_ps;

extern const D3D12_SHADER_BYTECODE basic_mesh_lighting_ps;

extern const D3D12_SHADER_BYTECODE normal_mesh_ps;

extern const D3D12_SHADER_BYTECODE terrain_patch_vs;

extern const D3D12_SHADER_BYTECODE terrain_basic_ps;
extern const D3D12_SHADER_BYTECODE terrain_lighting_ps;
extern const D3D12_SHADER_BYTECODE terrain_normal_ps;

extern const D3D12_SHADER_BYTECODE meta_object_mesh_vs;
extern const D3D12_SHADER_BYTECODE meta_object_mesh_ps;
extern const D3D12_SHADER_BYTECODE meta_object_mesh_outlined_gs;
extern const D3D12_SHADER_BYTECODE meta_object_mesh_outlined_ps;
extern const D3D12_SHADER_BYTECODE meta_line_vs;

}
