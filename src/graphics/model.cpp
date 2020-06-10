
#include "model.hpp"
#include "hresult_error.hpp"
#include "math/align.hpp"

#include <array>

#include <range/v3/action.hpp>
#include <range/v3/numeric.hpp>
#include <range/v3/view.hpp>

#include <d3dx12.h>

namespace sk::graphics {

model::model(const assets::msh::flat_model& model)
{
   parts.reserve(model.meshes.size());

   const auto vertex_count =
      ranges::accumulate(ranges::views::transform(model.meshes,
                                                  [](const assets::msh::mesh& mesh) {
                                                     return mesh.positions.size();
                                                  }),
                         std::size_t{0});

   vertices.positions.reserve(vertex_count);
   vertices.normals.reserve(vertex_count);
   vertices.texcoords.reserve(vertex_count);

   indices.reserve(
      ranges::accumulate(ranges::views::transform(model.meshes,
                                                  [](const assets::msh::mesh& mesh) {
                                                     return mesh.triangles.size();
                                                  }),
                         std::size_t{0}));

   for (const auto& mesh : model.meshes) {
      parts.push_back(
         {.index_count = static_cast<uint32>(mesh.triangles.size() * 3),
          .start_index = static_cast<uint32>(indices.size() * 3),
          .start_vertex = static_cast<uint32>(vertices.positions.size())});

      for (const auto& [position, normal, texcoord] :
           ranges::views::zip(mesh.positions, mesh.normals, mesh.texcoords)) {
         vertices.positions.push_back(position);
         vertices.normals.push_back(normal);
         vertices.texcoords.push_back(texcoord);
      }

      ranges::push_back(indices, mesh.triangles);
   }

   bbox = model.bounding_box;
}

auto model::init_gpu_buffer_async(gpu::device& device) -> UINT64
{
   const auto indices_data_size =
      indices.size() * sizeof(decltype(indices)::value_type);
   const auto positions_data_size =
      vertices.positions.size() * sizeof(decltype(vertices.positions)::value_type);
   const auto normals_data_size =
      vertices.normals.size() * sizeof(decltype(vertices.normals)::value_type);
   const auto texcoords_data_size =
      vertices.texcoords.size() * sizeof(decltype(vertices.texcoords)::value_type);

   const auto aligned_indices_size =
      math::align_up(indices_data_size, D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT);
   const auto aligned_positions_size =
      math::align_up(positions_data_size, D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT);
   const auto aligned_normals_size =
      math::align_up(normals_data_size, D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT);
   const auto aligned_texcoords_size =
      math::align_up(texcoords_data_size, D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT);

   const auto indices_data_offset = 0;
   const auto positions_data_offset = aligned_indices_size;
   const auto normals_data_offset = positions_data_offset + aligned_positions_size;
   const auto texcoords_data_offset = normals_data_offset + aligned_normals_size;

   const auto buffer_size =
      static_cast<uint32>(aligned_indices_size + aligned_positions_size +
                          aligned_normals_size + aligned_texcoords_size);

   auto copy_context = device.copy_manager.aquire_context();

   ID3D12Resource& upload_buffer =
      copy_context.create_upload_resource(CD3DX12_RESOURCE_DESC::Buffer(buffer_size));

   std::byte* const upload_buffer_ptr = [&] {
      const D3D12_RANGE read_range{};
      void* map_void_ptr = nullptr;

      throw_if_failed(upload_buffer.Map(0, &read_range, &map_void_ptr));

      return static_cast<std::byte*>(map_void_ptr);
   }();

   std::memcpy(upload_buffer_ptr + indices_data_offset, indices.data(),
               indices_data_size);
   std::memcpy(upload_buffer_ptr + positions_data_offset,
               vertices.positions.data(), positions_data_size);
   std::memcpy(upload_buffer_ptr + normals_data_offset, vertices.normals.data(),
               normals_data_size);
   std::memcpy(upload_buffer_ptr + texcoords_data_offset,
               vertices.texcoords.data(), texcoords_data_size);

   const D3D12_RANGE write_range{0, buffer_size};
   upload_buffer.Unmap(0, &write_range);

   gpu_buffer = {.buffer = device.create_buffer({.size = buffer_size}, D3D12_HEAP_TYPE_DEFAULT,
                                                D3D12_RESOURCE_STATE_COMMON)};
   gpu_buffer.index_buffer_view = {.BufferLocation =
                                      gpu_buffer.buffer.resource()->GetGPUVirtualAddress() +
                                      indices_data_offset,
                                   .SizeInBytes = static_cast<uint32>(indices_data_size),
                                   .Format = DXGI_FORMAT_R16_UINT};
   gpu_buffer.position_vertex_buffer_view =
      {.BufferLocation = gpu_buffer.buffer.resource()->GetGPUVirtualAddress() +
                         positions_data_offset,
       .SizeInBytes = static_cast<uint32>(positions_data_size),
       .StrideInBytes = sizeof(decltype(vertices.positions)::value_type)};
   gpu_buffer.normal_vertex_buffer_view =
      {.BufferLocation = gpu_buffer.buffer.resource()->GetGPUVirtualAddress() +
                         normals_data_offset,
       .SizeInBytes = static_cast<uint32>(normals_data_size),
       .StrideInBytes = sizeof(decltype(vertices.normals)::value_type)};
   gpu_buffer.texcoord_vertex_buffer_view =
      {.BufferLocation = gpu_buffer.buffer.resource()->GetGPUVirtualAddress() +
                         texcoords_data_offset,
       .SizeInBytes = static_cast<uint32>(texcoords_data_size),
       .StrideInBytes = sizeof(decltype(vertices.texcoords)::value_type)};

   copy_context.command_list.CopyResource(gpu_buffer.buffer.resource(), &upload_buffer);

   return device.copy_manager.close_and_execute(copy_context);
}

}
