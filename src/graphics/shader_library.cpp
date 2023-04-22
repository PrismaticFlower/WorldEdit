
#include "shader_library.hpp"
#include "async/for_each.hpp"
#include "io/output_file.hpp"
#include "io/read_file.hpp"
#include "utility/com_ptr.hpp"

#include <algorithm>
#include <array>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <shared_mutex>

#include <absl/container/inlined_vector.h>

#include <D3DCommon.h>
#include <dxcapi.h>

using namespace std::literals;

namespace we::graphics {

namespace {

const std::filesystem::path shader_pdb_path = L"shaders/pdb/"sv;

namespace dxc {

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
   -> absl::InlinedVector<std::wstring, max_shader_defines>
{
   absl::InlinedVector<std::wstring, max_shader_defines> macros;

   macros.resize(desc.defines.size());

   for (std::size_t i = 0; i < desc.defines.size(); ++i) {
      macros[i] =
         std::format(L"{}={}", desc.defines[i].var,
                     static_cast<const std::wstring&>(desc.defines[i].value));
   }

   return macros;
}

auto get_shader_pdb_path(const shader_description& desc) -> std::filesystem::path
{
   return std::filesystem::current_path() / shader_pdb_path / desc.name += L".pdb"sv;
}

auto compile(const shader_description& desc) -> std::vector<std::byte>
{
   using utility::com_ptr;

   com_ptr<IDxcUtils> utils;
   com_ptr<IDxcCompiler3> compiler;
   com_ptr<IDxcIncludeHandler> include_handler;
   DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(utils.clear_and_assign()));
   DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(compiler.clear_and_assign()));
   utils->CreateDefaultIncludeHandler(include_handler.clear_and_assign());

   const auto shader_defines = get_shader_defines(desc);
   const auto target = get_target(desc);
   const auto pdb_path = get_shader_pdb_path(desc);
   std::filesystem::path file{desc.file};

   const std::array static_compile_args = {
      file.c_str(), // shader name
      L"-E",
      desc.entrypoint.c_str(),
      L"-T",
      target.c_str(),
      L"-Zs", // enable debug information
      L"-Fd",
      pdb_path.c_str(), //
      L"-Qstrip_reflect",
   };

   absl::InlinedVector<const wchar_t*, 64> compile_args;
   compile_args.reserve(static_compile_args.size() + shader_defines.size() * 2);

   compile_args.insert(compile_args.begin(), static_compile_args.begin(),
                       static_compile_args.end());

   for (auto& define : shader_defines) {
      compile_args.push_back(L"-D");
      compile_args.push_back(define.c_str());
   }

   // open source file
   com_ptr<IDxcBlobEncoding> source_blob;

   if (FAILED(utils->LoadFile(file.c_str(), nullptr, source_blob.clear_and_assign()))) {
      std::cerr << std::format("Unable to load file {} for shader {}\n"sv,
                               file.make_preferred().string(), desc.name);

      return {};
   }

   const DxcBuffer source{.Ptr = source_blob->GetBufferPointer(),
                          .Size = source_blob->GetBufferSize(),
                          .Encoding = DXC_CP_ACP};

   com_ptr<IDxcResult> results;
   compiler->Compile(&source, compile_args.data(),
                     static_cast<UINT32>(compile_args.size()), include_handler.get(),
                     IID_PPV_ARGS(results.clear_and_assign()));

   com_ptr<IDxcBlobUtf8> errors;
   results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(errors.clear_and_assign()), nullptr);

   if (errors != nullptr && errors->GetStringLength() != 0) {
      std::cerr << std::format("Warnings and Errors from Compiling {}:\n{}\n"sv,
                               desc.name, errors->GetStringPointer());
   }

   HRESULT status;
   results->GetStatus(&status);

   if (FAILED(status)) {
      std::cerr << std::format("Compiling {} failed!\n"sv, desc.name);

      return {};
   }

   com_ptr<IDxcBlob> dxc_shader;
   com_ptr<IDxcBlobUtf16> shader_name;
   results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(dxc_shader.clear_and_assign()),
                      shader_name.clear_and_assign());

   com_ptr<IDxcBlob> pdb_blob;
   com_ptr<IDxcBlobUtf16> pdb_name;
   results->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(pdb_blob.clear_and_assign()),
                      pdb_name.clear_and_assign());

   if (pdb_blob != nullptr) {
      io::output_file pbd_out{pdb_name->GetStringPointer()};

      pbd_out.write(
         std::span{static_cast<const std::byte*>(pdb_blob->GetBufferPointer()),
                   pdb_blob->GetBufferSize()});
   }

   std::cout << std::format("Compiled {}\n"sv, desc.name);

   return {static_cast<std::byte*>(dxc_shader->GetBufferPointer()),
           static_cast<std::byte*>(dxc_shader->GetBufferPointer()) +
              dxc_shader->GetBufferSize()};
}

}

}

shader_library::shader_library(std::initializer_list<shader_description> shaders,
                               std::shared_ptr<async::thread_pool> thread_pool)
   : _thread_pool{thread_pool}
{
   reload(shaders);
}

auto shader_library::operator[](const std::string_view name) const noexcept
   -> std::span<const std::byte>
{
   if (auto shader =
          std::lower_bound(_compiled_shaders.cbegin(), _compiled_shaders.cend(), name,
                           [](const compiled_shader& l,
                              const std::string_view r) { return l.name < r; });
       shader != _compiled_shaders.cend() and shader->name == name) {
      return shader->bytecode;
   }
   else {
      std::cerr << std::format("Unable to find shader named '{}'\n", name);
      std::terminate();
   }
}

void shader_library::reload(std::initializer_list<shader_description> shaders) noexcept
{
   std::filesystem::create_directories(shader_pdb_path);

   std::shared_mutex compiled_mutex;
   _compiled_shaders.clear();
   _compiled_shaders.reserve(shaders.size());

   async::for_each(*_thread_pool, async::task_priority::normal, shaders,
                   [&](const shader_description& desc) noexcept {
                      compiled_shader compiled{.name = desc.name};

                      compiled.bytecode = dxc::compile(desc);

                      if (compiled.bytecode.empty()) return;

                      std::scoped_lock lock{compiled_mutex};
                      _compiled_shaders.emplace_back(std::move(compiled));
                   });

   std::sort(_compiled_shaders.begin(), _compiled_shaders.end(),
             [](const compiled_shader& l, const compiled_shader& r) {
                return l.name < r.name;
             });
}

}
