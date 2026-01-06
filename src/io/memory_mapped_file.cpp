#include "memory_mapped_file.hpp"
#include "error.hpp"
#include "path.hpp"
#include "types.hpp"

#include <system_error>
#include <utility>

#include <Windows.h>

#include <fmt/core.h>
#include <wil/resource.h>

namespace we::io {

namespace {

auto desired_access(map_mode map_mode) noexcept -> int
{
   if (map_mode == map_mode::read) return GENERIC_READ;
   if (map_mode == map_mode::read_write) return GENERIC_READ | GENERIC_WRITE;

   std::terminate();
}

auto desired_access_file_map(map_mode map_mode) noexcept -> int
{
   if (map_mode == map_mode::read) return FILE_MAP_READ;
   if (map_mode == map_mode::read_write) return FILE_MAP_WRITE;

   std::terminate();
}

auto page_protection(map_mode map_mode) noexcept -> int
{
   if (map_mode == map_mode::read) return PAGE_READONLY;
   if (map_mode == map_mode::read_write) return PAGE_READWRITE;

   std::terminate();
}

}

memory_mapped_file::memory_mapped_file(const memory_mapped_file_params& params)
{
   wil::unique_handle file{
      CreateFileW(wide_path{params.path}.c_str(), desired_access(params.map_mode),
                  0x0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr)};

   if (not file) {
      const DWORD system_error = GetLastError();

      throw open_error{fmt::format("Failed to open file '{}'.\n   Reason: {}",
                                   params.path.string_view(),
                                   std::system_category()
                                      .default_error_condition(system_error)
                                      .message()),
                       map_os_open_error_code(system_error)};
   }

   const LARGE_INTEGER file_size{.QuadPart = static_cast<LONGLONG>(params.size)};

   if (LARGE_INTEGER current_file_size{};
       params.truncate_to_size and GetFileSizeEx(file.get(), &current_file_size) and
       current_file_size.QuadPart > file_size.QuadPart) {
      if (not SetFilePointerEx(file.get(), file_size, nullptr, FILE_BEGIN)) {
         const DWORD system_error = GetLastError();

         throw open_error{
            fmt::format("Failed to truncate file '{}'.\n   Reason: {}",
                        params.path.string_view(),
                        std::system_category()
                           .default_error_condition(system_error)
                           .message()),
            map_os_open_error_code(system_error)};
      }

      if (not SetEndOfFile(file.get())) {
         const DWORD system_error = GetLastError();

         throw open_error{
            fmt::format("Failed to truncate file '{}'.\n   Reason: {}",
                        params.path.string_view(),
                        std::system_category()
                           .default_error_condition(system_error)
                           .message()),
            map_os_open_error_code(system_error)};
      }
   }

   wil::unique_handle file_mapping{
      CreateFileMappingW(file.get(), nullptr, page_protection(params.map_mode),
                         file_size.HighPart, file_size.LowPart, nullptr)};

   if (not file_mapping) {
      const DWORD system_error = GetLastError();

      throw open_error{
         fmt::format(
            "Failed to create file mapping for '{}'.\n   Reason: {}",
            params.path.string_view(),
            std::system_category().default_error_condition(system_error).message()),
         map_os_open_error_code(system_error)};
   }

   wil::unique_mapview_ptr<void> mapped_view{
      MapViewOfFile(file_mapping.get(),
                    desired_access_file_map(params.map_mode), 0x0, 0x0, 0x0)};

   if (not mapped_view) {
      const DWORD system_error = GetLastError();

      throw open_error{
         fmt::format(
            "Failed to map file '{}' into memory.\n   Reason: {}",
            params.path.string_view(),
            std::system_category().default_error_condition(system_error).message()),
         map_os_open_error_code(system_error)};
   }

   _bytes = static_cast<std::byte*>(mapped_view.release());
   _size = file_size.QuadPart;
   _mapping_handle = file_mapping.release();
   _file_handle = file.release();
   _map_mode = params.map_mode;
}

memory_mapped_file::~memory_mapped_file()
{
   reset();
}

memory_mapped_file::memory_mapped_file(memory_mapped_file&& other) noexcept
{
   memory_mapped_file discard;

   discard.swap(other);
   discard.swap(*this);
}

auto memory_mapped_file::operator=(memory_mapped_file&& other) noexcept
   -> memory_mapped_file&
{
   memory_mapped_file discard;

   discard.swap(other);
   discard.swap(*this);

   return *this;
}

auto memory_mapped_file::data() noexcept -> std::byte*
{
   return _bytes;
}

auto memory_mapped_file::data() const noexcept -> const std::byte*
{
   return _bytes;
}

auto memory_mapped_file::size() const noexcept -> std::size_t
{
   return _size;
}

bool memory_mapped_file::is_open() const noexcept
{
   return _bytes != nullptr;
}

void memory_mapped_file::reset() noexcept
{
   if (not is_open()) return;

   UnmapViewOfFile(std::exchange(_bytes, nullptr));
   _size = 0;
   CloseHandle(std::exchange(_mapping_handle, nullptr));
   CloseHandle(std::exchange(_file_handle, nullptr));
}

void memory_mapped_file::swap(memory_mapped_file& other) noexcept
{
   using std::swap;

   swap(other._bytes, this->_bytes);
   swap(other._size, this->_size);
   swap(other._mapping_handle, this->_mapping_handle);
   swap(other._file_handle, this->_file_handle);
}

}