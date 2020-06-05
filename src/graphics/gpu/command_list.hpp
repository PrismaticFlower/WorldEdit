#pragma once

#include "device.hpp"
#include "dynamic_buffer_allocator.hpp"
#include "hresult_error.hpp"
#include "types.hpp"
#include "utility/com_ptr.hpp"

#include <array>
#include <optional>
#include <span>

#include <d3d12.h>

namespace sk::graphics::gpu {

class command_list {
public:
   command_list(const D3D12_COMMAND_LIST_TYPE type, device& device)
   {
      _command_list = device.create_command_list(type);
   }

   void close()
   {
      throw_if_failed(_command_list->Close());
   }

   void reset(ID3D12CommandAllocator& allocator,
              dynamic_buffer_allocator& dynamic_buffer_allocator,
              ID3D12PipelineState* const initial_state = nullptr)
   {
      _dynamic_buffer_allocator = &dynamic_buffer_allocator;
      throw_if_failed(_command_list->Reset(&allocator, initial_state));
   }

   void clear_state(ID3D12PipelineState* const initial_state = nullptr)
   {
      _command_list->ClearState(initial_state);
   }

   void draw_instanced(const uint32 vertex_count_per_instance,
                       const uint32 instance_count, const uint32 start_vertex_location,
                       const uint32 start_instance_location)
   {
      _command_list->DrawInstanced(vertex_count_per_instance, instance_count,
                                   start_vertex_location, start_instance_location);
   }

   void draw_indexed_instanced(const uint32 index_count_per_instance,
                               const uint32 instance_count,
                               const uint32 start_index_location,
                               const int32 base_vertex_location,
                               const uint32 start_instance_location)
   {
      _command_list->DrawIndexedInstanced(index_count_per_instance, instance_count,
                                          start_index_location, base_vertex_location,
                                          start_instance_location);
   }

   void dispatch(const uint32 thread_group_count_x,
                 const uint32 thread_group_count_y, const uint32 thread_group_count_z)
   {
      _command_list->Dispatch(thread_group_count_x, thread_group_count_y,
                              thread_group_count_z);
   }

   void execute_indirect(ID3D12CommandSignature& command_signature,
                         const uint32 max_command_count, ID3D12Resource& argument_buffer,
                         const uint64 argument_buffer_offset,
                         ID3D12Resource* const count_buffer = nullptr,
                         const uint64 count_buffer_offset = 0)
   {
      _command_list->ExecuteIndirect(&command_signature, max_command_count,
                                     &argument_buffer, argument_buffer_offset,
                                     count_buffer, count_buffer_offset);
   }

   void copy_buffer_region(ID3D12Resource& dst_buffer, const uint64 dst_offset,
                           ID3D12Resource& src_buffer, const uint64 src_offset,
                           const uint64 num_bytes)
   {
      _command_list->CopyBufferRegion(&dst_buffer, dst_offset, &src_buffer,
                                      src_offset, num_bytes);
   }

   void copy_texture_region(const D3D12_TEXTURE_COPY_LOCATION& dst,
                            const uint32 dst_x, const uint32 dst_y, const uint32 dst_z,
                            const D3D12_TEXTURE_COPY_LOCATION& src,
                            const std::optional<D3D12_BOX> src_box = std::nullopt)
   {
      _command_list->CopyTextureRegion(&dst, dst_x, dst_y, dst_z, &src,
                                       src_box ? &src_box.value() : nullptr);
   }

   void copy_resource(ID3D12Resource& dst, ID3D12Resource& src)
   {
      _command_list->CopyResource(&dst, &src);
   }

   void copy_tiles(ID3D12Resource& tiled_resource,
                   const D3D12_TILED_RESOURCE_COORDINATE& tile_region_start_coordinate,
                   const D3D12_TILE_REGION_SIZE& tile_region_size,
                   ID3D12Resource& buffer, const uint64 buffer_start_offset_in_bytes,
                   const D3D12_TILE_COPY_FLAGS flags)
   {
      _command_list->CopyTiles(&tiled_resource, &tile_region_start_coordinate,
                               &tile_region_size, &buffer,
                               buffer_start_offset_in_bytes, flags);
   }

