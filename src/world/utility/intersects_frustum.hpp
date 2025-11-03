#pragma once

#include "../world.hpp"

#include "math/frustum.hpp"

#include <span>

struct object_class_library;

namespace we::world {

bool intersects(const frustum& frustumWS, const object& object,
                const object_class_library& object_classes) noexcept;

bool intersects(const frustum& frustumWS, const light& light) noexcept;

bool intersects(const frustum& frustumWS, const region& region) noexcept;

bool intersects(const frustum& frustumWS, const sector& sector) noexcept;

bool intersects(const frustum& frustumWS, const portal& portal) noexcept;

bool intersects(const frustum& frustumWS, const barrier& barrier,
                const float visualizer_height) noexcept;

bool intersects(const frustum& frustumWS, const hintnode& hintnode) noexcept;

bool intersects(const frustum& frustumWS, const planning_hub& hub,
                const float visualizer_height) noexcept;

bool intersects(const frustum& frustumWS, const planning_connection& connection,
                std::span<const planning_hub> hubs,
                const float visualizer_height) noexcept;

bool intersects(const frustum& frustumWS, const boundary& boundary,
                const float visualizer_height) noexcept;

bool intersects(const frustum& frustumWS, const measurement& measurement) noexcept;

}