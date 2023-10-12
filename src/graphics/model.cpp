
#include "model.hpp"
#include "math/align.hpp"
#include "math/vector_funcs.hpp"

#include <memory>

namespace we::graphics {

namespace {

auto pack_snorm(float3 value) -> std::array<int16, 4>
{
   value = clamp(value, -1.0f, 1.0f);
   value = value * 32767.0f;

   if (value.x >= 0.0f) {
      value.x += 0.5f;
   }
   else {
      value.x -= 0.5f;
   }

   if (value.y >= 0.0f) {
      value.y += 0.5f;
   }
   else {
      value.y -= 0.5f;
   }

   if (value.z >= 0.0f) {
      value.z += 0.5f;
   }
   else {
      value.z -= 0.5f;
   }

   return {static_cast<int16>(value.x), static_cast<int16>(value.y),
           static_cast<int16>(value.z), 0};
}

auto count_vertices(const assets::msh::flat_model& model) -> uint64
{
   uint64 count = 0;

   for (auto& mesh : model.meshes) {
      count += mesh.positions.size();
   }

   return count;
}

auto count_terrain_cut_vertices(const assets::msh::flat_model& model) -> uint64
{
   uint64 count = 0;

   for (auto& cut : model.terrain_cuts) {
      count += cut.positions.size();
   }

   return count;
}

auto count_triangles(const assets::msh::flat_model& model) -> uint64
{
   uint64 count = 0;

   for (auto& mesh : model.meshes) {
      count += mesh.triangles.size();
   }

   for (auto& cut : model.terrain_cuts) {
      count += cut.triangles.size();
   }

   return count;
}

}

model::model(const assets::msh::flat_model& model, gpu::device& device,
             copy_command_list_pool& copy_command_list_pool,
             texture_manager& texture_manager)
{
   if (model.meshes.size() == 0 and model.terrain_cuts.size() == 0) {
      throw std::runtime_error{
         "Can't create empty model! This may be a collision only model in "
         "which case this can be safely ignored."};
   }

   parts.reserve(model.meshes.size());

   const uint64 vertex_count = count_vertices(model);
   const uint64 terrain_cut_vertex_count = count_terrain_cut_vertices(model);
   const uint64 triangle_count = count_triangles(model);

   const uint64 indices_data_size = triangle_count * sizeof(std::array<uint16, 3>);
   const uint64 positions_data_size =
      (vertex_count + terrain_cut_vertex_count) *
      sizeof(decltype(mesh_vertices::positions)::value_type);
   const uint64 attributes_data_size =
      vertex_count * sizeof(decltype(mesh_vertices::attributes)::value_type);

   const uint64 aligned_indices_size =
      math::align_up(indices_data_size, gpu::raw_uav_srv_byte_alignment);
   const uint64 aligned_positions_size =
      math::align_up(positions_data_size, gpu::raw_uav_srv_byte_alignment);
   const uint64 aligned_attributes_size =
      math::align_up(attributes_data_size, gpu::raw_uav_srv_byte_alignment);

   const uint64 indices_data_offset = 0;
   const uint64 positions_data_offset = aligned_indices_size;
   const uint64 attributes_data_offset = positions_data_offset + aligned_positions_size;

   const uint64 buffer_size =
      aligned_indices_size + aligned_positions_size + aligned_attributes_size;

   if (buffer_size > std::numeric_limits<uint32>::max()) {
      throw std::runtime_error{"Model is over size limits."};
   }

   std::vector<std::byte> buffer{buffer_size};

   // Caution, UB below, but hey what's the worst that could happen? (Lots probably.)
   const std::span<std::array<uint16, 3>> indices =
      {reinterpret_cast<std::array<uint16, 3>*>(&buffer[indices_data_offset]),
       triangle_count};

   const mesh_vertices vertices{.positions = {reinterpret_cast<float3*>(
                                                 &buffer[positions_data_offset]),
                                              vertex_count + terrain_cut_vertex_count},
                                .attributes = {reinterpret_cast<mesh_attributes*>(
                                                  &buffer[attributes_data_offset]),
                                               vertex_count}};

   uint32 triangle_offset = 0;
   uint32 vertex_offset = 0;

   parts.reserve(model.meshes.size());

   for (const auto& mesh : model.meshes) {
      parts.push_back({.index_count = static_cast<uint32>(mesh.triangles.size() * 3),
                       .start_index = triangle_offset * 3,
                       .start_vertex = vertex_offset,
                       .material = {mesh.material, mesh.colors_are_lighting, device,
                                    copy_command_list_pool, texture_manager}});

      std::uninitialized_copy_n(mesh.triangles.cbegin(), mesh.triangles.size(),
                                indices.begin() + triangle_offset);
      std::uninitialized_copy_n(mesh.positions.cbegin(), mesh.positions.size(),
                                vertices.positions.begin() + vertex_offset);

      for (std::size_t i = 0; i < mesh.positions.size(); ++i) {
         vertices.attributes[i + vertex_offset] =
            mesh_attributes{.normals = pack_snorm(mesh.normals[i]),
                            .tangents = pack_snorm(mesh.tangents[i]),
                            .bitangents = pack_snorm(mesh.bitangents[i]),
                            .texcoords = mesh.texcoords[i],
                            .color = mesh.colors[i]};
      }

      triangle_offset += static_cast<uint32>(mesh.triangles.size());
      vertex_offset += static_cast<uint32>(mesh.positions.size());
   }

   // It's important to pack cuts after mesh parts as we didn't allocate attribute space for them.

   terrain_cuts.reserve(model.terrain_cuts.size());

   for (const auto& cut : model.terrain_cuts) {
      terrain_cuts.push_back(
         {.bbox = cut.bounding_box,
          .index_count = static_cast<uint32>(cut.triangles.size() * 3),
          .start_index = triangle_offset * 3,
          .start_vertex = vertex_offset});

      std::uninitialized_copy_n(cut.triangles.cbegin(), cut.triangles.size(),
                                indices.begin() + triangle_offset);
      std::uninitialized_copy_n(cut.positions.cbegin(), cut.positions.size(),
                                vertices.positions.begin() + vertex_offset);

      triangle_offset += static_cast<uint32>(cut.triangles.size());
      vertex_offset += static_cast<uint32>(cut.positions.size());
   }

   data_offsets = {.indices = indices_data_offset,
                   .positions = positions_data_offset,
                   .attributes = attributes_data_offset};

   bbox = model.bounding_box;
   terrain_cuts_bbox = model.terrain_cuts_bounding_box;

   init_gpu_buffer_async(device, copy_command_list_pool, buffer, indices, vertices);
}

void model::init_gpu_buffer_async(gpu::device& device,
                                  copy_command_list_pool& copy_command_list_pool,
                                  std::span<const std::byte> buffer,
                                  std::span<std::array<uint16, 3>> indices,
                                  mesh_vertices vertices)
{
   pooled_copy_command_list command_list = copy_command_list_pool.aquire_and_reset();

   gpu::unique_resource_handle upload_buffer =
      {device.create_buffer({.size = buffer.size(), .debug_name = "Mesh Upload Buffer"},
                            gpu::heap_type::upload),
       device.background_copy_queue};

   std::byte* const upload_buffer_ptr =
      static_cast<std::byte*>(device.map(upload_buffer.get(), 0, {}));

   std::memcpy(upload_buffer_ptr, buffer.data(), buffer.size());

   device.unmap(upload_buffer.get(), 0, {0, buffer.size()});

   gpu_buffer = {
      .buffer = {device.create_buffer({.size = static_cast<uint32>(buffer.size()),
                                       .debug_name =
                                          "Mesh Index & Vertex Buffer"},
                                      gpu::heap_type::default_),
                 device.direct_queue}};

   const auto gpu_virtual_address =
      device.get_gpu_virtual_address(gpu_buffer.buffer.get());

   gpu_buffer.index_buffer_view = {.buffer_location =
                                      gpu_virtual_address + data_offsets.indices,
                                   .size_in_bytes =
                                      static_cast<uint32>(indices.size_bytes())};
   gpu_buffer.position_vertex_buffer_view =
      {.buffer_location = gpu_virtual_address + data_offsets.positions,
       .size_in_bytes = static_cast<uint32>(vertices.positions.size_bytes()),
       .stride_in_bytes = sizeof(decltype(mesh_vertices::positions)::value_type)};
   gpu_buffer.attributes_vertex_buffer_view =
      {.buffer_location = gpu_virtual_address + data_offsets.attributes,
       .size_in_bytes = static_cast<uint32>(vertices.attributes.size_bytes()),
       .stride_in_bytes = sizeof(decltype(mesh_vertices::attributes)::value_type)};

   command_list->copy_resource(gpu_buffer.buffer.get(), upload_buffer.get());

   command_list->close();

   device.background_copy_queue.execute_command_lists(command_list.get());
}

}
