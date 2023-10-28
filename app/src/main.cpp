
#include "utility/command_line.hpp"
#include "world_edit.hpp"

#include <atomic>
#include <concepts>
#include <exception>
#include <functional>
#include <iostream>

#include <Windows.h>
#include <dwmapi.h>
#include <dxgi1_6.h>

#include <wil/resource.h>
#include <wil/result.h>

#include <imgui.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

using we::utility::command_line;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg,
                                              WPARAM wParam, LPARAM lParam);

void run_application(command_line command_line)
{
   static std::atomic_flag entered{};

   if (entered.test_and_set()) std::terminate();

   const auto entered_clear = wil::scope_exit([&] { entered.clear(); });

   const wchar_t* window_name = L"WorldEdit";
   const wchar_t* window_class_name = window_name;
   const HINSTANCE current_instance = GetModuleHandle(nullptr);

   static std::move_only_function<LRESULT(HWND, UINT, WPARAM, LPARAM)> window_procedure =
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
      CreateWindowExW(WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP,
                      window_class_name, window_name, WS_OVERLAPPEDWINDOW,
                      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                      nullptr, nullptr, current_instance, nullptr)};

   if (window_handle == nullptr) std::terminate();

   BOOL use_immersive_dark_mode = true;
   DwmSetWindowAttribute(window_handle.get(), DWMWA_USE_IMMERSIVE_DARK_MODE,
                         &use_immersive_dark_mode, sizeof(use_immersive_dark_mode));

   RAWINPUTDEVICE raw_input_device{.usUsagePage = 0x01, // HID_USAGE_PAGE_GENERIC
                                   .usUsage = 0x02, // HID_USAGE_GENERIC_MOUSE
                                   .dwFlags = 0x0,
                                   .hwndTarget = window_handle.get()};

   if (RegisterRawInputDevices(&raw_input_device, 1, sizeof(raw_input_device)) == false) {
      std::terminate();
   }

   ShowWindowAsync(window_handle.get(), SW_SHOWMAXIMIZED);

   we::world_edit app{window_handle.get(), command_line};

   window_procedure = [&](HWND window, UINT message, WPARAM wparam,
                          LPARAM lparam) noexcept -> LRESULT {
      if (LRESULT result =
             ImGui_ImplWin32_WndProcHandler(window, message, wparam, lparam);
          result != 0) {
         return result;
      }

      switch (message) {
      case WM_DESTROY:
         PostQuitMessage(0);

         return 0;
      case WM_CLOSE: {
         if (not app.can_close()) return 0;

         return DefWindowProcW(window, message, wparam, lparam);
      }
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
      case WM_DPICHANGED: {
         auto& rect = *reinterpret_cast<const RECT*>(lparam);

         SetWindowPos(window, nullptr, rect.left, rect.top, rect.right,
                      rect.bottom, SWP_ASYNCWINDOWPOS);

         app.dpi_changed(LOWORD(wparam));

         return 0;
      }
      case WM_KEYDOWN: {
         app.key_down(we::translate_virtual_key(wparam));

         return 0;
      }
      case WM_SYSKEYDOWN: {
         app.key_down(we::translate_virtual_key(wparam));

         return DefWindowProcW(window, message, wparam, lparam);
      }
      case WM_KEYUP: {
         app.key_up(we::translate_virtual_key(wparam));

         return 0;
      }
      case WM_SYSKEYUP: {
         app.key_up(we::translate_virtual_key(wparam));

         return DefWindowProcW(window, message, wparam, lparam);
      }
      case WM_LBUTTONDOWN: {
         app.key_down(we::key::mouse1);

         return 0;
      }
      case WM_LBUTTONUP: {
         app.key_up(we::key::mouse1);

         return 0;
      }
      case WM_RBUTTONDOWN: {
         app.key_down(we::key::mouse2);

         return 0;
      }
      case WM_RBUTTONUP: {
         app.key_up(we::key::mouse2);

         return 0;
      }
      case WM_MBUTTONDOWN: {
         app.key_down(we::key::mouse3);

         return 0;
      }
      case WM_MBUTTONUP: {
         app.key_up(we::key::mouse3);

         return 0;
      }
      case WM_XBUTTONDOWN: {
         if (const int button = GET_XBUTTON_WPARAM(wparam) == XBUTTON1;
             button == XBUTTON1) {
            app.key_down(we::key::mouse4);
         }
         else if (button == XBUTTON2) {
            app.key_down(we::key::mouse5);
         }

         return 0;
      }
      case WM_XBUTTONUP: {
         if (const int button = GET_XBUTTON_WPARAM(wparam) == XBUTTON1;
             button == XBUTTON1) {
            app.key_up(we::key::mouse4);
         }
         else if (button == XBUTTON2) {
            app.key_up(we::key::mouse5);
         }

         return 0;
      }
      case WM_MOUSEWHEEL: {
         static int delta = 0;

         delta += GET_WHEEL_DELTA_WPARAM(wparam);

         const int steps = delta / WHEEL_DELTA;

         for (int i = 0; i < std::abs(steps); ++i) {
            const we::key key = delta > 0 ? we::key::mouse_wheel_forward
                                          : we::key::mouse_wheel_back;

            app.key_down(key);
            app.key_up(key);
         }

         delta -= (steps * WHEEL_DELTA);

         return 0;
      };
      case WM_INPUT: {
         const HRAWINPUT raw_input_handle = reinterpret_cast<HRAWINPUT>(lparam);

         UINT data_size = 0;

         GetRawInputData(raw_input_handle, RID_INPUT, nullptr, &data_size,
                         sizeof(RAWINPUTHEADER));

         std::byte* const header_bytes = static_cast<std::byte*>(alloca(data_size));
         RAWINPUT* raw = new (header_bytes) RAWINPUT;

         if (GetRawInputData(raw_input_handle, RID_INPUT, raw, &data_size,
                             sizeof(RAWINPUTHEADER)) == static_cast<UINT>(-1)) {
            return 0;
         }

         if (raw->header.dwType == RIM_TYPEMOUSE) {
            app.mouse_movement(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
         }

         return 0;
      }
      case WM_SYSCOMMAND: {
         if (wparam == SC_KEYMENU or wparam == SC_MOUSEMENU) return 0;

         return DefWindowProcW(window, message, wparam, lparam);
      }
      }

      return DefWindowProcW(window, message, wparam, lparam);
   };

   do {
      MSG message{};

      while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
         TranslateMessage(&message);
         DispatchMessageW(&message);

         if (message.message == WM_QUIT) return;
      }

      if (app.idling()) WaitMessage();

      app.update();
   } while (true);
}

int main(int arg_count, const char** args)
{
   std::ios_base::sync_with_stdio(false);

   ImGui::SetAllocatorFunctions(
      [](std::size_t size, [[maybe_unused]] void* user_data) noexcept {
         return ::operator new(size);
      },
      [](void* allocation, [[maybe_unused]] void* user_data) noexcept {
         return ::operator delete(allocation);
      });

   DXGIDeclareAdapterRemovalSupport();

   run_application(command_line{arg_count, args});
}
