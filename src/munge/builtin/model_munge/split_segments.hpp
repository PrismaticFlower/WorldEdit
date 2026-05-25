#pragma once

#include "model.hpp"

namespace we::munge {

auto split_skinned_segments(const model_segment& segment, const uint32 max_bones)
   -> std::vector<model_segment>;

}