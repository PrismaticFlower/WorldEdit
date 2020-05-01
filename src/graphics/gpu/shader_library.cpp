
#include "shader_library.hpp"

#include <cstddef>

namespace sk::graphics::gpu::shader_library {

namespace {

#include "basic_object_meshPS_dxil.h"
#include "basic_object_meshVS_dxil.h"
#include "meta_object_meshPS_dxil.h"
#include "meta_object_meshVS_dxil.h"
#include "meta_object_mesh_outlinedPS_dxil.h"

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
   bytecode_init(basic_object_meshVS_dxil);
const D3D12_SHADER_BYTECODE basic_object_mesh_ps =
   bytecode_init(basic_object_meshPS_dxil);

const D3D12_SHADER_BYTECODE meta_object_mesh_vs =
   bytecode_init(meta_object_meshVS_dxil);
const D3D12_SHADER_BYTECODE meta_object_mesh_ps =
   bytecode_init(meta_object_meshPS_dxil);
const D3D12_SHADER_BYTECODE meta_object_mesh_outlined_ps =
   bytecode_init(meta_object_mesh_outlinedPS_dxil);

}