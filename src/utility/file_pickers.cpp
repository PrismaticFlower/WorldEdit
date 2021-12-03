
#include "file_pickers.hpp"
#include "utility/com_ptr.hpp"

#include <shobjidl.h>
#include <wil/resource.h>

namespace we::utility {

auto show_folder_picker(const folder_picker options) noexcept
   -> std::optional<std::filesystem::path>
{
   utility::com_ptr<IFileOpenDialog> dialog;

   if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
      return std::nullopt;
   }

   const auto co_uninit = wil::scope_exit([] { CoUninitialize(); });

   if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                               IID_IFileOpenDialog, dialog.void_clear_and_assign()))) {
      return std::nullopt;
   }

   if (FAILED(dialog->SetOptions(FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST |
                                 FOS_NOCHANGEDIR | FOS_PICKFOLDERS))) {
      return std::nullopt;
   }

   if (options.title) {
      if (FAILED(dialog->SetTitle(options.title->c_str()))) {
         return std::nullopt;
      }
   }

   if (options.ok_button_label) {
      if (FAILED(dialog->SetOkButtonLabel(options.ok_button_label->c_str()))) {
         return std::nullopt;
      }
   }

   if (options.default_folder) {
      utility::com_ptr<IShellItem> starting_item;

      if (FAILED(SHCreateItemFromParsingName(options.default_folder->c_str(),
                                             nullptr, IID_IShellItem,
                                             starting_item.void_clear_and_assign()))) {
         return std::nullopt;
      }

      if (FAILED(dialog->SetDefaultFolder(starting_item.get()))) {
         return std::nullopt;
      }
   }

   if (options.picker_guid) {
      if (FAILED(dialog->SetClientGuid(*options.picker_guid))) {
         return std::nullopt;
      }
   }

   if (FAILED(dialog->Show(options.window.value_or(nullptr)))) {
      return std::nullopt;
   }

   utility::com_ptr<IShellItem> item;

   if (FAILED(dialog->GetResult(item.clear_and_assign()))) {
      return std::nullopt;
   }

   wchar_t* name = nullptr;
   const auto entered_clear = wil::scope_exit([&] {
      if (name) CoTaskMemFree(name);
   });

   if (FAILED(item->GetDisplayName(SIGDN_FILESYSPATH, &name))) {
      return std::nullopt;
   }

   return std::make_optional<std::filesystem::path>(name);
}

}