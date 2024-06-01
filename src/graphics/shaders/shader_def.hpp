#pragma once

#include <cstddef>
#include <span>

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
   const char* name;
   const wchar_t* entrypoint;
   const wchar_t* target_6_1;
   const wchar_t* target_6_6;
   const wchar_t* file;

   std::span<const std::byte> dxil_6_1;
   std::span<const std::byte> dxil_6_6;
};

}
