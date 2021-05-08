#pragma once

#include <atomic>
#include <concepts>
#include <cstddef>
#include <filesystem>
#include <mutex>
#include <span>
#include <thread>

#include <absl/container/flat_hash_set.h>
#include <wil/resource.h>

namespace sk::utility {

template<typename Callback>
concept file_watcher_callback =
   std::invocable<Callback, const std::filesystem::path&>;

class file_watcher {
public:
   explicit file_watcher(const std::filesystem::path& path);

   void evaluate_changed_files(file_watcher_callback auto callback) noexcept
   {
      std::lock_guard lock{_changed_files_mutex};

      for (const auto& path : _changed_files) {
         callback(path);
      }

      _changed_files.clear();
   }

   bool unknown_files_changed() noexcept
   {
      return _unknown_files_changed.exchange(false, std::memory_order_relaxed);
   }

private:
   void query_loop(std::stop_token stop_token) noexcept;

   void process_changes(const std::span<std::byte, 65536> buffer) noexcept;

   const std::filesystem::path _path;
   std::jthread _thread;
   wil::unique_hfile _directory;
   wil::unique_event _destroy_event{CreateEventW(nullptr, false, false, nullptr)};
   wil::event_set_scope_exit _destroy_event_setter =
      _destroy_event.SetEvent_scope_exit();

   std::mutex _changed_files_mutex;
   absl::flat_hash_set<std::filesystem::path, decltype([](const std::filesystem::path& path) {
                          return std::hash<std::wstring_view>{}(path.native());
                       })>
      _changed_files;
   std::atomic_bool _unknown_files_changed;
};

}
