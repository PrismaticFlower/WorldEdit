#pragma once

#include "command_list.hpp"
#include "hresult_error.hpp"
#include "types.hpp"
#include "utility/com_ptr.hpp"

#include <array>
#include <span>

#include <d3d12.h>

namespace we::graphics::gpu {

class command_queue {
public:
   command_queue() = default;

   command_queue(utility::com_ptr<ID3D12CommandQueue> command_queue) noexcept
      : _command_queue{command_queue}
   {
   }

   void update_tile_mappings(
      ID3D12Resource& resource, uint32 num_resource_regions,
      const D3D12_TILED_RESOURCE_COORDINATE* resource_region_start_coordinates,
      const D3D12_TILE_REGION_SIZE* resource_region_sizes, ID3D12Heap& heap,
      uint32 num_ranges, const D3D12_TILE_RANGE_FLAGS* range_flags,
      uint32* heap_range_start_offsets, uint32* range_tile_counts) noexcept
   {
      _command_queue->UpdateTileMappings(&resource, num_resource_regions,
                                         resource_region_start_coordinates,
                                         resource_region_sizes, &heap, num_ranges,
                                         range_flags, heap_range_start_offsets,
                                         range_tile_counts,
                                         D3D12_TILE_MAPPING_FLAG_NONE);
   }

   void copy_tile_mappings(ID3D12Resource& dst_resource,
                           const D3D12_TILED_RESOURCE_COORDINATE& dst_region_start_coordinate,
                           ID3D12Resource& src_resource,
                           const D3D12_TILED_RESOURCE_COORDINATE& src_region_start_coordinate,
                           const D3D12_TILE_REGION_SIZE& region_size) noexcept
   {
      _command_queue->CopyTileMappings(&dst_resource, &dst_region_start_coordinate,
                                       &src_resource, &src_region_start_coordinate,
                                       &region_size, D3D12_TILE_MAPPING_FLAG_NONE);
   }

   void execute_command_lists(std::derived_from<command_list> auto&... command_lists) noexcept
   {
      std::array d3d_command_lists{
         static_cast<ID3D12CommandList*>(command_lists.get_underlying())...};

      _command_queue->ExecuteCommandLists(sizeof...(command_lists),
                                          d3d_command_lists.data());
   }

   void signal(ID3D12Fence& fence, UINT64 value)
   {
      throw_if_failed(_command_queue->Signal(&fence, value));
   }

   void wait(ID3D12Fence& fence, UINT64 value)
   {
      throw_if_failed(_command_queue->Wait(&fence, value));
   }

private:
   utility::com_ptr<ID3D12CommandQueue> _command_queue;
};

}
