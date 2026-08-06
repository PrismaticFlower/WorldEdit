#include "billboard_patches.hpp"
#include "cull_objects.hpp"

#include "gpu/resource.hpp"

#include "allocators/aligned_allocator.hpp"
#include "allocators/temp_buffer_allocator.hpp"

#include "world/object_classes/billboard_patch_class.hpp"

#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"

#include <algorithm>
#include <vector>

namespace we::graphics {

namespace {

auto select_pipeline(const billboard_patches_draw draw, pipeline_library& pipelines)
   -> gpu::pipeline_handle
{
   switch (draw) {
   case billboard_patches_draw::depth_prepass:
      return pipelines.billboard_patch_depth_prepass.get();
   case billboard_patches_draw::main:
      return pipelines.billboard_patch_normal.get();
   case billboard_patches_draw::shadow:
      return pipelines.billboard_patch_shadow.get();
   }

   std::unreachable();
}

}

struct billboard_patches::impl {
   impl(gpu::device& device, texture_manager& texture_manager)
      : _device{device}, _texture_manager{texture_manager}
   {
   }

   void update(const float4x4& world_matrix, const world::world& world, const bool animated)
   {
      _world_matrix = world_matrix;
      _animated = animated;

      if (world.global_lights.global_light_1.has_index()) {
         _light_direction =
            normalize(world.lights[world.global_lights.global_light_1.index()].rotation *
                      float3{0.0f, 0.0f, -1.0f});
      }
      else {
         _light_direction = {};
      }

      std::erase_if(_billboard_patches,
                    [](const billboard_patch_class_gpu& billboard_patch) {
                       return not billboard_patch.used_this_frame;
                    });

      for (billboard_patch_class_gpu& billboard_patch : _billboard_patches) {
         billboard_patch.used_this_frame = false;
         billboard_patch.instances.world_from_object.clear();
         billboard_patch.instances.bbox_minWS_x.clear();
         billboard_patch.instances.bbox_minWS_y.clear();
         billboard_patch.instances.bbox_minWS_z.clear();
         billboard_patch.instances.bbox_maxWS_x.clear();
         billboard_patch.instances.bbox_maxWS_y.clear();
         billboard_patch.instances.bbox_maxWS_z.clear();
      }

      _temp_buffer_allocator.reset();
   }

   void update_index_buffer(gpu::copy_command_list& command_list,
                            dynamic_buffer_allocator& allocator)
   {
      uint32 max_particles = 0;

      for (const billboard_patch_class_gpu& billboard_patch : _billboard_patches) {
         max_particles = std::max(billboard_patch.particle_count, max_particles);
      }

      if (max_particles > _index_buffer_particle_capacity) {
         const uint32 buffer_size = max_particles * 6 * sizeof(uint16);

         _index_buffer = {_device.create_buffer(
                             {
                                .size = buffer_size,
                                .debug_name = "Leaf Patch Index Buffer",
                             },
                             gpu::heap_type::default_),
                          _device};

         dynamic_buffer_allocator::allocation allocation =
            allocator.allocate(buffer_size);

         for (uint32 i = 0; i < max_particles; ++i) {
            std::array<uint16, 6> particles = {static_cast<uint16>(0 + i * 4),
                                               static_cast<uint16>(1 + i * 4),
                                               static_cast<uint16>(2 + i * 4),
                                               static_cast<uint16>(0 + i * 4),
                                               static_cast<uint16>(2 + i * 4),
                                               static_cast<uint16>(3 + i * 4)};

            std::memcpy(allocation.cpu_address + i * sizeof(particles),
                        &particles, sizeof(particles));
         }

         _index_buffer_particle_capacity = max_particles;

         command_list.copy_buffer_region(_index_buffer.get(), 0, allocation.resource,
                                         allocation.offset, buffer_size);
      }
   }

   void update_view_same_frame(const float4x4& world_matrix,
                               dynamic_buffer_allocator& allocator)
   {
      _world_matrix = world_matrix;

      for (billboard_patch_class_gpu& gpu_billboard_patch : _billboard_patches) {
         const world::billboard_patch_class& billboard_patch_class =
            *reinterpret_cast<world::billboard_patch_class*>(gpu_billboard_patch.cpu_key);

         update_vertices(billboard_patch_class, gpu_billboard_patch, allocator);
      }
   }

