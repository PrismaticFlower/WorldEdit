#pragma once

#include <stdexcept>

namespace we::graphics::gpu {

enum class error { out_of_memory, device_removed, no_suitable_device };

struct exception final : std::runtime_error {
   explicit exception(const error error, const char* msg) noexcept
      : std::runtime_error{msg}, _error{error}
   {
   }

   auto error() const noexcept -> error
   {
      return _error;
   }

private:
   gpu::error _error;
};

}
