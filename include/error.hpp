#pragma once

#include <system_error>

#include <Windows.h>

namespace sk {

void throw_if_failed(const HRESULT hr)
{
   if (FAILED(hr))
      throw std::system_error{std::error_code{hr, std::system_category()}};
}

}