   void resolve_subresource(ID3D12Resource& dst_resource,
                            const uint32 dst_subresource, ID3D12Resource& src_resource,
                            const uint32 src_subresource, const DXGI_FORMAT format)
   {
      _command_list->ResolveSubresource(&dst_resource, dst_subresource,
                                        &src_resource, src_subresource, format);
   }

   void resource_barrier(const std::span<const D3D12_RESOURCE_BARRIER> barriers)
   {
      _command_list->ResourceBarrier(static_cast<UINT>(barriers.size()),
                                     barriers.data());
   }

   void resource_barrier(const D3D12_RESOURCE_BARRIER& barrier)
   {
      _command_list->ResourceBarrier(1, &barrier);
   }

   void execute_bundle(ID3D12GraphicsCommandList& bundle)
   {
      _command_list->ExecuteBundle(&bundle);
   }

   void set_descriptor_heaps(ID3D12DescriptorHeap& cbv_uav_srv_heap,
                             ID3D12DescriptorHeap* sampler_heap = nullptr)
   {
      const std::array heaps{&cbv_uav_srv_heap, sampler_heap};

      _command_list->SetDescriptorHeaps(sampler_heap ? 2 : 1, heaps.data());
   }

   void set_pipeline_state(ID3D12PipelineState& pipeline_state)
   {
      _command_list->SetPipelineState(&pipeline_state);
   }

   void set_compute_root_signature(ID3D12RootSignature& root_signature)
   {
      _command_list->SetComputeRootSignature(&root_signature);
   }

   void set_graphics_root_signature(ID3D12RootSignature& root_signature)
   {
      _command_list->SetGraphicsRootSignature(&root_signature);
   }

   void set_compute_root_descriptor_table(const uint32 root_parameter_index,
                                          const D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor)
   {
      _command_list->SetComputeRootDescriptorTable(root_parameter_index, base_descriptor);
   }

   void set_graphics_root_descriptor_table(const uint32 root_parameter_index,
                                           const D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor)
   {
      _command_list->SetGraphicsRootDescriptorTable(root_parameter_index,
                                                    base_descriptor);
   }

   void set_compute_root_32bit_constant(const uint32 root_parameter_index,
                                        const uint32 constant,
                                        const uint32 dest_offset_in_32bit_values)
   {
      _command_list->SetComputeRoot32BitConstant(root_parameter_index, constant,
                                                 dest_offset_in_32bit_values);
   }

   void set_graphics_root_32bit_constant(const uint32 root_parameter_index,
                                         const uint32 constant,
                                         const uint32 dest_offset_in_32bit_values)
   {
      _command_list->SetGraphicsRoot32BitConstant(root_parameter_index, constant,
                                                  dest_offset_in_32bit_values);
   }

   void set_compute_root_32bit_constants(const uint32 root_parameter_index,
                                         const std::span<const std::byte> constants,
                                         const uint32 dest_offset_in_32bit_values)
   {
      _command_list->SetComputeRoot32BitConstants(root_parameter_index,
                                                  static_cast<UINT>(constants.size() /
                                                                    sizeof(uint32)),
                                                  constants.data(),
                                                  dest_offset_in_32bit_values);
   }

   void set_graphics_root_32bit_constants(const uint32 root_parameter_index,
                                          const std::span<const std::byte> constants,
                                          const uint32 dest_offset_in_32bit_values)
   {
      _command_list->SetGraphicsRoot32BitConstants(root_parameter_index,
                                                   static_cast<UINT>(
                                                      constants.size() / sizeof(uint32)),
                                                   constants.data(),
                                                   dest_offset_in_32bit_values);
   }

   void set_compute_root_constant_buffer_view(const uint32 root_parameter_index,
                                              const D3D12_GPU_VIRTUAL_ADDRESS buffer_location)
   {
      _command_list->SetComputeRootConstantBufferView(root_parameter_index,
                                                      buffer_location);
   }

   void set_graphics_root_constant_buffer_view(const uint32 root_parameter_index,
                                               const D3D12_GPU_VIRTUAL_ADDRESS buffer_location)
   {
      _command_list->SetGraphicsRootConstantBufferView(root_parameter_index,
                                                       buffer_location);
   }

