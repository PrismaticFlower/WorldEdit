#pragma once

#include "io/path.hpp"
#include "utility/event.hpp"

#include <cstddef>
#include <span>
#include <thread>

#include <wil/resource.h>

namespace we::utility {

class file_watcher {
public:
   explicit file_watcher(const io::path& path);

   auto listen_file_changed(std::move_only_function<void(const io::path& path) const> callback) noexcept
      -> event_listener<void(const io::path& path)>
   {
      return _file_changed_event.listen(std::move(callback));
   }

   auto listen_file_removed(std::move_only_function<void(const io::path& path) const> callback) noexcept
      -> event_listener<void(const io::path& path)>
   {
      return _file_removed_event.listen(std::move(callback));
   }

   auto listen_unknown_files_changed(std::move_only_function<void() const> callback) noexcept
      -> event_listener<void()>
   {
      return _unknown_files_changed_event.listen(std::move(callback));
   }

private:
   void query_loop(std::stop_token stop_token) noexcept;

   void process_changes(const std::span<std::byte, 65536> buffer) noexcept;

   const io::path _path;
   std::jthread _thread;
   wil::unique_hfile _directory;
   wil::unique_event _destroy_event{CreateEventW(nullptr, false, false, nullptr)};
   wil::event_set_scope_exit _destroy_event_setter =
      _destroy_event.SetEvent_scope_exit();

   utility::event<void(const io::path& path)> _file_changed_event;
   utility::event<void(const io::path& path)> _file_removed_event;
   utility::event<void()> _unknown_files_changed_event;
};

}
