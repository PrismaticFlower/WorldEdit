#pragma once

#include <cstddef>
#include <span>
#include <string_view>

namespace we::graphics {

enum class shader_type {
   vertex,
   hull,
   domain,
   geometry,
   pixel,
   compute,
   library
};

struct shader_def {
   std::string_view name;
   const wchar_t* entrypoint;
   const wchar_t* target;
   const wchar_t* file;

   std::span<const std::byte> dxil;
};

}
