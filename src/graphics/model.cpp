
#include "model.hpp"
#include "hresult_error.hpp"
#include "math/align.hpp"

#include <memory>

#include <range/v3/action.hpp>
#include <range/v3/numeric.hpp>
#include <range/v3/view.hpp>

#include <d3dx12.h>

using ranges::views::iota;
using ranges::views::transform;

namespace we::graphics {

model::model(const assets::msh::flat_model& model, gpu::device& device,
             texture_manager& texture_manager)
{
   if (model.meshes.size() == 0) {
      throw std::runtime_error{
         "Can't create empty model! This may be a terrain cutter only model in "
         "which case this can be safely ignored."};
   }

   parts.reserve(model.meshes.size());

   const auto vertex_count =
      ranges::accumulate(transform(model.meshes,
                                   [](const assets::msh::mesh& mesh) {
                                      return mesh.positions.size();
                                   }),
                         std::size_t{0});
   const auto triangle_count =
      ranges::accumulate(transform(model.meshes,
                                   [](const assets::msh::mesh& mesh) {
                                      return mesh.triangles.size();
                                   }),
                         std::size_t{0});

   const auto indices_data_size = triangle_count * sizeof(std::array<uint16, 3>);
   const auto positions_data_size =
      vertex_count * sizeof(decltype(mesh_vertices::positions)::value_type);
   const auto attributes_data_size =
      vertex_count * sizeof(decltype(mesh_vertices::attributes)::value_type);

   const auto aligned_indices_size =
      math::align_up(indices_data_size, D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT);
   const auto aligned_positions_size =
      math::align_up(positions_data_size, D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT);
   const auto aligned_attributes_size =
      math::align_up(attributes_data_size, D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT);

   const auto indices_data_offset = 0;
   const auto positions_data_offset = aligned_indices_size;
   const auto attributes_data_offset = positions_data_offset + aligned_positions_size;

   const auto buffer_size = static_cast<uint32>(
      aligned_indices_size + aligned_positions_size + aligned_attributes_size);

   std::vector<std::byte> buffer{buffer_size};

   // Caution, UB below, but hey what's the worst that could happen? (Lots probably, but that's not the point.)
   const std::span<std::array<uint16, 3>> indices =
      {reinterpret_cast<std::array<uint16, 3>*>(&buffer[indices_data_offset]),
       triangle_count};

   const mesh_vertices vertices{.positions = {reinterpret_cast<float3*>(
                                                 &buffer[positions_data_offset]),
                                              vertex_count},
                                .attributes = {reinterpret_cast<mesh_attributes*>(
                                                  &buffer[attributes_data_offset]),
                                               vertex_count}};

   uint32 triangle_offset = 0;
   uint32 vertex_offset = 0;

   for (const auto& mesh : model.meshes) {
      parts.push_back({.index_count = static_cast<uint32>(mesh.triangles.size() * 3),
                       .start_index = triangle_offset * 3,
                       .start_vertex = vertex_offset,
                       .vertex_count = static_cast<uint32>(mesh.positions.size()),
                       .material = {mesh.material, device, texture_manager}});

      std::uninitialized_copy_n(mesh.triangles.cbegin(), mesh.triangles.size(),
                                indices.begin() + triangle_offset);
      std::uninitialized_copy_n(mesh.positions.cbegin(), mesh.positions.size(),
                                vertices.positions.begin() + vertex_offset);

      auto attributes =
         iota(std::size_t{0}, mesh.positions.size()) | transform([&](std::size_t i) {
            return mesh_attributes{.normals = mesh.normals[i],
                                   .tangents = mesh.tangents[i],
                                   .bitangents = mesh.bitangents[i],
                                   .texcoords = mesh.texcoords[i]};
         });

      std::uninitialized_copy_n(attributes.begin(), mesh.positions.size(),
                                vertices.attributes.begin() + vertex_offset);

      triangle_offset += static_cast<uint32>(mesh.triangles.size());
      vertex_offset += static_cast<uint32>(mesh.positions.size());
   }

   data_offsets = {.indices = indices_data_offset,
                   .positions = positions_data_offset,
                   .attributes = attributes_data_offset};

   bbox = model.bounding_box;

   init_gpu_buffer_async(device, buffer, indices, vertices);
}

void model::init_gpu_buffer_async(gpu::device& device, std::span<const std::byte> buffer,
                                  std::span<std::array<uint16, 3>> indices,
                                  mesh_vertices vertices)
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

   const auto gpu_virtual_address = gpu_buffer.buffer.gpu_virtual_address();

   gpu_buffer.index_buffer_view = {.BufferLocation =
                                      gpu_virtual_address + data_offsets.indices,
                                   .SizeInBytes =
                                      static_cast<uint32>(indices.size_bytes()),
                                   .Format = DXGI_FORMAT_R16_UINT};
   gpu_buffer.position_vertex_buffer_view =
      {.BufferLocation = gpu_virtual_address + data_offsets.positions,
       .SizeInBytes = static_cast<uint32>(vertices.positions.size_bytes()),
       .StrideInBytes = sizeof(decltype(mesh_vertices::positions)::value_type)};
   gpu_buffer.attributes_vertex_buffer_view =
      {.BufferLocation = gpu_virtual_address + data_offsets.attributes,
       .SizeInBytes = static_cast<uint32>(vertices.attributes.size_bytes()),
       .StrideInBytes = sizeof(decltype(mesh_vertices::attributes)::value_type)};

   copy_context.command_list().copy_resource(*gpu_buffer.buffer.view_resource(),
                                             upload_buffer);

   device.copy_manager.close_and_execute(copy_context);
}

}
