#include "rotation.hpp"

#include "math/quaternion_funcs.hpp"

namespace we::world {

rotation::rotation(const quaternion& from) noexcept : quat{from}
{
   // TODO: Make euler from quat.
}

rotation::rotation(const float3& from_euler) noexcept
   : quat{make_quat_from_euler(from_euler)}, euler{from_euler}
{
}

}