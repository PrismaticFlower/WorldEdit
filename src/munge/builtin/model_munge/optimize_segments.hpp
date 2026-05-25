#pragma once

#include "model.hpp"

namespace we::munge {

struct optimization_options {
   uint32 max_bones = 15;
};

void optimize_segments(std::vector<model_segment>& segments,
                       const optimization_options& options);

void optimize_segments(std::vector<model_shadow>& segments);

}