#include "load_blocks.hpp"
#include "load_failure.hpp"

#include "../blocks/utility/bounding_box.hpp"

#include "assets/config/io.hpp"

#include "io/error.hpp"
#include "io/read_file.hpp"

#include "math/bounding_box.hpp"
#include "math/vector_funcs.hpp"

#include "utility/stopwatch.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

namespace we::world {

using string::iequals;

namespace {

auto read_texture_mode(const uint8 mode) noexcept -> block_texture_mode
{
   switch (mode) {
   case static_cast<uint8>(block_texture_mode::world_space_auto):
   case static_cast<uint8>(block_texture_mode::world_space_zy):
   case static_cast<uint8>(block_texture_mode::world_space_xz):
   case static_cast<uint8>(block_texture_mode::world_space_xy):
   case static_cast<uint8>(block_texture_mode::local_space_auto):
   case static_cast<uint8>(block_texture_mode::local_space_zy):
   case static_cast<uint8>(block_texture_mode::local_space_xz):
   case static_cast<uint8>(block_texture_mode::local_space_xy):
   case static_cast<uint8>(block_texture_mode::unwrapped):
      return block_texture_mode{mode};
   }

   return {};
}

void load_boxes(assets::config::node& node, const layer_remap& layer_remap,
                blocks& blocks_out)
{
   for (const auto& key_node : node) {
      if (not iequals(key_node.key, "Box")) continue;

      if (blocks_out.boxes.size() == max_blocks) {
         throw load_failure{fmt::format(
            "Too many blocks (of type box) for WorldEdit to handle. \n   "
            "Max Supported Count: {}\n",
            max_blocks)};
      }

      block_description_box box;
      int8 layer = 0;

      for (const auto& prop : key_node) {
         if (iequals(prop.key, "Rotation")) {
            box.rotation = {prop.values.get<float>(0), prop.values.get<float>(1),
                            prop.values.get<float>(2), prop.values.get<float>(3)};
         }
         else if (iequals(prop.key, "Position")) {
            box.position = {prop.values.get<float>(0), prop.values.get<float>(1),
                            prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "Size")) {
            box.size = {prop.values.get<float>(0), prop.values.get<float>(1),
                        prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "SurfaceMaterials")) {
            for (uint32 i = 0; i < box.surface_materials.size(); ++i) {
               box.surface_materials[i] = prop.values.get<uint8>(i);
            }
         }
         else if (iequals(prop.key, "SurfaceTextureMode")) {
            for (uint32 i = 0; i < box.surface_texture_mode.size(); ++i) {
               box.surface_texture_mode[i] =
                  read_texture_mode(prop.values.get<uint8>(i));
            }
         }
         else if (iequals(prop.key, "SurfaceTextureRotation")) {
            for (uint32 i = 0; i < box.surface_texture_rotation.size(); ++i) {
               const uint8 rotation = prop.values.get<uint8>(i);

               switch (rotation) {
               case static_cast<uint8>(block_texture_rotation::d0):
               case static_cast<uint8>(block_texture_rotation::d90):
               case static_cast<uint8>(block_texture_rotation::d180):
               case static_cast<uint8>(block_texture_rotation::d270):
                  box.surface_texture_rotation[i] = block_texture_rotation{rotation};
                  break;
               }
            }
         }
         else if (iequals(prop.key, "SurfaceTextureScale")) {
            for (uint32 i = 0; i < box.surface_texture_scale.size(); ++i) {
               box.surface_texture_scale[i] =
                  {std::clamp(prop.values.get<int8>(i * 2 + 0),
                              block_min_texture_scale, block_max_texture_scale),
                   std::clamp(prop.values.get<int8>(i * 2 + 1),
                              block_min_texture_scale, block_max_texture_scale)};
            }
         }
         else if (iequals(prop.key, "SurfaceTextureOffset")) {
            for (uint32 i = 0; i < box.surface_texture_offset.size(); ++i) {
               box.surface_texture_offset[i] =
                  {std::min(prop.values.get<uint16>(i * 2 + 0), block_max_texture_offset),
                   std::min(prop.values.get<uint16>(i * 2 + 1), block_max_texture_offset)};
            }
         }
         else if (iequals(prop.key, "Layer")) {
            layer = layer_remap[prop.values.get<int>(0)];
         }
      }

      const math::bounding_box bbox = get_bounding_box(box);

      blocks_out.boxes.bbox.min_x.push_back(bbox.min.x);
      blocks_out.boxes.bbox.min_y.push_back(bbox.min.y);
      blocks_out.boxes.bbox.min_z.push_back(bbox.min.z);
      blocks_out.boxes.bbox.max_x.push_back(bbox.max.x);
      blocks_out.boxes.bbox.max_y.push_back(bbox.max.y);
      blocks_out.boxes.bbox.max_z.push_back(bbox.max.z);
      blocks_out.boxes.hidden.push_back(false);
      blocks_out.boxes.layer.push_back(layer);
      blocks_out.boxes.description.push_back(box);
      blocks_out.boxes.ids.push_back(blocks_out.next_id.boxes.aquire());
   }
}

void load_ramps(assets::config::node& node, const layer_remap& layer_remap,
                blocks& blocks_out)
{
   for (const auto& key_node : node) {
      if (not iequals(key_node.key, "Ramp")) continue;

      if (blocks_out.ramps.size() == max_blocks) {
         throw load_failure{fmt::format(
            "Too many blocks (of type ramp) for WorldEdit to handle. \n   "
            "Max Supported Count: {}\n",
            max_blocks)};
      }

      block_description_ramp ramp;
      int8 layer = 0;

      for (const auto& prop : key_node) {
         if (iequals(prop.key, "Rotation")) {
            ramp.rotation = {prop.values.get<float>(0), prop.values.get<float>(1),
                             prop.values.get<float>(2), prop.values.get<float>(3)};
         }
         else if (iequals(prop.key, "Position")) {
            ramp.position = {prop.values.get<float>(0), prop.values.get<float>(1),
                             prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "Size")) {
            ramp.size = {prop.values.get<float>(0), prop.values.get<float>(1),
                         prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "SurfaceMaterials")) {
            for (uint32 i = 0; i < ramp.surface_materials.size(); ++i) {
               ramp.surface_materials[i] = prop.values.get<uint8>(i);
            }
         }
         else if (iequals(prop.key, "SurfaceTextureMode")) {
            for (uint32 i = 0; i < ramp.surface_texture_mode.size(); ++i) {
               ramp.surface_texture_mode[i] =
                  read_texture_mode(prop.values.get<uint8>(i));
            }
         }
         else if (iequals(prop.key, "SurfaceTextureRotation")) {
            for (uint32 i = 0; i < ramp.surface_texture_rotation.size(); ++i) {
               const uint8 rotation = prop.values.get<uint8>(i);

               switch (rotation) {
               case static_cast<uint8>(block_texture_rotation::d0):
               case static_cast<uint8>(block_texture_rotation::d90):
               case static_cast<uint8>(block_texture_rotation::d180):
               case static_cast<uint8>(block_texture_rotation::d270):
                  ramp.surface_texture_rotation[i] = block_texture_rotation{rotation};
                  break;
               }
            }
         }
         else if (iequals(prop.key, "SurfaceTextureScale")) {
            for (uint32 i = 0; i < ramp.surface_texture_scale.size(); ++i) {
               ramp.surface_texture_scale[i] =
                  {std::clamp(prop.values.get<int8>(i * 2 + 0),
                              block_min_texture_scale, block_max_texture_scale),
                   std::clamp(prop.values.get<int8>(i * 2 + 1),
                              block_min_texture_scale, block_max_texture_scale)};
            }
         }
         else if (iequals(prop.key, "SurfaceTextureOffset")) {
            for (uint32 i = 0; i < ramp.surface_texture_offset.size(); ++i) {
               ramp.surface_texture_offset[i] =
                  {std::min(prop.values.get<uint16>(i * 2 + 0), block_max_texture_offset),
                   std::min(prop.values.get<uint16>(i * 2 + 1), block_max_texture_offset)};
            }
         }
         else if (iequals(prop.key, "Layer")) {
            layer = layer_remap[prop.values.get<int>(0)];
         }
      }

      const math::bounding_box bbox = get_bounding_box(ramp);

      blocks_out.ramps.bbox.min_x.push_back(bbox.min.x);
      blocks_out.ramps.bbox.min_y.push_back(bbox.min.y);
      blocks_out.ramps.bbox.min_z.push_back(bbox.min.z);
      blocks_out.ramps.bbox.max_x.push_back(bbox.max.x);
      blocks_out.ramps.bbox.max_y.push_back(bbox.max.y);
      blocks_out.ramps.bbox.max_z.push_back(bbox.max.z);
      blocks_out.ramps.hidden.push_back(false);
      blocks_out.ramps.layer.push_back(layer);
      blocks_out.ramps.description.push_back(ramp);
      blocks_out.ramps.ids.push_back(blocks_out.next_id.ramps.aquire());
   }
}

void load_quads(assets::config::node& node, const layer_remap& layer_remap,
                blocks& blocks_out)
{
   for (const auto& key_node : node) {
      if (not iequals(key_node.key, "Quad")) continue;

      if (blocks_out.quads.size() == max_blocks) {
         throw load_failure{fmt::format(
            "Too many blocks (of type quad) for WorldEdit to handle. \n   "
            "Max Supported Count: {}\n",
            max_blocks)};
      }

      block_description_quad quad;
      int8 layer = 0;

      for (const auto& prop : key_node) {
         if (iequals(prop.key, "Vertex0")) {
            quad.vertices[0] = {prop.values.get<float>(0), prop.values.get<float>(1),
                                prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "Vertex1")) {
            quad.vertices[1] = {prop.values.get<float>(0), prop.values.get<float>(1),
                                prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "Vertex2")) {
            quad.vertices[2] = {prop.values.get<float>(0), prop.values.get<float>(1),
                                prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "Vertex3")) {
            quad.vertices[3] = {prop.values.get<float>(0), prop.values.get<float>(1),
                                prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "QuadSplit")) {
            const uint8 quad_split = prop.values.get<uint8>(0);

            switch (quad_split) {
            case static_cast<uint8>(block_quad_split::regular):
            case static_cast<uint8>(block_quad_split::alternate):
               quad.quad_split = block_quad_split{quad_split};
               break;
            }
         }
         else if (iequals(prop.key, "SurfaceMaterials")) {
            for (uint32 i = 0; i < quad.surface_materials.size(); ++i) {
               quad.surface_materials[i] = prop.values.get<uint8>(i);
            }
         }
         else if (iequals(prop.key, "SurfaceTextureMode")) {
            for (uint32 i = 0; i < quad.surface_texture_mode.size(); ++i) {
               quad.surface_texture_mode[i] =
                  read_texture_mode(prop.values.get<uint8>(i));
            }
         }
         else if (iequals(prop.key, "SurfaceTextureRotation")) {
            for (uint32 i = 0; i < quad.surface_texture_rotation.size(); ++i) {
               const uint8 rotation = prop.values.get<uint8>(i);

               switch (rotation) {
               case static_cast<uint8>(block_texture_rotation::d0):
               case static_cast<uint8>(block_texture_rotation::d90):
               case static_cast<uint8>(block_texture_rotation::d180):
               case static_cast<uint8>(block_texture_rotation::d270):
                  quad.surface_texture_rotation[i] = block_texture_rotation{rotation};
                  break;
               }
            }
         }
         else if (iequals(prop.key, "SurfaceTextureScale")) {
            for (uint32 i = 0; i < quad.surface_texture_scale.size(); ++i) {
               quad.surface_texture_scale[i] =
                  {std::clamp(prop.values.get<int8>(i * 2 + 0),
                              block_min_texture_scale, block_max_texture_scale),
                   std::clamp(prop.values.get<int8>(i * 2 + 1),
                              block_min_texture_scale, block_max_texture_scale)};
            }
         }
         else if (iequals(prop.key, "SurfaceTextureOffset")) {
            for (uint32 i = 0; i < quad.surface_texture_offset.size(); ++i) {
               quad.surface_texture_offset[i] =
                  {std::min(prop.values.get<uint16>(i * 2 + 0), block_max_texture_offset),
                   std::min(prop.values.get<uint16>(i * 2 + 1), block_max_texture_offset)};
            }
         }
         else if (iequals(prop.key, "Layer")) {
            layer = layer_remap[prop.values.get<int>(0)];
         }
      }

      const math::bounding_box bbox = get_bounding_box(quad);

      blocks_out.quads.bbox.min_x.push_back(bbox.min.x);
      blocks_out.quads.bbox.min_y.push_back(bbox.min.y);
      blocks_out.quads.bbox.min_z.push_back(bbox.min.z);
      blocks_out.quads.bbox.max_x.push_back(bbox.max.x);
      blocks_out.quads.bbox.max_y.push_back(bbox.max.y);
      blocks_out.quads.bbox.max_z.push_back(bbox.max.z);
      blocks_out.quads.hidden.push_back(false);
      blocks_out.quads.layer.push_back(layer);
      blocks_out.quads.description.push_back(quad);
      blocks_out.quads.ids.push_back(blocks_out.next_id.quads.aquire());
   }
}

void load_cylinders(assets::config::node& node, const layer_remap& layer_remap,
                    blocks& blocks_out)
{
   for (const auto& key_node : node) {
      if (not iequals(key_node.key, "Cylinder")) continue;

      if (blocks_out.cylinders.size() == max_blocks) {
         throw load_failure{fmt::format(
            "Too many blocks (of type cylinder) for WorldEdit to handle. \n   "
            "Max Supported Count: {}\n",
            max_blocks)};
      }

      block_description_cylinder cylinder;
      int8 layer = 0;

      for (const auto& prop : key_node) {
         if (iequals(prop.key, "Rotation")) {
            cylinder.rotation = {prop.values.get<float>(0),
                                 prop.values.get<float>(1),
                                 prop.values.get<float>(2),
                                 prop.values.get<float>(3)};
         }
         else if (iequals(prop.key, "Position")) {
            cylinder.position = {prop.values.get<float>(0),
                                 prop.values.get<float>(1),
                                 prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "Size")) {
            cylinder.size = {prop.values.get<float>(0), prop.values.get<float>(1),
                             prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "SurfaceMaterials")) {
            for (uint32 i = 0; i < cylinder.surface_materials.size(); ++i) {
               cylinder.surface_materials[i] = prop.values.get<uint8>(i);
            }
         }
         else if (iequals(prop.key, "SurfaceTextureMode")) {
            for (uint32 i = 0; i < cylinder.surface_texture_mode.size(); ++i) {
               cylinder.surface_texture_mode[i] =
                  read_texture_mode(prop.values.get<uint8>(i));
            }
         }
         else if (iequals(prop.key, "SurfaceTextureRotation")) {
            for (uint32 i = 0; i < cylinder.surface_texture_rotation.size(); ++i) {
               const uint8 rotation = prop.values.get<uint8>(i);

               switch (rotation) {
               case static_cast<uint8>(block_texture_rotation::d0):
               case static_cast<uint8>(block_texture_rotation::d90):
               case static_cast<uint8>(block_texture_rotation::d180):
               case static_cast<uint8>(block_texture_rotation::d270):
                  cylinder.surface_texture_rotation[i] =
                     block_texture_rotation{rotation};
                  break;
               }
            }
         }
         else if (iequals(prop.key, "SurfaceTextureScale")) {
            for (uint32 i = 0; i < cylinder.surface_texture_scale.size(); ++i) {
               cylinder.surface_texture_scale[i] =
                  {std::clamp(prop.values.get<int8>(i * 2 + 0),
                              block_min_texture_scale, block_max_texture_scale),
                   std::clamp(prop.values.get<int8>(i * 2 + 1),
                              block_min_texture_scale, block_max_texture_scale)};
            }
         }
         else if (iequals(prop.key, "SurfaceTextureOffset")) {
            for (uint32 i = 0; i < cylinder.surface_texture_offset.size(); ++i) {
               cylinder.surface_texture_offset[i] =
                  {std::min(prop.values.get<uint16>(i * 2 + 0), block_max_texture_offset),
                   std::min(prop.values.get<uint16>(i * 2 + 1), block_max_texture_offset)};
            }
         }
         else if (iequals(prop.key, "Layer")) {
            layer = layer_remap[prop.values.get<int>(0)];
         }
      }

      const math::bounding_box bbox = get_bounding_box(cylinder);

      blocks_out.cylinders.bbox.min_x.push_back(bbox.min.x);
      blocks_out.cylinders.bbox.min_y.push_back(bbox.min.y);
      blocks_out.cylinders.bbox.min_z.push_back(bbox.min.z);
      blocks_out.cylinders.bbox.max_x.push_back(bbox.max.x);
      blocks_out.cylinders.bbox.max_y.push_back(bbox.max.y);
      blocks_out.cylinders.bbox.max_z.push_back(bbox.max.z);
      blocks_out.cylinders.hidden.push_back(false);
      blocks_out.cylinders.layer.push_back(layer);
      blocks_out.cylinders.description.push_back(cylinder);
      blocks_out.cylinders.ids.push_back(blocks_out.next_id.cylinders.aquire());
   }
}

void load_custom(assets::config::node& node, const layer_remap& layer_remap,
                 blocks& blocks_out)
{
   for (const auto& key_node : node) {
      if (blocks_out.custom.size() == max_blocks) {
         throw load_failure{fmt::format(
            "Too many blocks (of type custom) for WorldEdit to handle. \n   "
            "Max Supported Count: {}\n",
            max_blocks)};
      }

      block_description_custom block;
      int8 layer = 0;

      if (iequals(key_node.key, "Stairway")) {
         block.mesh_description.stairway = block_custom_mesh_description_stairway{};
         block_custom_mesh_description_stairway& stairway =
            block.mesh_description.stairway;

         for (const auto& prop : key_node) {
            if (iequals(prop.key, "Rotation")) {
               block.rotation = {prop.values.get<float>(0),
                                 prop.values.get<float>(1),
                                 prop.values.get<float>(2),
                                 prop.values.get<float>(3)};
            }
            else if (iequals(prop.key, "Position")) {
               block.position = {prop.values.get<float>(0),
                                 prop.values.get<float>(1),
                                 prop.values.get<float>(2)};
            }
            else if (iequals(prop.key, "Size")) {
               stairway.size = {prop.values.get<float>(0), prop.values.get<float>(1),
                                prop.values.get<float>(2)};
            }
            else if (iequals(prop.key, "StepHeight")) {
               stairway.step_height = prop.values.get<float>(0);
            }
            else if (iequals(prop.key, "FirstStepOffset")) {
               stairway.first_step_offset = prop.values.get<float>(0);
            }
            else if (iequals(prop.key, "SurfaceMaterials")) {
               for (uint32 i = 0; i < block.surface_materials.size(); ++i) {
                  block.surface_materials[i] = prop.values.get<uint8>(i);
               }
            }
            else if (iequals(prop.key, "SurfaceTextureMode")) {
               for (uint32 i = 0; i < block.surface_texture_mode.size(); ++i) {
                  block.surface_texture_mode[i] =
                     read_texture_mode(prop.values.get<uint8>(i));
               }
            }
            else if (iequals(prop.key, "SurfaceTextureRotation")) {
               for (uint32 i = 0; i < block.surface_texture_rotation.size(); ++i) {
                  const uint8 rotation = prop.values.get<uint8>(i);

                  switch (rotation) {
                  case static_cast<uint8>(block_texture_rotation::d0):
                  case static_cast<uint8>(block_texture_rotation::d90):
                  case static_cast<uint8>(block_texture_rotation::d180):
                  case static_cast<uint8>(block_texture_rotation::d270):
                     block.surface_texture_rotation[i] =
                        block_texture_rotation{rotation};
                     break;
                  }
               }
            }
            else if (iequals(prop.key, "SurfaceTextureScale")) {
               for (uint32 i = 0; i < block.surface_texture_scale.size(); ++i) {
                  block.surface_texture_scale[i] =
                     {std::clamp(prop.values.get<int8>(i * 2 + 0),
                                 block_min_texture_scale, block_max_texture_scale),
                      std::clamp(prop.values.get<int8>(i * 2 + 1),
                                 block_min_texture_scale, block_max_texture_scale)};
               }
            }
            else if (iequals(prop.key, "SurfaceTextureOffset")) {
               for (uint32 i = 0; i < block.surface_texture_offset.size(); ++i) {
                  block.surface_texture_offset[i] =
                     {std::min(prop.values.get<uint16>(i * 2 + 0), block_max_texture_offset),
                      std::min(prop.values.get<uint16>(i * 2 + 1),
                               block_max_texture_offset)};
               }
            }
            else if (iequals(prop.key, "Layer")) {
               layer = layer_remap[prop.values.get<int>(0)];
            }
         }
      }
      else {
         continue;
      }

      const math::bounding_box bbox = get_bounding_box(block);

      blocks_out.custom.bbox.min_x.push_back(bbox.min.x);
      blocks_out.custom.bbox.min_y.push_back(bbox.min.y);
      blocks_out.custom.bbox.min_z.push_back(bbox.min.z);
      blocks_out.custom.bbox.max_x.push_back(bbox.max.x);
      blocks_out.custom.bbox.max_y.push_back(bbox.max.y);
      blocks_out.custom.bbox.max_z.push_back(bbox.max.z);
      blocks_out.custom.hidden.push_back(false);
      blocks_out.custom.layer.push_back(layer);
      blocks_out.custom.description.push_back(block);
      blocks_out.custom.mesh.push_back(
         blocks_out.custom_meshes.add(block.mesh_description));
      blocks_out.custom.ids.push_back(blocks_out.next_id.custom.aquire());
   }
}

void load_cones(assets::config::node& node, const layer_remap& layer_remap,
                blocks& blocks_out)
{
   for (const auto& key_node : node) {
      if (not iequals(key_node.key, "Cone")) continue;

      if (blocks_out.cones.size() == max_blocks) {
         throw load_failure{fmt::format(
            "Too many blocks (of type cone) for WorldEdit to handle. \n   "
            "Max Supported Count: {}\n",
            max_blocks)};
      }

      block_description_cone cone;
      int8 layer = 0;

      for (const auto& prop : key_node) {
         if (iequals(prop.key, "Rotation")) {
            cone.rotation = {prop.values.get<float>(0), prop.values.get<float>(1),
                             prop.values.get<float>(2), prop.values.get<float>(3)};
         }
         else if (iequals(prop.key, "Position")) {
            cone.position = {prop.values.get<float>(0), prop.values.get<float>(1),
                             prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "Size")) {
            cone.size = {prop.values.get<float>(0), prop.values.get<float>(1),
                         prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "SurfaceMaterials")) {
            for (uint32 i = 0; i < cone.surface_materials.size(); ++i) {
               cone.surface_materials[i] = prop.values.get<uint8>(i);
            }
         }
         else if (iequals(prop.key, "SurfaceTextureMode")) {
            for (uint32 i = 0; i < cone.surface_texture_mode.size(); ++i) {
               cone.surface_texture_mode[i] =
                  read_texture_mode(prop.values.get<uint8>(i));
            }
         }
         else if (iequals(prop.key, "SurfaceTextureRotation")) {
            for (uint32 i = 0; i < cone.surface_texture_rotation.size(); ++i) {
               const uint8 rotation = prop.values.get<uint8>(i);

               switch (rotation) {
               case static_cast<uint8>(block_texture_rotation::d0):
               case static_cast<uint8>(block_texture_rotation::d90):
               case static_cast<uint8>(block_texture_rotation::d180):
               case static_cast<uint8>(block_texture_rotation::d270):
                  cone.surface_texture_rotation[i] = block_texture_rotation{rotation};
                  break;
               }
            }
         }
         else if (iequals(prop.key, "SurfaceTextureScale")) {
            for (uint32 i = 0; i < cone.surface_texture_scale.size(); ++i) {
               cone.surface_texture_scale[i] =
                  {std::clamp(prop.values.get<int8>(i * 2 + 0),
                              block_min_texture_scale, block_max_texture_scale),
                   std::clamp(prop.values.get<int8>(i * 2 + 1),
                              block_min_texture_scale, block_max_texture_scale)};
            }
         }
         else if (iequals(prop.key, "SurfaceTextureOffset")) {
            for (uint32 i = 0; i < cone.surface_texture_offset.size(); ++i) {
               cone.surface_texture_offset[i] =
                  {std::min(prop.values.get<uint16>(i * 2 + 0), block_max_texture_offset),
                   std::min(prop.values.get<uint16>(i * 2 + 1), block_max_texture_offset)};
            }
         }
         else if (iequals(prop.key, "Layer")) {
            layer = layer_remap[prop.values.get<int>(0)];
         }
      }

      const math::bounding_box bbox = get_bounding_box(cone);

      blocks_out.cones.bbox.min_x.push_back(bbox.min.x);
      blocks_out.cones.bbox.min_y.push_back(bbox.min.y);
      blocks_out.cones.bbox.min_z.push_back(bbox.min.z);
      blocks_out.cones.bbox.max_x.push_back(bbox.max.x);
      blocks_out.cones.bbox.max_y.push_back(bbox.max.y);
      blocks_out.cones.bbox.max_z.push_back(bbox.max.z);
      blocks_out.cones.hidden.push_back(false);
      blocks_out.cones.layer.push_back(layer);
      blocks_out.cones.description.push_back(cone);
      blocks_out.cones.ids.push_back(blocks_out.next_id.cones.aquire());
   }
}

void load_hemispheres(assets::config::node& node,
                      const layer_remap& layer_remap, blocks& blocks_out)
{
   for (const auto& key_node : node) {
      if (not iequals(key_node.key, "Hemisphere")) continue;

      if (blocks_out.hemispheres.size() == max_blocks) {
         throw load_failure{fmt::format("Too many blocks (of type hemisphere) "
                                        "for WorldEdit to handle. \n   "
                                        "Max Supported Count: {}\n",
                                        max_blocks)};
      }

      block_description_hemisphere hemisphere;
      int8 layer = 0;

      for (const auto& prop : key_node) {
         if (iequals(prop.key, "Rotation")) {
            hemisphere.rotation = {prop.values.get<float>(0),
                                   prop.values.get<float>(1),
                                   prop.values.get<float>(2),
                                   prop.values.get<float>(3)};
         }
         else if (iequals(prop.key, "Position")) {
            hemisphere.position = {prop.values.get<float>(0),
                                   prop.values.get<float>(1),
                                   prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "Size")) {
            hemisphere.size = {prop.values.get<float>(0), prop.values.get<float>(1),
                               prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "SurfaceMaterials")) {
            for (uint32 i = 0; i < hemisphere.surface_materials.size(); ++i) {
               hemisphere.surface_materials[i] = prop.values.get<uint8>(i);
            }
         }
         else if (iequals(prop.key, "SurfaceTextureMode")) {
            for (uint32 i = 0; i < hemisphere.surface_texture_mode.size(); ++i) {
               hemisphere.surface_texture_mode[i] =
                  read_texture_mode(prop.values.get<uint8>(i));
            }
         }
         else if (iequals(prop.key, "SurfaceTextureRotation")) {
            for (uint32 i = 0; i < hemisphere.surface_texture_rotation.size(); ++i) {
               const uint8 rotation = prop.values.get<uint8>(i);

               switch (rotation) {
               case static_cast<uint8>(block_texture_rotation::d0):
               case static_cast<uint8>(block_texture_rotation::d90):
               case static_cast<uint8>(block_texture_rotation::d180):
               case static_cast<uint8>(block_texture_rotation::d270):
                  hemisphere.surface_texture_rotation[i] =
                     block_texture_rotation{rotation};
                  break;
               }
            }
         }
         else if (iequals(prop.key, "SurfaceTextureScale")) {
            for (uint32 i = 0; i < hemisphere.surface_texture_scale.size(); ++i) {
               hemisphere.surface_texture_scale[i] =
                  {std::clamp(prop.values.get<int8>(i * 2 + 0),
                              block_min_texture_scale, block_max_texture_scale),
                   std::clamp(prop.values.get<int8>(i * 2 + 1),
                              block_min_texture_scale, block_max_texture_scale)};
            }
         }
         else if (iequals(prop.key, "SurfaceTextureOffset")) {
            for (uint32 i = 0; i < hemisphere.surface_texture_offset.size(); ++i) {
               hemisphere.surface_texture_offset[i] =
                  {std::min(prop.values.get<uint16>(i * 2 + 0), block_max_texture_offset),
                   std::min(prop.values.get<uint16>(i * 2 + 1), block_max_texture_offset)};
            }
         }
         else if (iequals(prop.key, "Layer")) {
            layer = layer_remap[prop.values.get<int>(0)];
         }
      }

      const math::bounding_box bbox = get_bounding_box(hemisphere);

      blocks_out.hemispheres.bbox.min_x.push_back(bbox.min.x);
      blocks_out.hemispheres.bbox.min_y.push_back(bbox.min.y);
      blocks_out.hemispheres.bbox.min_z.push_back(bbox.min.z);
      blocks_out.hemispheres.bbox.max_x.push_back(bbox.max.x);
      blocks_out.hemispheres.bbox.max_y.push_back(bbox.max.y);
      blocks_out.hemispheres.bbox.max_z.push_back(bbox.max.z);
      blocks_out.hemispheres.hidden.push_back(false);
      blocks_out.hemispheres.layer.push_back(layer);
      blocks_out.hemispheres.description.push_back(hemisphere);
      blocks_out.hemispheres.ids.push_back(blocks_out.next_id.hemispheres.aquire());
   }
}

void load_pyramids(assets::config::node& node, const layer_remap& layer_remap,
                   blocks& blocks_out)
{
   for (const auto& key_node : node) {
      if (not iequals(key_node.key, "Pyramid")) continue;

      if (blocks_out.pyramids.size() == max_blocks) {
         throw load_failure{fmt::format(
            "Too many blocks (of type pyramid) for WorldEdit to handle. \n   "
            "Max Supported Count: {}\n",
            max_blocks)};
      }

      block_description_pyramid pyramid;
      int8 layer = 0;

      for (const auto& prop : key_node) {
         if (iequals(prop.key, "Rotation")) {
            pyramid.rotation = {prop.values.get<float>(0),
                                prop.values.get<float>(1), prop.values.get<float>(2),
                                prop.values.get<float>(3)};
         }
         else if (iequals(prop.key, "Position")) {
            pyramid.position = {prop.values.get<float>(0), prop.values.get<float>(1),
                                prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "Size")) {
            pyramid.size = {prop.values.get<float>(0), prop.values.get<float>(1),
                            prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "SurfaceMaterials")) {
            for (uint32 i = 0; i < pyramid.surface_materials.size(); ++i) {
               pyramid.surface_materials[i] = prop.values.get<uint8>(i);
            }
         }
         else if (iequals(prop.key, "SurfaceTextureMode")) {
            for (uint32 i = 0; i < pyramid.surface_texture_mode.size(); ++i) {
               pyramid.surface_texture_mode[i] =
                  read_texture_mode(prop.values.get<uint8>(i));
            }
         }
         else if (iequals(prop.key, "SurfaceTextureRotation")) {
            for (uint32 i = 0; i < pyramid.surface_texture_rotation.size(); ++i) {
               const uint8 rotation = prop.values.get<uint8>(i);

               switch (rotation) {
               case static_cast<uint8>(block_texture_rotation::d0):
               case static_cast<uint8>(block_texture_rotation::d90):
               case static_cast<uint8>(block_texture_rotation::d180):
               case static_cast<uint8>(block_texture_rotation::d270):
                  pyramid.surface_texture_rotation[i] =
                     block_texture_rotation{rotation};
                  break;
               }
            }
         }
         else if (iequals(prop.key, "SurfaceTextureScale")) {
            for (uint32 i = 0; i < pyramid.surface_texture_scale.size(); ++i) {
               pyramid.surface_texture_scale[i] =
                  {std::clamp(prop.values.get<int8>(i * 2 + 0),
                              block_min_texture_scale, block_max_texture_scale),
                   std::clamp(prop.values.get<int8>(i * 2 + 1),
                              block_min_texture_scale, block_max_texture_scale)};
            }
         }
         else if (iequals(prop.key, "SurfaceTextureOffset")) {
            for (uint32 i = 0; i < pyramid.surface_texture_offset.size(); ++i) {
               pyramid.surface_texture_offset[i] =
                  {std::min(prop.values.get<uint16>(i * 2 + 0), block_max_texture_offset),
                   std::min(prop.values.get<uint16>(i * 2 + 1), block_max_texture_offset)};
            }
         }
         else if (iequals(prop.key, "Layer")) {
            layer = layer_remap[prop.values.get<int>(0)];
         }
      }

      const math::bounding_box bbox = get_bounding_box(pyramid);

      blocks_out.pyramids.bbox.min_x.push_back(bbox.min.x);
      blocks_out.pyramids.bbox.min_y.push_back(bbox.min.y);
      blocks_out.pyramids.bbox.min_z.push_back(bbox.min.z);
      blocks_out.pyramids.bbox.max_x.push_back(bbox.max.x);
      blocks_out.pyramids.bbox.max_y.push_back(bbox.max.y);
      blocks_out.pyramids.bbox.max_z.push_back(bbox.max.z);
      blocks_out.pyramids.hidden.push_back(false);
      blocks_out.pyramids.layer.push_back(layer);
      blocks_out.pyramids.description.push_back(pyramid);
      blocks_out.pyramids.ids.push_back(blocks_out.next_id.pyramids.aquire());
   }
}

void load_materials(assets::config::node& node, blocks& blocks_out,
                    output_stream& output) noexcept
{
   for (const auto& key_node : node) {
      if (not iequals(key_node.key, "Material")) continue;

      const uint32 material_index = key_node.values.get<uint32>(0);

      if (material_index >= max_block_materials) {
         output.write("Block material index '{}' is out of supported range. "
                      "Max index is '{}'. Skipping material.\n",
                      material_index, max_block_materials - 1);
      }

      block_material& material = blocks_out.materials[material_index];

      for (const auto& prop : key_node) {
         if (iequals(prop.key, "Name")) {
            material.name = prop.values.get<std::string>(0);
         }
         else if (iequals(prop.key, "DiffuseMap")) {
            material.diffuse_map = prop.values.get<std::string>(0);
         }
         else if (iequals(prop.key, "NormalMap")) {
            material.normal_map = prop.values.get<std::string>(0);
         }
         else if (iequals(prop.key, "DetailMap")) {
            material.detail_map = prop.values.get<std::string>(0);
         }
         else if (iequals(prop.key, "EnvMap")) {
            material.env_map = prop.values.get<std::string>(0);
         }
         else if (iequals(prop.key, "DetailTiling")) {
            material.detail_tiling = {prop.values.get<uint8>(0),
                                      prop.values.get<uint8>(1)};
         }
         else if (iequals(prop.key, "TileNormalMap")) {
            material.tile_normal_map = prop.values.get<uint8>(0) != 0;
         }
         else if (iequals(prop.key, "SpecularLighting")) {
            material.specular_lighting = prop.values.get<uint8>(0) != 0;
         }
         else if (iequals(prop.key, "SpecularColor")) {
            material.specular_color = {prop.values.get<float>(0),
                                       prop.values.get<float>(1),
                                       prop.values.get<float>(2)};
         }
         else if (iequals(prop.key, "FoleyFXGroup")) {
            const uint8 foley_group = prop.values.get<uint8>(0);

            switch (foley_group) {
            case static_cast<uint8>(block_foley_group::stone):
            case static_cast<uint8>(block_foley_group::dirt):
            case static_cast<uint8>(block_foley_group::grass):
            case static_cast<uint8>(block_foley_group::metal):
            case static_cast<uint8>(block_foley_group::snow):
            case static_cast<uint8>(block_foley_group::terrain):
            case static_cast<uint8>(block_foley_group::water):
            case static_cast<uint8>(block_foley_group::wood):
               material.foley_group = block_foley_group{foley_group};
               break;
            }
         }
      }
   }
}
}

auto load_blocks(const io::path& path, const layer_remap& layer_remap,
                 output_stream& output) -> blocks
{
   try {
      utility::stopwatch load_timer;

      blocks blocks;

      for (auto& key_node :
           assets::config::read_config(io::read_file_to_string(path))) {
         if (iequals(key_node.key, "Boxes")) {
            const std::size_t box_reservation = key_node.values.get<std::size_t>(0);

            blocks.boxes.reserve(box_reservation);

            load_boxes(key_node, layer_remap, blocks);
         }
         else if (iequals(key_node.key, "Ramps")) {
            const std::size_t box_reservation = key_node.values.get<std::size_t>(0);

            blocks.ramps.reserve(box_reservation);

            load_ramps(key_node, layer_remap, blocks);
         }
         else if (iequals(key_node.key, "Quads")) {
            const std::size_t quad_reservation = key_node.values.get<std::size_t>(0);

            blocks.quads.reserve(quad_reservation);

            load_quads(key_node, layer_remap, blocks);
         }
         else if (iequals(key_node.key, "Cylinders")) {
            const std::size_t box_reservation = key_node.values.get<std::size_t>(0);

            blocks.cylinders.reserve(box_reservation);

            load_cylinders(key_node, layer_remap, blocks);
         }
         else if (iequals(key_node.key, "Custom")) {
            const std::size_t box_reservation = key_node.values.get<std::size_t>(0);

            blocks.custom.reserve(box_reservation);

            load_custom(key_node, layer_remap, blocks);
         }
         else if (iequals(key_node.key, "Cones")) {
            const std::size_t box_reservation = key_node.values.get<std::size_t>(0);

            blocks.cones.reserve(box_reservation);

            load_cones(key_node, layer_remap, blocks);
         }
         else if (iequals(key_node.key, "Hemispheres")) {
            const std::size_t box_reservation = key_node.values.get<std::size_t>(0);

            blocks.hemispheres.reserve(box_reservation);

            load_hemispheres(key_node, layer_remap, blocks);
         }
         else if (iequals(key_node.key, "Pyramids")) {
            const std::size_t box_reservation = key_node.values.get<std::size_t>(0);

            blocks.pyramids.reserve(box_reservation);

            load_pyramids(key_node, layer_remap, blocks);
         }
         else if (iequals(key_node.key, "Materials")) {
            load_materials(key_node, blocks, output);
         }
      }

      output.write("Loaded {} (time taken {:f}ms)\n", path.string_view(),
                   load_timer.elapsed_ms());

      blocks.untracked_fill_dirty_ranges();

      return blocks;
   }
   catch (io::error& e) {
      output.write("Error while loading blocks:\n   Blocks: {}\n   "
                   "Message: \n{}\n",
                   path.string_view(), string::indent(2, e.what()));

      throw load_failure{e.what()};
   }
}

}