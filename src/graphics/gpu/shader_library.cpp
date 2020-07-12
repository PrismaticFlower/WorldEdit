
#include "shader_library.hpp"
#include "hresult_error.hpp"

#include <algorithm>
#include <exception>
#include <execution>
#include <iostream>
#include <mutex>
#include <vector>

#include <d3dcompiler.h>
#include <fmt/format.h>

#include <range/v3/algorithm.hpp>

namespace sk::graphics::gpu {

namespace {

namespace fxc {

auto get_shader_macros(const shader_description& desc)
   -> boost::container::static_vector<D3D_SHADER_MACRO, max_shader_defines + 1>
{
   boost::container::static_vector<D3D_SHADER_MACRO, max_shader_defines + 1> macros;

   macros.resize(desc.defines.size() + 1);

   ranges::transform(desc.defines, macros.begin(), [](const shader_define& define) {
      return D3D_SHADER_MACRO{.Name = define.value.c_str(),
                              .Definition = define.value.c_str()};
   });

   macros.back() = D3D_SHADER_MACRO{nullptr, nullptr};

   return macros;
}

auto get_target(const shader_description& desc) noexcept -> const char*
{
   switch (desc.type) { // clang-format off
   case shader_type::vertex:   return "vs_5_1";
   case shader_type::hull:     return "hs_5_1";
   case shader_type::domain:   return "ds_5_1";
   case shader_type::geometry: return "gs_5_1";
   case shader_type::pixel:    return "ps_5_1";
   case shader_type::compute:  return "cs_5_1";
   case shader_type::library:  return "lib_5_1";  
   } // clang-format on

   std::terminate();
}

auto compile(const shader_description& desc) -> utility::com_ptr<ID3DBlob>
{
   auto macros = get_shader_macros(desc);

   utility::com_ptr<ID3DBlob> bytecode;
   utility::com_ptr<ID3DBlob> error;

   auto flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3 |
                D3DCOMPILE_ALL_RESOURCES_BOUND;

#ifndef NDEBUG
   flags |= D3DCOMPILE_DEBUG;
#endif

   if (const auto hr = D3DCompileFromFile(desc.file.c_str(), macros.data(),
                                          D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                          desc.entrypoint.data(), get_target(desc),
                                          flags, 0, bytecode.clear_and_assign(),
                                          error.clear_and_assign());
       FAILED(hr)) {
      if (error) {
         std::cerr << std::string_view{static_cast<const char*>(error->GetBufferPointer()),
                                       error->GetBufferSize()}
                   << "\n";
      }
      else {
         std::cerr << "Error while compiling shader!\n"; // TODO: HR printing.
      }

      return nullptr;
   }

   return bytecode;
}

}

}

shader_library::shader_library(std::initializer_list<shader_description> shaders)
{
   std::mutex compiled_mutex;
   _compiled_shaders.reserve(shaders.size());

   std::for_each(std::execution::par, shaders.begin(), shaders.end(),
                 [&](const shader_description& desc) {
                    compiled_shader compiled{desc};

                    compiled.bytecode = fxc::compile(desc);

                    std::scoped_lock lock{compiled_mutex};
                    _compiled_shaders.emplace_back(std::move(compiled));
                 });

   std::sort(std::execution::par, _compiled_shaders.begin(), _compiled_shaders.end(),
             [](const compiled_shader& l, const compiled_shader& r) {
                return l.name < r.name;
             });
}

auto shader_library::operator[](const std::string_view name) const noexcept
   -> D3D12_SHADER_BYTECODE
{
   if (auto shader =
          std::lower_bound(_compiled_shaders.cbegin(), _compiled_shaders.cend(), name,
                           [](const compiled_shader& l,
                              const std::string_view r) { return l.name < r; });
       shader != _compiled_shaders.cend() and shader->name == name) {
      return {shader->bytecode->GetBufferPointer(), shader->bytecode->GetBufferSize()};
   }
   else {
      std::cerr << fmt::format("Unable to find shader named '{}'\n", name);
      std::terminate();
   }
}

}
