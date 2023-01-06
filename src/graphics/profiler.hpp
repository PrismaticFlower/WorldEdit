#pragma once

#include "container/enum_array.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"

#include <array>
#include <atomic>
#include <memory>
#include <string_view>
#include <vector>

#include <absl/container/flat_hash_map.h>

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
   gpu::device& _device;

   std::atomic_uint32_t _section_count = 0;
   const uint32 _max_sections;

   std::atomic_uint32_t _timestamp_count = 0;
   const uint32 _max_timestamps;

   struct section {
      const char* name = nullptr;
      gpu::compute_command_list* command_list = nullptr;
      uint32 begin_timestamp_index = 0xff'ff'ff'ffu;
      uint32 end_timestamp_index = 0xff'ff'ff'ffu;
      profiler_queue queue = profiler_queue::direct;
   };

   std::unique_ptr<section[]> _sections;

   gpu::unique_query_heap_handle _query_heap;
   gpu::unique_resource_handle _readback_buffer;

   std::array<const uint64*, gpu::frame_pipeline_length> _timestamp_readback;

   constexpr static uint32 sample_count = 64;

   uint32 _frames_sampled = 0;

   // string_view as the key is intentional, only string literals are allowed to be passed to profiler::begin so it should be safe.
   absl::flat_hash_map<std::string_view, std::array<double, sample_count>> _data;
   std::vector<std::pair<uint32, std::string_view>> _data_order;

   container::enum_array<double, profiler_queue> _timestamp_frequencies;

   bool _print_microseconds = false;
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