
#include "shader_library.hpp"

#include <cstddef>

namespace sk::graphics::gpu::shader_library {

namespace {

#include "basic_object_meshPS_dxbc.h"
#include "basic_object_meshVS_dxbc.h"

#include "basic_mesh_lightingPS_dxbc.h"

#include "normal_meshPS_dxbc.h"

#include "terrain_patchVS_dxbc.h"

#include "terrain_basicPS_dxbc.h"

#include "meta_object_meshPS_dxbc.h"
#include "meta_object_meshVS_dxbc.h"
#include "meta_object_mesh_outlinedGS_dxbc.h"
#include "meta_object_mesh_outlinedPS_dxbc.h"

#include "meta_lineVS_dxbc.h"

template<std::size_t n>
constexpr auto bytecode_init(const unsigned char (&bytecode)[n]) -> D3D12_SHADER_BYTECODE
{
   return {
      .pShaderBytecode = &bytecode,
      .BytecodeLength = n,
   };
}

}

const D3D12_SHADER_BYTECODE basic_object_mesh_vs =
   bytecode_init(basic_object_meshVS_dxbc);
const D3D12_SHADER_BYTECODE basic_object_mesh_ps =
   bytecode_init(basic_object_meshPS_dxbc);

const D3D12_SHADER_BYTECODE basic_mesh_lighting_ps =
   bytecode_init(basic_mesh_lightingPS_dxbc);

const D3D12_SHADER_BYTECODE normal_mesh_ps = bytecode_init(normal_meshPS_dxbc);

const D3D12_SHADER_BYTECODE terrain_patch_vs = bytecode_init(terrain_patchVS_dxbc);

const D3D12_SHADER_BYTECODE terrain_basic_ps = bytecode_init(terrain_basicPS_dxbc);

const D3D12_SHADER_BYTECODE meta_object_mesh_vs =
   bytecode_init(meta_object_meshVS_dxbc);
const D3D12_SHADER_BYTECODE meta_object_mesh_ps =
   bytecode_init(meta_object_meshPS_dxbc);
const D3D12_SHADER_BYTECODE meta_object_mesh_outlined_gs =
   bytecode_init(meta_object_mesh_outlinedGS_dxbc);
const D3D12_SHADER_BYTECODE meta_object_mesh_outlined_ps =
   bytecode_init(meta_object_mesh_outlinedPS_dxbc);

const D3D12_SHADER_BYTECODE meta_line_vs = bytecode_init(meta_lineVS_dxbc);
}