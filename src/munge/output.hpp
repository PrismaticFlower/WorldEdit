#pragma once

#include "utility/implementation_storage.hpp"

#include <span>
#include <string>
#include <string_view>

namespace we::munge {

struct output {
   output();

   ~output();

   output(const output&) = delete;
   output(output&&) = delete;

   /// @brief Write a string into the output. This function is free threaded.
   /// @param str The string to write.
   void write(std::string str) noexcept;

   /// @brief Write a string into the output. This function can be called from one thread at a time (but may be called at the same time as write).
   /// @return A span of the lines that make up the output. Valid until the next view_lines call or when the output object is destroyed.
   auto view_lines() noexcept -> std::span<const std::string_view>;

   /// @brief Clear the output. This function can be called from one thread at a time (but may be called at the same time as write).
   void clear() noexcept;

private:
   struct impl;

   implementation_storage<impl, 128> impl;
};

}