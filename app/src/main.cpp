
#include "container/enum_array.hpp"
#include "resource.h"
#include "utility/command_line.hpp"
#include "world_edit.hpp"

#include <atomic>
#include <concepts>
#include <exception>
#include <functional>

#include <Windows.h>
#include <dwmapi.h>
#include <dxgi1_6.h>

#include <wil/resource.h>
#include <wil/result.h>

#include <imgui.h>

using we::utility::command_line;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg,
                                              WPARAM wParam, LPARAM lParam);

// This is a documented cursor but has no IDC_ define.
#define WE_IDC_PEN MAKEINTRESOURCE(32631)

const static we::container::enum_array<HCURSOR, we::mouse_cursor> mouse_cursors = {
   nullptr,
   LoadCursorW(nullptr, IDC_ARROW),
   LoadCursorW(nullptr, IDC_IBEAM),
   LoadCursorW(nullptr, IDC_WAIT),
   LoadCursorW(nullptr, IDC_CROSS),
   LoadCursorW(nullptr, IDC_SIZENWSE),
   LoadCursorW(nullptr, IDC_SIZENESW),
   LoadCursorW(nullptr, IDC_SIZEWE),
   LoadCursorW(nullptr, IDC_SIZENS),
   LoadCursorW(nullptr, IDC_SIZEALL),
   LoadCursorW(nullptr, IDC_NO),
   LoadCursorW(nullptr, IDC_HAND),
   LoadCursorW(nullptr, IDC_APPSTARTING),
   [] { // Incase Wine or something similar doesn't have the pen cursor.
      HCURSOR cursor = LoadCursorW(nullptr, WE_IDC_PEN);
      return cursor ? cursor : LoadCursorW(nullptr, IDC_ARROW);
   }(),
   [] {
      HCURSOR cursor = LoadCursorW(GetModuleHandleW(nullptr),
                                   MAKEINTRESOURCEW(RES_ID_CURSOR_ROTATE_CW));
      return cursor ? cursor : LoadCursorW(nullptr, IDC_ARROW);
   }(),
   [] {
      HCURSOR cursor = LoadCursorW(GetModuleHandleW(nullptr),
                                   MAKEINTRESOURCEW(RES_ID_CURSOR_ENLARGE_TEXTURE));
      return cursor ? cursor : LoadCursorW(nullptr, IDC_ARROW);
   }(),
   [] {
      HCURSOR cursor = LoadCursorW(GetModuleHandleW(nullptr),
                                   MAKEINTRESOURCEW(RES_ID_CURSOR_SHRINK_TEXTURE));
      return cursor ? cursor : LoadCursorW(nullptr, IDC_ARROW);
   }(),
   [] {
      HCURSOR cursor = LoadCursorW(GetModuleHandleW(nullptr),
                                   MAKEINTRESOURCEW(RES_ID_CURSOR_PAINTBRUSH));
      return cursor ? cursor : LoadCursorW(nullptr, IDC_ARROW);
   }(),
};

