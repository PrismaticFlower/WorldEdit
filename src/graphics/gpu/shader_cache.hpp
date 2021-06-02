#pragma once

#include "shader_library.hpp"

#include <filesystem>
#include <span>

namespace we::graphics::gpu {

struct shader_cache_entry {
   utility::com_ptr<ID3DBlob> bytecode;
   std::filesystem::file_time_type::duration file_last_write;
   std::vector<shader_dependency> dependencies;
};

auto load_shader_cache(const std::filesystem::path& path)
   -> std::unordered_map<shader_description, shader_cache_entry>;

void save_shader_cache(const std::filesystem::path& path,
                       const std::span<const compiled_shader> shaders);

}
