#pragma once

#include "path.hpp"

#include <exception>
#include <utility>
#include <vector>

#include <Windows.h>

namespace we::io {

namespace {

constexpr char native_path_sep = '\\';
constexpr char alt_path_sep = '/';
constexpr uint32 max_path_capacity = UINT32_MAX >> 1u;

}

path::path() noexcept = default;

path::path(const path& other) noexcept
{
   this->_is_small = other._is_small;
   this->_capacity = other._capacity;
   this->_length = other._length;

   if (other._is_small) {
      this->_small_storage = other._small_storage;
      this->_data = &this->_small_storage[0];
   }
   else {
      this->_data = new char[this->_capacity + 1]{};

      for (uint32 i = 0; i < other._length; ++i) {
         this->_data[i] = other._data[i];
      }
   }
}

path::path(path&& other) noexcept
{
   path discard;

   other.swap(discard);
   this->swap(discard);
}

path::path(std::string_view str) noexcept
{
   *this += str;
}

path::path(const char* c_string) noexcept : path{std::string_view{c_string}} {}

auto path::operator=(const path& other) noexcept -> path&
{
   path discard{other};

   this->swap(discard);

   return *this;
}

auto path::operator=(path&& other) noexcept -> path&
{
   path discard;

   other.swap(discard);
   this->swap(discard);

   return *this;
}

path::~path()
{
   if (not _is_small) {
      delete[] _data;
   }
}

void path::swap(path& other) noexcept
{
   using std::swap;

   swap(this->_data, other._data);
   swap(this->_length, other._length);
   swap(this->_small_storage, other._small_storage);

   const uint32 this_is_small = this->_is_small;
   const uint32 this_capacity = this->_capacity;

   this->_is_small = other._is_small;
   other._is_small = this_is_small;
   this->_capacity = other._capacity;
   other._capacity = this_capacity;

   if (this->_is_small) this->_data = &this->_small_storage[0];
   if (other._is_small) other._data = &other._small_storage[0];
}

auto path::parent_path() const noexcept -> std::string_view
{
   const std::string_view path_view = string_view();
   const std::size_t seperator_position = path_view.find_last_of(native_path_sep);

   if (seperator_position == path_view.npos) return path_view;

   return path_view.substr(0, seperator_position);
}

auto path::filename() const noexcept -> std::string_view
{
   const std::string_view path_view = string_view();
   const std::size_t seperator_position = path_view.find_last_of(native_path_sep);

   if (seperator_position == path_view.npos) return path_view;

   return path_view.substr(seperator_position + 1);
}

auto path::stem() const noexcept -> std::string_view
{
   const std::string_view filename_view = filename();
   const std::size_t dot_position = filename_view.find_last_of('.');

   if (dot_position == filename_view.npos) return filename_view;

   return filename_view.substr(0, dot_position);
}

auto path::extension() const noexcept -> std::string_view
{
   const std::string_view path_view = string_view();
   const std::size_t dot_position = path_view.find_last_of('.');

   if (dot_position == path_view.npos) return "";

   return path_view.substr(dot_position);
}

auto path::c_str() const noexcept -> const char*
{
   return _data;
}

bool path::empty() const noexcept
{
   return _length == 0;
}

auto path::string_view() const noexcept -> std::string_view
{
   return {_data, _length};
}

auto path::operator+=(std::string_view right) noexcept -> path&
{
   if (_length + right.size() > _capacity) {
      const std::size_t new_capacity =
         _capacity + std::max(right.size(), _capacity / 2ull);

      if (new_capacity > max_path_capacity) std::terminate();

      char* new_data = new char[new_capacity + 1]{};

      std::memcpy(new_data, _data, _length);

      if (_is_small) {
         _is_small = false;
      }
      else {
         delete[] _data;
      }

      _data = new_data;
      _capacity = new_capacity;
   }

   if (_length + right.size() > UINT32_MAX) std::terminate();

   for (char c : right) {
      if (c == alt_path_sep) c = native_path_sep;

      _data[_length] = c;
      _length += 1;
   }

   return *this;
}

bool operator==(const path& left, const path& right) noexcept
{
   return left.string_view() == right.string_view();
}

auto compose_path(const path& parent_path, std::string_view stem) noexcept -> path
{
   path new_path = parent_path;

   if (not new_path.string_view().ends_with(native_path_sep)) {
      const char seperator[2] = {native_path_sep, '\0'};

      new_path += seperator;
   }

   new_path += stem;

   return new_path;
}

auto compose_path(const path& parent_path, std::string_view stem,
                  std::string_view extension) noexcept -> path
{
   path new_path = compose_path(parent_path, stem);

   new_path += extension;

   return new_path;
}

auto make_path_with_new_extension(const path& old_path,
                                  std::string_view new_extension) noexcept -> path
{
   const std::string_view old_path_view = old_path.string_view();
   const std::size_t last_dot = old_path_view.find_last_of('.');

   path new_path = last_dot != old_path_view.npos ? old_path_view.substr(0, last_dot)
                                                  : old_path_view;

   new_path += new_extension;

   return new_path;
}

auto make_path_from_wide_cstring(const wchar_t* str) noexcept -> io::path
{
   if (lstrlenW(str) == 0) return "";

   const int32 needed_bytes =
      WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);

