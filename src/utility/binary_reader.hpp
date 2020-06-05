#pragma once

#include "make_from_bytes.hpp"

#include <cstddef>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace sk::utility {

class binary_reader {
public:
   binary_reader() = delete;

   binary_reader(const std::span<const std::byte> bytes) noexcept
      : _bytes{bytes}
   {
   }

   template<typename Type>
   auto read() -> Type
   {
      static_assert(std::is_default_constructible_v<Type> &&
                    std::is_trivially_copyable_v<Type>);

      if (_bytes.size() < sizeof(Type)) {
         throw std::runtime_error{"binary_reader ran out of bytes!"};
      }

      auto value = make_from_bytes<Type>(_bytes.subspan(0, sizeof(Type)));

      _bytes = _bytes.subspan(sizeof(Type));

      return value;
   }

   void skip(const std::size_t count)
   {
      if (static_cast<std::size_t>(_bytes.size()) < count) {
         throw std::runtime_error{"binary_reader ran out of bytes!"};
      }

      _bytes = _bytes.subspan(count);
   }

   explicit operator bool() const noexcept
   {
      return _bytes.size() != 0;
   }

private:
   std::span<const std::byte> _bytes;
};

}