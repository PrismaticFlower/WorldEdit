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

void save_stairways(io::output_file& out, const blocks_stairways& stairways) noexcept
{
   if (stairways.size() == 0) return;

   out.write_ln("Stairways({})", stairways.size());
   out.write_ln("{");

   for (uint32 stairway_index = 0; stairway_index < stairways.size(); ++stairway_index) {
      const block_description_stairway& stairway =
         stairways.description[stairway_index];
      const int8 stairway_layer = stairways.layer[stairway_index];

      out.write_ln("   Stairway()");
      out.write_ln("   {");

      out.write_ln("      Rotation({}, {}, {}, {});", stairway.rotation.w,
                   stairway.rotation.x, stairway.rotation.y, stairway.rotation.z);
      out.write_ln("      Position({}, {}, {});", stairway.position.x,
                   stairway.position.y, stairway.position.z);
      out.write_ln("      Size({}, {}, {});", stairway.size.x, stairway.size.y,
                   stairway.size.z);
      out.write_ln("      StepHeight({});", stairway.step_height);
      out.write_ln("      FirstStepOffset({});", stairway.first_step_offset);
      out.write_ln("      SurfaceMaterials({}, {}, {}, {}, {}, {});",
                   stairway.surface_materials[0], stairway.surface_materials[1],
                   stairway.surface_materials[2], stairway.surface_materials[3],
                   stairway.surface_materials[4], stairway.surface_materials[5]);
      out.write_ln("      SurfaceTextureMode({}, {}, {}, {}, {}, {});",
                   stairway.surface_texture_mode[0],
                   stairway.surface_texture_mode[1], stairway.surface_texture_mode[2],
                   stairway.surface_texture_mode[3], stairway.surface_texture_mode[4],
                   stairway.surface_texture_mode[5]);
      out.write_ln("      SurfaceTextureRotation({}, {}, {}, {}, {}, {});",
                   stairway.surface_texture_rotation[0],
                   stairway.surface_texture_rotation[1],
                   stairway.surface_texture_rotation[2],
                   stairway.surface_texture_rotation[3],
                   stairway.surface_texture_rotation[4],
                   stairway.surface_texture_rotation[5]);
      out.write_ln(
         "      SurfaceTextureScale({}, {}, {}, {}, {}, {}, {}, {}, {}, "
         "{}, {}, {});",
         stairway.surface_texture_scale[0][0], stairway.surface_texture_scale[0][1],
         stairway.surface_texture_scale[1][0], stairway.surface_texture_scale[1][1],
         stairway.surface_texture_scale[2][0], stairway.surface_texture_scale[2][1],
         stairway.surface_texture_scale[3][0], stairway.surface_texture_scale[3][1],
         stairway.surface_texture_scale[4][0], stairway.surface_texture_scale[4][1],
         stairway.surface_texture_scale[5][0], stairway.surface_texture_scale[5][1]);
      out.write_ln(
         "      SurfaceTextureOffset({}, {}, {}, {}, {}, {}, {}, {}, {}, "
         "{}, {}, {});",
         stairway.surface_texture_offset[0][0],
         stairway.surface_texture_offset[0][1],
         stairway.surface_texture_offset[1][0],
         stairway.surface_texture_offset[1][1],
         stairway.surface_texture_offset[2][0],
         stairway.surface_texture_offset[2][1],
         stairway.surface_texture_offset[3][0],
         stairway.surface_texture_offset[3][1],
         stairway.surface_texture_offset[4][0],
         stairway.surface_texture_offset[4][1],
         stairway.surface_texture_offset[5][0],
         stairway.surface_texture_offset[5][1]);
      if (stairway_layer != 0) out.write_ln("      Layer({});", stairway_layer);
      out.write_ln("   }");
   }

   out.write_ln("}\n");
}

