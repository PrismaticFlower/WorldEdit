#pragma once

#include "utility/com_ptr.hpp"

#include <filesystem>
#include <initializer_list>
#include <string>
#include <vector>

#include <d3d12.h>

#include <boost/container/static_vector.hpp>

namespace sk::graphics::gpu {

constexpr static auto max_shader_defines = 64;

enum class shader_type {
   vertex,
   hull,
   domain,
   geometry,
   pixel,
   compute,
   library
};

struct shader_define {
   struct value : std::string {
      value() = default;
      value(long long v) : std::string{std::to_string(v)} {}

      using std::string::string;
   };

   std::string var;
   value value = 1;
};

struct shader_description {
   std::string name;
   std::string entrypoint;
   shader_type type;
   std::filesystem::path file;

   boost::container::static_vector<shader_define, max_shader_defines> defines;
};

struct compiled_shader : shader_description {
   utility::com_ptr<ID3DBlob> bytecode;
};

class shader_library {
public:
   explicit shader_library(std::initializer_list<shader_description> shaders);

   auto operator[](const std::string_view name) const noexcept -> D3D12_SHADER_BYTECODE;

private:
   std::vector<compiled_shader> _compiled_shaders;
};

}