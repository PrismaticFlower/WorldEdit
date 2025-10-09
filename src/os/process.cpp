#include "process.hpp"

#include <array>
#include <atomic>
#include <system_error>
#include <vector>

#include <fmt/format.h>

#include <Windows.h>
#include <wil/resource.h>

namespace we::os {

namespace {

constexpr DWORD pipe_buffer_size = 4096;

auto get_priority_class(const process_priority priority) noexcept -> DWORD
{
   switch (priority) {
   case process_priority::idle:
      return IDLE_PRIORITY_CLASS;
   case process_priority::below_normal:
      return BELOW_NORMAL_PRIORITY_CLASS;
   case process_priority::normal:
      return NORMAL_PRIORITY_CLASS;
   case process_priority::above_normal:
      return ABOVE_NORMAL_PRIORITY_CLASS;
   case process_priority::high:
      return HIGH_PRIORITY_CLASS;
   }

   return NORMAL_PRIORITY_CLASS;
}

bool create_overlapped_pipe(HANDLE* read_handle, HANDLE* write_handle,
                            LPSECURITY_ATTRIBUTES security_attributes, DWORD size)
{
   if (not read_handle or not write_handle) return false;

   static std::atomic_uint64_t pipe_count = 0;

   constexpr std::size_t max_pipe_name_length =
      std::string_view{R"(\\.\pipe\LOCAL\anon\proc_io.4294967295.18446744073709551615)"}
         .size() +
      1;

   std::array<char, max_pipe_name_length> pipe_name;

   *fmt::format_to(pipe_name.begin(), R"(\\.\pipe\LOCAL\anon\proc_io.{}.{})",
                   GetCurrentProcessId(),
                   pipe_count.fetch_add(1, std::memory_order_relaxed)) = '\0';

   wil::unique_hfile pipe_read_handle{
      CreateNamedPipeA(pipe_name.data(), PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
                       PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
                       1, size, size, 120 * 1000, security_attributes)};

   if (not pipe_read_handle) return false;

   wil::unique_hfile pipe_write_handle{
      CreateFileA(pipe_name.data(), GENERIC_WRITE, 0, security_attributes,
                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)};

   if (not pipe_write_handle) return false;

   *read_handle = pipe_read_handle.release();
   *write_handle = pipe_write_handle.release();

   return true;
}

}

struct process::storage {

   wil::unique_process_handle process_handle;
   wil::unique_handle thread_handle;

