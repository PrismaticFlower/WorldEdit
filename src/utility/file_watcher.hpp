#pragma once

#include "utility/event.hpp"

#include <atomic>
#include <concepts>
#include <cstddef>
#include <filesystem>
#include <mutex>
#include <span>
#include <thread>

#include <wil/resource.h>

namespace we::utility {

template<typename Callback>
concept file_watcher_callback =
   std::invocable<Callback, const std::filesystem::path&>;

class file_watcher {
public:
   explicit file_watcher(const std::filesystem::path& path);

   auto listen_file_changed(std::function<void(const std::filesystem::path& path)> callback) noexcept
      -> event_listener<void(const std::filesystem::path& path)>
   {
      return _file_changed_event.listen(callback);
   }

   auto listen_unknown_files_changed(std::function<void()> callback) noexcept
      -> event_listener<void()>
   {
      return _unknown_files_changed_event.listen(callback);
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

   utility::event<void(const std::filesystem::path& path)> _file_changed_event;
   utility::event<void()> _unknown_files_changed_event;
};

}