void save_cones(io::output_file& out, const blocks_cones& cones) noexcept
{
   if (cones.size() == 0) return;

   out.write_ln("Cones({})", cones.size());
   out.write_ln("{");

   for (uint32 box_index = 0; box_index < cones.size(); ++box_index) {
      const block_description_cone& cone = cones.description[box_index];
      const int8 cone_layer = cones.layer[box_index];

      out.write_ln("   Cone()");
      out.write_ln("   {");

      out.write_ln("      Rotation({}, {}, {}, {});", cone.rotation.w,
                   cone.rotation.x, cone.rotation.y, cone.rotation.z);
      out.write_ln("      Position({}, {}, {});", cone.position.x,
                   cone.position.y, cone.position.z);
      out.write_ln("      Size({}, {}, {});", cone.size.x, cone.size.y,
                   cone.size.z);
      out.write_ln("      SurfaceMaterials({}, {});", cone.surface_materials[0],
                   cone.surface_materials[1]);
      out.write_ln("      SurfaceTextureMode({}, {});",
                   cone.surface_texture_mode[0], cone.surface_texture_mode[1]);
      out.write_ln("      SurfaceTextureRotation({}, {});",
                   cone.surface_texture_rotation[0],
                   cone.surface_texture_rotation[1]);
      out.write_ln("      SurfaceTextureScale({}, {}, {}, {});",
                   cone.surface_texture_scale[0][0],
                   cone.surface_texture_scale[0][1],
                   cone.surface_texture_scale[1][0],
                   cone.surface_texture_scale[1][1]);
      out.write_ln("      SurfaceTextureOffset({}, {}, {}, {});",
                   cone.surface_texture_offset[0][0],
                   cone.surface_texture_offset[0][1],
                   cone.surface_texture_offset[1][0],
                   cone.surface_texture_offset[1][1]);
      if (cone_layer != 0) out.write_ln("      Layer({});", cone_layer);
      out.write_ln("   }");
   }

   out.write_ln("}\n");
}

void save_hemispheres(io::output_file& out, const blocks_hemispheres& hemispheres) noexcept
{
   if (hemispheres.size() == 0) return;

   out.write_ln("Hemispheres({})", hemispheres.size());
   out.write_ln("{");

   for (uint32 box_index = 0; box_index < hemispheres.size(); ++box_index) {
      const block_description_hemisphere& hemisphere =
         hemispheres.description[box_index];
      const int8 hemisphere_layer = hemispheres.layer[box_index];

      out.write_ln("   Hemisphere()");
      out.write_ln("   {");

      out.write_ln("      Rotation({}, {}, {}, {});", hemisphere.rotation.w,
                   hemisphere.rotation.x, hemisphere.rotation.y,
                   hemisphere.rotation.z);
      out.write_ln("      Position({}, {}, {});", hemisphere.position.x,
                   hemisphere.position.y, hemisphere.position.z);
      out.write_ln("      Size({}, {}, {});", hemisphere.size.x,
                   hemisphere.size.y, hemisphere.size.z);
      out.write_ln("      SurfaceMaterials({}, {});",
                   hemisphere.surface_materials[0], hemisphere.surface_materials[1]);
      out.write_ln("      SurfaceTextureMode({}, {});",
                   hemisphere.surface_texture_mode[0],
                   hemisphere.surface_texture_mode[1]);
      out.write_ln("      SurfaceTextureRotation({}, {});",
                   hemisphere.surface_texture_rotation[0],
                   hemisphere.surface_texture_rotation[1]);
      out.write_ln("      SurfaceTextureScale({}, {}, {}, {});",
                   hemisphere.surface_texture_scale[0][0],
                   hemisphere.surface_texture_scale[0][1],
                   hemisphere.surface_texture_scale[1][0],
                   hemisphere.surface_texture_scale[1][1]);
      out.write_ln("      SurfaceTextureOffset({}, {}, {}, {});",
                   hemisphere.surface_texture_offset[0][0],
                   hemisphere.surface_texture_offset[0][1],
                   hemisphere.surface_texture_offset[1][0],
                   hemisphere.surface_texture_offset[1][1]);
      if (hemisphere_layer != 0)
         out.write_ln("      Layer({});", hemisphere_layer);
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
   save_quads(out, blocks.quads);
   save_cylinders(out, blocks.cylinders);
   save_stairways(out, blocks.stairways);
   save_cones(out, blocks.cones);
   save_hemispheres(out, blocks.hemispheres);
   save_materials(out, blocks);
}

}