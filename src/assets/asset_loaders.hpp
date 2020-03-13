#pragma once

#include "msh/flat_model.hpp"
#include "odf/definition.hpp"

#include <filesystem>

namespace sk::assets {

template<typename T>
struct loaders {
};

template<>
struct loaders<odf::definition> {
   static auto load(const std::filesystem::path& path) -> odf::definition;
};

template<>
struct loaders<msh::flat_model> {
   static auto load(const std::filesystem::path& path) -> msh::flat_model;
};

}
