#pragma once

#include "../world.hpp"

#include "utility/function_ptr.hpp"

#include <span>

namespace we::world {

void evaluate_treeline(
   const tree_line& tree_line, std::span<const path> paths,
   function_ptr<void(const float4x4& world_from_object, const object_class_handle class_handle) noexcept>
      evaluate_callback);

}
