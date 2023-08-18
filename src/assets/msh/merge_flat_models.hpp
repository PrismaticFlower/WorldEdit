#pragma once

#include "flat_model.hpp"

#include <span>
#include <stdexcept>

namespace we::assets::msh {

struct merge_error : std::runtime_error {
   using std::runtime_error::runtime_error;
};

struct flat_model_instance {
   transform transform;
   const flat_model* flat_model;
};

auto merge_flat_models(std::span<const flat_model_instance> flat_models) -> flat_model;

}