   std::string standard_output;
   std::string standard_error;
};

process::process() noexcept = default;

process::process(const process_create_desc& desc)
{
   std::string command_line =
      fmt::format("{} {}", desc.executable_path.string_view(), desc.command_line);

   STARTUPINFOEXA startup_info = {
      .StartupInfo =
         {
            .cb = sizeof(STARTUPINFOEXA),
         },
      .lpAttributeList = {},
   };

   std::vector<HANDLE> inherited_handle_list;
   wil::unique_hfile stdout_read;
   wil::unique_hfile stdout_write;
   wil::unique_hfile stderr_read;
   wil::unique_hfile stderr_write;

   if (desc.capture_stdout or desc.capture_stderr) {
      if (desc.capture_stdout) {
         if (not create_overlapped_pipe(stdout_read.put(), stdout_write.put(),
                                        nullptr, pipe_buffer_size)) {
            throw process_launch_error{fmt::format(
               "Failed to create process.\n   Reason: Failed to create pipe "
               "for captured standard output.\n   Executable Path: {}\n   ",
               desc.executable_path.string_view())};
         }

         if (not SetHandleInformation(stdout_write.get(), HANDLE_FLAG_INHERIT,
                                      HANDLE_FLAG_INHERIT)) {
            throw process_launch_error{fmt::format(
               "Failed to create process.\n   Reason: Failed to make standard "
               "output pipe write handle inheritable.\n   Executable Path: "
               "{}\n   ",
               desc.executable_path.string_view())};
         }

         inherited_handle_list.push_back(stdout_write.get());
      }

      if (desc.capture_stderr) {
         if (not create_overlapped_pipe(stderr_read.put(), stderr_write.put(),
                                        nullptr, pipe_buffer_size)) {
            throw process_launch_error{fmt::format(
               "Failed to create process.\n   Reason: Failed to create pipe "
               "for captured standard error.\n   Executable Path: {}\n   ",
               desc.executable_path.string_view())};
         }

         if (not SetHandleInformation(stderr_write.get(), HANDLE_FLAG_INHERIT,
                                      HANDLE_FLAG_INHERIT)) {
            throw process_launch_error{fmt::format(
               "Failed to create process.\n   Reason: Failed to make standard "
               "error pipe write handle inheritable.\n   Executable Path: "
               "{}\n   ",
               desc.executable_path.string_view())};
         }

         inherited_handle_list.push_back(stderr_write.get());
      }

      startup_info.StartupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
      startup_info.StartupInfo.hStdOutput =
         desc.capture_stdout ? stdout_write.get() : GetStdHandle(STD_OUTPUT_HANDLE);
      startup_info.StartupInfo.hStdError =
         desc.capture_stderr ? stderr_write.get() : GetStdHandle(STD_ERROR_HANDLE);

      startup_info.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
   }

   std::vector<std::byte> proc_thread_attribute_list;

   if (not inherited_handle_list.empty()) {
      SIZE_T needed_size = {};

      InitializeProcThreadAttributeList(nullptr, 1, 0, &needed_size);

      proc_thread_attribute_list.resize(needed_size);

      LPPROC_THREAD_ATTRIBUTE_LIST attribute_list =
         reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(
            proc_thread_attribute_list.data());

      if (not InitializeProcThreadAttributeList(attribute_list, 1, 0, &needed_size)) {
         throw process_launch_error{
            fmt::format("Failed to create process.\n   Reason: "
                        "InitializeProcThreadAttributeList call failed.\n   "
                        "Executable Path: "
                        "{}\n   ",
                        desc.executable_path.string_view())};
      }

      if (not UpdateProcThreadAttribute(attribute_list, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                                        inherited_handle_list.data(),
                                        inherited_handle_list.size() * sizeof(HANDLE),
                                        nullptr, nullptr)) {
         throw process_launch_error{
            fmt::format("Failed to create process.\n   Reason: "
                        "UpdateProcThreadAttribute call failed.\n   "
                        "Executable Path: "
                        "{}\n   ",
                        desc.executable_path.string_view())};
      }

      startup_info.lpAttributeList = attribute_list;
   }

   PROCESS_INFORMATION process_info = {};

   if (not CreateProcessA(desc.executable_path.c_str(), command_line.data(),
                          nullptr, nullptr, not inherited_handle_list.empty(),
                          EXTENDED_STARTUPINFO_PRESENT | get_priority_class(desc.priority),
                          nullptr,
                          not desc.working_directory.empty()
                             ? desc.working_directory.c_str()
                             : nullptr,
                          &startup_info.StartupInfo, &process_info)) {
      const DWORD last_error = GetLastError();

      throw process_launch_error{
         fmt::format("Failed to create process.\n   Error Code: "
                     "{}\n   Reason: {}\n   Executable Path: {}\n   Command "
                     "Line: {}\n   Working Directory: {}",
                     last_error,
                     std::system_category().default_error_condition(last_error).message(),
                     desc.executable_path.string_view(), desc.command_line,
                     desc.working_directory.string_view())};
   }

   storage->process_handle.reset(process_info.hProcess);
   storage->thread_handle.reset(process_info.hThread);

   stdout_write.reset();
   stderr_write.reset();

   if (desc.capture_stdout and desc.capture_stderr) {
      storage->standard_output.reserve(pipe_buffer_size);
      storage->standard_error.reserve(pipe_buffer_size);

      wil::unique_event stdout_event{wil::EventOptions::ManualReset};
      wil::unique_event stderr_event{wil::EventOptions::ManualReset};

      std::array<char, pipe_buffer_size> stdout_buffer;
      std::array<char, pipe_buffer_size> stderr_buffer;

      OVERLAPPED stdout_overlapped{.hEvent = stdout_event.get()};
      OVERLAPPED stderr_overlapped{.hEvent = stderr_event.get()};

      bool stdout_done = false;
      bool stderr_done = false;

      if (not ReadFile(stdout_read.get(), stdout_buffer.data(),
                       pipe_buffer_size, nullptr, &stdout_overlapped) and
          GetLastError() != ERROR_IO_PENDING) {
         stdout_done = true;
      }

      if (not ReadFile(stderr_read.get(), stderr_buffer.data(),
                       pipe_buffer_size, nullptr, &stderr_overlapped) and
          GetLastError() != ERROR_IO_PENDING) {
         stderr_done = true;
      }

      while (not stdout_done and not stderr_done) {
         switch (WaitForMultipleObjects(
            2, std::array{stdout_event.get(), stderr_event.get()}.data(), false,
            INFINITE)) {
         case WAIT_OBJECT_0: {
            DWORD bytes_read = 0;

            if (not GetOverlappedResult(stdout_read.get(), &stdout_overlapped,
                                        &bytes_read, false) or
                bytes_read == 0) {
               stdout_done = true;
            }
            else {
               storage->standard_output.append(stdout_buffer.data(), bytes_read);

               stdout_overlapped = {.hEvent = stdout_event.get()};

               if (not ReadFile(stdout_read.get(), stdout_buffer.data(),
                                pipe_buffer_size, nullptr, &stdout_overlapped) and
                   GetLastError() != ERROR_IO_PENDING) {
                  stdout_done = true;
               }
            }
         } break;
         case WAIT_OBJECT_0 + 1: {
            DWORD bytes_read = 0;

            if (not GetOverlappedResult(stderr_read.get(), &stderr_overlapped,
                                        &bytes_read, false) or
                bytes_read == 0) {
               stderr_done = true;
            }
            else {
               storage->standard_error.append(stderr_buffer.data(), bytes_read);

               stderr_overlapped = {.hEvent = stderr_event.get()};

               if (not ReadFile(stderr_read.get(), stderr_buffer.data(),
                                pipe_buffer_size, nullptr, &stderr_overlapped) and
                   GetLastError() != ERROR_IO_PENDING) {
                  stderr_done = true;
               }
            }
         } break;
         default:
            std::terminate();
         }
      }

      while (not stdout_done) {
         switch (WaitForSingleObject(stdout_event.get(), INFINITE)) {
         case WAIT_OBJECT_0: {
            DWORD bytes_read = 0;

            if (not GetOverlappedResult(stdout_read.get(), &stdout_overlapped,
                                        &bytes_read, false) or
                bytes_read == 0) {
               stdout_done = true;
            }
            else {
               storage->standard_output.append(stdout_buffer.data(), bytes_read);

               stdout_overlapped = {.hEvent = stdout_event.get()};

               if (not ReadFile(stdout_read.get(), stdout_buffer.data(),
                                pipe_buffer_size, nullptr, &stdout_overlapped) and
                   GetLastError() != ERROR_IO_PENDING) {
                  stdout_done = true;
               }
            }
         } break;
         default:
            std::terminate();
         }
      }

      while (not stderr_done) {
         switch (WaitForSingleObject(stderr_event.get(), INFINITE)) {
         case WAIT_OBJECT_0: {
            DWORD bytes_read = 0;

            if (not GetOverlappedResult(stderr_read.get(), &stderr_overlapped,
                                        &bytes_read, false) or
                bytes_read == 0) {
               stderr_done = true;
            }
            else {
               storage->standard_error.append(stderr_buffer.data(), bytes_read);

               stderr_overlapped = {.hEvent = stderr_event.get()};

               if (not ReadFile(stderr_read.get(), stderr_buffer.data(),
                                pipe_buffer_size, nullptr, &stderr_overlapped) and
                   GetLastError() != ERROR_IO_PENDING) {
                  stderr_done = true;
               }
            }
         } break;
         default:
            std::terminate();
         }
      }
   }
   else if (desc.capture_stdout) {
      storage->standard_output.reserve(pipe_buffer_size);

      std::array<char, pipe_buffer_size> buffer;
      DWORD bytes_read = 0;

      while (ReadFile(stdout_read.get(), buffer.data(), pipe_buffer_size,
                      &bytes_read, nullptr) and
             bytes_read != 0) {
         storage->standard_output.append(buffer.data(), bytes_read);
      }
   }
   else if (desc.capture_stderr) {
      storage->standard_error.reserve(pipe_buffer_size);

      std::array<char, pipe_buffer_size> buffer;
      DWORD bytes_read = 0;

      while (ReadFile(stderr_read.get(), buffer.data(), pipe_buffer_size,
                      &bytes_read, nullptr) and
             bytes_read != 0) {
         storage->standard_error.append(buffer.data(), bytes_read);
      }
   }
}

process::process(process&& other) noexcept = default;

auto process::operator=(process&& other) noexcept -> process& = default;

process::~process() = default;

int process::wait_for_exit() noexcept
{
   if (not storage->process_handle) return -1;

   if (WaitForSingleObject(storage->process_handle.get(), INFINITE) != WAIT_OBJECT_0) {
      std::terminate();
   }

   DWORD exit_code = 0;

   if (not GetExitCodeProcess(storage->process_handle.get(), &exit_code)) {
      std::terminate();
   }

   return exit_code;
}

auto process::get_standard_output() noexcept -> std::string
{
   return std::move(storage->standard_output);
}

auto process::get_standard_error() noexcept -> std::string
{
   return std::move(storage->standard_error);
}

}