   void add_billboard_patch(const world::billboard_patch_class& billboard_patch_class,
                            const float4x4& world_from_object,
                            dynamic_buffer_allocator& allocator)
   {
      const std::uintptr_t cpu_key =
         reinterpret_cast<std::uintptr_t>(&billboard_patch_class);

      if (auto gpu_billboard_patch =
             std::find_if(_billboard_patches.begin(), _billboard_patches.end(),
                          [&](const billboard_patch_class_gpu& billboard_patch) {
                             return billboard_patch.cpu_key == cpu_key;
                          });
          gpu_billboard_patch != _billboard_patches.end()) {
         if (not gpu_billboard_patch->used_this_frame) {
            update_vertices(billboard_patch_class, *gpu_billboard_patch, allocator);

            gpu_billboard_patch->used_this_frame = true;
         }

         const math::bounding_box bboxWS =
            world_from_object * billboard_patch_class.bbox();
         const float y_scale = billboard_patch_class.height_scale();

         gpu_billboard_patch->instances.world_from_object.push_back({{
            {world_from_object[0].x, world_from_object[0].y * y_scale,
             world_from_object[0].z},
            {world_from_object[1].x, world_from_object[1].y * y_scale,
             world_from_object[1].z},
            {world_from_object[2].x, world_from_object[2].y * y_scale,
             world_from_object[2].z},
            {world_from_object[3].x, world_from_object[3].y,
             world_from_object[3].z},
         }});
         gpu_billboard_patch->instances.bbox_minWS_x.push_back(bboxWS.min.x);
         gpu_billboard_patch->instances.bbox_minWS_y.push_back(bboxWS.min.y);
         gpu_billboard_patch->instances.bbox_minWS_z.push_back(bboxWS.min.z);
         gpu_billboard_patch->instances.bbox_maxWS_x.push_back(bboxWS.max.x);
         gpu_billboard_patch->instances.bbox_maxWS_y.push_back(bboxWS.max.y);
         gpu_billboard_patch->instances.bbox_maxWS_z.push_back(bboxWS.max.z);

         return;
      }

      billboard_patch_class_gpu& gpu_billboard_patch =
         _billboard_patches.emplace_back();

      gpu_billboard_patch.cpu_key = cpu_key;

      gpu_billboard_patch.texture_name =
         lowercase_string{billboard_patch_class.texture()};
      gpu_billboard_patch.texture =
         _texture_manager.at_or(gpu_billboard_patch.texture_name,
                                world_texture_dimension::_2d,
                                _texture_manager.null_diffuse_map());

      if (not gpu_billboard_patch.texture_name.empty() and
          gpu_billboard_patch.texture == _texture_manager.null_diffuse_map()) {
         gpu_billboard_patch.texture_load_token =
            _texture_manager.acquire_load_token(gpu_billboard_patch.texture_name);
      }

      update_vertices(billboard_patch_class, gpu_billboard_patch, allocator);

      const math::bounding_box bboxWS =
         world_from_object * billboard_patch_class.bbox();

      gpu_billboard_patch.instances.world_from_object.push_back({{
         {world_from_object[0].x, world_from_object[0].y, world_from_object[0].z},
         {world_from_object[1].x, world_from_object[1].y, world_from_object[1].z},
         {world_from_object[2].x, world_from_object[2].y, world_from_object[2].z},
         {world_from_object[3].x, world_from_object[3].y, world_from_object[3].z},
      }});
      gpu_billboard_patch.instances.bbox_minWS_x.push_back(bboxWS.min.x);
      gpu_billboard_patch.instances.bbox_minWS_y.push_back(bboxWS.min.y);
      gpu_billboard_patch.instances.bbox_minWS_z.push_back(bboxWS.min.z);
      gpu_billboard_patch.instances.bbox_maxWS_x.push_back(bboxWS.max.x);
      gpu_billboard_patch.instances.bbox_maxWS_y.push_back(bboxWS.max.y);
      gpu_billboard_patch.instances.bbox_maxWS_z.push_back(bboxWS.max.z);
   }

