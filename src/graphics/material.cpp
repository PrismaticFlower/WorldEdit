#include "material.hpp"
#include "gpu/barrier_helpers.hpp"

#include <range/v3/view.hpp>

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

struct alignas(256) normal_material_constants {
   shader_flags flags;
   uint32 diffuse_map;
   uint32 normal_map;
   uint32 pad;
   float3 specular_color;
};

}

material::material(const assets::msh::material& material,
                   gpu::device& gpu_device, texture_manager& texture_manager)
{
   texture_names.resize(2);
   texture_names[0] = lowercase_string{material.textures[0]};
   texture_names[1] = lowercase_string{material.textures[1]};

   textures.resize(2);

   textures[0] =
      texture_manager.at_or(texture_names[0], texture_manager.null_diffuse_map());

   if (has_normalmap(material) and not texture_names[1].empty()) {
      textures[1] = texture_manager.at_or(texture_names[1],
                                          texture_manager.null_normal_map());
   }

   if (not textures[0]) textures[0] = texture_manager.null_diffuse_map();
   if (not textures[1]) textures[1] = texture_manager.null_normal_map();

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

   init_resources(gpu_device);
}

void material::init_resources(gpu::device& gpu_device)
{
   constant_buffer =
      gpu_device.create_buffer({.size = sizeof(normal_material_constants)},
                               D3D12_HEAP_TYPE_DEFAULT,
                               D3D12_RESOURCE_STATE_COPY_DEST);

   std::array resource_view_descriptions{gpu::resource_view_desc{
      .resource = constant_buffer.resource(),

      .view_desc = gpu::constant_buffer_view{.buffer_location =
                                                constant_buffer.gpu_virtual_address(),
                                             .size = constant_buffer.size()}}};

   constant_buffer_view =
      gpu_device.create_resource_view_set(resource_view_descriptions);

   normal_material_constants constants{.flags =
                                          make_shader_flags(flags, msh_flags,
                                                            texture_names[1].empty()),
                                       .diffuse_map = textures[0]->srv_srgb_index,
                                       .normal_map = textures[1]->srv_index,
                                       .specular_color = specular_color};

   auto copy_context = gpu_device.copy_manager.aquire_context();

   const gpu::buffer_desc upload_buffer_desc = [&] {
      gpu::buffer_desc upload_buffer_desc;

      upload_buffer_desc.size = sizeof(normal_material_constants);

      return upload_buffer_desc;
   }();

   ID3D12Resource& upload_buffer =
      copy_context.create_upload_resource(upload_buffer_desc);

   std::byte* const upload_buffer_ptr = [&] {
      const D3D12_RANGE read_range{};
      void* map_void_ptr = nullptr;

      throw_if_failed(upload_buffer.Map(0, &read_range, &map_void_ptr));

      return static_cast<std::byte*>(map_void_ptr);
   }();

   std::memcpy(upload_buffer_ptr, &constants, sizeof(constants));

   const D3D12_RANGE write_range{0, sizeof(constants)};
   upload_buffer.Unmap(0, &write_range);

   gpu::copy_command_list& command_list = copy_context.command_list();

   command_list.copy_resource(*constant_buffer.resource(), upload_buffer);

   gpu_device.copy_manager.close_and_execute(copy_context);
}

void material::update_constant_buffer(gpu::graphics_command_list& command_list,
                                      gpu::dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   normal_material_constants constants{.flags =
                                          make_shader_flags(flags, msh_flags,
                                                            texture_names[1].empty()),
                                       .diffuse_map = textures[0]->srv_srgb_index,
                                       .normal_map = textures[1]->srv_index,
                                       .specular_color = specular_color};

   auto allocation = dynamic_buffer_allocator.allocate(sizeof(constants));

   std::memcpy(allocation.cpu_address, &constants, sizeof(constants));

   command_list.copy_buffer_region(*constant_buffer.resource(), 0,
                                   *dynamic_buffer_allocator.view_resource(),
                                   allocation.gpu_address -
                                      dynamic_buffer_allocator.gpu_base_address(),
                                   sizeof(constants));

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(*constant_buffer.resource(), D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}

void material::process_updated_textures(gpu::graphics_command_list& command_list,
                                        gpu::dynamic_buffer_allocator& dynamic_buffer_allocator,
                                        const updated_textures& updated)
{
   using namespace ranges::views;

   bool update = false;

   for (auto [name, texture] : zip(texture_names, textures)) {
      if (auto new_texture = updated.find(name); new_texture != updated.end()) {
         texture = new_texture->second;
         update = true;
      }
   }

   if (update) update_constant_buffer(command_list, dynamic_buffer_allocator);
}
}
