
#include "shader_library.hpp"
#include "hresult_error.hpp"
#include "shader_cache.hpp"
#include "utility/read_file.hpp"

#include <algorithm>
#include <exception>
#include <execution>
#include <iostream>
#include <mutex>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/container/small_vector.hpp>
#include <boost/functional/hash.hpp>
#include <d3dcompiler.h>
#include <fmt/format.h>

#include <range/v3/algorithm.hpp>

using namespace std::literals;

namespace we::graphics::gpu {

namespace {

#ifdef NDEBUG
const std::filesystem::path shader_cache_path = L"worldedit/shaders/cache/release.bin"sv;
#else
const std::filesystem::path shader_cache_path = L"worldedit/shaders/cache/debug.bin"sv;
#endif

struct file_includer final : public ID3DInclude {
   constexpr static auto dependencies_hasher = [](const shader_dependency& dependency) {
      return std::hash<std::wstring>{}(dependency.path.native());
   };

   constexpr static auto dependencies_equals = [](const shader_dependency& l,
                                                  const shader_dependency& r) {
      return l.path == r.path;
   };

   std::filesystem::path base_path;
   std::unordered_set<shader_dependency, decltype(dependencies_hasher), decltype(dependencies_equals)> dependencies;

private:
   HRESULT Open(D3D_INCLUDE_TYPE, LPCSTR file_name, LPCVOID, LPCVOID* data,
                UINT* bytes) override
   {
      try {
         const std::filesystem::path path = base_path / file_name;

         auto& file = _open_files.emplace_back(utility::read_file_to_string(path));

         *data = file.data();
         *bytes = static_cast<UINT>(file.size());

         dependencies.insert(
            shader_dependency{.path = path,
                              .last_write =
                                 std::filesystem::last_write_time(path).time_since_epoch()});

         return S_OK;
      }
      catch (std::exception&) {
         return E_FAIL;
      }
   }

   HRESULT Close(LPCVOID data)
   {
      _open_files.erase(ranges::find_if(_open_files, [data](const std::string& str) {
         return str.data() == data;
      }));

      return S_OK;
   }

   boost::container::small_vector<std::string, 4> _open_files;
};

namespace fxc {

auto get_shader_macros(const shader_description& desc)
   -> boost::container::static_vector<D3D_SHADER_MACRO, max_shader_defines + 1>
{
   boost::container::static_vector<D3D_SHADER_MACRO, max_shader_defines + 1> macros;

   macros.resize(desc.defines.size() + 1);

   ranges::transform(desc.defines, macros.begin(), [](const shader_define& define) {
      return D3D_SHADER_MACRO{.Name = define.var.c_str(),
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

auto compile(const shader_description& desc)
   -> std::pair<utility::com_ptr<ID3DBlob>, std::vector<shader_dependency>>
{
   auto macros = get_shader_macros(desc);

   utility::com_ptr<ID3DBlob> bytecode;
   utility::com_ptr<ID3DBlob> error;

   auto flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3 |
                D3DCOMPILE_ALL_RESOURCES_BOUND;

#ifndef NDEBUG
   flags |= D3DCOMPILE_DEBUG;
#endif

   file_includer file_includer;

   file_includer.base_path = desc.file.parent_path();

   if (const auto hr =
          D3DCompileFromFile(desc.file.c_str(), macros.data(), &file_includer,
                             desc.entrypoint.data(), get_target(desc), flags, 0,
                             bytecode.clear_and_assign(), error.clear_and_assign());
       FAILED(hr)) {
      if (error) {
         std::cerr << std::string_view{static_cast<const char*>(error->GetBufferPointer()),
                                       error->GetBufferSize()}
                   << "\n";
      }
      else {
         std::cerr << "Error while compiling shader!\n"; // TODO: HR printing.
      }

      return {nullptr, {}};
   }

   std::cout << fmt::format("Compiled '{}' {}\n", desc.name, desc.file.string());

   return {bytecode,
           {file_includer.dependencies.begin(), file_includer.dependencies.end()}};
}

}

}

shader_library::shader_library(std::initializer_list<shader_description> shaders)
{
   const auto shader_cache = load_shader_cache(shader_cache_path);

   std::mutex compiled_mutex;
   _compiled_shaders.reserve(shaders.size());

   std::for_each(std::execution::par, shaders.begin(), shaders.end(),
                 [&](const shader_description& desc) {
                    compiled_shader compiled{desc};

                    if (auto it = shader_cache.find(desc); it != shader_cache.end()) {
                       compiled.bytecode = it->second.bytecode;
                       compiled.file_last_write = it->second.file_last_write;
                       compiled.dependencies = it->second.dependencies;
                    }
                    else {
                       compiled.file_last_write =
                          std::filesystem::last_write_time(desc.file).time_since_epoch();
                       std::tie(compiled.bytecode, compiled.dependencies) =
                          fxc::compile(desc);
                    }

                    if (not compiled.bytecode) return;

                    std::scoped_lock lock{compiled_mutex};
                    _compiled_shaders.emplace_back(std::move(compiled));
                 });

   save_shader_cache(shader_cache_path, _compiled_shaders);

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

auto std::hash<we::graphics::gpu::shader_description>::operator()(
   const we::graphics::gpu::shader_description& desc) const noexcept -> std::size_t
{
   std::size_t hash = 0;

   boost::hash_combine(hash, desc.name);
   boost::hash_combine(hash, desc.entrypoint);
   boost::hash_combine(hash, desc.type);
   boost::hash_combine(hash, desc.file);

   for (auto& define : desc.defines) {
      boost::hash_combine(hash, define.var);
      boost::hash_combine(hash, define.value);
   }

   return 0;
}
