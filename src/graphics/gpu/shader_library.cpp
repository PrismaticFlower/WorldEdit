
#include "shader_library.hpp"

#include <cstddef>

namespace sk::graphics::gpu::shader_library {

namespace {

#include "basic_testPS_dxil.h"
#include "basic_testVS_dxil.h"

template<std::size_t n>
constexpr auto bytecode_init(const unsigned char (&bytecode)[n]) -> D3D12_SHADER_BYTECODE
{
   return {
      .pShaderBytecode = &bytecode,
      .BytecodeLength = n,
   };
}

}

const D3D12_SHADER_BYTECODE basic_test_vs = bytecode_init(basic_testVS_dxil);
const D3D12_SHADER_BYTECODE basic_test_ps = bytecode_init(basic_testPS_dxil);

}