#include "export_selection.hpp"

#include "../object_class.hpp"

#include "../blocks/export/mesh_gather.hpp"

#include "../utility/world_utilities.hpp"

#include "io/output_file.hpp"

#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"

#include <absl/container/flat_hash_set.h>

namespace we::world {

void export_selection_to_obj(const io::path& path,
                             const export_selection_options& options,
                             const selection& selection, const world& world,
                             const object_class_library& object_classes,
                             assets::libraries_manager& assets,
                             output_stream& warning_output)
{
   io::output_file obj{path};
   io::output_file mtl{io::make_path_with_new_extension(path, ".mtl")};

   obj.write_ln("mtllib {}.mtl", path.stem());

   absl::flat_hash_set<std::string> created_materials;

   uint64 vertex_offset = 1;

   for (const selected_entity& selected : selection) {
      if (selected.is<object_id>()) {
         const object* object =
            find_entity(world.objects, selected.get<object_id>());

         if (object) {
            if (object->name.empty()) {
               obj.write_ln("o Unnamed Object:{}", object - world.objects.data());
            }
            else {
               obj.write_ln("o {}", object->name);
            }

            float4x4 world_from_local = to_matrix(object->rotation);
            world_from_local[3] = {object->position, 1.0f};

            const float3x3 world_from_local_3x3{world_from_local};

            const assets::msh::flat_model& model =
               *object_classes[object->class_handle].model;

            uint64 mesh_index = 0;

            for (const assets::msh::mesh& mesh : model.meshes) {
               obj.write_ln("g {}", mesh_index++);
               obj.write_ln("usemtl {}", mesh.material.textures[0]);

               for (const float3& positionLS : mesh.positions) {
                  const float3 positionWS = world_from_local * positionLS;

                  obj.write_ln("v {} {} {}", positionWS.x, positionWS.y,
                               positionWS.z);
               }

               for (const float2& texcoords : mesh.texcoords) {
                  obj.write_ln("vt {} {}", texcoords.x, 1.0f - texcoords.y);
               }

               for (const float3& normalLS : mesh.normals) {
                  const float3 normalWS = world_from_local_3x3 * normalLS;

                  obj.write_ln("vn {} {} {}", normalWS.x, normalWS.y, normalWS.z);
               }

               for (const auto& [i0, i1, i2] : mesh.triangles) {
                  obj.write_ln("f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}",
                               vertex_offset + i0, vertex_offset + i1,
                               vertex_offset + i2);
               }

               vertex_offset += mesh.positions.size();

               if (not created_materials.contains(mesh.material.textures[0])) {
                  mtl.write_ln("newmtl {}", mesh.material.textures[0]);
                  mtl.write_ln("illum 0");
                  mtl.write_ln("Kd 1 1 1");
                  mtl.write_ln("map_Kd {}.tga", mesh.material.textures[0]);

                  if (options.copy_textures) {
                     io::path texture_path = assets.textures.query_path(
                        lowercase_string{mesh.material.textures[0]});

                     if (not texture_path.empty()) {
                        if (not io::copy_file(texture_path,
                                              io::compose_path(path.parent_path(),
                                                               texture_path.filename()))) {
                           warning_output
                              .write("Warning! Failed to copy texture {} when "
                                     "exporting selection.\n",
                                     texture_path.string_view());
                        }
                     }
                  }

                  created_materials.insert(mesh.material.textures[0]);
               }
            }
         }
      }
   }

   const std::vector<block_export_scene> scenes =
      gather_export_meshes(world.blocks, selection);

   for (std::size_t i = 0; i < scenes.size(); ++i) {
      const block_export_scene& scene = scenes[i];

      obj.write_ln("o Block Mesh:{}", i);

      for (const assets::msh::node& node : scene.msh_scene.nodes) {
         if (node.hidden) continue;

         std::size_t segment_index = 0;

         for (const assets::msh::geometry_segment& segment : node.segments) {
            if (not segment.texcoords) continue;
            if (not segment.normals) continue;

            const std::string& material_texture =
               scene.msh_scene.materials[segment.material_index].textures[0];

            obj.write_ln("g {}", segment_index++);
            obj.write_ln("usemtl {}", material_texture);

            for (const float3& positionWS : segment.positions) {
               obj.write_ln("v {} {} {}", positionWS.x, positionWS.y, positionWS.z);
            }

            for (const float2& texcoords : *segment.texcoords) {
               obj.write_ln("vt {} {}", texcoords.x, 1.0f - texcoords.y);
            }

            for (const float3& normalWS : *segment.normals) {
               obj.write_ln("vn {} {} {}", normalWS.x, normalWS.y, normalWS.z);
            }

            for (const auto& [i0, i1, i2] : segment.triangles) {
               obj.write_ln("f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}",
                            vertex_offset + i0, vertex_offset + i1,
                            vertex_offset + i2);
            }

            vertex_offset += segment.positions.size();

            if (not created_materials.contains(material_texture)) {
               mtl.write_ln("newmtl {}", material_texture);
               mtl.write_ln("illum 0");
               mtl.write_ln("Kd 1 1 1");
               mtl.write_ln("map_Kd {}.tga", material_texture);

               if (options.copy_textures) {
                  io::path texture_path =
                     assets.textures.query_path(lowercase_string{material_texture});

                  if (not texture_path.empty()) {
                     if (not io::copy_file(texture_path,
                                           io::compose_path(path.parent_path(),
                                                            texture_path.filename()))) {
                        warning_output
                           .write("Warning! Failed to copy texture {} when "
                                  "exporting selection.\n",
                                  texture_path.string_view());
                     }
                  }
               }

               created_materials.insert(material_texture);
            }
         }
      }
   }

   if (not options.include_terrain) return;
   if (not world.terrain.active_flags.terrain) return;

   obj.write_ln("o Terrain");
   obj.write_ln("usemtl {}", world.terrain.texture_names[0]);

   if (not created_materials.contains(world.terrain.texture_names[0])) {
      mtl.write_ln("newmtl {}", world.terrain.texture_names[0]);
      mtl.write_ln("illum 0");
      mtl.write_ln("Kd 1 1 1");
      mtl.write_ln("map_Kd {}.tga", world.terrain.texture_names[0]);

      if (options.copy_textures) {
         io::path texture_path = assets.textures.query_path(
            lowercase_string{world.terrain.texture_names[0]});

         if (not texture_path.empty()) {
            if (not io::copy_file(texture_path,
                                  io::compose_path(path.parent_path(),
                                                   texture_path.filename()))) {
               warning_output.write("Warning! Failed to copy texture {} when "
                                    "exporting selection.\n",
                                    texture_path.string_view());
            }
         }
      }

      created_materials.insert(world.terrain.texture_names[0]);
   }

   const terrain& terrain = world.terrain;
   const int32 terrain_half_length = terrain.length / 2;

   for (int32 z = 0; z < terrain.length; ++z) {
      for (int32 x = 0; x < terrain.length; ++x) {
         const float3 positionWS = {(x - terrain_half_length) * terrain.grid_scale,
                                    terrain.height_map[{x, z}] * terrain.height_scale,
                                    (z - terrain_half_length + 1) * terrain.grid_scale};

         float2 texcoords;

         switch (terrain.texture_axes[0]) {
         case texture_axis::xz:
            texcoords = {positionWS.x, positionWS.z};
            break;
         case texture_axis::xy:
            texcoords = {positionWS.x, positionWS.y};
            break;
         case texture_axis::yz:
            texcoords = {positionWS.y, positionWS.z};
            break;
         case texture_axis::zx:
            texcoords = {positionWS.z, positionWS.x};
            break;
         case texture_axis::yx:
            texcoords = {positionWS.y, positionWS.x};
            break;
         case texture_axis::zy:
            texcoords = {positionWS.z, positionWS.y};
            break;
         case texture_axis::negative_xz:
            texcoords = {-positionWS.x, -positionWS.z};
            break;
         case texture_axis::negative_xy:
            texcoords = {-positionWS.x, -positionWS.y};
         case texture_axis::negative_yz:
            texcoords = {-positionWS.y, -positionWS.z};
         case texture_axis::negative_zx:
            texcoords = {-positionWS.z, -positionWS.x};
         case texture_axis::negative_yx:
            texcoords = {-positionWS.y, -positionWS.x};
         case texture_axis::negative_zy:
            texcoords = {-positionWS.z, -positionWS.y};
         }

         texcoords *= terrain.texture_scales[0];

         const int16 height0x =
            terrain.height_map[{std::clamp(x - 1, 0, terrain.length - 1), z}];
         const int16 height1x =
            terrain.height_map[{std::clamp(x + 1, 0, terrain.length - 1), z}];
         const int16 height0z =
            terrain.height_map[{x, std::clamp(z - 1, 0, terrain.length - 1)}];
         const int16 height1z =
            terrain.height_map[{x, std::clamp(z + 1, 0, terrain.length - 1)}];

         const float3 normalWS =
            normalize(float3((height0x - height1x) * terrain.height_scale /
                                (terrain.grid_scale * 2.0f),
                             1.0,
                             (height0z - height1z) * terrain.height_scale /
                                (terrain.grid_scale * 2.0f)));

         obj.write_ln("v {} {} {}", positionWS.x, positionWS.y, positionWS.z);
         obj.write_ln("vt {} {}", texcoords.x, 1.0f - texcoords.y);
         obj.write_ln("vn {} {} {}", normalWS.x, normalWS.y, normalWS.z);
      }
   }

   const int32 terrain_length_quads = terrain.length - 1;

   for (int32 z = 0; z < terrain_length_quads; ++z) {
      for (int32 x = 0; x < terrain_length_quads; ++x) {
         const auto index = [&](int32 x, int32 z) noexcept -> uint16 {
            return static_cast<uint16>(z * terrain.length + x);
         };

         if (z & 1) {
            obj.write_ln("f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}",
                         vertex_offset + index(x, z), vertex_offset + index(x, z + 1),
                         vertex_offset + index(x + 1, z));
            obj.write_ln("f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}",
                         vertex_offset + index(x, z + 1),
                         vertex_offset + index(x + 1, z + 1),
                         vertex_offset + index(x + 1, z));
         }
         else {
            obj.write_ln("f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}",
                         vertex_offset + index(x, z),
                         vertex_offset + index(x + 1, z + 1),
                         vertex_offset + index(x + 1, z));
            obj.write_ln("f {0}/{0}/{0} {1}/{1}/{1} {2}/{2}/{2}",
                         vertex_offset + index(x, z), vertex_offset + index(x, z + 1),
                         vertex_offset + index(x + 1, z + 1));
         }
      }
   }
}

}