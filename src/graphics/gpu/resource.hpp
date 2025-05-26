#pragma once

#include "rhi.hpp"

#include <utility>

namespace we::graphics::gpu {

/// @brief Convenience helper for managing GPU handles. Owns a handle and calls release on it when destroyed.
/// Care must be taken when destroying unsynchronized handles.
template<typename H, H null_value, auto releaser>
struct unique_handle {
   using handle_type = H;

   constexpr unique_handle() = default;

   constexpr ~unique_handle()
   {
      if (_handle != null_value) {
         releaser(*_device, _handle);
      }
   }

   constexpr unique_handle(handle_type handle, device& device) noexcept
      : _handle{handle}, _device{&device}
   {
   }

   constexpr unique_handle(unique_handle&& other) noexcept
   {
      this->swap(other);
   }

   constexpr auto operator=(unique_handle&& other) noexcept -> unique_handle&
   {
      unique_handle discard;

      discard.swap(other);
      this->swap(discard);

      return *this;
   }

   constexpr unique_handle(const unique_handle&) = delete;

   constexpr auto operator=(const unique_handle&) -> unique_handle& = delete;

   [[nodiscard]] constexpr auto get() const noexcept -> handle_type
   {
      return _handle;
   }

   constexpr void reset() noexcept
   {
      unique_handle discard;

      discard.swap(*this);
   }

   constexpr auto release() noexcept -> handle_type
   {
      _device = nullptr;

      return std::exchange(_handle, null_value);
   }

   constexpr void swap(unique_handle& other) noexcept
   {
      using std::swap;

      swap(this->_handle, other._handle);
      swap(this->_device, other._device);
   }

   [[nodiscard]] constexpr explicit operator bool() const noexcept
   {
      return _handle != null_value;
   }

   [[nodiscard]] constexpr bool operator==(const unique_handle&) const noexcept = default;

   [[nodiscard]] constexpr bool operator==(handle_type other) const noexcept
   {
      return other == _handle;
   }

private:
   handle_type _handle = null_value;
   device* _device = nullptr;
};

using unique_resource_handle =
   unique_handle<resource_handle, null_resource_handle, [](device& device, resource_handle handle) {
      device.release_resource(handle);
   }>;

using unique_rtv_handle =
   unique_handle<rtv_handle, null_rtv_handle, [](device& device, rtv_handle handle) {
      device.release_render_target_view(handle);
   }>;

using unique_dsv_handle =
   unique_handle<dsv_handle, null_dsv_handle, [](device& device, dsv_handle handle) {
      device.release_depth_stencil_view(handle);
   }>;

using unique_resource_view =
   unique_handle<resource_view, invalid_resource_view, [](device& device, resource_view handle) {
      device.release_resource_view(handle);
   }>;

using unique_root_signature_handle =
   unique_handle<root_signature_handle, null_root_signature_handle,
                 [](device& device, root_signature_handle handle) {
                    device.release_root_signature(handle);
                 }>;

using unique_pipeline_handle =
   unique_handle<pipeline_handle, null_pipeline_handle, [](device& device, pipeline_handle handle) {
      device.release_pipeline(handle);
   }>;

using unique_query_heap_handle =
   unique_handle<query_heap_handle, null_query_heap_handle, [](device& device, query_heap_handle handle) {
      device.release_query_heap(handle);
   }>;

using unique_command_signature_handle =
   unique_handle<command_signature_handle, null_command_signature_handle,
                 [](device& device, command_signature_handle handle) {
                    device.release_command_signature(handle);
                 }>;

}
