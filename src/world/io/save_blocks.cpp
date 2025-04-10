#include "save_blocks.hpp"

#include "io/output_file.hpp"

namespace we::world {

auto format_as(block_texture_mode mode) noexcept -> uint8
{
   return fmt::underlying(mode);
}

auto format_as(block_texture_rotation rotation) noexcept -> uint8
{
   return fmt::underlying(rotation);
}

namespace {

void save_boxes(io::output_file& out, const blocks_boxes& boxes) noexcept
{
   if (boxes.size() == 0) return;

   out.write_ln("Boxes({})", boxes.size());
   out.write_ln("{");

   for (uint32 box_index = 0; box_index < boxes.size(); ++box_index) {
      const block_description_box& box = boxes.description[box_index];
      const int8 box_layer = boxes.layer[box_index];

      out.write_ln("   Box()");
      out.write_ln("   {");

      out.write_ln("      Rotation({}, {}, {}, {});", box.rotation.w,
                   box.rotation.x, box.rotation.y, box.rotation.z);
      out.write_ln("      Position({}, {}, {});", box.position.x,
                   box.position.y, box.position.z);
      out.write_ln("      Size({}, {}, {});", box.size.x, box.size.y, box.size.z);
      out.write_ln("      SurfaceMaterials({}, {}, {}, {}, {}, {});",
                   box.surface_materials[0], box.surface_materials[1],
                   box.surface_materials[2], box.surface_materials[3],
                   box.surface_materials[4], box.surface_materials[5]);
      out.write_ln("      SurfaceTextureMode({}, {}, {}, {}, {}, {});",
                   box.surface_texture_mode[0], box.surface_texture_mode[1],
                   box.surface_texture_mode[2], box.surface_texture_mode[3],
                   box.surface_texture_mode[4], box.surface_texture_mode[5]);
      out.write_ln("      SurfaceTextureRotation({}, {}, {}, {}, {}, {});",
                   box.surface_texture_rotation[0], box.surface_texture_rotation[1],
                   box.surface_texture_rotation[2], box.surface_texture_rotation[3],
                   box.surface_texture_rotation[4], box.surface_texture_rotation[5]);
      out.write_ln(
         "      SurfaceTextureScale({}, {}, {}, {}, {}, {}, {}, {}, {}, "
         "{}, {}, {});",
         box.surface_texture_scale[0][0], box.surface_texture_scale[0][1],
         box.surface_texture_scale[1][0], box.surface_texture_scale[1][1],
         box.surface_texture_scale[2][0], box.surface_texture_scale[2][1],
         box.surface_texture_scale[3][0], box.surface_texture_scale[3][1],
         box.surface_texture_scale[4][0], box.surface_texture_scale[4][1],
         box.surface_texture_scale[5][0], box.surface_texture_scale[5][1]);
      out.write_ln(
         "      SurfaceTextureOffset({}, {}, {}, {}, {}, {}, {}, {}, {}, "
         "{}, {}, {});",
         box.surface_texture_offset[0][0], box.surface_texture_offset[0][1],
         box.surface_texture_offset[1][0], box.surface_texture_offset[1][1],
         box.surface_texture_offset[2][0], box.surface_texture_offset[2][1],
         box.surface_texture_offset[3][0], box.surface_texture_offset[3][1],
         box.surface_texture_offset[4][0], box.surface_texture_offset[4][1],
         box.surface_texture_offset[5][0], box.surface_texture_offset[5][1]);
      if (box_layer != 0) out.write_ln("      Layer({});", box_layer);
      out.write_ln("   }");
   }

   out.write_ln("}\n");
}

void save_materials(io::output_file& out, const blocks& blocks) noexcept
{
   const block_material empty_material;

   out.write_ln("Materials()");
   out.write_ln("{");

   for (uint32 material_index = 0; material_index < blocks.materials.size();
        ++material_index) {
      const block_material& material = blocks.materials[material_index];

      if (material == empty_material) continue;

      out.write_ln("   Material({})", material_index);
      out.write_ln("   {");
      out.write_ln("      Name(\"{}\");", material.name);
      out.write_ln("      DiffuseMap(\"{}\");", material.diffuse_map);
      out.write_ln("      NormalMap(\"{}\");", material.normal_map);
      out.write_ln("      DetailMap(\"{}\");", material.detail_map);
      out.write_ln("      EnvMap(\"{}\");", material.env_map);
      out.write_ln("      DetailTiling({}, {});", material.detail_tiling[0],
                   material.detail_tiling[1]);
      out.write_ln("      TileNormalMap({:d});", material.tile_normal_map);
      out.write_ln("      SpecularLighting({:d});", material.specular_lighting);
      out.write_ln("      SpecularColor({}, {}, {});", material.specular_color.x,
                   material.specular_color.y, material.specular_color.z);
      out.write_ln("   }");
   }

   out.write_ln("}\n");
}

}

void save_blocks(const io::path& path, const blocks& blocks)
{
   io::output_file out{path};

   save_boxes(out, blocks.boxes);
   save_materials(out, blocks);
}

}