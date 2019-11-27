#pragma once

#include <types.hpp>
#include <utility/make_from_bytes.hpp>
#include <utility/string_ops.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

#include <boost/container/small_vector.hpp>
#include <fmt/format.h>
#include <gsl/gsl>

namespace sk::ucfb {

enum class chunk_id : uint32 {};

inline auto to_string(const chunk_id id) -> std::string
{
   return {reinterpret_cast<const char*>(&id), sizeof(id)};
}

inline namespace literals {

constexpr auto operator""_id(const char* chars, const std::size_t) noexcept -> chunk_id
{
   uint32 result = 0;

   result |= (static_cast<uint32>(chars[0]) << 0);
   result |= (static_cast<uint32>(chars[1]) << 8);
   result |= (static_cast<uint32>(chars[2]) << 16);
   result |= (static_cast<uint32>(chars[3]) << 24);

   return static_cast<chunk_id>(result);
}
}

template<chunk_id type_id>
class reader_strict;

class reader {
public:
   reader() = delete;

   explicit reader(const gsl::span<const std::byte> bytes)
      : _id{utility::make_from_bytes<chunk_id>(bytes.subspan(0, sizeof(chunk_id)))},
        _size{utility::make_from_bytes<uint32>(
           bytes.subspan(sizeof(chunk_id), sizeof(uint32)))},
        _data{&bytes[8]},
        _abs_offset{0}
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
   auto read(const bool unaligned = false) -> Type
   {
      static_assert(std::is_trivially_copyable_v<Type>,
                    "Type must be trivially copyable.");
      static_assert(!std::is_pointer_v<Type>, "Type can not be a pointer.");

      const auto cur_pos = _head;
      _head += sizeof(Type);

      check_head();

      if (!unaligned) align_head();

      return utility::make_from_bytes<Type>(
         gsl::make_span(&_data[cur_pos], sizeof(Type)));
   }

   template<typename Type>
   auto read_unaligned() -> Type
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
   auto read_multi_unaligned() -> std::tuple<Types...>
   {
      return read_multi<Types...>(std::array{(sizeof(Types), true)...});
   }

   auto read_string(const bool unaligned = false) -> std::string_view
   {
      const char* const string = reinterpret_cast<const char*>(_data + _head);
      const auto string_length = cstring_length(string, _size - _head);

      _head += (string_length + 1);

      check_head();

      if (!unaligned) align_head();

      return {string, string_length};
   }

   auto read_string_unaligned() -> std::string_view
   {
      return read_string(true);
   }

   auto read_child() -> reader
   {
      const auto child_header_abs_offset = _abs_offset + 8 + _head;
      const auto child_id = read<chunk_id>();
      const auto child_size = read<std::uint32_t>();
      const auto child_data_offset = _head;

      _head += child_size;

      check_head();

      align_head();

      auto child_trace_stack = _trace_stack;
      child_trace_stack.push_back({.id = child_id, .offset = child_header_abs_offset});

      return reader{child_id, child_size, _data + child_data_offset,
                    child_header_abs_offset, std::move(child_trace_stack)};
   }

   template<chunk_id type_id>
   auto read_child_strict() -> reader_strict<type_id>
   {
      return {read_child_strict(type_id), reader_strict<type_id>::unchecked_tag{}};
   }

   void consume(const std::size_t amount, const bool unaligned = false)
   {
      _head += amount;

      check_head();

      if (!unaligned) align_head();
   }

   void consume_unaligned(const std::size_t amount)
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
      using namespace std::literals;

      std::string str;

      for (auto level : _trace_stack) {
         str += fmt::format("{} at offset {}\n"sv, to_string(level.id), level.offset);
      }

      str += fmt::format("   Read head of {} is {}\n"sv, to_string(_id), _head);

      if ((_head + 8) < _size) {
         str +=
            fmt::format("   Bytes at read head: {:02x}",
                        fmt::join(gsl::make_span(reinterpret_cast<const unsigned char*>(
                                                    _data + _head),
                                                 8),
                                  " "sv));
      }
      else if ((_head - 8) < _size) {
         str +=
            fmt::format("   Bytes preceding read head: {:02x}",
                        fmt::join(gsl::make_span(reinterpret_cast<const unsigned char*>(
                                                    _data + _head - 8),
                                                 8),
                                  " "sv));
      }
      else {
         str +=
            fmt::format("   Bytes in {}: {:02x}", to_string(_id),
                        fmt::join(gsl::make_span(reinterpret_cast<const unsigned char*>(_data),
                                                 _size),
                                  " "sv));
      }

      return str;
   }

private:
   struct trace_entry {
      chunk_id id;
      std::size_t offset;
   };

   using trace_stack = boost::container::small_vector<trace_entry, 8>;

   reader(const chunk_id id, const std::uint32_t size, const std::byte* const data,
          const std::size_t abs_offset, trace_stack trace_stack)
      : _id{id}, _size{size}, _data{data}, _abs_offset{abs_offset}, _trace_stack{trace_stack}
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
            "Read chunk ID is {} expected was {}. Chunk Trace Stack: \n{}"sv,
            to_string(child.id()), to_string(child_id),
            utility::string::indent(1, trace()))};
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
            fmt::format("Attempt to read {} bytes past end of chunk {}. Chunk Trace Stack: \n{}"sv,
                        _head - _size, to_string(id()),
                        utility::string::indent(1, trace()))};
      }
   }

   void align_head() noexcept
   {
      const auto remainder = _head % 4;

      if (remainder != 0) _head += (4 - remainder);
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

   explicit reader_strict(const gsl::span<const std::byte> bytes)
      : reader_strict{reader{bytes}}
   {
   }

private:
   friend class reader;

   struct unchecked_tag {
   };

   reader_strict(reader ucfb_reader, unchecked_tag) noexcept
      : reader{ucfb_reader}
   {
   }
};

}
