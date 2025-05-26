
#include "profiler.hpp"
#include "container/enum_array.hpp"
#include "gpu/resource.hpp"

#include <span>

#include <array>
#include <atomic>
#include <memory>
#include <string_view>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <fmt/core.h>

#include <imgui.h>

namespace we::graphics {

struct profiler::impl {
   impl(gpu::device& device, const uint32 max_sections)
      : _device{device}, _max_sections{max_sections}, _max_timestamps{max_sections * 2}
   {
      _sections = std::make_unique<section[]>(max_sections);

      _query_heap = {_device.create_timestamp_query_heap(_max_timestamps),
                     _device};
      _readback_buffer =
         {_device.create_buffer({.size = _max_timestamps * sizeof(uint64) * gpu::frame_pipeline_length,
                                 .debug_name =
                                    "Profiler Timestamp Readback Buffer"},
                                gpu::heap_type::readback),
          _device};

      const uint64* const readback_buffer_address =
         static_cast<uint64*>(_device.map(_readback_buffer.get(), 0,
                                          {0, _max_timestamps * sizeof(uint64) *
                                                 gpu::frame_pipeline_length}));

      for (std::size_t i = 0; i < gpu::frame_pipeline_length; ++i) {
         _timestamp_readback[i] = readback_buffer_address + (i * max_sections);
      }

      _data.reserve(_section_count);
      _data_order.reserve(_section_count);

      _timestamp_frequencies[profiler_queue::direct] =
         static_cast<double>(_device.direct_queue.get_timestamp_frequency());
      _timestamp_frequencies[profiler_queue::compute] =
         static_cast<double>(_device.async_compute_queue.get_timestamp_frequency());
   }

   auto begin(const char* name, gpu::compute_command_list& command_list,
              const profiler_queue queue) -> uint32
   {
      const uint32 section_index = _section_count.fetch_add(1);
      const uint32 begin_timestamp_index = _timestamp_count.fetch_add(2);
      const uint32 end_timestamp_index = begin_timestamp_index + 1;

      if (section_index >= _max_sections) return UINT32_MAX;

      _sections[section_index] = {.name = name,
                                  .command_list = &command_list,
                                  .begin_timestamp_index = begin_timestamp_index,
                                  .end_timestamp_index = end_timestamp_index,
                                  .queue = queue};

      command_list.query_timestamp(_query_heap.get(), begin_timestamp_index);

      return section_index;
   }

   void end(const uint32 index)
   {
      if (index >= _max_sections) return;

      section& section = _sections[index];

      section.command_list->query_timestamp(_query_heap.get(),
                                            section.end_timestamp_index);
   }

   void end_frame(gpu::compute_command_list& command_list)
   {
      const uint32 section_count = std::min(_section_count.exchange(0), _max_sections);
      const uint32 timestamp_count =
         std::min(_timestamp_count.exchange(0), _max_timestamps);

      if (timestamp_count == 0) return;
      if (timestamp_count % 2 != 0) std::terminate();

      command_list.resolve_query_timestamp(_query_heap.get(), 0, timestamp_count,
                                           _readback_buffer.get(),
                                           static_cast<uint32>(
                                              _max_timestamps * sizeof(uint64) *
                                              _device.frame_index()));

      _frames_sampled += 1;

      const std::span<const uint64> current_readback{_timestamp_readback[_device.frame_index()],
                                                     timestamp_count};

      _data_order.clear();

      const uint32 current_sample = _frames_sampled % sample_count;

      for (uint32 i = 0; i < section_count; ++i) {
         section& section = _sections[i];
         const double frequency = _timestamp_frequencies[section.queue];

         const uint64 begin = current_readback[section.begin_timestamp_index];
         const uint64 end = current_readback[section.end_timestamp_index];

         const uint64 duration_ticks = end - begin;

         const double duration_seconds = duration_ticks / frequency;

         _data[section.name][current_sample] = duration_seconds;

         _data_order.emplace_back(i, section.name);
      }

      std::ranges::sort(_data_order, std::less{},
                        &std::pair<uint32, std::string_view>::first);
   }

   void show()
   {
      if (ImGui::Begin("GPU Profiler")) {
         for (auto& [_, name] : _data_order) {
            double max_time = 0.0;
            double total_time = 0.0;

            for (double sample : _data[name]) {
               max_time = std::max(max_time, sample);
               total_time += sample;
            }

            constexpr double sample_count_flt = sample_count;

            const double time = total_time / sample_count_flt;

            if (_print_microseconds) {
               ImGui::Text(fmt::format("{}: {:.0f}us (max {:.0f}us)", name,
                                       time * 1'000'000.0, max_time * 1'000'000.0)
                              .c_str());
            }
            else {
               ImGui::Text(fmt::format("{}: {:.3f}ms (max {:.3f}ms)", name,
                                       time * 1000.0, max_time * 1000.0)
                              .c_str());
            }
         }

         ImGui::Separator();

         ImGui::Checkbox("Microseconds", &_print_microseconds);
      }

      ImGui::End();
   }

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

profiler::profiler(gpu::device& device, const uint32 max_sections)
   : _impl{device, max_sections}
{
}

profiler::~profiler() = default;

auto profiler::begin(const char* name, gpu::compute_command_list& command_list,
                     const profiler_queue queue) -> uint32
{
   return _impl->begin(name, command_list, queue);
}

void profiler::end(const uint32 index)
{
   return _impl->end(index);
}

void profiler::end_frame(gpu::compute_command_list& command_list)
{
   return _impl->end_frame(command_list);
}

void profiler::show()
{
   return _impl->show();
}

}