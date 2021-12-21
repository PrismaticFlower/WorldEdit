
#include "shader_library.hpp"
#include "hresult_error.hpp"
#include "io/read_file.hpp"
#include "shader_cache.hpp"

#include <algorithm>
#include <array>
#include <exception>
#include <execution>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/container/small_vector.hpp>
#include <boost/functional/hash.hpp>
#include <dxcapi.h>

#include <range/v3/algorithm.hpp>

using namespace std::literals;

namespace we::graphics {

namespace {

const std::filesystem::path shader_cache_path = L"shaders.bin"sv;
const std::filesystem::path shader_pdb_path = L"shaders/pdb/"sv;

namespace dxc {

struct file_includer final : public IDxcIncludeHandler {
   constexpr static auto dependencies_hasher = [](const shader_dependency& dependency) {
      return std::hash<std::wstring>{}(dependency.path.native());
   };

   constexpr static auto dependencies_equals = [](const shader_dependency& l,
                                                  const shader_dependency& r) {
      return l.path == r.path;
   };

   std::unordered_set<shader_dependency, decltype(dependencies_hasher), decltype(dependencies_equals)> dependencies;
   utility::com_ptr<IDxcUtils> utils;

private:
   HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR filename, IDxcBlob** include_source) override
   {
      utility::com_ptr<IDxcBlobEncoding> blob;

      if (auto hr = utils->LoadFile(filename, nullptr, blob.clear_and_assign());
          FAILED(hr)) {
         return hr;
      }

      *include_source = blob.release();

      const auto path = std::filesystem::relative(filename);

      dependencies.insert(
         shader_dependency{.path = path,
                           .last_write =
                              std::filesystem::last_write_time(path).time_since_epoch()});

      return S_OK;
   }

   HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** object) override
   {
      if (iid == __uuidof(IUnknown)) {
         *object = static_cast<IUnknown*>(this);

         return S_OK;
      }
      else if (iid == __uuidof(IDxcIncludeHandler)) {
         *object = static_cast<IDxcIncludeHandler*>(this);

         return S_OK;
      }

      return E_NOINTERFACE;
   }

   ULONG STDMETHODCALLTYPE AddRef() override
   {
      return ++_ref_count;
   }

   ULONG STDMETHODCALLTYPE Release() override
   {
      return --_ref_count;
   }

