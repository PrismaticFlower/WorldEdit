#pragma once

#include "model.hpp"

namespace we::munge {

auto generate_tangents(const model_segment& segment) -> std::vector<model_segment>;

}