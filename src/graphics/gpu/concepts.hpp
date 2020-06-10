#pragma once

#include <concepts>
#include <cstddef>

#include <d3d12.h>

namespace sk::graphics::gpu {

class device;

// clang-format off

template<typename T>
concept device_child = requires(T child) {
   { child.parent_device() } -> std::convertible_to<device*>;
};

template<typename T>
concept resource_owner = device_child<T> and requires(T owner) {
   { owner.resource() } -> ID3D12Resource*;
};

template<typename T>
concept resource_ref = requires(T ref) {
   { ref.resource() } -> ID3D12Resource*;
   { ref.resource_state } -> D3D12_RESOURCE_STATES;
};

template<typename T>
concept buffer_ref = resource_ref<T> and requires(T buffer) {
   { buffer.size() } -> std::convertible_to<std::size_t>;
};

// clang-format on

}
