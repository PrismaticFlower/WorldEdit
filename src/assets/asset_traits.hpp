#pragma once

#include "io/path.hpp"
#include "msh/flat_model.hpp"
#include "odf/definition.hpp"
#include "sky/sky.hpp"
#include "texture/texture.hpp"

#include <string_view>

namespace we::assets {

template<typename T>
struct asset_traits {};

template<>
struct asset_traits<odf::definition> {
   static constexpr std::string_view error_type_name = "object definition";

   static auto load(const io::path& path) -> odf::definition;
};

template<>
struct asset_traits<msh::flat_model> {
   static constexpr std::string_view error_type_name = "model";

   static auto load(const io::path& path) -> msh::flat_model;
};

template<>
struct asset_traits<texture::texture> {
   static constexpr std::string_view error_type_name = "texture";

   static auto load(const io::path& path) -> texture::texture;
};

template<>
struct asset_traits<sky::config> {
   static constexpr std::string_view error_type_name = "sky";

   static auto load(const io::path& path) -> sky::config;
};

}