   ULONG _ref_count = 1;
};

auto get_type_string(const shader_description& desc) -> std::wstring_view
{
   switch (desc.type) { // clang-format off
   case shader_type::vertex:   return L"vs";
   case shader_type::hull:     return L"hs";
   case shader_type::domain:   return L"ds";
   case shader_type::geometry: return L"gs";
   case shader_type::pixel:    return L"ps";
   case shader_type::compute:  return L"cs";
   case shader_type::library:  return L"lib";  
   } // clang-format on

   std::terminate();
}

auto get_model_string(const shader_description& desc) -> std::wstring_view
{
   switch (desc.model) { // clang-format off
   case shader_model_6_0: return L"6_0";
   case shader_model_6_1: return L"6_1";
   case shader_model_6_2: return L"6_2";
   case shader_model_6_3: return L"6_3";
   case shader_model_6_4: return L"6_4";
   case shader_model_6_5: return L"6_5";  
   case shader_model_6_6: return L"6_6";  
   } // clang-format on

   std::terminate();
}

auto get_target(const shader_description& desc) noexcept -> std::wstring
{
   return std::format(L"{}_{}", get_type_string(desc), get_model_string(desc));
}

auto get_shader_defines(const shader_description& desc)
   -> boost::container::static_vector<std::wstring, max_shader_defines>
{
   boost::container::static_vector<std::wstring, max_shader_defines> macros;

   macros.resize(desc.defines.size());

   ranges::transform(desc.defines, macros.begin(), [](const shader_define& define) {
      return std::format(L"{}={}", define.var, define.value);
   });

   return macros;
}

auto get_shader_pdb_path(const shader_description& desc) -> std::filesystem::path
{
   return std::filesystem::current_path() / shader_pdb_path / desc.name += L".pdb"sv;
}

auto compile(const shader_description& desc)
   -> std::pair<utility::com_ptr<ID3DBlob>, std::vector<shader_dependency>>
{
   utility::com_ptr<IDxcUtils> utils;
   utility::com_ptr<IDxcCompiler3> compiler;
   DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(utils.clear_and_assign()));
   DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(compiler.clear_and_assign()));

   file_includer includer;

   includer.utils = utils;

   const auto shader_defines = get_shader_defines(desc);
   const auto target = get_target(desc);
   const auto pdb_path = get_shader_pdb_path(desc);

   const std::array static_compile_args = {
      desc.file.c_str(), // shader name
      L"-E",
      desc.entrypoint.c_str(),
      L"-T",
      target.c_str(),
      L"-Zs", // enable debug information
      L"-Fd",
      pdb_path.c_str(),
      L"-Qstrip_reflect",
   };

   boost::container::small_vector<const wchar_t*, 64> compile_args;
   compile_args.reserve(static_compile_args.size() + shader_defines.size() * 2);

   compile_args.insert(compile_args.begin(), static_compile_args.begin(),
                       static_compile_args.end());

   for (auto& define : shader_defines) {
      compile_args.push_back(L"-D");
      compile_args.push_back(define.c_str());
   }

   // open source file
   utility::com_ptr<IDxcBlobEncoding> source_blob;

   if (FAILED(utils->LoadFile(desc.file.c_str(), nullptr,
                              source_blob.clear_and_assign()))) {
      std::cerr << std::format("Unable to load file {} for shader {}\n"sv,
                               desc.file.string(), desc.name);

      return {nullptr, {}};
   }

   const DxcBuffer source{.Ptr = source_blob->GetBufferPointer(),
                          .Size = source_blob->GetBufferSize(),
                          .Encoding = DXC_CP_ACP};

   utility::com_ptr<IDxcResult> results;
   compiler->Compile(&source, compile_args.data(),
                     static_cast<UINT32>(compile_args.size()), &includer,
                     IID_PPV_ARGS(results.clear_and_assign()));

   utility::com_ptr<IDxcBlobUtf8> errors;
   results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(errors.clear_and_assign()), nullptr);

   if (errors != nullptr && errors->GetStringLength() != 0) {
      std::cerr << std::format("Warnings and Errors from Compiling {}:\n{}\n"sv,
                               desc.name, errors->GetStringPointer());
   }

   HRESULT status;
   results->GetStatus(&status);

   if (FAILED(status)) {
      std::cerr << std::format("Compiling {} failed!\n"sv, desc.name);

      return {nullptr, {}};
   }

   utility::com_ptr<IDxcBlob> dxc_shader;
   utility::com_ptr<IDxcBlobUtf16> shader_name;
   results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(dxc_shader.clear_and_assign()),
                      shader_name.clear_and_assign());

   utility::com_ptr<IDxcBlob> pdb_blob;
   utility::com_ptr<IDxcBlobUtf16> pdb_name;
   results->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(pdb_blob.clear_and_assign()),
                      pdb_name.clear_and_assign());

   if (pdb_blob != nullptr) {
      std::ofstream file{pdb_name->GetStringPointer(), std::ios::binary};

      file.write(static_cast<const char*>(pdb_blob->GetBufferPointer()),
                 pdb_blob->GetBufferSize());
   }

   utility::com_ptr<ID3DBlob> shader;
   dxc_shader->QueryInterface(shader.clear_and_assign());

   std::cout << std::format("Compiled {}\n"sv, desc.name);

   return {shader, {includer.dependencies.begin(), includer.dependencies.end()}};
}

}

}

shader_library::shader_library(std::initializer_list<shader_description> shaders)
{
   const auto shader_cache = load_shader_cache(shader_cache_path);

   std::filesystem::create_directories(shader_pdb_path);

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
                          dxc::compile(desc);
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
      std::cerr << std::format("Unable to find shader named '{}'\n", name);
      std::terminate();
   }
}

}

auto std::hash<we::graphics::shader_description>::operator()(
   const we::graphics::shader_description& desc) const noexcept -> std::size_t
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
