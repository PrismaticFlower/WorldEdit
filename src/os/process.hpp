#pragma once

#include "io/path.hpp"

#include "utility/implementation_storage.hpp"

#include <span>
#include <stdexcept>
#include <string>

namespace we::os {

/// @brief Thrown when a process fails to launch.
struct process_launch_error : std::runtime_error {
   using std::runtime_error::runtime_error;
};

enum class process_priority { idle, below_normal, normal, above_normal, high };

/// @brief Process creation info.
struct process_create_desc {
   /// @brief Path to the executable for the process.
   const io::path& executable_path;

   /// @brief Command line for the process.
   const std::string& command_line;

   /// @brief If not empty holds the path to the working directory for the process.
   const io::path& working_directory;

   /// @brief Redirect and capture standard output for the process.
   ///
   /// Unless capture_asynchronously is true this turns the process launch
   /// into a synchronous operation as it must enter a loop to read output
   /// from the process so it doesn't stall.
   bool capture_stdout = false;

   /// @brief Redirect and capture standard error for the process.
   ///
   /// Unless capture_asynchronously is true this turns the process launch
   /// into a synchronous operation as it must enter a loop to read output
   /// from the process so it doesn't stall.
   bool capture_stderr = false;

   /// @brief Priority class for the process.
   process_priority priority = process_priority::normal;
};

struct process {
   process(const process_create_desc& desc);

   process() noexcept;

   process(process&& other) noexcept;

   auto operator=(process&& other) noexcept -> process&;

   ~process();

   /// @brief Wait for the process to exit.
   /// @return The process' exit code or -1 if the process object does not represent a process.
   int wait_for_exit() noexcept;

   /// @brief Gets the standard output from the process. May only be called once, else returns an empty string.
   /// @return The complete standard output from the process as a string.
   auto get_standard_output() noexcept -> std::string;

   /// @brief Gets the standard error from the process. May only be called once, else returns an empty string.
   /// @return The complete standard error from the process as a string.
   auto get_standard_error() noexcept -> std::string;

private:
   struct storage;

   implementation_storage<storage, 96> storage;
};

}
