#include "show_in_explorer.hpp"

#include <Windows.h>

#include <Shlobj.h>
#include <shellapi.h>
#include <wil/resource.h>

namespace we::utility {

void try_show_in_explorer(const std::filesystem::path& file) noexcept
{
   const std::filesystem::path parent_path = file.parent_path();

   if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
      return;
   }

   const auto co_uninit = wil::scope_exit([] { CoUninitialize(); });

   LPITEMIDLIST folder_item = ILCreateFromPathW(parent_path.c_str());

   if (not folder_item) return;

   const auto folder_item_free = wil::scope_exit([&] { ILFree(folder_item); });

   LPITEMIDLIST file_item = ILCreateFromPathW(file.c_str());

   if (not file_item) return;

   const auto file_item_free = wil::scope_exit([&] { ILFree(file_item); });

   LPCITEMIDLIST file_item_const = file_item;

   SHOpenFolderAndSelectItems(folder_item, 1, &file_item_const, 0);
}

}
