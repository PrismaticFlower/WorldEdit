#pragma once

#include "../world.hpp"

#include "region_properties.hpp"

namespace we::world {

auto to_ui_string(const light_type type) noexcept -> const char*;

auto to_ui_string(const texture_addressing addressing) noexcept -> const char*;

auto to_ui_string(const ps2_blend_mode mode) noexcept -> const char*;

auto to_ui_string(const path_type type) noexcept -> const char*;

auto to_ui_string(const path_spline_type type) noexcept -> const char*;

auto to_ui_string(const region_type type) noexcept -> const char*;

auto to_ui_string(const region_shape shape) noexcept -> const char*;

auto to_ui_string(const hintnode_type type) noexcept -> const char*;

auto to_ui_string(const hintnode_mode mode) noexcept -> const char*;

}