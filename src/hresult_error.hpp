#pragma once

#include <fmt/format.h>

#include <Windows.h>

namespace sk {

class hresult_exception : public std::runtime_error {
public:
   hresult_exception(const HRESULT hr) noexcept
      : std::runtime_error{fmt::format("HRESULT: {:#x}",
                                       static_cast<std::make_unsigned_t<HRESULT>>(hr))},
        _hr{hr} {};

   auto error() const noexcept -> HRESULT
   {
      return _hr;
   }

private:
   HRESULT _hr;
};

inline void throw_if_failed(const HRESULT hr)
{
   if (FAILED(hr)) {
      throw hresult_exception{hr};
   }
}

}