   if (needed_bytes == 0) std::terminate();

   std::vector<char> utf8_str;
   utf8_str.resize(needed_bytes);

   if (WideCharToMultiByte(CP_UTF8, 0, str, -1, utf8_str.data(), needed_bytes,
                           nullptr, nullptr) == 0) {
      std::terminate();
   }

   return {utf8_str.data()};
}

wide_path::wide_path(const path& path)
{
   if (path.empty()) return;
   if (path.string_view().size() + 1 > INT_MAX) std::terminate();

   const int utf8_path_size = static_cast<int>(path.string_view().size() + 1);

   const int wchar_path_size =
      MultiByteToWideChar(CP_UTF8, 0, path.c_str(), utf8_path_size, nullptr, 0);

   if (wchar_path_size == 0) std::terminate();

   if (wchar_path_size > _small_storage.size()) {
      _data = new wchar_t[wchar_path_size]{};
      _is_small = false;
   }

   if (MultiByteToWideChar(CP_UTF8, 0, path.c_str(), utf8_path_size, _data,
                           wchar_path_size) == 0) {
      std::terminate();
   }
}

wide_path::~wide_path()
{
   if (not _is_small) delete[] _data;
}

auto wide_path::c_str() const noexcept -> const wchar_t*
{
   return _data;
}

bool exists(const path& path) noexcept
{
   return GetFileAttributesW(wide_path{path}.c_str()) != INVALID_FILE_ATTRIBUTES;
}

bool remove(const path& path) noexcept
{
   if (DeleteFileW(wide_path{path}.c_str())) return true;

   if (RemoveDirectoryW(wide_path{path}.c_str())) return true;

   return false;
}

void update_last_write_time(const path& path) noexcept
{
   SYSTEMTIME system_time{};

   GetSystemTime(&system_time);

   FILETIME file_time{};

   if (SystemTimeToFileTime(&system_time, &file_time) == 0) return;

   HANDLE file = CreateFileW(wide_path{path}.c_str(), FILE_WRITE_ATTRIBUTES,
                             FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                             FILE_FLAG_BACKUP_SEMANTICS, nullptr);

   if (file == INVALID_HANDLE_VALUE) return;

   SetFileTime(file, nullptr, nullptr, &file_time);

   CloseHandle(file);
}

auto get_last_write_time(const path& path) noexcept -> uint64
{
   HANDLE file = CreateFileW(wide_path{path}.c_str(), FILE_WRITE_ATTRIBUTES,
                             FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                             FILE_FLAG_BACKUP_SEMANTICS, nullptr);

   if (file == INVALID_HANDLE_VALUE) return 0;

   FILETIME file_time{};

   if (not GetFileTime(file, nullptr, nullptr, &file_time)) return 0;

   CloseHandle(file);

   return (uint64{file_time.dwHighDateTime} << 32ull) |
          uint64{file_time.dwLowDateTime};
}

bool create_directory(const path& path) noexcept
{
   return CreateDirectoryW(wide_path{path}.c_str(), nullptr) != 0;
}

struct directory_iterator::impl {
   impl(const path& directory) noexcept;

   ~impl();

   auto get_entry() noexcept -> const directory_entry&;

   void advance() noexcept;

   void mark_skip_directory() noexcept;

   bool is_at_end() const noexcept;

   HANDLE find_handle = INVALID_HANDLE_VALUE;
   WIN32_FIND_DATAW find_data{};
   io::path current_path;

   directory_entry entry;

   bool skip_directory = false;

   std::vector<io::path> pending_directories;

   uint64 ref_count = 1;

   void open_directory(const path& directory) noexcept;

   void close_find_handle() noexcept;

   void fill_entry() noexcept;
};

directory_iterator::directory_iterator() noexcept = default;

directory_iterator::directory_iterator(const path& directory) noexcept
{
   _impl = new directory_iterator::impl{directory};
}

directory_iterator::directory_iterator(const directory_iterator& other) noexcept
{
   this->_impl = other._impl;

   if (this->_impl) this->_impl->ref_count += 1;
}

directory_iterator::directory_iterator(directory_iterator&& other) noexcept
{
   this->swap(other);
}

auto directory_iterator::operator=(const directory_iterator& other) noexcept
   -> directory_iterator&
{
   directory_iterator discard{other};

   this->swap(discard);

   return *this;
}

