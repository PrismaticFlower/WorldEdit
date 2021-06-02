#pragma once

#include <stdexcept>

#include <Windows.h>

namespace we {

class hresult_exception : public std::runtime_error {
public:
   hresult_exception(const HRESULT hr) noexcept;

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
