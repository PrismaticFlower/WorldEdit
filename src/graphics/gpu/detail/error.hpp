#pragma once

#include "../exception.hpp"

#include <d3d12.h>

namespace we::graphics::gpu::detail {

/// @brief Check a HRESULT and call std::terminate on failure.
/// @param hresult The HRESULT to check.
inline void terminate_if_fail(const HRESULT hresult) noexcept
{
   [[unlikely]] if (FAILED(hresult)) {
      std::terminate();
   }
}

/// @brief Checks a HRESULT. On failure attempts to map it to a gpu::error code
/// and then throw gpu::exception. If it doesn't map to gpu::error (and hence probably isn't recoverable or useful for the application to know about) calls std::terminate instead.
/// @param hresult The HRESULT to check.
inline void throw_if_fail(const HRESULT hresult)
{
   [[unlikely]] if (FAILED(hresult)) {
      switch (hresult) {
      case E_OUTOFMEMORY:
         throw exception{error::out_of_memory,
                         "The GPU ran out of accessible memory."};
      case DXGI_ERROR_DEVICE_REMOVED:
      case DXGI_ERROR_DEVICE_RESET:
         throw exception{
            error::device_removed,
            "The GPU was removed, reset or it's driver was updated."};
      default:
         std::terminate();
      }
   }
}

}
