#pragma once

#include <concepts>
#include <cstddef>

#include <d3d12.h>

namespace sk::graphics {

struct gpu_device;

// clang-format off

template<typename T>
concept gpu_device_child = requires(T child) {
   { child.parent_gpu_device.operator->() } -> std::convertible_to<gpu_device*>;
   { *child.parent_gpu_device } -> std::convertible_to<gpu_device&>;
};

template<typename T>
concept gpu_resource_owner = gpu_device_child<T> and requires(T owner) {
   { owner.resource } -> ID3D12Resource*;
   { owner.resource_state } -> D3D12_RESOURCE_STATES;
};

template<typename T>
concept gpu_resource_ref = requires(T ref) {
   { ref.resource } -> ID3D12Resource*;
   { ref.resource_state } -> D3D12_RESOURCE_STATES;
};

template<typename T>
concept gpu_buffer_ref = gpu_resource_ref<T> and requires(T buffer) {
   { buffer.size } -> std::convertible_to<std::size_t>;
};

// clang-format on

}
