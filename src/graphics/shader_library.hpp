#pragma once

#include "async/thread_pool.hpp"
#include "utility/com_ptr.hpp"

#include <filesystem>
#include <initializer_list>
#include <string>
#include <vector>

#include <d3d12.h>

#include <boost/container/static_vector.hpp>

namespace we::graphics {

constexpr static auto max_shader_defines = 64;

enum class shader_type {
   vertex,
   hull,
   domain,
   geometry,
   pixel,
   compute,
   library
};

enum class shader_model {
   shader_model_6_0,
   shader_model_6_1,
   shader_model_6_2,
   shader_model_6_3,
   shader_model_6_4,
   shader_model_6_5,
   shader_model_6_6
};

constexpr auto shader_model_6_0 = shader_model::shader_model_6_0;
constexpr auto shader_model_6_1 = shader_model::shader_model_6_1;
constexpr auto shader_model_6_2 = shader_model::shader_model_6_2;
constexpr auto shader_model_6_3 = shader_model::shader_model_6_3;
constexpr auto shader_model_6_4 = shader_model::shader_model_6_4;
constexpr auto shader_model_6_5 = shader_model::shader_model_6_5;
constexpr auto shader_model_6_6 = shader_model::shader_model_6_6;

struct shader_define_value : std::wstring {
   shader_define_value() = default;

   shader_define_value(long long v) : std::wstring{std::to_wstring(v)} {}

   template<typename... T,
            typename = std::enable_if_t<not(... and std::is_integral_v<std::decay_t<T>>)>>
   shader_define_value(T&&... args) : std::wstring{std::forward<T>(args)...}
   {
   }
};

struct shader_define {
   std::wstring var;
   shader_define_value value;

   bool operator==(const shader_define&) const noexcept = default;
};

using shader_defines =
   boost::container::static_vector<shader_define, max_shader_defines>;

struct shader_description {
   std::string name;
   std::wstring entrypoint;
   shader_type type;
   shader_model model;
   std::filesystem::path file;

   shader_defines defines;

   bool operator==(const shader_description&) const noexcept = default;
};

struct shader_dependency {
   std::filesystem::path path;
   std::filesystem::file_time_type::duration last_write;
};

struct compiled_shader : shader_description {
   utility::com_ptr<ID3DBlob> bytecode;
   std::filesystem::file_time_type::duration file_last_write;
   std::vector<shader_dependency> dependencies;
};

class shader_library {
public:
   explicit shader_library(std::initializer_list<shader_description> shaders,
                           std::shared_ptr<async::thread_pool> thread_pool);

   auto operator[](const std::string_view name) const noexcept -> D3D12_SHADER_BYTECODE;

private:
   std::vector<compiled_shader> _compiled_shaders;
   std::shared_ptr<async::thread_pool> _thread_pool;
};

}

namespace std {

template<>
struct hash<we::graphics::shader_description> {
   auto operator()(const we::graphics::shader_description& entry) const noexcept
      -> std::size_t;
};

}
