#pragma once

#include <filesystem>
#include <optional>

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

}
