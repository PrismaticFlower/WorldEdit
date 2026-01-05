#pragma once

#include "io/path.hpp"

#include "utility/implementation_storage.hpp"

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace we::async {
class thread_pool;
}

namespace we::munge {

struct project_config;
struct project;
struct message;

struct report {
   std::vector<message> warnings;
   std::vector<message> errors;
};

struct manager {
   explicit manager(async::thread_pool& thread_pool);

   ~manager();

   void open_project(const io::path& project_directory) noexcept;

   void close_project() noexcept;

   void start_munge(const io::path& game_directory) noexcept;

   void start_clean() noexcept;

   auto get_munge_report() noexcept -> const report&;

   bool is_busy() const noexcept;

   /// @brief Wait for the munge manager to be idle.
   void wait_for_idle() const noexcept;

   /// @brief Get the standard output from the currently running (or most recently ran) task.
   /// @return The lines of the output. Valid until the next call to view_standard_output_lines.
   auto view_standard_output_lines() noexcept -> std::span<const std::string_view>;

   /// @brief Get the standard error output from the currently running (or most recently ran) task.
   /// @return The lines of the output. Valid until the next call to view_standard_error_lines.
   auto view_standard_error_lines() noexcept -> std::span<const std::string_view>;

   auto get_project() noexcept -> project&;

   manager() = delete;
   manager(const manager&) = delete;
   manager(manager&&) = delete;

private:
   struct impl;

   implementation_storage<impl, 2376> impl;
};

}