   void set_compute_root_shader_resource_view(const uint32 root_parameter_index,
                                              const D3D12_GPU_VIRTUAL_ADDRESS buffer_location)
   {
      _command_list->SetComputeRootShaderResourceView(root_parameter_index,
                                                      buffer_location);
   }

   void set_graphics_root_shader_resource_view(const uint32 root_parameter_index,
                                               const D3D12_GPU_VIRTUAL_ADDRESS buffer_location)
   {
      _command_list->SetGraphicsRootShaderResourceView(root_parameter_index,
                                                       buffer_location);
   }

   void set_compute_root_unordered_access_view(const uint32 root_parameter_index,
                                               const D3D12_GPU_VIRTUAL_ADDRESS buffer_location)
   {
      _command_list->SetComputeRootUnorderedAccessView(root_parameter_index,
                                                       buffer_location);
   }

   void set_graphics_root_unordered_access_view(const uint32 root_parameter_index,
                                                const D3D12_GPU_VIRTUAL_ADDRESS buffer_location)
   {
      _command_list->SetGraphicsRootUnorderedAccessView(root_parameter_index,
                                                        buffer_location);
   }

   template<typename Type>
   void set_compute_root_constant_buffer(const uint32 root_parameter_index,
                                         const Type& constant_buffer)
   {
      set_root_constant_buffer<Type, true>(root_parameter_index, constant_buffer);
   }

   template<typename Type>
   void set_graphics_root_constant_buffer(const uint32 root_parameter_index,
                                          const Type& constant_buffer)
   {
      set_root_constant_buffer<Type, false>(root_parameter_index, constant_buffer);
   }

   void ia_set_primitive_topology(const D3D12_PRIMITIVE_TOPOLOGY primitive_topology)
   {
      _command_list->IASetPrimitiveTopology(primitive_topology);
   }

   void ia_set_index_buffer(const D3D12_INDEX_BUFFER_VIEW& view)
   {
      _command_list->IASetIndexBuffer(&view);
   }

   void ia_set_vertex_buffers(const uint32 start_slot,
                              const std::span<const D3D12_VERTEX_BUFFER_VIEW> views)
   {
      _command_list->IASetVertexBuffers(start_slot, static_cast<UINT>(views.size()),
                                        views.data());
   }

   void ia_set_vertex_buffers(const uint32 start_slot,
                              const D3D12_VERTEX_BUFFER_VIEW& view)
   {
      _command_list->IASetVertexBuffers(start_slot, 1, &view);
   }

   void so_set_targets(const uint32 start_slot,
                       const std::span<const D3D12_STREAM_OUTPUT_BUFFER_VIEW> views)
   {
      _command_list->SOSetTargets(start_slot, static_cast<UINT>(views.size()),
                                  views.data());
   }

   void rs_set_viewports(const std::span<const D3D12_VIEWPORT> viewports)
   {
      _command_list->RSSetViewports(static_cast<UINT>(viewports.size()),
                                    viewports.data());
   }

   void rs_set_viewports(const D3D12_VIEWPORT& viewport)
   {
      _command_list->RSSetViewports(1, &viewport);
   }

   void rs_set_scissor_rects(const std::span<const D3D12_RECT> scissor_rects)
   {
      _command_list->RSSetScissorRects(static_cast<UINT>(scissor_rects.size()),
                                       scissor_rects.data());
   }

   void rs_set_scissor_rects(const D3D12_RECT& scissor_rect)
   {
      _command_list->RSSetScissorRects(1, &scissor_rect);
   }

   void om_set_blend_factor(const float4& blend_factor)
   {
      _command_list->OMSetBlendFactor(&blend_factor[0]);
   }

   void om_set_stencil_ref(const uint32 stencil_ref)
   {
      _command_list->OMSetStencilRef(stencil_ref);
   }

