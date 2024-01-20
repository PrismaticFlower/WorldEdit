#pragma once

#include "gpu/rhi.hpp"
#include "utility/implementation_storage.hpp"

namespace we::graphics {

/// @brief Tells the profiler what time stamp frequency to use.
enum class profiler_queue {
   direct,
   compute,

   count
};

struct profiler {
   /// @brief Construct the profiler.
   /// @param max_sections The max number of sections that can be profiled before begin calls will start to get dropped.
   profiler(gpu::device& device, const uint32 max_sections);

   profiler(const profiler&) noexcept = delete;
   profiler(profiler&&) noexcept = delete;

   ~profiler();

   /// @brief Begin a profiler section.
   /// @param name The name of the section. Assumed to be a string literal to avoid copy overhead.
   /// @param command_list The command list to issue the timestamp queries on.
   /// @return The index of the profiler section, must be passed to profiler::end
   auto begin(const char* name, gpu::compute_command_list& command_list,
              const profiler_queue queue) -> uint32;

   /// @brief Ends a profiler section.
   /// @param index The index of the profiler section to end.
   void end(const uint32 index);

   /// @brief End the frame and resolve the timestamp queries in bulk.
   /// @param command_list The command list to use to resolve the queries.
   void end_frame(gpu::compute_command_list& command_list);

   /// @brief Display the profiler in an ImGui window.
   void show();

private:
   struct impl;

   implementation_storage<impl, 184> _impl;
};

/// @brief RAII wrapper around profiler.
struct profile_section {
   profile_section(const char* name, gpu::compute_command_list& command_list,
                   profiler& profiler, profiler_queue queue)
      : _profiler{profiler}, _index{_profiler.begin(name, command_list, queue)}
   {
   }

   ~profile_section()
   {
      _profiler.end(_index);
   }

private:
   profiler& _profiler;
   const uint32 _index;
};

}