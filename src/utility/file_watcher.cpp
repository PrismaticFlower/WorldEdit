
#include "file_watcher.hpp"

#include <array>
#include <cstddef>
#include <system_error>

#include <fmt/core.h>

using namespace std::literals;

namespace we::utility {

namespace {

auto make_full_path(const io::path& parent_path, const FILE_NOTIFY_INFORMATION& info)
   -> io::path
{
   const DWORD file_name_chars = info.FileNameLength / 2;

   const int32 needed_bytes =
      WideCharToMultiByte(CP_UTF8, 0, &info.FileName[0], file_name_chars,
                          nullptr, 0, nullptr, nullptr);

   if (needed_bytes == 0) std::terminate();

   char utf8_small_str[MAX_PATH];
   std::vector<char> utf8_large_str;
   char* utf8_str = nullptr;

   if (needed_bytes + 1 <= sizeof(utf8_small_str)) {
      utf8_str = &utf8_small_str[0];
   }
   else {
      utf8_large_str.resize(needed_bytes);
      utf8_str = utf8_large_str.data();
   }

   if (WideCharToMultiByte(CP_UTF8, 0, &info.FileName[0], file_name_chars,
                           utf8_str, needed_bytes, nullptr, nullptr) == 0) {
      std::terminate();
   }

   const std::string_view utf8_path = {utf8_str,
                                       static_cast<std::size_t>(needed_bytes)};

   return io::compose_path(parent_path, utf8_path);
}

bool is_file(const io::path& path) noexcept
{
   DWORD attributes = GetFileAttributesW(io::wide_path{path}.c_str());

   if (attributes == INVALID_FILE_ATTRIBUTES) return false;

   return (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

}

file_watcher::file_watcher(const io::path& path) : _path{path}
{
   _directory = wil::unique_hfile{
      CreateFileW(io::wide_path{path}.c_str(), FILE_LIST_DIRECTORY,
                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                  nullptr, OPEN_EXISTING,
                  FILE_FLAG_OVERLAPPED | FILE_FLAG_BACKUP_SEMANTICS, nullptr)};

   if (not _directory) {
      const int error = GetLastError();

      throw std::system_error{std::error_code{error, std::system_category()},
                              fmt::format("Unable to open directory '{}'",
                                          path.string_view())};
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

      if (not ReadDirectoryChangesW(_directory.get(), buffer.data(),
                                    static_cast<DWORD>(buffer.size()), true,
                                    FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
                                    nullptr, &overlapped, nullptr)) {
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

   constexpr auto notify_base_size = sizeof(FILE_NOTIFY_INFORMATION) - sizeof(wchar_t);

   for (bool process = true; process;) {
      auto& info = *reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&buffer[head]);

      if ((head + notify_base_size + info.FileNameLength) > buffer.size() or
          (head + info.NextEntryOffset) > buffer.size()) {
         return;
      }

      head += info.NextEntryOffset;
      process = info.NextEntryOffset != 0;

      const io::path path = make_full_path(_path, info);

      switch (info.Action) {
      case FILE_ACTION_ADDED: {
         if (is_file(path)) {
            _file_changed_event.broadcast(path);
         }
      } break;
      case FILE_ACTION_REMOVED: {
         _file_removed_event.broadcast(path);
      } break;
      case FILE_ACTION_MODIFIED: {
         if (is_file(path)) {
            _file_changed_event.broadcast(path);
         }
      } break;
      case FILE_ACTION_RENAMED_OLD_NAME: {
         _file_removed_event.broadcast(path);
      } break;
      case FILE_ACTION_RENAMED_NEW_NAME: {
         if (is_file(path)) {
            _file_changed_event.broadcast(path);
         }
      } break;
      }
   }
}

}
