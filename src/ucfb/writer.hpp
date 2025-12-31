#pragma once

#include "chunk_id.hpp"

#include "io/output_file.hpp"

#include <span>
#include <string_view>
#include <type_traits>

namespace we::ucfb {

struct writer_options {
   /// @brief If padding for child trail alignment is included in the child. Should be true for .msh files, false for ucfb files.
   bool child_trail_alignment_padding_included = false;
};

struct writer {
   writer(chunk_id id, io::output_file& file, const writer_options options,
          writer* parent = nullptr);

   ~writer();

   void write(const std::string_view str);

   void write(const char* str);

   void write(std::span<const std::byte> bytes);

   template<typename... Ts>
      requires std::conjunction_v<std::is_trivially_copyable<Ts>...>
   void write(const Ts&... values)
   {
      (..., write(std::as_bytes(std::span{&values, 1})));
   }

   auto write_child(chunk_id id) -> writer;

   writer(const writer&) = delete;
   auto operator=(const writer&) -> writer& = delete;

   writer(writer&&) noexcept = delete;
   auto operator=(writer&&) noexcept -> writer& = delete;

private:
   io::output_file& _out;
   uint64 _written_bytes = 0;
   io::file_write_offset _size_offset = {};
   writer_options _options;
   writer* const _parent = nullptr;
};

}
