#pragma once

#include <filesystem>
#include <optional>
#include <vector>

#include <Windows.h>

namespace we::utility {

struct folder_picker {
   std::optional<std::wstring> title;
   std::optional<std::wstring> ok_button_label;
   std::optional<std::filesystem::path> default_folder;
   std::optional<GUID> picker_guid;
   std::optional<HWND> window;
};

/// @brief Creates and shows a folder picker. This functions calls CoInitializeEx and CoUninitialize.
/// @param options The options for the folder picker.
/// @return The path selected or nullopt.
[[nodiscard]] auto show_folder_picker(const folder_picker options) noexcept
   -> std::optional<std::filesystem::path>;

struct file_picker_filter {
   std::wstring name;
   std::wstring filter; // see Microsoft docs for IFileDialog::SetFileTypes
};

struct file_picker {
   std::optional<std::wstring> title;
   std::optional<std::wstring> ok_button_label;
   std::optional<std::filesystem::path> default_folder;
   std::optional<std::filesystem::path> forced_start_folder;
   std::vector<file_picker_filter> filters;
   std::optional<GUID> picker_guid;
   std::optional<HWND> window;
   bool must_exist = false; // hint only, race condition to depend on
};

/// @brief Creates and shows a file picker. This functions calls CoInitializeEx and CoUninitialize.
/// @param options The options for the file picker.
/// @return The path selected or nullopt.
[[nodiscard]] auto show_file_open_picker(const file_picker options) noexcept
   -> std::optional<std::filesystem::path>;

/// @brief Creates and shows a file picker. This functions calls CoInitializeEx and CoUninitialize.
/// @param options The options for the file picker.
/// @return The path selected or nullopt.
[[nodiscard]] auto show_file_save_picker(const file_picker options) noexcept
   -> std::optional<std::filesystem::path>;

}