auto directory_iterator::operator=(directory_iterator&& other) noexcept
   -> directory_iterator&
{
   directory_iterator discard;

   other.swap(discard);
   this->swap(discard);

   return *this;
}

directory_iterator::~directory_iterator()
{
   if (_impl) {
      _impl->ref_count -= 1;

      if (_impl->ref_count == 0) {
         delete _impl;
      }
   }
}

auto directory_iterator::operator*() noexcept -> const directory_entry&
{
   return _impl->get_entry();
}

auto directory_iterator::operator->() noexcept -> const directory_entry*
{
   return &_impl->get_entry();
}

auto directory_iterator::operator++() noexcept -> directory_iterator&
{
   _impl->advance();

   return *this;
}

void directory_iterator::skip_directory() noexcept
{
   return _impl->mark_skip_directory();
}

auto directory_iterator::begin() noexcept -> directory_iterator
{
   return *this;
}

auto directory_iterator::end() noexcept -> directory_iterator
{
   return directory_iterator{};
}

bool directory_iterator::operator==(const directory_iterator&) const noexcept
{
   if (not _impl) return true;

   return _impl->is_at_end();
}

void directory_iterator::swap(directory_iterator& other) noexcept
{
   std::swap(this->_impl, other._impl);
}

auto directory_iterator::impl::get_entry() noexcept -> const directory_entry&
{
   return entry;
}

directory_iterator::impl::impl(const path& directory) noexcept
{
   open_directory(directory);
}

directory_iterator::impl::~impl()
{
   close_find_handle();
}

void directory_iterator::impl::advance() noexcept
{
   if (entry.is_directory and not skip_directory) {
      pending_directories.push_back(entry.path);
   }

   skip_directory = false;

retry:
   const BOOL found_file = FindNextFileW(find_handle, &find_data);

   if (not found_file) {
      close_find_handle();

      while (not pending_directories.empty()) {
         open_directory(pending_directories.front());

         pending_directories.erase(pending_directories.begin());

         if (find_handle != INVALID_HANDLE_VALUE) break;
      }

      return;
   }

   if ((find_data.cFileName[0] == L'.' and find_data.cFileName[1] == L'\0') or
       (find_data.cFileName[0] == L'.' and find_data.cFileName[1] == L'.' and
        find_data.cFileName[2] == L'\0')) {
      goto retry;
   }

   fill_entry();
}

void directory_iterator::impl::mark_skip_directory() noexcept
{
   skip_directory = true;
}

bool directory_iterator::impl::is_at_end() const noexcept
{
   return find_handle == INVALID_HANDLE_VALUE;
}

void directory_iterator::impl::open_directory(const path& directory) noexcept
{
   close_find_handle();

   const path search_path = io::compose_path(directory, "*");

   find_handle = FindFirstFileExW(wide_path{search_path}.c_str(), FindExInfoStandard,
                                  &find_data, FindExSearchNameMatch, nullptr,
                                  FIND_FIRST_EX_LARGE_FETCH);
   current_path = directory;

   while ((find_data.cFileName[0] == L'.' and find_data.cFileName[1] == L'\0') or
          (find_data.cFileName[0] == L'.' and find_data.cFileName[1] == L'.' and
           find_data.cFileName[2] == L'\0')) {
      if (not FindNextFileW(find_handle, &find_data)) {
         close_find_handle();

         return;
      }
   }

   fill_entry();
}

void directory_iterator::impl::fill_entry() noexcept
{
   const int32 needed_bytes =
      WideCharToMultiByte(CP_UTF8, 0, &find_data.cFileName[0], -1, nullptr, 0,
                          nullptr, nullptr);

   if (needed_bytes == 0) std::terminate();

   char utf8_small_str[MAX_PATH];
   std::vector<char> utf8_large_str;
   char* utf8_str = nullptr;

   if (needed_bytes <= sizeof(utf8_small_str)) {
      utf8_str = &utf8_small_str[0];
   }
   else {
      utf8_large_str.resize(needed_bytes);
      utf8_str = utf8_large_str.data();
   }

   if (WideCharToMultiByte(CP_UTF8, 0, &find_data.cFileName[0], -1, utf8_str,
                           needed_bytes, nullptr, nullptr) == 0) {
      std::terminate();
   }

   entry.is_directory = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
   entry.is_file = not entry.is_directory;
   entry.path = compose_path(current_path, utf8_str);
   entry.last_write_time =
      (uint64{find_data.ftLastWriteTime.dwHighDateTime} << 32ull) |
      uint64{find_data.ftLastWriteTime.dwLowDateTime};
}

void directory_iterator::impl::close_find_handle() noexcept
{
   if (find_handle != INVALID_HANDLE_VALUE) {
      FindClose(find_handle);

      find_handle = INVALID_HANDLE_VALUE;
   }
}

}