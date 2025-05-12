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

auto format_as(block_quad_split quad_split) noexcept -> uint8
{
   return fmt::underlying(quad_split);
}

auto format_as(block_foley_group group) noexcept -> uint8
{
   return fmt::underlying(group);
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

void save_ramps(io::output_file& out, const blocks_ramps& ramps) noexcept
{
   if (ramps.size() == 0) return;

   out.write_ln("Ramps({})", ramps.size());
   out.write_ln("{");

   for (uint32 box_index = 0; box_index < ramps.size(); ++box_index) {
      const block_description_ramp& ramp = ramps.description[box_index];
      const int8 ramp_layer = ramps.layer[box_index];

      out.write_ln("   Ramp()");
      out.write_ln("   {");

      out.write_ln("      Rotation({}, {}, {}, {});", ramp.rotation.w,
                   ramp.rotation.x, ramp.rotation.y, ramp.rotation.z);
      out.write_ln("      Position({}, {}, {});", ramp.position.x,
                   ramp.position.y, ramp.position.z);
      out.write_ln("      Size({}, {}, {});", ramp.size.x, ramp.size.y,
                   ramp.size.z);
      out.write_ln("      SurfaceMaterials({}, {}, {}, {}, {});",
                   ramp.surface_materials[0], ramp.surface_materials[1],
                   ramp.surface_materials[2], ramp.surface_materials[3],
                   ramp.surface_materials[4]);
      out.write_ln("      SurfaceTextureMode({}, {}, {}, {}, {});",
                   ramp.surface_texture_mode[0], ramp.surface_texture_mode[1],
                   ramp.surface_texture_mode[2], ramp.surface_texture_mode[3],
                   ramp.surface_texture_mode[4]);
      out.write_ln("      SurfaceTextureRotation({}, {}, {}, {}, {});",
                   ramp.surface_texture_rotation[0], ramp.surface_texture_rotation[1],
                   ramp.surface_texture_rotation[2], ramp.surface_texture_rotation[3],
                   ramp.surface_texture_rotation[4]);
      out.write_ln(
         "      SurfaceTextureScale({}, {}, {}, {}, {}, {}, {}, {}, {}, {});",
         ramp.surface_texture_scale[0][0], ramp.surface_texture_scale[0][1],
         ramp.surface_texture_scale[1][0], ramp.surface_texture_scale[1][1],
         ramp.surface_texture_scale[2][0], ramp.surface_texture_scale[2][1],
         ramp.surface_texture_scale[3][0], ramp.surface_texture_scale[3][1],
         ramp.surface_texture_scale[4][0], ramp.surface_texture_scale[4][1]);
      out.write_ln(
         "      SurfaceTextureOffset({}, {}, {}, {}, {}, {}, {}, {}, {}, {});",
         ramp.surface_texture_offset[0][0], ramp.surface_texture_offset[0][1],
         ramp.surface_texture_offset[1][0], ramp.surface_texture_offset[1][1],
         ramp.surface_texture_offset[2][0], ramp.surface_texture_offset[2][1],
         ramp.surface_texture_offset[3][0], ramp.surface_texture_offset[3][1],
         ramp.surface_texture_offset[4][0], ramp.surface_texture_offset[4][1]);
      if (ramp_layer != 0) out.write_ln("      Layer({});", ramp_layer);
      out.write_ln("   }");
   }

   out.write_ln("}\n");
}

void save_quads(io::output_file& out, const blocks_quads& quads) noexcept
{
   if (quads.size() == 0) return;

   out.write_ln("Quads({})", quads.size());
   out.write_ln("{");

   for (uint32 box_index = 0; box_index < quads.size(); ++box_index) {
      const block_description_quad& quad = quads.description[box_index];
      const int8 quad_layer = quads.layer[box_index];

      out.write_ln("   Quad()");
      out.write_ln("   {");

      out.write_ln("      Vertex0({}, {}, {});", quad.vertices[0].x,
                   quad.vertices[0].y, quad.vertices[0].z);
      out.write_ln("      Vertex1({}, {}, {});", quad.vertices[1].x,
                   quad.vertices[1].y, quad.vertices[1].z);
      out.write_ln("      Vertex2({}, {}, {});", quad.vertices[2].x,
                   quad.vertices[2].y, quad.vertices[2].z);
      out.write_ln("      Vertex3({}, {}, {});", quad.vertices[3].x,
                   quad.vertices[3].y, quad.vertices[3].z);
      out.write_ln("      QuadSplit({});", quad.quad_split);
      out.write_ln("      SurfaceMaterials({});", quad.surface_materials[0]);
      out.write_ln("      SurfaceTextureMode({});", quad.surface_texture_mode[0]);
      out.write_ln("      SurfaceTextureRotation({});",
                   quad.surface_texture_rotation[0]);
      out.write_ln("      SurfaceTextureScale({}, {});",
                   quad.surface_texture_scale[0][0],
                   quad.surface_texture_scale[0][1]);
      out.write_ln("      SurfaceTextureOffset({}, {});",
                   quad.surface_texture_offset[0][0],
                   quad.surface_texture_offset[0][1]);
      if (quad_layer != 0) out.write_ln("      Layer({});", quad_layer);
      out.write_ln("   }");
   }

   out.write_ln("}\n");
}

void save_cylinders(io::output_file& out, const blocks_cylinders& cylinders) noexcept
{
   if (cylinders.size() == 0) return;

   out.write_ln("Cylinders({})", cylinders.size());
   out.write_ln("{");

   for (uint32 box_index = 0; box_index < cylinders.size(); ++box_index) {
      const block_description_cylinder& cylinder = cylinders.description[box_index];
      const int8 cylinder_layer = cylinders.layer[box_index];

      out.write_ln("   Cylinder()");
      out.write_ln("   {");

      out.write_ln("      Rotation({}, {}, {}, {});", cylinder.rotation.w,
                   cylinder.rotation.x, cylinder.rotation.y, cylinder.rotation.z);
      out.write_ln("      Position({}, {}, {});", cylinder.position.x,
                   cylinder.position.y, cylinder.position.z);
      out.write_ln("      Size({}, {}, {});", cylinder.size.x, cylinder.size.y,
                   cylinder.size.z);
      out.write_ln("      SurfaceMaterials({}, {}, {});",
                   cylinder.surface_materials[0], cylinder.surface_materials[1],
                   cylinder.surface_materials[2]);
      out.write_ln("      SurfaceTextureMode({}, {}, {});",
                   cylinder.surface_texture_mode[0], cylinder.surface_texture_mode[1],
                   cylinder.surface_texture_mode[2]);
      out.write_ln("      SurfaceTextureRotation({}, {}, {});",
                   cylinder.surface_texture_rotation[0],
                   cylinder.surface_texture_rotation[1],
                   cylinder.surface_texture_rotation[2]);
      out.write_ln("      SurfaceTextureScale({}, {}, {}, {}, {}, {});",
                   cylinder.surface_texture_scale[0][0],
                   cylinder.surface_texture_scale[0][1],
                   cylinder.surface_texture_scale[1][0],
                   cylinder.surface_texture_scale[1][1],
                   cylinder.surface_texture_scale[2][0],
                   cylinder.surface_texture_scale[2][1]);
      out.write_ln("      SurfaceTextureOffset({}, {}, {}, {}, {}, {});",
                   cylinder.surface_texture_offset[0][0],
                   cylinder.surface_texture_offset[0][1],
                   cylinder.surface_texture_offset[1][0],
                   cylinder.surface_texture_offset[1][1],
                   cylinder.surface_texture_offset[2][0],
                   cylinder.surface_texture_offset[2][1]);
      if (cylinder_layer != 0) out.write_ln("      Layer({});", cylinder_layer);
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
      out.write_ln("      FoleyFXGroup({});", material.foley_group);
      out.write_ln("   }");
   }

   out.write_ln("}\n");
}

}

void save_blocks(const io::path& path, const blocks& blocks)
{
   io::output_file out{path};

   save_boxes(out, blocks.boxes);
   save_ramps(out, blocks.ramps);
   save_cylinders(out, blocks.cylinders);
   save_quads(out, blocks.quads);
   save_materials(out, blocks);
}

}