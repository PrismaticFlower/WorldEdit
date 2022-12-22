
#include "matrix_funcs.hpp"

#include <bit>

#include <DirectXMath.h>

namespace we {

auto inverse(const float4x4& matrix) -> float4x4
{
   DirectX::XMMATRIX inverse_matrix =
      DirectX::XMMatrixInverse(nullptr, std::bit_cast<DirectX::XMMATRIX>(matrix));

   return std::bit_cast<float4x4>(inverse_matrix);
}

}