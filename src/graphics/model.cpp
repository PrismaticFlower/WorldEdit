
#include "model.hpp"
#include "hresult_error.hpp"
#include "math/align.hpp"

#include <memory>

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
   const auto triangle_count =
      ranges::accumulate(ranges::views::transform(model.meshes,
                                                  [](const assets::msh::mesh& mesh) {
                                                     return mesh.triangles.size();
                                                  }),
                         std::size_t{0});

   const auto indices_data_size =
      triangle_count * sizeof(decltype(indices)::value_type);
   const auto positions_data_size =
      vertex_count * sizeof(decltype(vertices.positions)::value_type);
   const auto normals_data_size =
      vertex_count * sizeof(decltype(vertices.normals)::value_type);
   const auto texcoords_data_size =
      vertex_count * sizeof(decltype(vertices.texcoords)::value_type);

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

   buffer.resize(buffer_size);

   // Caution, UB below, but hey what's the worst that could happen? (Lots probably, but that's not the point.)
   indices = {reinterpret_cast<std::array<uint16, 3>*>(&buffer[indices_data_offset]),
              triangle_count};
   vertices.positions = {reinterpret_cast<float3*>(&buffer[positions_data_offset]),
                         vertex_count};
   vertices.normals = {reinterpret_cast<float3*>(&buffer[normals_data_offset]),
                       vertex_count};
   vertices.texcoords = {reinterpret_cast<float2*>(&buffer[texcoords_data_offset]),
                         vertex_count};

   uint32 triangle_offset = 0;
   uint32 vertex_offset = 0;

   for (const auto& mesh : model.meshes) {
      parts.push_back({.index_count = static_cast<uint32>(mesh.triangles.size() * 3),
                       .start_index = triangle_offset * 3,
                       .start_vertex = vertex_offset});

      std::uninitialized_copy_n(mesh.triangles.cbegin(), mesh.triangles.size(),
                                indices.begin() + triangle_offset);
      std::uninitialized_copy_n(mesh.positions.cbegin(), mesh.positions.size(),
                                vertices.positions.begin() + vertex_offset);
      std::uninitialized_copy_n(mesh.normals.cbegin(), mesh.normals.size(),
                                vertices.normals.begin() + vertex_offset);
      std::uninitialized_copy_n(mesh.texcoords.cbegin(), mesh.texcoords.size(),
                                vertices.texcoords.begin() + vertex_offset);

      triangle_offset += static_cast<uint32>(mesh.triangles.size());
      vertex_offset += static_cast<uint32>(mesh.positions.size());
   }

   data_offsets = {.indices = indices_data_offset,
                   .positions = positions_data_offset,
                   .normals = normals_data_offset,
                   .texcoords = texcoords_data_offset};

   bbox = model.bounding_box;
}

auto model::init_gpu_buffer_async(gpu::device& device) -> UINT64
{
   auto copy_context = device.copy_manager.aquire_context();

   ID3D12Resource& upload_buffer = copy_context.create_upload_resource(
      CD3DX12_RESOURCE_DESC::Buffer(buffer.size()));

   std::byte* const upload_buffer_ptr = [&] {
      const D3D12_RANGE read_range{};
      void* map_void_ptr = nullptr;

      throw_if_failed(upload_buffer.Map(0, &read_range, &map_void_ptr));

      return static_cast<std::byte*>(map_void_ptr);
   }();

   std::memcpy(upload_buffer_ptr, buffer.data(), buffer.size());

   const D3D12_RANGE write_range{0, buffer.size()};
   upload_buffer.Unmap(0, &write_range);

   gpu_buffer = {
      .buffer = device.create_buffer({.size = static_cast<uint32>(buffer.size())},
                                     D3D12_HEAP_TYPE_DEFAULT,
                                     D3D12_RESOURCE_STATE_COMMON)};
   gpu_buffer.index_buffer_view = {.BufferLocation =
                                      gpu_buffer.buffer.resource()->GetGPUVirtualAddress() +
                                      data_offsets.indices,
                                   .SizeInBytes =
                                      static_cast<uint32>(indices.size_bytes()),
                                   .Format = DXGI_FORMAT_R16_UINT};
   gpu_buffer.position_vertex_buffer_view =
      {.BufferLocation = gpu_buffer.buffer.resource()->GetGPUVirtualAddress() +
                         data_offsets.positions,
       .SizeInBytes = static_cast<uint32>(vertices.positions.size_bytes()),
       .StrideInBytes = sizeof(decltype(vertices.positions)::value_type)};
   gpu_buffer.normal_vertex_buffer_view =
      {.BufferLocation = gpu_buffer.buffer.resource()->GetGPUVirtualAddress() +
                         data_offsets.normals,
       .SizeInBytes = static_cast<uint32>(vertices.normals.size_bytes()),
       .StrideInBytes = sizeof(decltype(vertices.normals)::value_type)};
   gpu_buffer.texcoord_vertex_buffer_view =
      {.BufferLocation = gpu_buffer.buffer.resource()->GetGPUVirtualAddress() +
                         data_offsets.texcoords,
       .SizeInBytes = static_cast<uint32>(vertices.texcoords.size_bytes()),
       .StrideInBytes = sizeof(decltype(vertices.texcoords)::value_type)};

   copy_context.command_list.CopyResource(gpu_buffer.buffer.resource(), &upload_buffer);

   return device.copy_manager.close_and_execute(copy_context);
}

}
