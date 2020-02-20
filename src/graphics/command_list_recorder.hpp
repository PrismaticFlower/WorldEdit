#pragma once

#include "concepts.hpp"

#include <utility/com_ptr.hpp>

#include <utility>

#include <boost/container/static_vector.hpp>
#include <d3d12.h>

namespace sk::graphics {

class command_list_recorder {
public:
   command_list_recorder(const command_list_recorder&) = delete;
   auto operator=(const command_list_recorder&) -> command_list_recorder& = delete;

   command_list_recorder(command_list_recorder&&) = delete;
   auto operator=(command_list_recorder&& other) -> command_list_recorder& = delete;

   auto close() -> HRESULT
   {
      flush_barriers();

      return command_list->Close();
   }

   auto reset(ID3D12CommandAllocator& allocator,
              ID3D12PipelineState* pipeline_state = nullptr) -> HRESULT
   {
      return command_list->Reset(&allocator, pipeline_state);
   }

   template<gpu_resource_ref Dest, gpu_resource_ref Src>
   void copy_buffer_region(Dest& dest, const UINT64 dest_offset, Src& src,
                           const UINT64 src_offset, const UINT64 byte_count)
   {
      assert(dest.resource);
      assert(dest.size - dest_offset <= byte_count);
      assert(src.resource);
      assert(src.size - src_offset <= byte_count);

      flush_barriers();

      command_list->CopyBufferRegion(dest.resource, dest_offset, src.resource,
                                     src_offset, byte_count);
   }

   template<gpu_resource_ref Dest, gpu_resource_ref Src>
   void copy_resource(Dest& dest, Src& src)
   {
      flush_barriers();

      command_list->CopyResource(dest.resource, src.resource);
   }

   template<gpu_resource_ref T>
   void resource_transition_barrier(T& resource, const D3D12_RESOURCE_STATES new_state,
                                    const D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
                                    const UINT subresource = 0)
   {
      if (_buffered_barriers.size() == _buffered_barriers.capacity()) {
         assert(false); // Trigger a debug break if we're debugging.

         flush_barriers();
      }

      _buffered_barriers.push_back(
         {.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
          .Flags = flags,
          .Transition = {.pResource = resource.resource,
                         .Subresource = subresource,
                         .StateBefore = std::exchange(resource.resource_state, new_state),
                         .StateAfter = new_state}});
   }

   void flush_barriers()
   {
      if (_buffered_barriers.empty()) return;

      command_list->ResourceBarrier(static_cast<UINT>(_buffered_barriers.size()),
                                    _buffered_barriers.data());
   }

   utility::com_ptr<ID3D12GraphicsCommandList5> command_list;

private:
   constexpr static int max_buffered_barriers = 16;

   boost::container::static_vector<D3D12_RESOURCE_BARRIER, max_buffered_barriers> _buffered_barriers;
};

class command_list_recorder_compute : public command_list_recorder {
public:
private:
};

class command_list_recorder_direct : public command_list_recorder_compute {
public:
   void draw(const UINT vertex_count, const UINT start_vertex_location)
   {
      draw_instanced(vertex_count, 1, start_vertex_location, 0);
   }

   void draw_instanced(const UINT vertex_count_per_instance,
                       const UINT instance_count, const UINT start_vertex_location,
                       const UINT start_instance_location)
   {
      flush_barriers();

      command_list->DrawInstanced(vertex_count_per_instance, instance_count,
                                  start_vertex_location, start_instance_location);
   }

   void draw_indexed(const UINT index_count, const UINT start_index_location,
                     const INT base_vertex_location)
   {
      draw_indexed_instanced(index_count, 1, start_index_location,
                             base_vertex_location, 0);
   }

   void draw_indexed_instanced(const UINT index_count_per_instance,
                               const UINT instance_count, const UINT start_index_location,
                               const INT base_vertex_location,
                               const UINT start_instance_location)
   {
      flush_barriers();

      command_list->DrawIndexedInstanced(index_count_per_instance, instance_count,
                                         start_index_location, base_vertex_location,
                                         start_instance_location);
   }

private:
};

}
