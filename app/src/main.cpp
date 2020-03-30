
#include "world_edit.hpp"

#include <atomic>
#include <concepts>
#include <exception>
#include <functional>

#include <Windows.h>

#include <wil/resource.h>
#include <wil/result.h>

// clang-format off

template<typename App>
concept application = requires(App app, HWND window, int size) {
   { App{window} };
   { app.update() } -> std::convertible_to<bool>;
   { app.resized(size, size) };
   { app.focused() };
   { app.unfocused() };
   { app.idling() } -> std::convertible_to<bool>;
};

// clang-format on

template<application Application>
void run_application()
{
   static std::atomic_flag entered{};

   if (entered.test_and_set()) std::terminate();

   const auto entered_clear = wil::scope_exit([&] { entered.clear(); });

   const wchar_t* window_name = L"WorldEdit";
   const wchar_t* window_class_name = window_name;
   const HINSTANCE current_instance = GetModuleHandle(nullptr);

   static std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> window_procedure =
      [](HWND window, UINT message, WPARAM wparam, LPARAM lparam) noexcept {
         return DefWindowProcW(window, message, wparam, lparam);
      };

   const auto window_procedure_bootstrap =
      [](HWND window, UINT message, WPARAM wparam, LPARAM lparam) noexcept {
         return window_procedure(window, message, wparam, lparam);
      };

   const WNDCLASSW window_class_desc{.style = CS_HREDRAW | CS_VREDRAW,
                                     .lpfnWndProc = window_procedure_bootstrap,
                                     .cbClsExtra = 0,
                                     .cbWndExtra = 0,
                                     .hInstance = current_instance,
                                     .hIcon = nullptr,
                                     .hCursor = nullptr,
                                     .hbrBackground = nullptr,
                                     .lpszMenuName = nullptr,
                                     .lpszClassName = window_class_name};

   const ATOM class_atom = RegisterClassW(&window_class_desc);

   if (class_atom == 0) std::terminate();

   const auto class_unregister = wil::scope_exit(
      [&] { UnregisterClassW(window_class_name, current_instance); });

   const wil::unique_hwnd window_handle{
      CreateWindowExW(WS_EX_APPWINDOW, window_class_name, window_name,
                      WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                      CW_USEDEFAULT, nullptr, nullptr, current_instance, nullptr)};

   if (window_handle == nullptr) std::terminate();

   Application app{window_handle.get()};

   window_procedure = [&](HWND window, UINT message, WPARAM wparam,
                          LPARAM lparam) noexcept -> LRESULT {
      switch (message) {
      case WM_DESTROY:
         PostQuitMessage(0);

         return 0;
      case WM_SIZE:
         app.resized(lparam & 0xffff, (lparam >> 16) & 0xffff);

         return 0;
      case WM_ACTIVATEAPP:
         if (wparam == TRUE) {
            app.focused();
         }
         else if (wparam == FALSE) {
            app.unfocused();
         }

         return 0;
      }

      return DefWindowProcW(window, message, wparam, lparam);
   };

   ShowWindowAsync(window_handle.get(), SW_SHOWDEFAULT);

   do {
      if (app.idling()) {
         MSG message{};
         if (const auto result = GetMessageW(&message, nullptr, 0, 0); result > 0) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
         }
         else {
            return;
         }
      }
      else {
         MSG message{};

         while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);

            if (message.message == WM_QUIT) return;
         }
      }
   } while (app.update());
}

int main()
{
   run_application<sk::world_edit>();
}
