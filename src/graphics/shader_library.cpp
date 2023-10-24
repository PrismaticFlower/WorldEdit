
#include "shader_library.hpp"

#include <algorithm>

#ifndef NDEBUG
#include "utility/com_ptr.hpp"

#include <array>
#include <filesystem>

#include <D3DCommon.h>
#include <dxcapi.h>
#endif

using namespace std::literals;

namespace we::graphics {

#ifndef NDEBUG
namespace {

struct compile_result {
   compile_result() = default;

   compile_result(IDxcBlob& dxil_blob)
   {
      size = dxil_blob.GetBufferSize();
      dxil = std::make_unique_for_overwrite<std::byte[]>(size);

      std::memcpy(dxil.get(), dxil_blob.GetBufferPointer(), size);
   }

   std::size_t size = 0;
   std::unique_ptr<std::byte[]> dxil = nullptr;
};

auto compile(const shader_def& def, output_stream& output,
             DxcCreateInstanceProc DxcCreateInstancePtr) -> compile_result
{
   using utility::com_ptr;

   com_ptr<IDxcUtils> utils;
   com_ptr<IDxcCompiler3> compiler;
   com_ptr<IDxcIncludeHandler> include_handler;
   DxcCreateInstancePtr(CLSID_DxcUtils, IID_PPV_ARGS(utils.clear_and_assign()));
   DxcCreateInstancePtr(CLSID_DxcCompiler, IID_PPV_ARGS(compiler.clear_and_assign()));
   utils->CreateDefaultIncludeHandler(include_handler.clear_and_assign());

   const std::filesystem::path file = L"shaders/"s + def.file;

   std::array compile_args = {
      file.c_str(), // shader name
      L"-E",
      def.entrypoint,
      L"-T",
      def.target,
      L"-Zi", // enable debug information
      L"-Qstrip_reflect",
      L"-Qembed_debug",
   };

   // open source file
   com_ptr<IDxcBlobEncoding> source_blob;

   if (FAILED(utils->LoadFile(file.c_str(), nullptr, source_blob.clear_and_assign()))) {
      output.write("Unable to load file {} for shader {}\n", file.string(), def.name);

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
      output.write("Warnings and Errors from Compiling {}:\n{}\n", def.name,
                   errors->GetStringPointer());
   }

   HRESULT status;
   results->GetStatus(&status);

   if (FAILED(status)) {
      output.write("Compiling {} failed!\n", def.name);

      return {};
   }

   com_ptr<IDxcBlob> dxil;
   com_ptr<IDxcBlobUtf16> shader_name;
   results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(dxil.clear_and_assign()),
                      shader_name.clear_and_assign());

   output.write("Compiled {}\n", def.name);

   return {*dxil};
}

}
#endif

shader_library::shader_library(std::initializer_list<shader_def> shaders,
                               [[maybe_unused]] std::shared_ptr<async::thread_pool> thread_pool,
                               [[maybe_unused]] output_stream& error_output)
#ifndef NDEBUG
   : _thread_pool{thread_pool}, _compile_output{error_output}
#endif
{
   _shaders.reserve(shaders.size());

   for (auto& shader : shaders) {
      _shaders.push_back({shader.name, shader.dxil});
   }

   std::sort(_shaders.begin(), _shaders.end(),
             [](const shader& l, const shader& r) { return l.name < r.name; });
}

auto shader_library::operator[](const std::string_view name) const noexcept
   -> std::span<const std::byte>
{
   if (auto it = std::lower_bound(_shaders.cbegin(), _shaders.cend(), name,
                                  [](const shader& l, const std::string_view r) {
                                     return l.name < r;
                                  });
       it != _shaders.cend() and it->name == name) {
      return it->dxil;
   }
   else {
      std::terminate();
   }
}

void shader_library::reload([[maybe_unused]] std::initializer_list<shader_def> shaders) noexcept
{
#ifndef NDEBUG
   static DxcCreateInstanceProc DxcCreateInstancePtr =
      reinterpret_cast<DxcCreateInstanceProc>(
         GetProcAddress(LoadLibraryW(L"dxcompiler.dll"), "DxcCreateInstance"));

   if (not DxcCreateInstancePtr) {
      _compile_output.write("Failed to load dxcompiler.dll\n");

      return;
   }

   std::vector<shader> new_shaders;
   new_shaders.resize(shaders.size());

   _thread_pool->for_each_n(async::task_priority::normal, shaders.size(),
                            [&](const std::size_t i) noexcept {
                               const shader_def shader = *(shaders.begin() + i);

                               compile_result result =
                                  compile(shader, _compile_output, DxcCreateInstancePtr);

                               new_shaders[i] = {.name = shader.name,
                                                 .dxil = {result.dxil.get(),
                                                          result.size},
                                                 .dynamic_compiled_dxil =
                                                    std::move(result.dxil)};
                            });

   _shaders = std::move(new_shaders);

   std::sort(_shaders.begin(), _shaders.end(),
             [](const shader& l, const shader& r) { return l.name < r.name; });
#endif
}

}
