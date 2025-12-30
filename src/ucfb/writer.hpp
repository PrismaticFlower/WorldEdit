#pragma once

#include "chunk_id.hpp"

#include "io/path.hpp"

#include "utility/implementation_storage.hpp"

#include <span>
#include <string_view>
#include <type_traits>

namespace we::ucfb {

struct writer_options {
   /// @brief If padding for child trail alignment is included in the child. Should be true for .msh files, false for ucfb files.
   bool child_trail_alignment_padding_included = false;
};

struct writer_output;

struct writer {
   writer(writer_output& out, uint32& size_out, writer* parent = nullptr);

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
   writer_output& _out;
   uint32& _size_out;
   uint64 _written_bytes = 0;
   writer* _parent = nullptr;
};

struct writer_root {
   writer_root(chunk_id id, const io::path& file, const writer_options options);

   ~writer_root();

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

   writer_root(const writer_root&) = delete;
   auto operator=(const writer_root&) -> writer_root& = delete;

   writer_root(writer_root&&) noexcept = delete;
   auto operator=(writer_root&&) noexcept -> writer_root& = delete;

private:
   implementation_storage<writer_output, 256> _out;
   writer _writer;
};

}
