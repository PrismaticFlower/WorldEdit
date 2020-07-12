#pragma once

#include <iostream>
#include <span>

namespace sk::utility {

class binary_stream_writer {
public:
   binary_stream_writer() = delete;

   binary_stream_writer(std::ostream& out) noexcept : _out{out} {}

   template<typename Type>
   void write(const Type& value)
   {
      static_assert(std::is_default_constructible_v<Type> &&
                    std::is_trivially_copyable_v<Type>);

      _out.write(reinterpret_cast<const char*>(&value), sizeof(Type));
   }

   template<typename Type>
   void write(const std::span<const Type> values)
   {
      static_assert(std::is_default_constructible_v<Type> &&
                    std::is_trivially_copyable_v<Type>);

      _out.write(reinterpret_cast<const char*>(values.data()), values.size_bytes());
   }

private:
   std::ostream& _out;
};

}