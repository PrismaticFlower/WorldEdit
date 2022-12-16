#include "material.hpp"
#include "gpu/barrier.hpp"

using namespace std::literals;

namespace we::graphics {

namespace {

enum class shader_flags : uint32 {
   none = 0b0,
   transparent = 0b1,
   unlit = 0b10,
   specular_visibility_in_diffuse_map = 0b100
};

constexpr bool marked_as_enum_bitflag(shader_flags)
{
   return true;
}

constexpr bool has_normalmap(const assets::msh::material& material)
{
   using assets::msh::material_flags;
   using assets::msh::rendertype;

   return are_flags_set(material.flags, material_flags::perpixel) or
          are_flags_set(material.flags, material_flags::specular) or
          material.rendertype == (rendertype::normalmap) or
          material.rendertype == (rendertype::normalmap_specular) or
          material.rendertype == (rendertype::normalmap_tiled_envmapped) or
          material.rendertype == (rendertype::normalmap_tiled) or
          material.rendertype == (rendertype::normalmap_envmapped);
}

constexpr auto make_shader_flags(const material_pipeline_flags pipeline_flags,
                                 assets::msh::material_flags msh_flags,
                                 const bool specular_visibility_in_diffuse_map) noexcept
   -> shader_flags
{
   shader_flags flags = shader_flags::none;

   if (are_flags_set(pipeline_flags, material_pipeline_flags::transparent)) {
      flags |= shader_flags::transparent;
   }

   if (are_flags_set(msh_flags, assets::msh::material_flags::unlit)) {
      flags |= shader_flags::unlit;
   }

   if (are_flags_set(msh_flags, assets::msh::material_flags::glow)) {
      flags |= shader_flags::unlit; // Actually implementing this properly is fairly pointless without a bloom pass.
   }

   if (specular_visibility_in_diffuse_map) {
      flags |= shader_flags::specular_visibility_in_diffuse_map;
   }

   return flags;
}

struct alignas(16) normal_material_constants {
   shader_flags flags;
   uint32 diffuse_map;
   uint32 normal_map;
   uint32 pad;
   float3 specular_color;
};

}

material::material(const assets::msh::material& material, gpu::device& device,
                   copy_command_list_pool& copy_command_list_pool,
                   texture_manager& texture_manager)
{
   texture_names.diffuse_map = lowercase_string{material.textures[0]};
   texture_names.normal_map = lowercase_string{material.textures[1]};

   textures.diffuse_map = texture_manager.at_or(texture_names.diffuse_map,
                                                texture_manager.null_diffuse_map());

   if (has_normalmap(material) and not texture_names.normal_map.empty()) {
      textures.normal_map = texture_manager.at_or(texture_names.normal_map,
                                                  texture_manager.null_normal_map());
   }

   if (not textures.diffuse_map) {
      textures.diffuse_map = texture_manager.null_diffuse_map();
   }
   if (not textures.normal_map) {
      textures.normal_map = texture_manager.null_normal_map();
   }

   using assets::msh::material_flags;

   msh_flags = material.flags;

   if (are_flags_set(material.flags, material_flags::transparent)) {
      flags |= material_pipeline_flags::transparent;
   }

   if (are_flags_set(material.flags, material_flags::transparent_doublesided)) {
      flags |= material_pipeline_flags::transparent;
      flags |= material_pipeline_flags::doublesided;
   }

   if (are_flags_set(material.flags, material_flags::hardedged)) {
      flags |= material_pipeline_flags::alpha_cutout;
      flags &= ~material_pipeline_flags::transparent;
   }

   if (are_flags_set(material.flags, material_flags::additive)) {
      flags |= material_pipeline_flags::additive;
   }

   if (are_flags_set(material.flags, material_flags::specular) or
       material.rendertype == assets::msh::rendertype::specular or
       material.rendertype == assets::msh::rendertype::normalmap_specular) {
      specular_color = material.specular_color;
   }
   else {
      specular_color = float3{0.0f};
   }

   init_resources(device, copy_command_list_pool);
}

void material::init_resources(gpu::device& device,
                              copy_command_list_pool& copy_command_list_pool)
{
   pooled_copy_command_list command_list = copy_command_list_pool.aquire_and_reset();

   constant_buffer = {device.create_buffer({.size = sizeof(normal_material_constants),
                                            .debug_name =
                                               "Material Constant Buffer"},
                                           gpu::heap_type::default_),
                      device.direct_queue};
   constant_buffer_view = device.get_gpu_virtual_address(constant_buffer.get());

   normal_material_constants constants{
      .flags = make_shader_flags(flags, msh_flags, texture_names.normal_map.empty()),
      .diffuse_map = textures.diffuse_map->srv_srgb.index,
      .normal_map = textures.normal_map->srv.index,
      .specular_color = specular_color};

   gpu::unique_resource_handle upload_buffer =
      {device.create_buffer({.size = sizeof(normal_material_constants),
                             .debug_name = "Material Constant Upload Buffer"},
                            gpu::heap_type::upload),
       device.background_copy_queue};

   std::byte* const upload_buffer_ptr =
      static_cast<std::byte*>(device.map(upload_buffer.get(), 0, {}));

   std::memcpy(upload_buffer_ptr, &constants, sizeof(constants));

   device.unmap(upload_buffer.get(), 0, {0, sizeof(constants)});

   command_list->copy_resource(constant_buffer.get(), upload_buffer.get());

   command_list->close();

   device.background_copy_queue.execute_command_lists(command_list.get());
}

void material::process_updated_textures(gpu::graphics_command_list& command_list,
                                        const updated_textures& updated,
                                        gpu::device& device)
{
   using namespace ranges::views;

   bool update = false;

   const gpu_virtual_address constant_buffer_address =
      device.get_gpu_virtual_address(constant_buffer.get());

   if (auto new_texture = updated.find(texture_names.diffuse_map);
       new_texture != updated.end()) {
      textures.diffuse_map = new_texture->second;

      command_list.write_buffer_immediate(constant_buffer_address +
                                             offsetof(normal_material_constants,
                                                      diffuse_map),
                                          textures.diffuse_map->srv_srgb.index);

      update = true;
   }

   if (auto new_texture = updated.find(texture_names.normal_map);
       new_texture != updated.end()) {
      textures.normal_map = new_texture->second;

      command_list.write_buffer_immediate(constant_buffer_address +
                                             offsetof(normal_material_constants, normal_map),
                                          textures.normal_map->srv.index);

      update = true;
   }

   if (update) {
      command_list.deferred_resource_barrier(
         gpu::transition_barrier(constant_buffer.get(), gpu::resource_state::copy_dest,
                                 gpu::resource_state::vertex_and_constant_buffer));
   }
}
}
