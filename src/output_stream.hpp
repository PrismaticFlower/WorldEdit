#pragma once

#include <string_view>

namespace sk {

class output_stream {
public:
   virtual ~output_stream() = default;

   virtual void write(const std::string_view string) noexcept = 0;
};

class standard_output_stream : public output_stream {
public:
   void write(const std::string_view string) noexcept override;
};

class null_output_stream final : public output_stream {
public:
   void write(const std::string_view) noexcept override{};
};

}