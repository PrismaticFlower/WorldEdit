#pragma once

#include "async/thread_pool.hpp"
#include "output_stream.hpp"
#include "shaders/shader_def.hpp"

#include <initializer_list>
#include <string_view>
#include <vector>

namespace we::graphics {

struct shader_library {
   shader_library(std::initializer_list<shader_def> shaders,
                  std::shared_ptr<async::thread_pool> thread_pool,
                  output_stream& compile_output);

   auto operator[](const std::string_view name) const noexcept
      -> std::span<const std::byte>;

   void reload(std::initializer_list<shader_def> shaders) noexcept;

private:
   struct shader {
      std::string_view name;
      std::span<const std::byte> dxil;

#ifndef NDEBUG
      std::unique_ptr<std::byte[]> dynamic_compiled_dxil;
#endif
   };

   std::vector<shader> _shaders;

#ifndef NDEBUG
   std::shared_ptr<async::thread_pool> _thread_pool;
   output_stream& _compile_output;
#endif
};
}
