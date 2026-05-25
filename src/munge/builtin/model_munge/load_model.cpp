#include "load_model.hpp"
#include "build_collision_mesh.hpp"
#include "build_shadow_segments.hpp"
#include "error.hpp"
#include "generate_tangents.hpp"
#include "optimize_segments.hpp"
#include "split_segments.hpp"

#include "assets/msh/error.hpp"
#include "assets/msh/scene_io.hpp"

#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"

#include "utility/string_icompare.hpp"

#include <fmt/format.h>

using namespace we::assets;
using namespace we::string;

namespace we::munge {

namespace {

struct build_context {
   const io::path& path;
   munge_feedback& feedback;
};

constexpr uint32 max_skeleton_bones = 64; // Limit reported by modelmunge
constexpr uint32 min_segment_bones = 3;
constexpr uint32 max_segment_bones = 15;
constexpr uint32 default_segment_bones = max_segment_bones;

constexpr uint32 max_collision_vertices = 0x10000;

auto load_scene(const io::path& path,
                const std::vector<assets::option>& directory_options) -> msh::scene
{
   try {
      return msh::load_scene(path, directory_options);
   }
   catch (msh::read_error& e) {
      throw model_error{e};
   }
   catch (std::exception& e) {
      throw model_error{e.what(), model_ec::msh_unknown};
   }
}

void scale_scene(msh::scene& scene)
{
   const float scale = scene.options.scale;

   for (msh::node& node : scene.nodes) {
      node.transform.translation *= scale;

      for (msh::geometry_segment& segment : node.segments) {
         for (float3& position : segment.positions) {
            position *= scale;
         }
      }

      for (msh::shadow_volume& shadow_volume : node.shadow_volumes) {
         for (float3& position : shadow_volume.positions) {
            position *= scale;
         }
      }

      if (node.collision_primitive) {
         node.collision_primitive->radius *= scale;
         node.collision_primitive->height *= scale;
         node.collision_primitive->length *= scale;
      }

      if (node.cloth) {
         for (float3& position : node.cloth->positions) {
            position *= scale;
         }

         for (msh::cloth_collision_primitive& primitive : node.cloth->collision) {
            primitive.size *= scale;
         }
      }

      for (msh::shadow_volume& shadow_volume : node.shadow_volumes) {
         for (float3& position : shadow_volume.positions) {
            position *= scale;
         }
      }
   }
}

void warning_check_scene(const msh::scene& scene, const build_context& context)
{
   // This functions checks for issues that don't prevent munging but might indicate user mistakes
   // and outputs warnings for them.
   //
   // Only issues that can not be readily caught during regular processing should be checked here.

   for (const std::string& keep_node : scene.options.keep_nodes) {
      bool found_node = false;

      for (const msh::node& node : scene.nodes) {
         if (iequals(keep_node, node.name)) {
            found_node = true;

            break;
         }
      }

      if (not found_node) {
         context.feedback.add_warning(
            {.file = context.path,
             .tool = "ModelMunge",
             .message = fmt::format(
                ".msh missing node ('{}') named in -keep option.\n\n{}", keep_node,
                get_descriptive_message(model_wc::missing_keep_node))});
      }
   }

   for (const std::string& keep_material : scene.options.keep_materials) {
      bool found_node = false;

      for (const msh::node& node : scene.nodes) {
         if (iequals(keep_material, node.name)) {
            found_node = true;

            break;
         }
      }

      if (not found_node) {
         context.feedback.add_warning(
            {.file = context.path,
             .tool = "ModelMunge",
             .message = fmt::format(
                ".msh missing node ('{}') named in -keepmaterial option.\n\n{}", keep_material,
                get_descriptive_message(model_wc::missing_keep_material))});
      }
   }

   for (const msh::node& node : scene.nodes) {
      if (node.name.starts_with("hP_") or node.name.starts_with("Hp_") or
          node.name.starts_with("HP_")) {
         context.feedback.add_warning(
            {.file = context.path,
             .tool = "ModelMunge",
             .message =
                fmt::format(".msh node ('{}') has possible typo.\n\n{}", node.name,
                            get_descriptive_message(model_wc::possible_typo_hp))});
      }
      else if (node.name.starts_with("P_")) {
         context.feedback.add_warning(
            {.file = context.path,
             .tool = "ModelMunge",
             .message =
                fmt::format(".msh node ('{}') has possible typo.\n\n{}", node.name,
                            get_descriptive_message(
                               model_wc::possible_typo_collision_primitive))});
      }
      else if (node.name.starts_with("sV_") or node.name.starts_with("Sv_") or
               node.name.starts_with("SV_")) {
         context.feedback.add_warning(
            {.file = context.path,
             .tool = "ModelMunge",
             .message =
                fmt::format(".msh node ('{}') has possible typo.\n\n{}", node.name,
                            get_descriptive_message(model_wc::possible_typo_shadowvolume))});
      }
   }

   for (const std::string& normal_map : scene.options.normal_maps) {
      bool found_texture = false;

      for (const msh::material& material : scene.materials) {
         if (iequals(normal_map, material.textures[0])) {
            found_texture = true;

            break;
         }
      }

      if (not found_texture) {
         context.feedback.add_warning(
            {.file = context.path,
             .tool = "ModelMunge",
             .message = fmt::format(
                "No .msh material references texture ('{}') named in "
                "-bump option.\n\n{}",
                normal_map, get_descriptive_message(model_wc::missing_bump_map))});
      }
   }
}

auto build_bone_from_parent(const std::size_t node_index, const msh::scene& scene,
                            const std::vector<bool>& keep_node,
                            const std::vector<bool>& keep_as_nameless_node,
                            const std::vector<uint32>& node_parents) -> float4x4
{
   const msh::node& node = scene.nodes[node_index];

   if (not node.parent) return {};

   const float4x4 bone_from_parent{node.transform};

   const std::size_t parent_index = node_parents[node_index];

   if (keep_node[parent_index] and not keep_as_nameless_node[parent_index]) {
      return bone_from_parent;
   }

   const float4x4 parent_from_grandparent =
      build_bone_from_parent(parent_index, scene, keep_node,
                             keep_as_nameless_node, node_parents);

   return parent_from_grandparent * bone_from_parent;
}

auto build_bone_from_vertex(const std::size_t bone_index,
                            const std::vector<skeleton_bone>& bones,
                            const std::vector<uint32>& bone_parents) -> float4x4
{
   const skeleton_bone& bone = bones[bone_index];

   if (bone.parent.empty()) return bone.bone_from_parent;

   const float4x4 parent_from_vertex =
      build_bone_from_vertex(bone_parents[bone_index], bones, bone_parents);

   return parent_from_vertex * bone.bone_from_parent;
}

auto build_node_from_vertex(const std::size_t node_index, const msh::scene& scene,
                            const std::vector<bool>& keep_node,
                            const std::vector<uint32>& node_parents) -> float4x4
{
   const msh::node& node = scene.nodes[node_index];

   if (not node.parent) return {};

   const float4x4 node_from_vertex{node.transform};

   const std::size_t parent_index = node_parents[node_index];

   if (keep_node[parent_index]) return node_from_vertex;

   const float4x4 parent_from_vertex =
      build_node_from_vertex(parent_index, scene, keep_node, node_parents);

   return parent_from_vertex * node_from_vertex;
}

auto build_local_from_node(const std::size_t node_index, const msh::scene& scene,
                           const std::vector<bool>& keep_node,
                           const std::vector<uint32>& node_parents,
                           const bool entered_keep_chain) -> float4x4
{
   const msh::node& node = scene.nodes[node_index];

   if (not node.parent) return {};

   const std::size_t parent_index = node_parents[node_index];

   if (not entered_keep_chain and not keep_node[parent_index]) {
      return build_local_from_node(parent_index, scene, keep_node, node_parents, false);
   }

   const float4x4 node_from_vertex{node.transform};
   const float4x4 parent_from_vertex =
      build_local_from_node(parent_index, scene, keep_node, node_parents, true);

   return parent_from_vertex * node_from_vertex;
}

auto build_local_from_vertex(const std::size_t node_index, const msh::scene& scene,
                             const std::vector<bool>& keep_node,
                             const std::vector<uint32>& node_parents) -> float4x4
{
   const msh::node& node = scene.nodes[node_index];

   if (not node.parent) return {};

   const float4x4 node_from_parent{node.transform};
   const float4x4 parent_from_grandparent =
      build_local_from_vertex(node_parents[node_index], scene, keep_node, node_parents);

   return parent_from_grandparent * node_from_parent;
}

auto build_skeleton(const msh::scene& scene, const build_context& context) -> skeleton

{
   std::vector<bool> keep_node;
   keep_node.resize(scene.nodes.size());

   for (std::size_t i = 0; i < scene.nodes.size(); ++i) {
      const msh::node& node = scene.nodes[i];

      if (scene.options.keep_all or            //
          node.type == msh::node_type::bone or //
          not node.parent or                   //
          istarts_with(node.name, "hp_")) {
         keep_node[i] = true;
      }
      else {
         for (const std::string& keep : scene.options.keep_nodes) {
            if (iequals(node.name, keep)) {
               keep_node[i] = true;

               break;
            }
         }
      }
   }

   // Nodes that aren't bones, hardpoints or explicitly kept but are used in
   // bone maps are turned into unparented, nameless nodes with identity
   // transforms by modelmunge.
   //
   // This is odd behaviour and is probably a bug in modelmunge. But for the
   // sake of compatibility we warn about it and do our best to mimic
   // modelmunge's behaviour.
   std::vector<bool> keep_as_nameless_node;
   keep_as_nameless_node.resize(scene.nodes.size());

   for (const msh::node& node : scene.nodes) {
      for (const uint32 bone_index : node.bone_map) {
         if (keep_node[bone_index]) continue;

         keep_as_nameless_node[bone_index] = true;

         context.feedback.add_warning(
            {.file = context.path,
             .tool = "ModelMunge",
             .message = fmt::format(
                "Node '{}' uses unkept node '{}' in it's bone map.\n\n{}",
                node.name, scene.nodes[bone_index].name,
                get_descriptive_message(model_wc::bone_map_used_unkept_node))});
      }
   }

   std::vector<uint32> node_parents;
   node_parents.resize(scene.nodes.size());

   for (std::size_t node_index = 0; node_index < scene.nodes.size(); ++node_index) {
      const msh::node& node = scene.nodes[node_index];

      if (not node.parent) continue;

      for (uint32 parent_index = 0; parent_index < scene.nodes.size(); ++parent_index) {
         const msh::node& parent = scene.nodes[parent_index];

         if (*node.parent == parent.name) {
            node_parents[node_index] = parent_index;

            break;
         }
      }
   }

   std::vector<uint32> node_parent_remap;
   node_parent_remap.resize(scene.nodes.size());

   for (std::size_t node_index = 0; node_index < scene.nodes.size(); ++node_index) {
      uint32 parent_index = node_parents[node_index];

      while (not keep_node[parent_index]) {
         parent_index = node_parents[parent_index];
      }

      node_parent_remap[node_index] = parent_index;
   }

   std::size_t bone_count = 0;

   for (std::size_t i = 0; i < scene.nodes.size(); ++i) {
      if (keep_node[i] or keep_as_nameless_node[i]) bone_count += 1;
   }

   skeleton skeleton;
   skeleton.bones.reserve(bone_count);
   skeleton.bone_remap.resize(scene.nodes.size());
   skeleton.node_parent_remap.resize(scene.nodes.size());

   for (std::size_t i = 0; i < scene.nodes.size(); ++i) {
      if (not keep_node[i]) continue;

      const msh::node& node = scene.nodes[i];

      const float4x4 bone_from_parent =
         build_bone_from_parent(i, scene, keep_node, keep_as_nameless_node, node_parents);

      skeleton.bone_remap[i] = static_cast<uint8>(skeleton.bones.size());
      skeleton.bones.push_back({
         .name = node.name,
         .parent = node.parent ? scene.nodes[node_parent_remap[i]].name : "",
         .bone_from_parent = node.parent ? bone_from_parent : float4x4{},
      });
   }

   for (std::size_t i = 0; i < scene.nodes.size(); ++i) {
      if (keep_node[i] or not keep_as_nameless_node[i]) continue;

      skeleton.bone_remap[i] = static_cast<uint8>(skeleton.bones.size());
      skeleton.bones.push_back({});
   }

   std::vector<uint32> bone_parents;
   bone_parents.resize(bone_count);

   for (std::size_t bone_index = 0; bone_index < skeleton.bones.size(); ++bone_index) {
      const skeleton_bone& bone = skeleton.bones[bone_index];

      if (bone.parent.empty()) continue;

      for (uint32 parent_index = 0; parent_index < skeleton.bones.size();
           ++parent_index) {
         const skeleton_bone& parent = skeleton.bones[parent_index];

         if (bone.parent == parent.name) {
            bone_parents[bone_index] = parent_index;

            break;
         }
      }
   }

   for (std::size_t i = 0; i < skeleton.bones.size(); ++i) {
      skeleton_bone& bone = skeleton.bones[i];

      if (bone.parent.empty()) continue;

      const float4x4 bone_from_vertex =
         build_bone_from_vertex(i, skeleton.bones, bone_parents);

      float4x4 vertex_from_bone =
         transpose(float4x4{bone_from_vertex[0], bone_from_vertex[1],
                            bone_from_vertex[2], float4{0.0f, 0.0f, 0.0f, 1.0f}});
      vertex_from_bone[3] =
         vertex_from_bone * float4{-bone_from_vertex[3].x, -bone_from_vertex[3].y,
                                   -bone_from_vertex[3].z, 1.0f};

      bone.bone_from_vertex = bone_from_vertex;
      bone.vertex_from_bone = vertex_from_bone;
   }

   for (std::size_t i = 0; i < skeleton.node_parent_remap.size(); ++i) {
      skeleton.node_parent_remap[i] = skeleton.bone_remap[node_parent_remap[i]];
   }

   if (skeleton.bones.size() > max_skeleton_bones) {
      throw model_error{fmt::format("Too many bones ({}) max is {}!",
                                    skeleton.bones.size(), max_skeleton_bones),
                        model_ec::skeleton_too_many_bones};
   }

   skeleton.node_from_vertex.resize(scene.nodes.size());
   skeleton.local_from_vertex.resize(scene.nodes.size());
   skeleton.local_from_node.resize(scene.nodes.size());
   skeleton.node_from_parent.resize(scene.nodes.size());

   for (std::size_t node_index = 0; node_index < scene.nodes.size(); ++node_index) {
      skeleton.node_from_vertex[node_index] =
         build_node_from_vertex(node_index, scene, keep_node, node_parents);
      skeleton.local_from_vertex[node_index] =
         build_local_from_vertex(node_index, scene, keep_node, node_parents);
      skeleton.local_from_node[node_index] =
         build_local_from_node(node_index, scene, keep_node, node_parents, false);
      skeleton.node_from_parent[node_index] =
         build_bone_from_parent(node_index, scene, keep_node,
                                keep_as_nameless_node, node_parents);
   }

   return skeleton;
}

bool is_geometry_node(const msh::node& node)
{
   if (node.hidden and                        //
       not icontains(node.name, "lod2") and   //
       not icontains(node.name, "lod3") and   //
       not icontains(node.name, "lowres") and //
       not icontains(node.name, "lowrez")) {
      return false;
   }
   if (node.type == msh::node_type::cloth) return false;

   if (node.name.starts_with("sv_")) return false;
   if (node.name.starts_with("p_")) return false;
   if (node.name.starts_with("hp_")) return false;

   if (istarts_with(node.name, "collision")) return false;
   if (istarts_with(node.name, "terraincutter")) return false;

   if (node.segments.empty()) return false;

   return true;
}

bool is_shadow_node(const msh::node& node)
{
   if (node.type == msh::node_type::cloth) return false;

   if (node.name.starts_with("p_")) return false;
   if (node.name.starts_with("hp_")) return false;

   if (istarts_with(node.name, "collision")) return false;
   if (istarts_with(node.name, "terraincutter")) return false;

   if (node.name.starts_with("sv_")) return true;
   if (not node.hidden and not node.shadow_volumes.empty()) return true;

   return false;
}

bool material_needs_tangents(const model_segment& segment) noexcept
{
   const material_flags normal_map_flags =
      material_flags::perpixel | material_flags::envmapped | material_flags::envmapped;

   return (segment.material.flags & normal_map_flags) != material_flags::none and
          segment.material.render_type == "Normal" and
          not segment.material.textures[1].empty();
}

auto build_material(const msh::scene& scene, const msh::node& node,
                    const msh::geometry_segment& geometry,
                    const build_context& context) -> material
{
   const msh::material& msh_material = scene.materials[geometry.material_index];

   material material = {
      .flags = material_flags::none,
      .param0 = msh_material.data0,
      .param1 = msh_material.data1,
      .render_type = "Normal",
      .textures = msh_material.textures,
   };

   if (are_flags_set(msh_material.flags, msh::material_flags::unlit) and
       scene.options.additive_emissive) {
      material.flags |= material_flags::additive;
   }

   if (are_flags_set(msh_material.flags, msh::material_flags::glow)) {
      material.flags |= material_flags::glow;
   }

   if (not are_flags_set(msh_material.flags, msh::material_flags::unlit) and
       not are_flags_set(msh_material.flags, msh::material_flags::glow)) {
      material.flags |= material_flags::lit;
   }

   if (are_flags_set(msh_material.flags, msh::material_flags::transparent)) {
      material.flags |= material_flags::transparent;
   }

   if (are_flags_set(msh_material.flags, msh::material_flags::transparent_doublesided)) {
      material.flags |= material_flags::transparent;
      material.flags |= material_flags::doublesided;
   }

   if (are_flags_set(msh_material.flags, msh::material_flags::hardedged)) {
      material.flags &= ~material_flags::transparent;
      material.flags |= material_flags::alpha_cutout;
   }

   if (are_flags_set(msh_material.flags, msh::material_flags::perpixel)) {
      material.flags |= material_flags::perpixel;
   }

   if (are_flags_set(msh_material.flags, msh::material_flags::additive)) {
      material.flags |= material_flags::additive;
   }

   if (are_flags_set(msh_material.flags, msh::material_flags::specular)) {
      material.flags |= material_flags::specular;

      if (not are_flags_set(material.flags, material_flags::transparent)) {
         material.flags |= material_flags::gloss_map;
      }
   }

   switch (msh_material.rendertype) {
   case msh::rendertype::normal: {
      material.textures[1] = "";
   } break;
   case msh::rendertype::scrolling: {
      material.flags |= material_flags::scroll;
   } break;
   case msh::rendertype::specular: {
      material.flags |= material_flags::specular;
      material.textures[1] = "";

      if (not are_flags_set(material.flags, material_flags::transparent)) {
         material.flags |= material_flags::gloss_map;
      }
   } break;
   case msh::rendertype::envmapped: {
      material.flags |= material_flags::envmapped;
   } break;
   case msh::rendertype::animated: {
      material.flags |= material_flags::animated;
   } break;
   case msh::rendertype::detail: {
      material.textures[2] = std::exchange(material.textures[1], "");
   } break;
   case msh::rendertype::refraction: {
      material.render_type = "Refraction";
   } break;
   case msh::rendertype::normalmap_tiled: {
      material.flags |= material_flags::perpixel;
      material.flags |= material_flags::normal_map_tile;
   } break;
   case msh::rendertype::blinking: {
      material.flags |= material_flags::blink;
   } break;
   case msh::rendertype::normalmap_envmapped: {
      material.flags |= material_flags::perpixel;
      material.flags |= material_flags::envmapped;
   } break;
   case msh::rendertype::normalmap: {
      material.flags |= material_flags::perpixel;
   } break;
   case msh::rendertype::normalmap_specular: {
      material.flags |= material_flags::perpixel;
      material.flags |= material_flags::specular;

      if (not are_flags_set(material.flags, material_flags::transparent)) {
         material.flags |= material_flags::gloss_map;
      }
   } break;
   case msh::rendertype::normalmap_tiled_envmapped: {
      material.flags |= material_flags::perpixel;
      material.flags |= material_flags::envmapped;
      material.flags |= material_flags::normal_map_tile;
   } break;
   default: {
      material.render_type = "*";

      context.feedback.add_warning(
         {.file = context.path,
          .tool = "ModelMunge",
          .message =
             fmt::format(".msh material '{}' invalid rendertype '{:X}'!\n\n{}",
                         msh_material.name, static_cast<uint8>(msh_material.rendertype),
                         get_descriptive_message(model_wc::material_invalid_rendertype))});
   } break;
   }

   for (const std::string& normal_map : scene.options.normal_maps) {
      if (iequals(normal_map, material.textures[0])) {
         material.flags |= material_flags::perpixel;

         material.textures[1] = fmt::format("{}_bump", material.textures[0]);
         material.render_type = "Normal";

         break;
      }
   }

   if (scene.options.vertex_lighting and geometry.colors) {
      material.flags |= material_flags::vertex_lit;
   }

   material.specular_color =
      static_cast<uint32>(msh_material.specular_color.x * 255.0f) << 16u;
   material.specular_color |=
      static_cast<uint32>(msh_material.specular_color.y * 255.0f) << 8u;
   material.specular_color |=
      static_cast<uint32>(msh_material.specular_color.z * 255.0f) << 0u;
   material.specular_color |=
      static_cast<uint32>(msh_material.specular_color.w * 255.0f) << 24u;

   for (const msh::scene_option_attach_light& attachment : scene.options.attach_lights) {
      if (iequals(node.name, attachment.node_name)) {
         material.flags |= material_flags::attached_light;
         material.attached_light = attachment.light_name;
         material.name = node.name;

         break;
      }
   }

   for (const std::string& keep_material : scene.options.keep_materials) {
      if (iequals(keep_material, node.name)) {
         material.name = node.name;

         break;
      }
   }

   if (istarts_with(node.name, "reflective_")) {
      material.flags |= material_flags::reflective;
   }
   else if (istarts_with(node.name, "reflected_")) {
      material.flags |= material_flags::reflected;
   }

   return material;
}

void apply_inverse_bind_pose(const std::size_t node_index,
                             const skeleton& skeleton, model_segment& segment)
{
   if (segment.vertices.vertex_count < 1) return;

   const float4x4& local_from_node = skeleton.local_from_node[node_index];

   for (std::size_t i = 0; i < segment.vertices.vertex_count; ++i) {
      const float4x4& vertex_from_bone =
         skeleton.bones[segment.bone_map[segment.vertices.bone_indices[i][0]]].vertex_from_bone;

      segment.vertices.positionSS[i] =
         vertex_from_bone * segment.vertices.positionSS[i];
      segment.vertices.positionLS[i] =
         local_from_node * segment.vertices.positionSS[i];
      segment.vertices.normalSS[i] =
         float3x3{vertex_from_bone} * segment.vertices.normalSS[i];

      if (segment.vertices.tangents) {
         segment.vertices.tangents[i].tangentSS =
            float3x3{vertex_from_bone} * segment.vertices.tangents[i].tangentSS;
         segment.vertices.tangents[i].bitangentSS =
            float3x3{vertex_from_bone} * segment.vertices.tangents[i].bitangentSS;
      }
   }

   segment.bboxSS.min = segment.vertices.positionSS[0];
   segment.bboxSS.max = segment.bboxSS.min;

   for (std::size_t i = 1; i < segment.vertices.vertex_count; ++i) {
      const float3& positionSS = segment.vertices.positionSS[i];

      segment.bboxSS.min = min(positionSS, segment.bboxSS.min);
      segment.bboxSS.max = max(positionSS, segment.bboxSS.max);
   }
}

void apply_inverse_bind_pose(const std::size_t node_index,
                             const skeleton& skeleton, model_shadow& segment)
{
   if (segment.vertices.type != model_shadow_vertex_type::hard_skinned) return;
   if (segment.vertices.hard_skinned.size() < 1) return;

   const float4x4& local_from_node = skeleton.local_from_node[node_index];

   for (model_shadow_hard_skinned_vertex& vertex : segment.vertices.hard_skinned) {

      for (std::size_t i = 0; i < vertex.positionSS.size(); ++i) {
         const float4x4& vertex_from_bone =
            skeleton.bones[segment.bone_map[vertex.bone_indices[i]]].vertex_from_bone;

         vertex.positionSS[i] = vertex_from_bone * vertex.positionSS[i];
      }

      vertex.positionLS = local_from_node * vertex.positionSS[0];
   }

   segment.bboxSS.min = segment.vertices.hard_skinned[0].positionSS[0];
   segment.bboxSS.max = segment.bboxSS.min;

   for (std::size_t i = 1; i < segment.vertices.hard_skinned.size(); ++i) {
      const float3& positionSS = segment.vertices.hard_skinned[i].positionSS[0];

      segment.bboxSS.min = min(positionSS, segment.bboxSS.min);
      segment.bboxSS.max = max(positionSS, segment.bboxSS.max);
   }
}

void build_segments(const std::size_t node_index, const msh::scene& scene,
                    const skeleton& skeleton, const uint32 target_segment_bones,
                    const build_context& context, std::vector<model_segment>& out)
{
   assert(node_index < scene.nodes.size());

   const msh::node& node = scene.nodes[node_index];

   for (const msh::geometry_segment& geometry : node.segments) {
      model_segment segment = {
         .material = build_material(scene, node, geometry, context),
      };

      segment.index_buffer = geometry.triangles;

      const std::size_t vertex_count = geometry.positions.size();

      if (vertex_count == 0) continue;

      segment.vertices.vertex_count = vertex_count;
      segment.vertices.positionSS =
         std::make_unique_for_overwrite<float3[]>(vertex_count);
      segment.vertices.positionLS =
         std::make_unique_for_overwrite<float3[]>(vertex_count);

      if (geometry.weights) {
         if (scene.options.soft_skin) {
            segment.vertices.bone_weights =
               std::make_unique_for_overwrite<float3[]>(vertex_count);
            segment.vertices.bone_indices =
               std::make_unique_for_overwrite<std::array<uint8, 3>[]>(vertex_count);
         }
         else {
            segment.vertices.bone_indices =
               std::make_unique_for_overwrite<std::array<uint8, 3>[]>(vertex_count);
         }
      }

      segment.vertices.normalSS =
         std::make_unique_for_overwrite<float3[]>(vertex_count);

      if (geometry.colors) {
         segment.vertices.color =
            std::make_unique_for_overwrite<uint32[]>(vertex_count);
      }

      segment.vertices.texcoords =
         std::make_unique_for_overwrite<float2[]>(vertex_count);

      const float4x4& node_from_vertex = skeleton.node_from_vertex[node_index];
      const float4x4& local_from_vertex = skeleton.local_from_vertex[node_index];

      for (std::size_t i = 0; i < vertex_count; ++i) {
         segment.vertices.positionSS[i] = node_from_vertex * geometry.positions[i];
         segment.vertices.positionLS[i] = local_from_vertex * geometry.positions[i];

         if (geometry.weights) {
            const std::array<msh::vertex_weight, 4>& weight = (*geometry.weights)[i];

            if (scene.options.soft_skin) {
               float3 weights = {weight[0].weight, weight[1].weight, weight[2].weight};
               const float total_weight = weights.x + weights.y + weights.z;

               if (total_weight == 0.0f) {
                  weights = {1.0f, 0.0f, 0.0f};
               }
               else {
                  weights /= total_weight;
               }

               segment.vertices.bone_weights[i] = weights;
               segment.vertices.bone_indices[i] =
                  {static_cast<uint8>(weight[0].bone_index),
                   static_cast<uint8>(weights.y > 0.0f ? weight[1].bone_index
                                                       : weight[0].bone_index),
                   static_cast<uint8>(weights.z > 0.0f ? weight[2].bone_index
                                                       : weight[0].bone_index)};
            }
            else {
               segment.vertices
                  .bone_indices[i] = {static_cast<uint8>(weight[0].bone_index),
                                      static_cast<uint8>(weight[0].bone_index),
                                      static_cast<uint8>(weight[0].bone_index)};
            }
         }

         if (geometry.normals) {
            segment.vertices.normalSS[i] =
               float3x3{node_from_vertex} * (*geometry.normals)[i];
         }
         else {
            segment.vertices.normalSS[i] = {};
         }

         if (geometry.texcoords) {
            segment.vertices.texcoords[i] = (*geometry.texcoords)[i];
         }
         else {
            segment.vertices.texcoords[i] = {};
         }

         if (geometry.colors) {
            segment.vertices.color[i] = (*geometry.colors)[i];
         }
      }

      segment.bboxLS.min = segment.vertices.positionLS[0];
      segment.bboxLS.max = segment.bboxLS.min;

      for (std::size_t i = 1; i < vertex_count; ++i) {
         const float3 positionLS = segment.vertices.positionLS[i];

         segment.bboxLS.min = min(positionLS, segment.bboxLS.min);
         segment.bboxLS.max = max(positionLS, segment.bboxLS.max);
      }

      segment.bboxSS.min = segment.vertices.positionSS[0];
      segment.bboxSS.max = segment.bboxSS.min;

      for (std::size_t i = 1; i < vertex_count; ++i) {
         const float3& positionSS = segment.vertices.positionSS[i];

         segment.bboxSS.min = min(positionSS, segment.bboxSS.min);
         segment.bboxSS.max = max(positionSS, segment.bboxSS.max);
      }

      if (segment.vertices.bone_indices) {
         segment.bone_map.reserve(node.bone_map.size());

         for (uint32 node_bone_index : node.bone_map) {
            segment.bone_map.push_back(skeleton.bone_remap[node_bone_index]);
         }
      }
      else {
         segment.bone_name =
            skeleton.bones[skeleton.node_parent_remap[node_index]].name;
      }

      if (scene.options.ambient_lighting and scene.options.vertex_lighting and
          segment.vertices.color) {
         const float3& ambient_lighting = *scene.options.ambient_lighting;

         for (std::size_t i = 0; i < segment.vertices.vertex_count; ++i) {
            uint32 packed_color = segment.vertices.color[i];

            float3 color = {
               ((packed_color >> 16) & 0xff) / 255.0f,
               ((packed_color >> 8) & 0xff) / 255.0f,
               ((packed_color) & 0xff) / 255.0f,
            };

            color = saturate(color + ambient_lighting);

            packed_color &= 0xff'00'00'00u;
            packed_color |= static_cast<uint32>(color.x * 255.0f) << 16;
            packed_color |= static_cast<uint32>(color.y * 255.0f) << 8;
            packed_color |= static_cast<uint32>(color.z * 255.0f);

            segment.vertices.color[i] = packed_color;
         }
      }

      const bool need_tangents = material_needs_tangents(segment);
      const bool need_apply_inverse_bind_pose =
         segment.vertices.bone_indices and not segment.vertices.bone_weights;

      if (segment.bone_map.size() > target_segment_bones) {
         for (model_segment& split_segment :
              split_skinned_segments(segment, target_segment_bones)) {
            if (need_tangents) {
               for (model_segment& tangents_segment : generate_tangents(segment)) {
                  if (need_apply_inverse_bind_pose) {
                     apply_inverse_bind_pose(node_index, skeleton, tangents_segment);
                  }

                  out.push_back(std::move(tangents_segment));
               }
            }
            else {
               if (need_apply_inverse_bind_pose) {
                  apply_inverse_bind_pose(node_index, skeleton, split_segment);
               }

               out.push_back(std::move(split_segment));
            }
         }
      }
      else {
         if (need_tangents) {
            for (model_segment& tangents_segment : generate_tangents(segment)) {
               if (need_apply_inverse_bind_pose) {
                  apply_inverse_bind_pose(node_index, skeleton, tangents_segment);
               }

               out.push_back(std::move(tangents_segment));
            }
         }
         else {
            if (need_apply_inverse_bind_pose) {
               apply_inverse_bind_pose(node_index, skeleton, segment);
            }

            out.push_back(std::move(segment));
         }
      }
   }
}

void build_shadow_segments(const std::size_t node_index, const msh::scene& scene,
                           const skeleton& skeleton, const uint32 target_segment_bones,
                           const build_context& context,
                           std::vector<model_shadow>& out)
{
   assert(node_index < scene.nodes.size());

   const msh::node& node = scene.nodes[node_index];

   if (not node.name.starts_with("sv_") and not node.shadow_volumes.empty()) {
      for (const msh::shadow_volume& shadow_volume : node.shadow_volumes) {
         build_shadow_volume_vertices vertices;
         vertices = {
            .vertex_count = shadow_volume.positions.size(),

            .positionSS = std::make_unique_for_overwrite<float3[]>(
               shadow_volume.positions.size()),
            .positionLS = std::make_unique_for_overwrite<float3[]>(
               shadow_volume.positions.size()),
         };

         const float4x4& node_from_vertex = skeleton.node_from_vertex[node_index];
         const float4x4& local_from_vertex = skeleton.local_from_vertex[node_index];

         for (std::size_t i = 0; i < vertices.vertex_count; ++i) {
            vertices.positionSS[i] = node_from_vertex * shadow_volume.positions[i];
            vertices.positionLS[i] = local_from_vertex * shadow_volume.positions[i];
         }

         for (model_shadow& shadow_segment : build_shadow_segments(
                 shadow_volume.edges, std::move(vertices),
                 skeleton.bones[skeleton.node_parent_remap[node_index]].name)) {
            out.push_back(std::move(shadow_segment));
         }
      }
   }
   else {
      for (const msh::geometry_segment& geometry : node.segments) {
         model_segment segment = {};

         segment.index_buffer = geometry.triangles;

         const std::size_t vertex_count = geometry.positions.size();

         if (vertex_count == 0) continue;

         segment.vertices.vertex_count = vertex_count;
         segment.vertices.positionSS =
            std::make_unique_for_overwrite<float3[]>(vertex_count);
         segment.vertices.positionLS =
            std::make_unique_for_overwrite<float3[]>(vertex_count);

         if (geometry.weights) {
            if (scene.options.soft_skin_shadow) {
               segment.vertices.bone_weights =
                  std::make_unique_for_overwrite<float3[]>(vertex_count);
               segment.vertices.bone_indices =
                  std::make_unique_for_overwrite<std::array<uint8, 3>[]>(vertex_count);
            }
            else {
               segment.vertices.bone_indices =
                  std::make_unique_for_overwrite<std::array<uint8, 3>[]>(vertex_count);
            }
         }

         const float4x4& node_from_vertex = skeleton.node_from_vertex[node_index];
         const float4x4& local_from_vertex = skeleton.local_from_vertex[node_index];

         for (std::size_t i = 0; i < vertex_count; ++i) {
            segment.vertices.positionSS[i] =
               node_from_vertex * geometry.positions[i];
            segment.vertices.positionLS[i] =
               local_from_vertex * geometry.positions[i];

            if (geometry.weights) {
               const std::array<msh::vertex_weight, 4>& weight =
                  (*geometry.weights)[i];

               if (scene.options.soft_skin_shadow) {
                  float3 weights = {weight[0].weight, weight[1].weight,
                                    weight[2].weight};
                  const float total_weight = weights.x + weights.y + weights.z;

                  if (total_weight == 0.0f) {
                     weights = {1.0f, 0.0f, 0.0f};
                  }
                  else {
                     weights /= total_weight;
                  }

                  segment.vertices.bone_weights[i] = weights;
                  segment.vertices.bone_indices[i] =
                     {static_cast<uint8>(weight[0].bone_index),
                      static_cast<uint8>(weights.y > 0.0f ? weight[1].bone_index
                                                          : weight[0].bone_index),
                      static_cast<uint8>(weights.z > 0.0f ? weight[2].bone_index
                                                          : weight[0].bone_index)};
               }
               else {
                  segment.vertices.bone_indices[i] =
                     {static_cast<uint8>(weight[0].bone_index),
                      static_cast<uint8>(weight[0].bone_index),
                      static_cast<uint8>(weight[0].bone_index)};
               }
            }
         }

         segment.bboxLS.min = segment.vertices.positionLS[0];
         segment.bboxLS.max = segment.bboxLS.min;

         for (std::size_t i = 1; i < vertex_count; ++i) {
            const float3 positionLS = segment.vertices.positionLS[i];

            segment.bboxLS.min = min(positionLS, segment.bboxLS.min);
            segment.bboxLS.max = max(positionLS, segment.bboxLS.max);
         }

         segment.bboxSS.min = segment.vertices.positionSS[0];
         segment.bboxSS.max = segment.bboxSS.min;

         for (std::size_t i = 1; i < vertex_count; ++i) {
            const float3& positionSS = segment.vertices.positionSS[i];

            segment.bboxSS.min = min(positionSS, segment.bboxSS.min);
            segment.bboxSS.max = max(positionSS, segment.bboxSS.max);
         }

         if (segment.vertices.bone_indices) {
            segment.bone_map.reserve(node.bone_map.size());

            for (uint32 node_bone_index : node.bone_map) {
               segment.bone_map.push_back(skeleton.bone_remap[node_bone_index]);
            }
         }
         else {
            segment.bone_name =
               skeleton.bones[skeleton.node_parent_remap[node_index]].name;
         }

         for (model_shadow& shadow_segment :
              build_shadow_segments(segment, {.max_bones = target_segment_bones,
                                              .path = context.path,
                                              .feedback = context.feedback})) {
            if (shadow_segment.vertices.type == model_shadow_vertex_type::hard_skinned) {
               apply_inverse_bind_pose(node_index, skeleton, shadow_segment);
            }

            out.push_back(std::move(shadow_segment));
         }
      }
   }
}

bool has_pre_inverse_transformed_vertices(const model& model, const msh::scene& scene)
{
   if (scene.options.soft_skin) return false;

   for (const model_segment& segment : model.segments) {
      if (segment.vertices.bone_indices) return true;
   }

   return false;
}

auto build_model(const model_lod lod, const std::span<const std::size_t> model_nodes,
                 const std::span<const std::size_t> shadow_nodes,
                 const msh::scene& scene, const skeleton& skeleton,
                 const build_context& context) -> model
{
   model model{
      .lod = lod,
      .node_name = scene.nodes[0].name,

      .no_projection_lights = scene.options.no_projection_lights,
      .vertex_lighting = scene.options.vertex_lighting,
   };

   const uint32 requested_segment_bones =
      scene.options.max_bones != 0 ? scene.options.max_bones : default_segment_bones;
   const uint32 target_segment_bones =
      std::clamp(requested_segment_bones, min_segment_bones, max_segment_bones);

   for (std::size_t node_index : model_nodes) {
      build_segments(node_index, scene, skeleton, target_segment_bones, context,
                     model.segments);
   }

   for (std::size_t node_index : shadow_nodes) {
      build_shadow_segments(node_index, scene, skeleton, target_segment_bones,
                            context, model.shadows);
   }

   model.pre_inverse_transformed_vertices =
      has_pre_inverse_transformed_vertices(model, scene);

   if (not model.segments.empty()) {
      model.vertex_box = model.segments[0].bboxSS;
      model.visibility_box = model.segments[0].bboxLS;

      for (std::size_t i = 1; i < model.segments.size(); ++i) {
         model.vertex_box = math::combine(model.vertex_box, model.segments[i].bboxSS);
         model.visibility_box =
            math::combine(model.visibility_box, model.segments[i].bboxLS);
      }
   }

   if (not model.shadows.empty()) {
      if (model.segments.empty()) {
         model.vertex_box = model.shadows[0].bboxSS;
         model.visibility_box = model.shadows[0].bboxLS;
      }

      for (const model_shadow& shadow : model.shadows) {
         model.vertex_box = math::combine(model.vertex_box, shadow.bboxSS);
         model.visibility_box = math::combine(model.visibility_box, shadow.bboxLS);
      }
   }

   const float3 model_centreLS =
      (model.visibility_box.min + model.visibility_box.max) / 2.0f;
   float model_radius_sq = 0.0f;

   for (const model_segment& segment : model.segments) {
      for (uint32 i = 0; i < segment.vertices.vertex_count; ++i) {
         const float3 vec = segment.vertices.positionLS[i] - model_centreLS;

         model_radius_sq = std::max(model_radius_sq, dot(vec, vec));
      }
   }

   for (const model_shadow& shadow : model.shadows) {
      switch (shadow.vertices.type) {
      case model_shadow_vertex_type::unskinned: {
         for (const model_shadow_unskinned_vertex& vertex : shadow.vertices.unskinned) {
            const float3 vec = vertex.positionLS - model_centreLS;

            model_radius_sq = std::max(model_radius_sq, dot(vec, vec));
         }
      } break;
      case model_shadow_vertex_type::hard_skinned: {
         for (const model_shadow_hard_skinned_vertex& vertex :
              shadow.vertices.hard_skinned) {
            const float3 vec = vertex.positionLS - model_centreLS;

            model_radius_sq = std::max(model_radius_sq, dot(vec, vec));
         }
      } break;
      case model_shadow_vertex_type::soft_skinned: {
         for (const model_shadow_soft_skinned_vertex& vertex :
              shadow.vertices.soft_skinned) {
            const float3 vec = vertex.positionLS - model_centreLS;

            model_radius_sq = std::max(model_radius_sq, dot(vec, vec));
         }
      } break;
      }
   }

   model.bounding_sphere = {model_centreLS + scene.options.bounding_box_offset,
                            sqrt(model_radius_sq) * scene.options.bounding_box_scale};

   for (const model_segment& segment : model.segments) {
      model.total_triangle_count += static_cast<uint32>(segment.index_buffer.size());
   }

   optimize_segments(model.segments, {.max_bones = target_segment_bones});
   optimize_segments(model.shadows);

   return model;
}

auto build_models(const msh::scene& scene, const skeleton& skeleton,
                  const build_context& context) -> std::vector<model>
{
   std::vector<std::size_t> shadow;
   std::vector<std::size_t> lod0;
   std::vector<std::size_t> lod1;
   std::vector<std::size_t> lod2;
   std::vector<std::size_t> lod_lowd;

   for (std::size_t i = 0; i < scene.nodes.size(); ++i) {
      const msh::node& node = scene.nodes[i];

      if (is_geometry_node(node)) {
         if (icontains(node.name, "lod2")) {
            lod1.push_back(i);
         }
         else if (icontains(node.name, "lod3")) {
            lod2.push_back(i);
         }
         else if (icontains(node.name, "lowres") or
                  icontains(node.name, "lowrez")) {
            lod_lowd.push_back(i);
         }
         else {
            lod0.push_back(i);
         }
      }
      else if (is_shadow_node(node)) {
         shadow.push_back(i);
      }
   }

   if (scene.options.high_res_shadow) {
      if (scene.options.high_res_shadow_lod >= 3 and not lod_lowd.empty()) {
         shadow = lod_lowd;
      }
      else if (scene.options.high_res_shadow_lod >= 2 and not lod2.empty()) {
         shadow = lod2;
      }
      else if (scene.options.high_res_shadow_lod >= 1 and not lod1.empty()) {
         shadow = lod1;
      }
      else {
         shadow = lod0;
      }
   }

   std::vector<model> models;

   if (not lod0.empty()) {
      models.push_back(
         build_model(model_lod::lod0, lod0, shadow, scene, skeleton, context));
   }

   if (not lod1.empty()) {
      models.push_back(build_model(model_lod::lod1, lod1, {}, scene, skeleton, context));
   }

   if (not lod2.empty()) {
      models.push_back(build_model(model_lod::lod2, lod2, {}, scene, skeleton, context));
   }

   if (not lod2.empty()) {
      models.push_back(
         build_model(model_lod::lowd, lod_lowd, {}, scene, skeleton, context));
   }

   return models;
}

auto build_game_model(const msh::scene& scene) -> game_model
{
   return {
      .no_game_model = scene.options.no_game_model,
      .lod_group = static_cast<uint32>(scene.options.lod_group),
      .lod_bias = scene.options.lod_bias,
   };
}

auto build_cloth(const msh::scene& scene, const msh::node& node) -> cloth
{
   cloth out = {
      .name = node.name,
      .parent = node.parent.value_or(""),
      .transform = float4x4{node.transform},
   };

   if (not node.cloth) return out;

   const msh::cloth& cloth = *node.cloth;

   out.texture_name = cloth.texture_name;

   const std::size_t vertex_count = cloth.positions.size();

   out.vertices.vertex_count = vertex_count;
   out.vertices.position = std::make_unique_for_overwrite<float3[]>(vertex_count);
   out.vertices.texcoords = std::make_unique_for_overwrite<float2[]>(vertex_count);

   std::vector<bool> fixed_vertex;
   fixed_vertex.resize(vertex_count, false);

   for (uint32 fixed_index : cloth.fixed_indices) {
      fixed_vertex[fixed_index] = true;
   }

   std::vector<uint32> vertex_remap;
   vertex_remap.resize(vertex_count);

   uint32 vertex_offset = 0;

   for (uint32 fixed_index : cloth.fixed_indices) {
      vertex_remap[fixed_index] = vertex_offset;
      vertex_offset += 1;
   }

   for (std::size_t i = 0; i < vertex_count; ++i) {
      if (fixed_vertex[i]) continue;

      vertex_remap[i] = vertex_offset;
      vertex_offset += 1;
   }

   for (std::size_t i = 0; i < vertex_count; ++i) {
      out.vertices.position[vertex_remap[i]] = cloth.positions[i];
      out.vertices.texcoords[vertex_remap[i]] = cloth.texcoords[i];
   }

   out.triangles.reserve(cloth.triangles.size());

   for (const std::array<uint32, 3>& tri : cloth.triangles) {
      out.triangles.push_back(
         {vertex_remap[tri[0]], vertex_remap[tri[1]], vertex_remap[tri[2]]});
   }

   out.fixed_weights = cloth.fixed_weights;

   out.stretch_constraints.reserve(cloth.stretch_constraints.size());

   for (const std::array<uint16, 2>& constraint : cloth.stretch_constraints) {
      out.stretch_constraints.push_back(
         {vertex_remap[constraint[0]], vertex_remap[constraint[1]]});
   }

   out.bend_constraints.reserve(cloth.bend_constraints.size());

   for (const std::array<uint16, 2>& constraint : cloth.bend_constraints) {
      out.bend_constraints.push_back(
         {vertex_remap[constraint[0]], vertex_remap[constraint[1]]});
   }

   out.cross_constraints.reserve(cloth.cross_constraints.size());

   for (const std::array<uint16, 2>& constraint : cloth.cross_constraints) {
      out.cross_constraints.push_back(
         {vertex_remap[constraint[0]], vertex_remap[constraint[1]]});
   }

   out.collision.reserve(cloth.collision.size());

   for (const msh::cloth_collision_primitive& primitive : cloth.collision) {
      float4x4 transform;

      for (const msh::node& other_node : scene.nodes) {
         if (other_node.name == primitive.name) {
            transform = float4x4{other_node.transform};

            break;
         }
      }

      out.collision.push_back({.parent = primitive.parent,
                               .shape = static_cast<uint32>(primitive.shape),
                               .transform = transform,
                               .size = primitive.size});
   }

   return out;
}

auto build_cloths(const msh::scene& scene) -> std::vector<cloth>
{
   std::vector<cloth> cloths;

   for (const msh::node& node : scene.nodes) {
      if (node.type != msh::node_type::cloth) continue;

      cloths.push_back(build_cloth(scene, node));
   }

   return cloths;
}

auto parse_collision_flags(std::string_view flag_chars, const std::string_view node_name,
                           const build_context& context)
{
   collision_flags flags = collision_flags::all;

   while (not flag_chars.empty()) {
      switch (flag_chars.front()) {
      case 's':
         flags |= collision_flags::soldier;
         break;
      case 'v':
         flags |= collision_flags::vehicle;
         break;
      case 'b':
         flags |= collision_flags::building;
         break;
      case 't':
         flags |= collision_flags::terrain;
         break;
      case 'o':
         flags |= collision_flags::ordnance;
         break;
      case 'f':
         flags |= collision_flags::unknown;
         break;
      default:
         flags = collision_flags::all;

         context.feedback.add_warning(
            {.file = context.path,
             .tool = "ModelMunge",
             .message =
                fmt::format("Node '{}' has invalid collision flag '{}'.\n\n{}",
                            node_name, flag_chars.front(),
                            get_descriptive_message(model_wc::collision_invalid_flag))});

         return collision_flags::all;
      }

      flag_chars.remove_prefix(1);
   }

   return flags;
}

auto read_collision_mesh_flags(const msh::scene& scene,
                               const std::span<const std::size_t> collision_nodes,
                               const build_context& context) -> collision_flags
{
   for (std::size_t node_index : collision_nodes) {
      const std::string_view name = scene.nodes[node_index].name;
      const std::string_view prefix = "collision_-";

      if (not istarts_with(name, prefix)) continue;

      const std::size_t flags_end = name.find_first_of("_-", prefix.size());

      if (flags_end == name.npos) continue;

      const collision_flags flags =
         parse_collision_flags({&name[prefix.size()], &name[flags_end]}, name, context);

      if (flags != collision_flags::all) return flags;
   }

   return collision_flags::all;
}

auto build_collision_mesh(const msh::scene& scene, const skeleton& skeleton,
                          const build_context& context) -> std::optional<collision_mesh>
{
   if (scene.options.no_collision) return std::nullopt;

   std::vector<std::size_t> collision;

   bool has_collision_primitives = false;

   for (std::size_t i = 0; i < scene.nodes.size(); ++i) {
      const msh::node& node = scene.nodes[i];

      if (istarts_with(node.name, "collision")) {
         collision.push_back(i);
      }
      else if (istarts_with(node.name, "p_") and node.collision_primitive) {
         has_collision_primitives |= true;
      }
   }

   const collision_flags flags = read_collision_mesh_flags(scene, collision, context);

   if (collision.empty() and not has_collision_primitives) {
      for (std::size_t i = 0; i < scene.nodes.size(); ++i) {
         const msh::node& node = scene.nodes[i];

         if (not is_geometry_node(node)) continue;

         if (not icontains(node.name, "lod2") and //
             not icontains(node.name, "lod3") and //
             not icontains(node.name, "lowres") and
             not icontains(node.name, "lowrez")) {
            collision.push_back(i);
         }
      }
   }

   if (collision.empty()) return std::nullopt;

   std::size_t vertex_count = 0;
   std::size_t triangle_count = 0;

   for (std::size_t node_index : collision) {
      for (const msh::geometry_segment& segment : scene.nodes[node_index].segments) {
         vertex_count += segment.positions.size();
         triangle_count += segment.triangles.size();
      }
   }

   if (vertex_count == 0) {
      throw model_error{"Collision meshes have no vertices!",
                        model_ec::collision_mesh_bad_vertex_count};
   }
   else if (triangle_count == 0) {
      throw model_error{"Collision meshes have no vertices!",
                        model_ec::collision_mesh_bad_triangle_count};
   }
   else if (vertex_count > max_collision_vertices) {
      throw model_error{fmt::format("Too many vertices ({}) max is {}!",
                                    vertex_count, max_collision_vertices),
                        model_ec::collision_mesh_too_many_vertices};
   }

   collision_mesh_input input = {
      .node_name = scene.nodes[0].name,
      .mask = flags,
      .do_not_simplify = scene.options.do_not_merge_collision,
   };

   input.vertices.resize(vertex_count);
   input.triangles.reserve(triangle_count);

   std::size_t vertex_offset = 0;

   for (std::size_t node_index : collision) {
      const msh::node& node = scene.nodes[node_index];
      const float4x4& node_from_vertex = skeleton.node_from_vertex[node_index];

      if (const skeleton_bone& parent_bone =
             skeleton.bones[skeleton.node_parent_remap[node_index]];
          parent_bone.bone_from_vertex[0] != float4{1.0f, 0.0f, 0.0f, 0.0f} and
          parent_bone.bone_from_vertex[1] != float4{0.0f, 1.0f, 0.0f, 0.0f} and
          parent_bone.bone_from_vertex[2] != float4{0.0f, 0.0f, 1.0f, 0.0f} and
          parent_bone.bone_from_vertex[3] != float4{0.0f, 0.0f, 0.0f, 1.0f}) {
         context.feedback.add_warning(
            {.file = context.path,
             .tool = "ModelMunge",
             .message =
                fmt::format("Bone ('{}') collision mesh ('{}') is parented to "
                            "has non-identity transform.\n\n{}",
                            parent_bone.name, node.name,
                            get_descriptive_message(
                               model_wc::collision_mesh_parent_transformed))});
      }

      for (const msh::geometry_segment& segment : node.segments) {
         for (const std::array<uint16, 3>& triangle : segment.triangles) {
            input.triangles.push_back({
               static_cast<uint16>(triangle[2] + vertex_offset),
               static_cast<uint16>(triangle[1] + vertex_offset),
               static_cast<uint16>(triangle[0] + vertex_offset),
            });
         }

         for (const float3& position : segment.positions) {
            input.vertices[vertex_offset] = node_from_vertex * position;

            vertex_offset += 1;
         }

         if (segment.weights and segment.weights->size() > 1) {
            context.feedback.add_warning(
               {.file = context.path,
                .tool = "ModelMunge",
                .message =
                   fmt::format("Collision mesh ('{}') has segment with vertex "
                               "weights.\n\n{}",
                               node.name,
                               get_descriptive_message(
                                  model_wc::collision_mesh_has_vertex_weights))});
         }
      }
   }

   return build_collision_mesh(std::move(input));
}

auto build_collision_primitives(const msh::scene& scene, const skeleton& skeleton,
                                const build_context& context)
   -> std::vector<collision_primitive>
{
   std::vector<std::size_t> primitive_nodes;

   for (std::size_t i = 0; i < scene.nodes.size(); ++i) {
      const msh::node& node = scene.nodes[i];

      if (not node.name.starts_with("p_")) {
         if (node.name.starts_with("P_") and node.collision_primitive) {
            context.feedback.add_warning(
               {.file = context.path,
                .tool = "ModelMunge",
                .message =
                   fmt::format("Node '{}' has a collision primitive but starts "
                               "with the wrong prefix!\n\n{}",
                               node.name,
                               get_descriptive_message(
                                  model_wc::collision_primitive_wrong_prefix))});
         }

         continue;
      }
      else if (not node.collision_primitive) {
         context.feedback.add_warning(
            {.file = context.path,
             .tool = "ModelMunge",
             .message = fmt::format("'{}' is named as a collision primitve but "
                                    "it is missing the primitive's data.\n\n{}",
                                    node.name,
                                    get_descriptive_message(
                                       model_wc::collision_primitive_missing_data))});
      }

      primitive_nodes.push_back(i);
   }

   std::vector<collision_primitive> primitives;
   primitives.reserve(primitive_nodes.size());

   for (const std::size_t node_index : primitive_nodes) {
      const msh::node& node = scene.nodes[node_index];

      collision_flags flags = collision_flags::all;

      if (const std::string_view prefix = "p_-"; node.name.starts_with(prefix)) {
         const std::size_t flags_end = node.name.find_first_of("_-", prefix.size());

         if (flags_end != node.name.npos) {
            flags = parse_collision_flags({&node.name[prefix.size()],
                                           &node.name[flags_end]},
                                          node.name, context);
         }
      }

      primitives.push_back({
         .name = node.name,
         .parent = skeleton.bones[skeleton.node_parent_remap[node_index]].name,
         .mask = flags,
         .transform = skeleton.node_from_parent[node_index],
         .shape = static_cast<uint32>(node.collision_primitive->shape),
         .size = {node.collision_primitive->radius, node.collision_primitive->height,
                  node.collision_primitive->length},
      });
   }

   return primitives;
}

}

auto load_model(const io::path& path,
                const std::vector<assets::option>& directory_options,
                munge_feedback& feedback) -> model_container
{
   const build_context context = {.path = path, .feedback = feedback};
   msh::scene scene = load_scene(path, directory_options);

   if (scene.options.scale != 1.0f) scale_scene(scene);

   warning_check_scene(scene, context);

   model_container model;

   model.name = path.stem();
   model.skeleton = build_skeleton(scene, context);
   model.models = build_models(scene, model.skeleton, context);
   model.game_model = build_game_model(scene);
   model.collision_mesh = build_collision_mesh(scene, model.skeleton, context);
   model.collision_primitives =
      build_collision_primitives(scene, model.skeleton, context);
   model.cloths = build_cloths(scene);

   return model;
}

}