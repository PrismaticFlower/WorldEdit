#pragma once

#include <atomic>
#include <filesystem>
#include <functional>
#include <memory>
#include <shared_mutex>

namespace we::assets {

/// @brief Private asset state.
/// @tparam T The type of the asset.
template<typename T>
struct asset_state {
   asset_state(std::weak_ptr<const T> data, bool exists,
               std::filesystem::path path, std::function<void()> start_load)
      : data{std::move(data)}, exists{exists}, path{std::move(path)}, start_load{std::move(start_load)}
   {
   }

   std::shared_mutex mutex;
   std::weak_ptr<const T> data;           // guarded by mutex
   bool exists = false;                   // guarded by mutex
   std::atomic_bool load_failure = false; // trivial state, no need to guard
   std::filesystem::path path;            // guarded by mutex
   std::function<void()> start_load;      // guarded by mutex
   std::atomic_size_t ref_count = 0;      // trivial state, no need to guard
};

}