void static process_mouse_input(we::world_edit& app, const RAWMOUSE& mouse) noexcept
{
   if (mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
      double width = 0.0;
      double height = 0.0;

      if (mouse.usFlags & MOUSE_VIRTUAL_DESKTOP) {
         width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
         height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
      }
      else {
         width = GetSystemMetrics(SM_CXSCREEN);
         height = GetSystemMetrics(SM_CYSCREEN);
      }

      int x = static_cast<int>((mouse.lLastX / 65535.0) * width);
      int y = static_cast<int>((mouse.lLastY / 65535.0) * height);

      static int last_x = 0;
      static int last_y = 0;

      app.mouse_movement(x - last_x, y - last_y);

      last_x = x;
      last_y = y;
   }
   else {
      app.mouse_movement(mouse.lLastX, mouse.lLastY);
   }
}

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
      CreateWindowExW(WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP, window_class_name,
                      window_name, WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
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
   we::mouse_cursor app_cursor = app.get_mouse_cursor();

   // const DWORD my_thread_id = GetCurrentThreadId();

   window_procedure =
      [&, self_window = window_handle.get()](HWND window, UINT message, WPARAM wparam,
                                             LPARAM lparam) noexcept -> LRESULT {
      if (window != self_window) {
         return DefWindowProcW(window, message, wparam, lparam);
      }

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
      case WM_SETFOCUS:
         app.focused();

         return 0;
      case WM_KILLFOCUS:
         app.unfocused();

         return 0;
      case WM_MOUSEMOVE:
         if (not app.mouse_over()) {
            TRACKMOUSEEVENT event{.cbSize = sizeof(TRACKMOUSEEVENT),
                                  .dwFlags = TME_LEAVE,
                                  .hwndTrack = window,
                                  .dwHoverTime = HOVER_DEFAULT};

            TrackMouseEvent(&event);

            app.mouse_enter();
         }

         return DefWindowProcW(window, message, wparam, lparam);
      case WM_MOUSELEAVE:
         app.mouse_leave();

         return 0;
      case WM_SETCURSOR: {
         if (LOWORD(lparam) == HTCLIENT) {
            app_cursor = app.get_mouse_cursor();

            SetCursor(mouse_cursors[app_cursor]);

            return 1;
         }

         return DefWindowProcW(window, message, wparam, lparam);
      }
      case WM_DPICHANGED: {
         auto& rect = *reinterpret_cast<const RECT*>(lparam);

         SetWindowPos(window, nullptr, rect.left, rect.top, rect.right,
                      rect.bottom, SWP_ASYNCWINDOWPOS);

         app.dpi_changed(LOWORD(wparam));

         return 0;
      }
      case WM_SETTINGCHANGE: {
         app.setting_change();

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
         static std::vector<std::byte> raw_input_buffer{sizeof(RAWINPUT) * 16};

         // Process WM_INPUT
         {
            const HRAWINPUT raw_input_handle = reinterpret_cast<HRAWINPUT>(lparam);

            UINT data_size = 0;

            GetRawInputData(raw_input_handle, RID_INPUT, nullptr, &data_size,
                            sizeof(RAWINPUTHEADER));

            if (raw_input_buffer.size() < data_size) {
               raw_input_buffer.resize(data_size);
            }

            RAWINPUT* raw = new (raw_input_buffer.data()) RAWINPUT;

            if (GetRawInputData(raw_input_handle, RID_INPUT, raw, &data_size,
                                sizeof(RAWINPUTHEADER)) != static_cast<UINT>(-1)) {
               if (raw->header.dwType == RIM_TYPEMOUSE) {
                  process_mouse_input(app, raw->data.mouse);
               }
            }
         }

         // Now do a buffered read of RawInput to clear out any other messages.
         // I've read this can avoid a performance pitfall on high poll rate
         // mice. Testing on even my humble (compared to 8khz mice) 1khz mouse
         // shows it can provide a benefit.
         //
         // It also apparently helps mitigate the impact of system wide input
         // hooks but I'm not keen to install many of those for testing.
         while (true) {
            RAWINPUT* raw = new (raw_input_buffer.data()) RAWINPUT;
            UINT data_size = static_cast<UINT>(raw_input_buffer.size());

            const UINT input_count =
               GetRawInputBuffer(raw, &data_size, sizeof(RAWINPUTHEADER));

            if (input_count == static_cast<UINT>(-1)) {
               raw_input_buffer.resize(raw_input_buffer.size() + sizeof(RAWINPUT) * 2);
            }
            else if (input_count == 0) {
               break;
            }
            else {
               for (UINT i = 0; i < input_count; ++i) {
                  using QWORD = UINT64; // For NEXTRAWINPUTBLOCK

                  if (raw->header.dwType == RIM_TYPEMOUSE) {
                     process_mouse_input(app, raw->data.mouse);
                  }

                  raw = NEXTRAWINPUTBLOCK(raw);
               }
            }
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
      HANDLE swap_chain_waitable_object = app.get_swap_chain_waitable_object();

      switch (MsgWaitForMultipleObjects(1, &swap_chain_waitable_object, false,
                                        INFINITE, QS_ALLINPUT)) {
      case WAIT_OBJECT_0: {
         app.update();
      } break;
      case WAIT_OBJECT_0 + 1: {
         MSG message{};

         while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);

            if (message.message == WM_QUIT) return;
         }
      } break;
      case WAIT_FAILED: {
         const DWORD error_code = GetLastError();

         if (app.can_close()) {
            MessageBoxA(
               window_handle.get(),
               fmt::format("An unexpected fatal error (last-error "
                           "code: {}) occured in the "
                           "message loop. The editor will now close and must "
                           "be reopened. There are no unsaved changes.",
                           error_code)
                  .c_str(),
               "Fatal Error", MB_OK);
         }
         else {
            switch (MessageBoxA(
               window_handle.get(),
               "Failed to recover from GPU device "
               "removal.\n\nThe editor will now exit and must be reopened."
               "\n\nSave world?",
               "Fatal Error", MB_YESNO | MB_ICONERROR)) {
            case IDYES:
               app.save_world_with_picker();
               [[fallthrough]];
            case IDNO:
            default:
               return;
            }
         }

         return;
      } break;
      }

      if (app.mouse_over()) {
         const we::mouse_cursor old_app_cursor = app_cursor;

         app_cursor = app.get_mouse_cursor();

         if (app_cursor != old_app_cursor) {
            SetCursor(mouse_cursors[app_cursor]);
         }
      }

      while (app.idling()) {
         WaitMessage();

         MSG message{};

         while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);

            if (message.message == WM_QUIT) return;
         }

         if (not app.idling()) app.idle_exit();
      }

      app.recreate_renderer_pending();
   } while (true);
}

int main(int arg_count, const char** args)
{
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

// Export Nahimic's kill switch. Should stop issues like: https://github.com/ocornut/imgui/issues/4542
//
// Dear ImGui viewports aren't used yet but there are plans to.
extern "C" __declspec(dllexport) extern UINT NoHotPatch = 0x1;