   auto prepare_view(billboard_patches_draw draw, const frustum& view_frustum,
                     dynamic_buffer_allocator& allocator) -> view
   {
      std::span<view::instances> instance_lists =
         {reinterpret_cast<view::instances*>(
             _temp_buffer_allocator.allocate<view::instances>(_billboard_patches.size())),
          _billboard_patches.size()};

      for (std::size_t i = 0; i < instance_lists.size(); ++i) {
         const billboard_patch_class_gpu& billboard_patch = _billboard_patches[i];

         if (_culling_storage.size() <
             billboard_patch.instances.world_from_object.size()) {
            _culling_storage.resize(billboard_patch.instances.world_from_object.size());
         }

         std::span<uint32> visible_instances =
            draw != billboard_patches_draw::shadow
               ? cull_objects(view_frustum, billboard_patch.instances.bbox_minWS_x,
                              billboard_patch.instances.bbox_minWS_y,
                              billboard_patch.instances.bbox_minWS_z,
                              billboard_patch.instances.bbox_maxWS_x,
                              billboard_patch.instances.bbox_maxWS_y,
                              billboard_patch.instances.bbox_maxWS_z, _culling_storage)
               : cull_objects_shadow_cascade(view_frustum,
                                             billboard_patch.instances.bbox_minWS_x,
                                             billboard_patch.instances.bbox_minWS_y,
                                             billboard_patch.instances.bbox_minWS_z,
                                             billboard_patch.instances.bbox_maxWS_x,
                                             billboard_patch.instances.bbox_maxWS_y,
                                             billboard_patch.instances.bbox_maxWS_z,
                                             _culling_storage);

         dynamic_buffer_allocator::allocation instance_allocations = allocator.allocate(
            sizeof(std::array<float3, 4>) * visible_instances.size());

         for (std::size_t visible_index = 0;
              visible_index < visible_instances.size(); ++visible_index) {
            const uint32 instance_index = visible_instances[visible_index];

            std::memcpy(instance_allocations.cpu_address +
                           sizeof(std::array<float3, 4>) * visible_index,
                        &billboard_patch.instances.world_from_object[instance_index],
                        sizeof(std::array<float3, 4>));
         }

         view::instances instances = {
            .count = static_cast<uint32>(visible_instances.size()),
            .world_from_object = instance_allocations.gpu_address,
         };

         std::memcpy(&instance_lists[i], &instances, sizeof(instances));
      }

      return {instance_lists};
   }

   void draw(billboard_patches_draw draw, const view& view,
             gpu_virtual_address frame_constant_buffer_view,
             gpu_virtual_address lights_constant_buffer_view,
             gpu::graphics_command_list& command_list,
             root_signature_library& root_signatures, pipeline_library& pipelines) const
   {
      if (_index_buffer_particle_capacity == 0) return;

      command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

      command_list.set_graphics_root_signature(root_signatures.billboard_patch.get());

      command_list.set_graphics_cbv(rs::billboard_patch::frame_cbv,
                                    frame_constant_buffer_view);
      command_list.set_graphics_cbv(rs::billboard_patch::lights_cbv,
                                    lights_constant_buffer_view);

      command_list.set_pipeline_state(select_pipeline(draw, pipelines));

      command_list.ia_set_index_buffer({
         .buffer_location = _device.get_gpu_virtual_address(_index_buffer.get()),
         .size_in_bytes = static_cast<uint32>(_index_buffer_particle_capacity *
                                              6 * sizeof(uint16)),
      });

      for (std::size_t i = 0;
           i < std::min(view.data.size(), _billboard_patches.size()); ++i) {
         const view::instances& visible = view.data[i];
         const billboard_patch_class_gpu& billboard_patch = _billboard_patches[i];

         if (visible.count == 0) continue;

         std::array<gpu::vertex_buffer_view, 2> vertex_buffer_views;

         vertex_buffer_views[0] = billboard_patch.vertex_buffer;
         vertex_buffer_views[1] = {
            .buffer_location = visible.world_from_object,
            .size_in_bytes =
               static_cast<uint32>(sizeof(std::array<float3, 4>) * visible.count),
            .stride_in_bytes = sizeof(std::array<float3, 4>),
         };

         command_list.set_graphics_32bit_constant(rs::billboard_patch::texture,
                                                  billboard_patch.texture->srv.index,
                                                  0);

         command_list.ia_set_vertex_buffers(0, vertex_buffer_views);

         command_list.draw_indexed_instanced(billboard_patch.particle_count * 6,
                                             visible.count, 0, 0, 0);
      }
   }

