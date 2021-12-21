
#include "shader_cache.hpp"
#include "hresult_error.hpp"
#include "io/read_file.hpp"
#include "types.hpp"
#include "utility/binary_reader.hpp"
#include "utility/binary_writer.hpp"

#include <fstream>

#include <dxcapi.h>

namespace we::graphics {

namespace {

constexpr uint32 cache_version = 0;
constexpr uint64 msvc_stl_update = _MSVC_STL_UPDATE;

void write_string(utility::binary_stream_writer& out, const std::string_view str)
{
   out.write(static_cast<uint32>(str.length()));
   out.write(std::span{str});
}

void write_wstring(utility::binary_stream_writer& out, const std::wstring_view str)
{
   out.write(static_cast<uint32>(str.length()));
   out.write(std::span{str});
}

template<typename C>
auto read_basic_string(utility::binary_reader& reader) -> std::basic_string<C>
{
   auto size = reader.read<uint32>();
   auto data = reader.read_bytes(size * sizeof(C));

   std::basic_string<C> str;
   str.resize(size);

   std::memcpy(str.data(), data.data(), data.size());

   return str;
}

auto read_string(utility::binary_reader& reader) -> std::string
{
   return read_basic_string<char>(reader);
}

auto read_wstring(utility::binary_reader& reader) -> std::wstring
{
   return read_basic_string<wchar_t>(reader);
}

bool out_of_date(const std::filesystem::path& path,
                 const std::filesystem::file_time_type::duration last_write,
                 const std::vector<shader_dependency>& dependencies)
{
   if (std::filesystem::last_write_time(path).time_since_epoch() > last_write) {
      return true;
   }

   for (auto& dependency : dependencies) {
      if (std::filesystem::last_write_time(dependency.path).time_since_epoch() >
          dependency.last_write) {
         return true;
      }
   }

   return false;
}

auto make_blob(IDxcUtils& utils, std::span<const std::byte> dxil)
   -> utility::com_ptr<ID3DBlob>
{
   utility::com_ptr<IDxcBlobEncoding> dxc_blob;
   throw_if_failed(utils.CreateBlob(dxil.data(), to_uint32(dxil.size()),
                                    DXC_CP_ACP, dxc_blob.clear_and_assign()));

   utility::com_ptr<ID3DBlob> blob;
   dxc_blob->QueryInterface(blob.clear_and_assign());

   return blob;
}

}

auto load_shader_cache(const std::filesystem::path& path)
   -> std::unordered_map<shader_description, shader_cache_entry>
{
   if (not std::filesystem::exists(path)) return {};

   utility::com_ptr<IDxcUtils> utils;
   DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(utils.clear_and_assign()));

   try {
      auto file = io::read_file_to_bytes(path);
      utility::binary_reader reader{file};

      const auto file_cache_version = reader.read<uint32>();
      const auto file_msvc_stl_update = reader.read<uint64>();

      if (file_cache_version != cache_version or msvc_stl_update != file_msvc_stl_update) {
         return {};
      }

      const auto shader_count = reader.read<uint32>();

      std::unordered_map<shader_description, shader_cache_entry> result;
      result.reserve(shader_count);

      for (std::size_t i = 0; i < shader_count; ++i) {
         shader_description desc{.name = read_string(reader),
                                 .entrypoint = read_wstring(reader),
                                 .type = shader_type{reader.read<uint8>()},
                                 .file = read_wstring(reader)};

         const auto define_count = reader.read<uint32>();

         desc.defines.reserve(define_count);

         for (std::size_t j = 0; j < define_count; ++j) {
            desc.defines.push_back(
               {.var = read_wstring(reader), .value = read_wstring(reader)});
         }

         const auto dxil_size = reader.read<uint32>();
         const auto dxil_span = reader.read_bytes(dxil_size);

         shader_cache_entry entry;

         entry.file_last_write =
            reader.read<std::filesystem::file_time_type::duration>();

         const auto dependency_count = reader.read<uint32>();

         entry.dependencies.reserve(dependency_count);

         for (std::size_t j = 0; j < dependency_count; ++j) {
            entry.dependencies.push_back(
               {.path = read_wstring(reader),
                .last_write =
                   reader.read<std::filesystem::file_time_type::duration>()});
         }

         if (not out_of_date(desc.file, entry.file_last_write, entry.dependencies)) {
            entry.bytecode = make_blob(*utils, dxil_span);

            result.emplace(std::move(desc), std::move(entry));
         }
      }

      return result;
   }
   catch (std::exception&) {
      return {};
   }
}

void save_shader_cache(const std::filesystem::path& path,
                       const std::span<const compiled_shader> shaders)
{
   std::ofstream out_file{path, std::ios::binary};
   utility::binary_stream_writer out{out_file};

   out.write(cache_version);
   out.write(msvc_stl_update);
   out.write(static_cast<uint32>(shaders.size()));

   for (auto& shader : shaders) {
      write_string(out, shader.name);
      write_wstring(out, shader.entrypoint);
      out.write(static_cast<uint8>(shader.type));
      write_wstring(out, shader.file.native());

      out.write(static_cast<uint32>(shader.defines.size()));

      for (auto& define : shader.defines) {
         write_wstring(out, define.var);
         write_wstring(out, define.value);
      }

      out.write(static_cast<uint32>(shader.bytecode->GetBufferSize()));
      out.write(std::span{static_cast<const std::byte*>(shader.bytecode->GetBufferPointer()),
                          shader.bytecode->GetBufferSize()});

      out.write(shader.file_last_write);

      out.write(static_cast<uint32>(shader.dependencies.size()));

      for (auto& dependency : shader.dependencies) {
         write_wstring(out, dependency.path.native());
         out.write(dependency.last_write);
      }
   }
}

}
