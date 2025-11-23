#pragma once

#include "chunk_id.hpp"

#include "types.hpp"

#include "utility/make_from_bytes.hpp"
#include "utility/string_ops.hpp"

#include <cstddef>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

#include <absl/container/inlined_vector.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

namespace we::ucfb {

template<chunk_id type_id>
class reader_strict;

struct reader_options {
   /// @brief Align to 4 byte boundaries after reading children.
   /// .msh files should have this set to false, ucfb files should have it set to true.
   bool aligned_children = true;
};

class reader {
public:
   reader() = delete;

   explicit reader(const std::span<const std::byte> bytes, const reader_options options)
      : _id{utility::make_from_bytes<chunk_id>(bytes.subspan(0, sizeof(chunk_id)))},
        _size{utility::make_from_bytes<uint32>(
           bytes.subspan(sizeof(chunk_id), sizeof(uint32)))},
        _data{&bytes[8]},
        _abs_offset{0},
        _aligned_children{options.aligned_children}
   {
      if (bytes.size() < 8) {
         throw std::runtime_error{
            "Unable to construct ucfb reader! Size of supplied memory is less "
            "than the minimum of 8 bytes."};
      }

      if (_size > static_cast<std::size_t>(bytes.size() - 8)) {
         throw std::runtime_error{
            "Unable to construct ucfb reader! Size of supplied memory is less "
            "than size of supposed chunk."};
      }

      _trace_stack.push_back({.id = _id, .offset = 0});
   }

   template<typename Type>
   auto read(const bool aligned = false) -> Type
   {
      static_assert(std::is_trivially_copyable_v<Type>,
                    "Type must be trivially copyable.");
      static_assert(!std::is_pointer_v<Type>, "Type can not be a pointer.");

      const std::span<const std::byte> bytes = read_bytes(sizeof(Type), aligned);

      return utility::make_from_bytes<Type>(bytes);
   }

   template<typename Type>
   auto read_aligned() -> Type
   {
      return read<Type>(true);
   }

   template<typename... Types>
   auto read_multi(const std::array<bool, sizeof...(Types)> unaligned = {})
      -> std::tuple<Types...>
   {
      return read_multi_impl<Types...>(unaligned, std::index_sequence_for<Types...>{});
   }

   template<typename... Types>
   auto read_multi_aligned() -> std::tuple<Types...>
   {
      return read_multi<Types...>(std::array{(sizeof(Types), true)...});
   }

   auto read_string(const bool aligned = false) -> std::string_view
   {
      check_head();

      const char* const string = reinterpret_cast<const char*>(_data + _head);
      const auto string_length = cstring_length(string, _size - _head);

      std::size_t new_head = _head + string_length + 1;

      check_new_head(new_head);

      _head = new_head;

      if (aligned) align_head();

      return {string, string_length};
   }

   auto read_string_aligned() -> std::string_view
   {
      return read_string(true);
   }

   auto read_bytes(const std::size_t count, const bool aligned = false)
      -> std::span<const std::byte>
   {
      std::size_t new_head = _head + count;

      check_new_head(new_head);

      const std::span<const std::byte> bytes = {_data + _head, count};

      _head = new_head;

      if (aligned) align_head();

      return bytes;
   }

   auto read_bytes_aligned() -> std::span<const std::byte>
   {
      return read_bytes(true);
   }

   auto read_child() -> reader
   {
      check_new_head(_head + 8);

      const auto child_header_abs_offset = _abs_offset + 8 + _head;
      const auto child_id = read<chunk_id>();
      const auto child_size = read<std::uint32_t>();
      const auto child_data_offset = _head;

      check_new_head(_head + child_size);

      _head += child_size;

      if (_aligned_children) align_head();

      auto child_trace_stack = _trace_stack;
      child_trace_stack.push_back({.id = child_id, .offset = child_header_abs_offset});

      return reader{child_id,
                    child_size,
                    _data + child_data_offset,
                    child_header_abs_offset,
                    _aligned_children,
                    std::move(child_trace_stack)};
   }

   template<chunk_id type_id>
   auto read_child_strict() -> reader_strict<type_id>
   {
      return {read_child_strict(type_id),
              typename reader_strict<type_id>::unchecked_tag{}};
   }

   void consume(const std::size_t amount, const bool aligned = false)
   {
      check_new_head(_head + amount);

      _head += amount;

      if (aligned) align_head();
   }

   void consume_aligned(const std::size_t amount)
   {
      consume(amount, true);
   }

   explicit operator bool() const noexcept
   {
      return (_head < _size);
   }

