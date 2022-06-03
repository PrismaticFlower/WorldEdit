#pragma once

#include <string_view>

#include <fmt/core.h>

namespace we {

class output_stream {
public:
   virtual ~output_stream() = default;

   virtual void write(const std::string_view string) noexcept = 0;

   template<typename... Args>
   void write(fmt::format_string<Args...> fmt, Args&&... args) noexcept
   {
      write(fmt::vformat(fmt, fmt::make_format_args(args...)));
   }
};

class standard_output_stream final : public output_stream {
public:
   void write(const std::string_view string) noexcept override;
};

class null_output_stream final : public output_stream {
public:
   void write(const std::string_view) noexcept override{};
};

}
