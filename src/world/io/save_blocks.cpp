#include "save_blocks.hpp"

#include "io/output_file.hpp"

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

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

void save_custom(io::output_file& out, const blocks_custom& blocks) noexcept
{
   if (blocks.size() == 0) return;

   out.write_ln("Custom({})", blocks.size());
   out.write_ln("{");

   for (uint32 block_index = 0; block_index < blocks.size(); ++block_index) {
      const block_description_custom& block = blocks.description[block_index];
      const int8 block_layer = blocks.layer[block_index];

      switch (block.mesh_description.type) {
      case block_custom_mesh_type::stairway: {
         out.write_ln("   Stairway()");
      } break;
      case block_custom_mesh_type::ring: {
         out.write_ln("   Ring()");
      } break;
      case block_custom_mesh_type::beveled_box: {
         out.write_ln("   BeveledBox()");
      } break;
      case block_custom_mesh_type::curve: {
         out.write_ln("   CubicCurve()");
      } break;
      case block_custom_mesh_type::cylinder: {
         out.write_ln("   Cylinder()");
      } break;
      case block_custom_mesh_type::cone: {
         out.write_ln("   Cone()");
      } break;
      case block_custom_mesh_type::arch: {
         out.write_ln("   Arch()");
      } break;
      }
      out.write_ln("   {");

      out.write_ln("      Rotation({}, {}, {}, {});", block.rotation.w,
                   block.rotation.x, block.rotation.y, block.rotation.z);
      out.write_ln("      Position({}, {}, {});", block.position.x,
                   block.position.y, block.position.z);

      switch (block.mesh_description.type) {
      case block_custom_mesh_type::stairway: {
         const world::block_custom_mesh_description_stairway& stairway =
            block.mesh_description.stairway;

         out.write_ln("      Size({}, {}, {});", stairway.size.x,
                      stairway.size.y, stairway.size.z);
         out.write_ln("      StepHeight({});", stairway.step_height);
         out.write_ln("      FirstStepOffset({});", stairway.first_step_offset);
      } break;
      case block_custom_mesh_type::ring: {
         const world::block_custom_mesh_description_ring& ring =
            block.mesh_description.ring;

         out.write_ln("      InnerRadius({});", ring.inner_radius);
         out.write_ln("      OuterRadius({});", ring.outer_radius);
         out.write_ln("      Height({});", ring.height);
         out.write_ln("      Segments({});", ring.segments);
         if (ring.flat_shading) out.write_ln("      FlatShading();");
         out.write_ln("      TextureLoops({});", ring.texture_loops);
      } break;
      case block_custom_mesh_type::beveled_box: {
         const world::block_custom_mesh_description_beveled_box& box =
            block.mesh_description.beveled_box;

         out.write_ln("      Size({}, {}, {});", box.size.x, box.size.y,
                      box.size.z);
         out.write_ln("      Amount({});", box.amount);
         out.write_ln("      BevelTop({:d});", box.bevel_top);
         out.write_ln("      BevelSides({:d});", box.bevel_sides);
         out.write_ln("      BevelBottom({:d});", box.bevel_bottom);
      } break;
      case block_custom_mesh_type::curve: {
         const world::block_custom_mesh_description_curve& curve =
            block.mesh_description.curve;

         out.write_ln("      Width({});", curve.width);
         out.write_ln("      Height({});", curve.height);
         out.write_ln("      Segments({});", curve.segments);
         out.write_ln("      TextureLoops({});", curve.texture_loops);

         out.write_ln("      P0({}, {}, {});", curve.p0.x, curve.p0.y, curve.p0.z);
         out.write_ln("      P1({}, {}, {});", curve.p1.x, curve.p1.y, curve.p1.z);
         out.write_ln("      P2({}, {}, {});", curve.p2.x, curve.p2.y, curve.p2.z);
         out.write_ln("      P3({}, {}, {});", curve.p3.x, curve.p3.y, curve.p3.z);
      } break;
      case block_custom_mesh_type::cylinder: {
         const world::block_custom_mesh_description_cylinder& cylinder =
            block.mesh_description.cylinder;

         out.write_ln("      Size({}, {}, {});", cylinder.size.x,
                      cylinder.size.y, cylinder.size.z);
         out.write_ln("      Segments({});", cylinder.segments);
         if (cylinder.flat_shading) out.write_ln("      FlatShading();");
         out.write_ln("      TextureLoops({});", cylinder.texture_loops);
      } break;
      case block_custom_mesh_type::cone: {
         const world::block_custom_mesh_description_cone& cone =
            block.mesh_description.cone;

         out.write_ln("      Size({}, {}, {});", cone.size.x, cone.size.y,
                      cone.size.z);
         out.write_ln("      Segments({});", cone.segments);
         if (cone.flat_shading) out.write_ln("      FlatShading();");
      } break;
      case block_custom_mesh_type::arch: {
         const world::block_custom_mesh_description_arch& arch =
            block.mesh_description.arch;

         out.write_ln("      Size({}, {}, {});", arch.size.x, arch.size.y,
                      arch.size.z);
         out.write_ln("      CrownLength({});", arch.crown_length);
         out.write_ln("      CrownHeight({});", arch.crown_height);
         out.write_ln("      CurveHeight({});", arch.curve_height);
         out.write_ln("      SpanLength({});", arch.span_length);
         out.write_ln("      Segments({});", arch.segments);
      } break;
      }

      out.write_ln("      SurfaceMaterials({}, {}, {}, {}, {}, {});",
                   block.surface_materials[0], block.surface_materials[1],
                   block.surface_materials[2], block.surface_materials[3],
                   block.surface_materials[4], block.surface_materials[5]);
      out.write_ln("      SurfaceTextureMode({}, {}, {}, {}, {}, {});",
                   block.surface_texture_mode[0], block.surface_texture_mode[1],
                   block.surface_texture_mode[2], block.surface_texture_mode[3],
                   block.surface_texture_mode[4], block.surface_texture_mode[5]);
      out.write_ln("      SurfaceTextureRotation({}, {}, {}, {}, {}, {});",
                   block.surface_texture_rotation[0],
                   block.surface_texture_rotation[1],
                   block.surface_texture_rotation[2],
                   block.surface_texture_rotation[3],
                   block.surface_texture_rotation[4],
                   block.surface_texture_rotation[5]);
      out.write_ln(
         "      SurfaceTextureScale({}, {}, {}, {}, {}, {}, {}, {}, {}, "
         "{}, {}, {});",
         block.surface_texture_scale[0][0], block.surface_texture_scale[0][1],
         block.surface_texture_scale[1][0], block.surface_texture_scale[1][1],
         block.surface_texture_scale[2][0], block.surface_texture_scale[2][1],
         block.surface_texture_scale[3][0], block.surface_texture_scale[3][1],
         block.surface_texture_scale[4][0], block.surface_texture_scale[4][1],
         block.surface_texture_scale[5][0], block.surface_texture_scale[5][1]);
      out.write_ln(
         "      SurfaceTextureOffset({}, {}, {}, {}, {}, {}, {}, {}, {}, "
         "{}, {}, {});",
         block.surface_texture_offset[0][0], block.surface_texture_offset[0][1],
         block.surface_texture_offset[1][0], block.surface_texture_offset[1][1],
         block.surface_texture_offset[2][0], block.surface_texture_offset[2][1],
         block.surface_texture_offset[3][0], block.surface_texture_offset[3][1],
         block.surface_texture_offset[4][0], block.surface_texture_offset[4][1],
         block.surface_texture_offset[5][0], block.surface_texture_offset[5][1]);
      if (block_layer != 0) out.write_ln("      Layer({});", block_layer);
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

void save_pyramids(io::output_file& out, const blocks_pyramids& pyramids) noexcept
{
   if (pyramids.size() == 0) return;

   out.write_ln("Pyramids({})", pyramids.size());
   out.write_ln("{");

   for (uint32 box_index = 0; box_index < pyramids.size(); ++box_index) {
      const block_description_pyramid& pyramid = pyramids.description[box_index];
      const int8 pyramid_layer = pyramids.layer[box_index];

      out.write_ln("   Pyramid()");
      out.write_ln("   {");

      out.write_ln("      Rotation({}, {}, {}, {});", pyramid.rotation.w,
                   pyramid.rotation.x, pyramid.rotation.y, pyramid.rotation.z);
      out.write_ln("      Position({}, {}, {});", pyramid.position.x,
                   pyramid.position.y, pyramid.position.z);
      out.write_ln("      Size({}, {}, {});", pyramid.size.x, pyramid.size.y,
                   pyramid.size.z);
      out.write_ln("      SurfaceMaterials({}, {}, {}, {}, {});",
                   pyramid.surface_materials[0], pyramid.surface_materials[1],
                   pyramid.surface_materials[2], pyramid.surface_materials[3],
                   pyramid.surface_materials[4]);
      out.write_ln("      SurfaceTextureMode({}, {}, {}, {}, {});",
                   pyramid.surface_texture_mode[0], pyramid.surface_texture_mode[1],
                   pyramid.surface_texture_mode[2], pyramid.surface_texture_mode[3],
                   pyramid.surface_texture_mode[4]);
      out.write_ln("      SurfaceTextureRotation({}, {}, {}, {}, {});",
                   pyramid.surface_texture_rotation[0],
                   pyramid.surface_texture_rotation[1],
                   pyramid.surface_texture_rotation[2],
                   pyramid.surface_texture_rotation[3],
                   pyramid.surface_texture_rotation[4]);
      out.write_ln(
         "      SurfaceTextureScale({}, {}, {}, {}, {}, {}, {}, {}, {}, {});",
         pyramid.surface_texture_scale[0][0], pyramid.surface_texture_scale[0][1],
         pyramid.surface_texture_scale[1][0], pyramid.surface_texture_scale[1][1],
         pyramid.surface_texture_scale[2][0], pyramid.surface_texture_scale[2][1],
         pyramid.surface_texture_scale[3][0], pyramid.surface_texture_scale[3][1],
         pyramid.surface_texture_scale[4][0], pyramid.surface_texture_scale[4][1]);
      out.write_ln(
         "      SurfaceTextureOffset({}, {}, {}, {}, {}, {}, {}, {}, {}, {});",
         pyramid.surface_texture_offset[0][0], pyramid.surface_texture_offset[0][1],
         pyramid.surface_texture_offset[1][0], pyramid.surface_texture_offset[1][1],
         pyramid.surface_texture_offset[2][0], pyramid.surface_texture_offset[2][1],
         pyramid.surface_texture_offset[3][0], pyramid.surface_texture_offset[3][1],
         pyramid.surface_texture_offset[4][0], pyramid.surface_texture_offset[4][1]);
      if (pyramid_layer != 0) out.write_ln("      Layer({});", pyramid_layer);
      out.write_ln("   }");
   }

   out.write_ln("}\n");
}

void save_terrain_cut_boxes(io::output_file& out,
                            const blocks_terrain_cut_boxes& terrain_cut_boxes) noexcept
{
   if (terrain_cut_boxes.size() == 0) return;

   out.write_ln("TerrainCutBoxes({})", terrain_cut_boxes.size());
   out.write_ln("{");

   for (uint32 box_index = 0; box_index < terrain_cut_boxes.size(); ++box_index) {
      const block_description_terrain_cut_box& terrain_cut_box =
         terrain_cut_boxes.description[box_index];
      const int8 terrain_cut_box_layer = terrain_cut_boxes.layer[box_index];

      out.write_ln("   TerrainCutBox()");
      out.write_ln("   {");

      out.write_ln("      Rotation({}, {}, {}, {});",
                   terrain_cut_box.rotation.w, terrain_cut_box.rotation.x,
                   terrain_cut_box.rotation.y, terrain_cut_box.rotation.z);
      out.write_ln("      Position({}, {}, {});", terrain_cut_box.position.x,
                   terrain_cut_box.position.y, terrain_cut_box.position.z);
      out.write_ln("      Size({}, {}, {});", terrain_cut_box.size.x,
                   terrain_cut_box.size.y, terrain_cut_box.size.z);
      if (terrain_cut_box_layer != 0) {
         out.write_ln("      Layer({});", terrain_cut_box_layer);
      }
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
   save_custom(out, blocks.custom);
   save_hemispheres(out, blocks.hemispheres);
   save_pyramids(out, blocks.pyramids);
   save_terrain_cut_boxes(out, blocks.terrain_cut_boxes);
   save_materials(out, blocks);
}

}