   void om_set_render_targets(
      const std::span<const D3D12_CPU_DESCRIPTOR_HANDLE> render_target_descriptors,
      const bool single_handle_to_descriptor_range,
      const std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> depth_stencil_descriptor = std::nullopt)
   {
      _command_list->OMSetRenderTargets(static_cast<UINT>(
                                           render_target_descriptors.size()),
                                        render_target_descriptors.data(),
                                        single_handle_to_descriptor_range,
                                        depth_stencil_descriptor
                                           ? &depth_stencil_descriptor.value()
                                           : nullptr);
   }

   void om_set_render_targets(
      const D3D12_CPU_DESCRIPTOR_HANDLE render_target_descriptor,
      const std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> depth_stencil_descriptor = std::nullopt)
   {
      _command_list->OMSetRenderTargets(1, &render_target_descriptor, true,
                                        depth_stencil_descriptor
                                           ? &depth_stencil_descriptor.value()
                                           : nullptr);
   }

   void clear_depth_stencil_view(const D3D12_CPU_DESCRIPTOR_HANDLE depth_stencil_view,
                                 const D3D12_CLEAR_FLAGS flags,
                                 const float depth, const uint8 stencil,
                                 const std::span<const D3D12_RECT> rects = {})
   {
      _command_list->ClearDepthStencilView(depth_stencil_view, flags, depth, stencil,
                                           static_cast<UINT>(rects.size()),
                                           rects.data());
   }

   void clear_render_target_view(const D3D12_CPU_DESCRIPTOR_HANDLE render_target_view,
                                 const float4& color,
                                 const std::span<const D3D12_RECT> rects = {})
   {
      _command_list->ClearRenderTargetView(render_target_view, &color[0],
                                           static_cast<UINT>(rects.size()),
                                           rects.data());
   }

   void clear_unordered_access_view_uint(
      const D3D12_GPU_DESCRIPTOR_HANDLE view_gpu_handle_in_current_heap,
      const D3D12_CPU_DESCRIPTOR_HANDLE view_cpu_handle,
      ID3D12Resource& resource, const std::span<const uint32, 4> values,
      const std::span<const D3D12_RECT> rects = {})
   {
      _command_list->ClearUnorderedAccessViewUint(view_gpu_handle_in_current_heap,
                                                  view_cpu_handle, &resource,
                                                  values.data(),
                                                  static_cast<UINT>(rects.size()),
                                                  rects.data());
   }

   void clear_unordered_access_view_float(
      const D3D12_GPU_DESCRIPTOR_HANDLE view_gpu_handle_in_current_heap,
      const D3D12_CPU_DESCRIPTOR_HANDLE view_cpu_handle, ID3D12Resource& resource,
      const float4& values, const std::span<const D3D12_RECT> rects = {})
   {
      _command_list->ClearUnorderedAccessViewFloat(view_gpu_handle_in_current_heap,
                                                   view_cpu_handle, &resource,
                                                   &values[0],
                                                   static_cast<UINT>(rects.size()),
                                                   rects.data());
   }

   void discard_resource(ID3D12Resource& resource,
                         const std::optional<D3D12_DISCARD_REGION> discard_region = std::nullopt)
   {
      _command_list->DiscardResource(&resource, discard_region
                                                   ? &discard_region.value()
                                                   : nullptr);
   }

   auto get_underlying() const noexcept -> ID3D12GraphicsCommandList5*
   {
      return _command_list.get();
   }

private:
   template<typename Type, bool compute>
   void set_root_constant_buffer(const uint32 root_parameter_index,
                                 const Type& constant_buffer)
   {
      static_assert(std::is_trivially_copyable_v<Type>,
                    "Constant buffer type should be trivially copyable.");

      auto allocation = _dynamic_buffer_allocator->allocate(sizeof(Type));

      std::memcpy(allocation.cpu_address, &constant_buffer, sizeof(Type));

      if constexpr (compute) {
         _command_list->SetComputeRootConstantBufferView(root_parameter_index,
                                                         allocation.gpu_address);
      }
      else {
         _command_list->SetGraphicsRootConstantBufferView(root_parameter_index,
                                                          allocation.gpu_address);
      }
   }

   utility::com_ptr<ID3D12GraphicsCommandList5> _command_list;
   dynamic_buffer_allocator* _dynamic_buffer_allocator = nullptr;
};

}