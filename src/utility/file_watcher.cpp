
#include "file_watcher.hpp"

#include <array>
#include <cstddef>
#include <optional>
#include <system_error>

#include <fmt/core.h>

using namespace std::literals;

namespace we::utility {

namespace {

auto get_long_path_name(const std::filesystem::path& path)
   -> std::optional<std::filesystem::path>
{
   std::wstring buffer;

   const auto needed_size = GetLongPathNameW(path.c_str(), buffer.data(),
                                             static_cast<DWORD>(buffer.size()));

   if (needed_size == 0 or GetLastError() == ERROR_FILE_NOT_FOUND)
      return std::nullopt;

   buffer.resize(needed_size - 1);

   if (GetLongPathNameW(path.c_str(), buffer.data(),
                        static_cast<DWORD>(buffer.size())) == 0 or
       GetLastError() != ERROR_SUCCESS) {
      return std::nullopt;
   }

   return std::filesystem::path{std::move(buffer)};
}

}

file_watcher::file_watcher(const std::filesystem::path& path) : _path{path}
{
   _directory = wil::unique_hfile{
      CreateFileW(path.c_str(), GENERIC_READ,
                  FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                  FILE_FLAG_OVERLAPPED | FILE_FLAG_BACKUP_SEMANTICS, nullptr)};

   if (not _directory) {
      const int error = GetLastError();

      throw std::system_error{std::error_code{error, std::system_category()},
                              fmt::format("(Unable to open directory '{}'",
                                          path.string())};
   }

   _thread =
      std::jthread{[this](std::stop_token stop_token) { query_loop(stop_token); }};

   SetThreadDescription(_thread.native_handle(),
                        L"WorldEdit File Watcher Thread");
}

void file_watcher::query_loop(std::stop_token stop_token) noexcept
{
   const wil::unique_event io_event{CreateEventW(nullptr, true, false, nullptr)};

   while (not stop_token.stop_requested()) {
      std::array<std::byte, 65536> buffer;
      OVERLAPPED overlapped{.hEvent = io_event.get()};

      if (not ReadDirectoryChangesExW(_directory.get(), buffer.data(),
                                      static_cast<DWORD>(buffer.size()), true,
                                      FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
                                      nullptr, &overlapped, nullptr,
                                      ReadDirectoryNotifyExtendedInformation)) {
         return;
      }

      switch (std::array events{io_event.get(), _destroy_event.get()};
              WaitForMultipleObjectsEx(static_cast<DWORD>(events.size()),
                                       events.data(), false, INFINITE, false)) {
      case WAIT_OBJECT_0: {
         DWORD bytes_returned = 0;
         GetOverlappedResult(_directory.get(), &overlapped, &bytes_returned, false);

         if (bytes_returned != 0) {
            process_changes(buffer);
         }
         else {
            _unknown_files_changed_event.broadcast();
         }

         break;
      }
      case WAIT_OBJECT_0 + 1:
      default:
         return;
      }
   }
}

void file_watcher::process_changes(const std::span<std::byte, 65536> buffer) noexcept
{
   std::size_t head = 0;

   constexpr auto notify_base_size =
      sizeof(FILE_NOTIFY_EXTENDED_INFORMATION) - sizeof(wchar_t);

   for (bool process = true; process;) {
      auto& info =
         *reinterpret_cast<FILE_NOTIFY_EXTENDED_INFORMATION*>(&buffer[head]);

      if ((head + notify_base_size + info.FileNameLength) > buffer.size() or
          (head + info.NextEntryOffset) > buffer.size()) {
         return;
      }

      head += info.NextEntryOffset;
      process = info.NextEntryOffset != 0;

      const std::wstring_view str{&info.FileName[0], info.FileNameLength / 2};
      std::filesystem::path path = _path / str;

      path.make_preferred();

      if (not std::filesystem::is_regular_file(path)) continue;

      // TODO: Investigate why this doesn't work as expected.
      // if (auto long_path = get_long_path_name(path); long_path) {
      //    std::swap(*long_path, path);
      // }

      switch (info.Action) {
      case FILE_ACTION_ADDED:
         _file_changed_event.broadcast(path);
         break;
      case FILE_ACTION_REMOVED:
         // File removed.
         break;
      case FILE_ACTION_MODIFIED:
         _file_changed_event.broadcast(path);
         break;
      case FILE_ACTION_RENAMED_OLD_NAME:
         // File removed.
         break;
      case FILE_ACTION_RENAMED_NEW_NAME:
         _file_changed_event.broadcast(path);
         break;
      }
   }
}

}
