#pragma once

#include "asset_loaders.hpp"
#include "exceptions.hpp"
#include "msh/flat_model.hpp"
#include "odf/definition.hpp"
#include "output_stream.hpp"
#include "utility/string_ops.hpp"

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <fmt/format.h>
#include <gsl/gsl>
#include <object_ptr.hpp>

namespace sk::assets {

class libraries_manager;

template<typename T>
class library {
public:
   void add_asset(std::filesystem::path asset_path) noexcept
   {
      std::lock_guard lock{_mutex};

      asset_path.make_preferred(); // makes for prettier output messages

      _known_assets.emplace(asset_path.stem().string(), std::move(asset_path));
   }

   auto aquire_if(const std::string_view name) noexcept -> std::shared_ptr<T>
   {
      if (auto existing = aquire_existing_if(name); existing) return existing;

      std::lock_guard lock{_mutex};

      // make sure another thread hasn't already loaded the asset inbetween us checking for it and taking the write lock
      // TODO: Heterogeneous Lookup to avoid the string temporary.
      if (auto existing = _cached_assets.find(std::string{name});
          existing != _cached_assets.end()) {
         if (auto asset = existing->second.lock(); asset) {
            return asset;
         }
      }

      return create_asset_if_known(name);
   }

   auto aquire_existing_if(const std::string_view name) noexcept -> std::shared_ptr<T>
   {
      std::shared_lock read_lock{_mutex};

      // TODO: Heterogeneous Lookup to avoid the string temporary.
      if (auto existing = _cached_assets.find(std::string{name});
          existing != _cached_assets.end()) {
         return existing->second.lock();
      }

      return nullptr;
   }

   void output_stream(gsl::not_null<sk::output_stream*> stream) noexcept
   {
      _output_stream = stream;
   }

private:
   friend libraries_manager;

   auto create_asset_if_known(const std::string_view name) noexcept
      -> std::shared_ptr<T>
   {
      using namespace std::literals;

      // TODO: Heterogeneous Lookup to avoid the string temporary.
      std::string name_str{name};
      auto known_it = _known_assets.find(name_str);

      if (known_it == _known_assets.end()) {
         return nullptr;
      }

      const auto& asset_path = known_it->second;

      try {
         auto asset = std::make_shared<T>(loaders<T>::load(asset_path));

         _cached_assets.insert_or_assign(name_str, asset);

         _output_stream->write(
            fmt::format("Loaded asset '{}'\n"sv, asset_path.string()));

         return asset;
      }
      catch (std::exception& e) {
         _output_stream->write(
            fmt::format("Error while loading asset:\n   File: {}\n   Message: \n{}\n"sv,
                        asset_path.string(), utility::string::indent(2, e.what())));

         return nullptr;
      }
   }

   gsl::not_null<sk::output_stream*> _output_stream =
      &null_output_stream::get_static_instance();

   std::shared_mutex _mutex;

   std::unordered_map<std::string, std::filesystem::path> _known_assets;
   std::unordered_map<std::string, std::weak_ptr<T>> _cached_assets;
};

class libraries_manager {
public:
   void output_stream(jss::object_ptr<output_stream> stream) noexcept;

   void project_directory(const std::filesystem::path& path) noexcept;

   library<odf::definition> odfs;
   library<msh::flat_model> models;

private:
   gsl::not_null<sk::output_stream*> _output_stream =
      &null_output_stream::get_static_instance();
};

extern libraries_manager libraries;

}