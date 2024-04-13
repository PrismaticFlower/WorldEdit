#pragma once

#include <cstdio>
#include <memory>
#include <string>

#include <fmt/core.h>

namespace we {

class output_stream {
public:
   virtual ~output_stream() = default;

   virtual void write(std::string string) noexcept = 0;

   void vwrite(fmt::string_view fmt, fmt::format_args args) noexcept;

   template<typename... Args>
   void write(fmt::format_string<Args...> fmt, Args&&... args) noexcept
   {
      vwrite(fmt, fmt::make_format_args(args...));
   }
};

auto make_async_output_stream_stdout() -> std::unique_ptr<output_stream>;

class null_output_stream final : public output_stream {
public:
   using output_stream::write;

   void write(std::string string) noexcept override;
};

}