   void process_updated_textures(const updated_textures& updated)
   {
      for (billboard_patch_class_gpu& billboard_patch : _billboard_patches) {
         if (auto new_texture = updated.check(billboard_patch.texture_name);
             new_texture and new_texture->dimension == world_texture_dimension::_2d) {
            billboard_patch.texture = std::move(new_texture);
            billboard_patch.texture_load_token = nullptr;
         }
      }
   }

private:
   struct billboard_patch_class_gpu {
      std::uintptr_t cpu_key = 0;
      bool used_this_frame = true;

      uint32 particle_count = 0;
      gpu::vertex_buffer_view vertex_buffer;

      struct instances {
         using float_soa_vector = std::vector<float, aligned_allocator<float, 32>>;

         std::vector<std::array<float3, 4>> world_from_object;
         float_soa_vector bbox_minWS_x;
         float_soa_vector bbox_minWS_y;
         float_soa_vector bbox_minWS_z;
         float_soa_vector bbox_maxWS_x;
         float_soa_vector bbox_maxWS_y;
         float_soa_vector bbox_maxWS_z;
      } instances;

      lowercase_string texture_name;
      std::shared_ptr<const world_texture> texture;
      std::shared_ptr<const world_texture_load_token> texture_load_token;
   };

   gpu::device& _device;
   texture_manager& _texture_manager;

   bool _animated = false;

   float4x4 _world_matrix;
   float3 _light_direction;
   std::vector<billboard_patch_class_gpu> _billboard_patches;

   uint32 _index_buffer_particle_capacity = 0;
   gpu::unique_resource_handle _index_buffer;

   std::vector<uint32> _culling_storage;

   temp_buffer_allocator _temp_buffer_allocator{0x10000};

   void update_vertices(const world::billboard_patch_class& billboard_patch_class,
                        billboard_patch_class_gpu& gpu_billboard_patch,
                        dynamic_buffer_allocator& allocator)
   {
      dynamic_buffer_allocator::allocation vertices =
         allocator.allocate(sizeof(std::array<world::billboard_patch_vertex, 4>) *
                            billboard_patch_class.num_particles());

      billboard_patch_class
         .get_quads(_world_matrix, _light_direction, _animated,
                    {reinterpret_cast<std::array<world::billboard_patch_vertex, 4>*>(
                        vertices.cpu_address),
                     billboard_patch_class.num_particles()});

      gpu_billboard_patch.vertex_buffer = {.buffer_location = vertices.gpu_address,
                                           .size_in_bytes =
                                              static_cast<uint32>(vertices.size),
                                           .stride_in_bytes =
                                              sizeof(world::billboard_patch_vertex)};
      gpu_billboard_patch.particle_count =
         static_cast<uint32>(billboard_patch_class.num_particles());
   }
};

billboard_patches::billboard_patches(gpu::device& device, texture_manager& texture_manager)
   : _impl{device, texture_manager}
{
}

void billboard_patches::update(const float4x4& world_matrix,
                               const world::world& world, const bool animated)
{
   _impl->update(world_matrix, world, animated);
}

void billboard_patches::update_index_buffer(gpu::copy_command_list& command_list,
                                            dynamic_buffer_allocator& allocator)
{
   _impl->update_index_buffer(command_list, allocator);
}

void billboard_patches::update_view_same_frame(const float4x4& world_matrix,
                                               dynamic_buffer_allocator& allocator)
{
   _impl->update_view_same_frame(world_matrix, allocator);
}

void billboard_patches::add_billboard_patch(const world::billboard_patch_class& billboard_patch_class,
                                            const float4x4& world_from_object,
                                            dynamic_buffer_allocator& allocator)
{
   _impl->add_billboard_patch(billboard_patch_class, world_from_object, allocator);
}

auto billboard_patches::prepare_view(billboard_patches_draw draw,
                                     const frustum& view_frustum,
                                     dynamic_buffer_allocator& allocator) -> view
{
   return _impl->prepare_view(draw, view_frustum, allocator);
}

void billboard_patches::draw(billboard_patches_draw draw, const view& view,
                             gpu_virtual_address frame_constant_buffer_view,
                             gpu_virtual_address lights_constant_buffer_view,
                             gpu::graphics_command_list& command_list,
                             root_signature_library& root_signatures,
                             pipeline_library& pipelines) const
{
   _impl->draw(draw, view, frame_constant_buffer_view, lights_constant_buffer_view,
               command_list, root_signatures, pipelines);
}

void billboard_patches::process_updated_textures(const updated_textures& updated)
{
   _impl->process_updated_textures(updated);
}

billboard_patches::~billboard_patches() = default;
}