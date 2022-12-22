
#include "scene.hpp"
#include "math/quaternion_funcs.hpp"

namespace we::assets::msh {

transform::operator float4x4() const noexcept
{
   float4x4 matrix = to_matrix(rotation);

   matrix[3] = {translation, 1.0f};

   return matrix;
}

}