   void reset_head() noexcept
   {
      _head = 0;
   }

   auto id() const noexcept -> chunk_id
   {
      return _id;
   }

   auto size() const noexcept -> std::size_t
   {
      return _size;
   }

   auto trace() const noexcept -> std::string
   {
      std::string str;

      for (auto level : _trace_stack) {
         str += fmt::format("{} at offset {}\n", to_string(level.id), level.offset);
      }

      str += fmt::format("   Read head of {} is {}\n", to_string(_id), _head);

      if ((_head + 8) < _size) {
         str += fmt::format("   Bytes at read head: {:02x}",
                            fmt::join(std::span{reinterpret_cast<const unsigned char*>(
                                                   _data + _head),
                                                8},
                                      " "));
      }
      else if ((_head - 8) < _size) {
         str += fmt::format("   Bytes preceding read head: {:02x}",
                            fmt::join(std::span{reinterpret_cast<const unsigned char*>(
                                                   _data + _head - 8),
                                                8},
                                      " "));
      }
      else {
         str +=
            fmt::format("   Bytes in {}: {:02x}", to_string(_id),
                        fmt::join(std::span{reinterpret_cast<const unsigned char*>(_data),
                                            _size},
                                  " "));
      }

      return str;
   }

private:
   struct trace_entry {
      chunk_id id;
      std::size_t offset;
   };

   using trace_stack = absl::InlinedVector<trace_entry, 8>;

   reader(const chunk_id id, const std::uint32_t size, const std::byte* const data,
          const std::size_t abs_offset, bool aligned_children, trace_stack trace_stack)
      : _id{id},
        _size{size},
        _data{data},
        _abs_offset{abs_offset},
        _aligned_children{aligned_children},
        _trace_stack{trace_stack}
   {
   }

   auto read_child_strict(const chunk_id child_id) -> reader
   {
      using namespace std::literals;

      const auto old_head = _head;

      const auto child = read_child();

      if (child.id() != child_id) {
         _head = old_head;

         throw std::runtime_error{fmt::format(
            "Chunk ID mistmatch when performing strict read of child chunk. "
            "Read chunk ID is {} expected was {}. Chunk Trace Stack: \n{}",
            to_string(child.id()), to_string(child_id), string::indent(1, trace()))};
      }

      return child;
   }

   template<typename... Types, std::size_t... indices>
   auto read_multi_impl(const std::array<bool, sizeof...(Types)> unaligned,
                        std::index_sequence<indices...>) -> std::tuple<Types...>
   {
      return {read<Types>(unaligned[indices])...};
   }

   void check_head()
   {
      using namespace std::literals;

      if (_head > _size) {
         throw std::runtime_error{
            fmt::format("Attempt to read {} bytes past end of chunk {}. Chunk "
                        "Trace Stack: \n{}",
                        _head - _size, to_string(id()), string::indent(1, trace()))};
      }
   }

   void align_head() noexcept
   {
      const auto remainder = _head % 4;

      if (remainder != 0) _head += (4 - remainder);
   }

   void check_new_head(const std::size_t new_head)
   {
      using namespace std::literals;

      if (new_head > _size) {
         throw std::runtime_error{
            fmt::format("Attempt to read {} bytes past end of chunk {}. Chunk "
                        "Trace Stack: \n{}",
                        new_head - _size, to_string(id()),
                        string::indent(1, trace()))};
      }
   }

   static auto cstring_length(const char* const string, const std::size_t max_length)
      -> std::size_t
   {
      const auto string_end = std::find(string, string + max_length, '\0');

      return static_cast<std::size_t>(std::distance(string, string_end));
   }

   const chunk_id _id;
   const std::size_t _size;
   const std::byte* const _data;

   std::size_t _head = 0;
   const std::size_t _abs_offset;

   bool _aligned_children = false;

   trace_stack _trace_stack;
};

template<chunk_id type_id>
class reader_strict : public reader {
public:
   reader_strict() = delete;

   explicit reader_strict(reader ucfb_reader) : reader{ucfb_reader}
   {
      if (id() != type_id) {
         throw std::runtime_error{
            fmt::format("ucfb reader chunk ID is {} but {} was required!",
                        to_string(id()), to_string(type_id))};
      }
   }

   explicit reader_strict(const std::span<const std::byte> bytes,
                          const reader_options options)
      : reader_strict{reader{bytes, options}}
   {
   }

private:
   friend class reader;

   struct unchecked_tag {};

   reader_strict(reader ucfb_reader, unchecked_tag) noexcept
      : reader{ucfb_reader}
   {
   }
};
}
