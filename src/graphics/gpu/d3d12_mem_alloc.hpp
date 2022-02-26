#pragma once

#include "D3D12MemAlloc/D3D12MemAlloc.h"

#include <memory>
#include <utility>

namespace we::graphics::gpu {

/// @brief For usage with D3D12MemAlloc classes.
/// @tparam T
template<typename T>
class release_ptr {
public:
   release_ptr() = default;

   explicit release_ptr(T* ptr) noexcept : _ptr{ptr} {};

   release_ptr(std::nullptr_t) noexcept : release_ptr{} {};

   release_ptr(const release_ptr&) = delete;
   auto operator=(const release_ptr&) -> release_ptr& = delete;

   release_ptr(release_ptr&& other) noexcept
   {
      swap(other);
   }

   auto operator=(release_ptr&& other) noexcept -> release_ptr&
   {
      release_ptr discard;

      discard.swap(other);
      discard.swap(*this);

      return *this;
   }

   ~release_ptr()
   {
      if (_ptr) _ptr->Release();
   }

   [[nodiscard]] auto clear_and_assign() noexcept -> T**
   {
      if (auto old = std::exchange(_ptr, nullptr); old) old->Release();

      return &_ptr;
   }

   [[nodiscard]] auto release() noexcept -> T*
   {
      return std::exchange(_ptr, nullptr);
   }

   [[nodiscard]] auto get() const noexcept -> T*
   {
      return _ptr;
   }

   [[nodiscard]] auto operator*() const noexcept -> T&
   {
      return *_ptr;
   }

   [[nodiscard]] auto operator->() const noexcept -> T*
   {
      return _ptr;
   }

   [[nodiscard]] explicit operator bool() const noexcept
   {
      return _ptr;
   }

   void swap(release_ptr& other) noexcept
   {
      std::swap(this->_ptr, other._ptr);
   }

private:
   T* _ptr = nullptr;
};

}
