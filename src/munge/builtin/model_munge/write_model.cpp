#include "write_model.hpp"
#include "error.hpp"

#include "io/error.hpp"

#include "ucfb/writer.hpp"

#include "utility/enum_bitflags.hpp"

#include "math/vector_funcs.hpp"

using namespace we::ucfb::literals;

namespace we::munge {

namespace {

enum class shadow_skin_type : uint32 {
   unskinned = 0,
   hard_skinned = 1,
   soft_skinned = 2,
};

enum class shadow_skin_state : uint32 {
   unskinned = 0,
   // unknown = 1, // This value might exist and mean something to the game but modelmunge only seems to output 0 or 2 for this field.
   skinned = 2,
};

// This matches D3DPRIMITIVETYPE from D3D9.
enum class pc_primitive_type : uint32 {
   point_list = 1,
   line_list = 2,
   line_strip = 3,
   triangle_list = 4,
   triangle_strip = 5,
   triangle_fan = 6,
};

enum class vbuf_flags : uint32 {
   none = 0b0,
   position = 0b10,
   bone_indices = 0b100,
   bone_weights = 0b1000,
   normal = 0b1000'00,
   tangents = 0b1000'000,
   color = 0b1000'0000,
   color_lighting = 0b1000'0000'0,
   texcoords = 0b1000'0000'00,

   shadow_data = 0b1000'0000'0000,

   position_compressed = 0b1000'0000'0000'0,
   weights_compressed = 0b1000'0000'0000'00,
   normals_compressed = 0b1000'0000'0000'000,
   texcoords_compressed = 0b1000'0000'0000'0000
};

constexpr bool marked_as_enum_bitflag(vbuf_flags)
{
   return true;
}

auto lod_suffix(const model& model) -> std::string_view
{
   switch (model.lod) {
   case model_lod::lod0:
      return "";
   case model_lod::lod1:
      return "LOD2";
   case model_lod::lod2:
      return "LOD3";
   case model_lod::lowd:
      return "LOWD";
   }

   return "";
}

auto lod_id(const model& model) -> ucfb::chunk_id
{
   switch (model.lod) {
   case model_lod::lod0:
      return "LOD0"_id;
   case model_lod::lod1:
      return "LOD1"_id;
   case model_lod::lod2:
      return "LOD2"_id;
   case model_lod::lowd:
      return "LOWD"_id;
   }

   return "LOD0"_id;
}

auto vertex_count(const model_shadow_vertices& vertices) -> uint32
{
   switch (vertices.type) {
   case model_shadow_vertex_type::unskinned:
      return static_cast<uint32>(vertices.unskinned.size());
   case model_shadow_vertex_type::hard_skinned:
      return static_cast<uint32>(vertices.hard_skinned.size());
   case model_shadow_vertex_type::soft_skinned:
      return static_cast<uint32>(vertices.soft_skinned.size());
   default:
      return 0;
   }
}

auto get_shadow_skin_type(const model_shadow_vertices& vertices) -> shadow_skin_type
{
   switch (vertices.type) {
   default:
   case model_shadow_vertex_type::unskinned:
      return shadow_skin_type::unskinned;
   case model_shadow_vertex_type::hard_skinned:
      return shadow_skin_type::hard_skinned;
   case model_shadow_vertex_type::soft_skinned:
      return shadow_skin_type::soft_skinned;
   }
}

auto get_shadow_skin_state(const model_shadow_vertices& vertices) -> shadow_skin_state
{
   switch (vertices.type) {
   default:
   case model_shadow_vertex_type::unskinned:
      return shadow_skin_state::unskinned;
   case model_shadow_vertex_type::hard_skinned:
   case model_shadow_vertex_type::soft_skinned:
      return shadow_skin_state::skinned;
   }
}

struct vbuf_options {
   bool position_compressed = false;
   bool weights_compressed = false;
   bool normals_compressed = false;
   bool texcoords_compressed = false;
   bool exclude_skinning = false;
   bool exclude_tangents = false;
};

void write_skel(ucfb::writer& skel, const skeleton& skeleton,
                const std::string_view skeleton_name)
{
   // INFO
   {
      ucfb::writer info = skel.write_child("INFO"_id);

      info.write(skeleton_name);
      info.write(static_cast<uint32>(skeleton.bones.size()));
   }

   // NAME
   {
      ucfb::writer name = skel.write_child("NAME"_id);

      for (const skeleton_bone& bone : skeleton.bones) {
         name.write(bone.name);
      }
   }

   // PRNT
   {
      ucfb::writer prnt = skel.write_child("PRNT"_id);

      for (const skeleton_bone& bone : skeleton.bones) {
         prnt.write(bone.parent);
      }
   }

   // XFRM
   {
      ucfb::writer xfrm = skel.write_child("XFRM"_id);

      for (const skeleton_bone& bone : skeleton.bones) {
         for (const float4& row : bone.bone_from_parent.rows) {
            xfrm.write(row.x, row.y, row.z);
         }
      }
   }
}

void write_vbuf(ucfb::writer& vbuf, const model_segment_vertices& vertices,
                const model& model, const vbuf_options& options)
{
   vbuf_flags flags = vbuf_flags::none;
   uint32 stride = 0;

   if (vertices.positionSS) {
      flags |= vbuf_flags::position;

      if (options.position_compressed) {
         flags |= vbuf_flags::position_compressed;

         stride += sizeof(std::array<int16, 4>);
      }
      else {
         stride += sizeof(float3);
      }
   }

   if (vertices.bone_indices and not options.exclude_skinning) {
      if (vertices.bone_weights) {
         flags |= vbuf_flags::bone_weights;

         if (options.weights_compressed) {
            flags |= vbuf_flags::weights_compressed;

            stride += sizeof(uint32);
         }
         else {
            stride += sizeof(float2);
         }
      }

      flags |= vbuf_flags::bone_indices;

      stride += sizeof(uint32);

      if (options.weights_compressed) {
         flags |= vbuf_flags::weights_compressed;
      }
   }

   if (vertices.normalSS) {
      flags |= vbuf_flags::normal;

      if (options.normals_compressed) {
         flags |= vbuf_flags::normals_compressed;

         stride += sizeof(uint32);
      }
      else {
         stride += sizeof(float3);
      }
   }

   if (vertices.tangents and not options.exclude_tangents) {
      flags |= vbuf_flags::tangents;

      if (options.normals_compressed) {
         flags |= vbuf_flags::normals_compressed;

         stride += sizeof(uint32);
         stride += sizeof(uint32);
      }
      else {
         stride += sizeof(float3);
         stride += sizeof(float3);
      }
   }

   if (vertices.color) {
      if (model.vertex_lighting) {
         flags |= vbuf_flags::color_lighting;
      }
      else {
         flags |= vbuf_flags::color;
      }

      stride += sizeof(uint32);
   }

   if (vertices.texcoords) {
      flags |= vbuf_flags::texcoords;

      if (options.texcoords_compressed) {
         flags |= vbuf_flags::texcoords_compressed;

         stride += sizeof(std::array<int16, 2>);
      }
      else {
         stride += sizeof(float2);
      }
   }

   vbuf.write(static_cast<uint32>(vertices.vertex_count));
   vbuf.write(stride);
   vbuf.write(flags);

   const float3 vertex_compress_sub =
      (model.vertex_box.max + model.vertex_box.min) * 0.5f;
   const float3 vertex_compress_mul = float3{32767.0f, 32767.0f, 32767.0f} * 2.0f /
                                      (model.vertex_box.max - model.vertex_box.min);

   for (std::size_t i = 0; i < vertices.vertex_count; ++i) {
      if (are_flags_set(flags, vbuf_flags::position)) {
         const float3& positionSS = vertices.positionSS[i];

         if (are_flags_set(flags, vbuf_flags::position_compressed)) {
            const float3 positionCS =
               floor((positionSS - vertex_compress_sub) * vertex_compress_mul + 0.5f);

            vbuf.write(std::array<int16, 4>{
               static_cast<int16>(positionCS.x),
               static_cast<int16>(positionCS.y),
               static_cast<int16>(positionCS.z),
               0,
            });
         }
         else {
            vbuf.write(positionSS);
         }
      }

      if (are_flags_set(flags, vbuf_flags::bone_weights)) {
         const float3& bone_weights = vertices.bone_weights[i];

         if (are_flags_set(flags, vbuf_flags::weights_compressed)) {
            uint32 packed_weights = 0;

            std::array<uint32, 2> unorm_weights =
               {static_cast<uint32>(bone_weights.x * 255.0f + 0.5f),
                static_cast<uint32>(bone_weights.y * 255.0f + 0.5f)};

            if (unorm_weights[0] + unorm_weights[1] > 255) {
               unorm_weights[1] = 255 - unorm_weights[1];
            }

            packed_weights |= unorm_weights[0] << 16;
            packed_weights |= unorm_weights[1] << 8;
            packed_weights |= (255u - unorm_weights[0] - unorm_weights[1]) << 0;

            vbuf.write(packed_weights);
         }
         else {
            vbuf.write(float2{bone_weights.x, bone_weights.y});
         }
      }

      if (are_flags_set(flags, vbuf_flags::bone_indices)) {
         const std::array<uint8, 3>& bone_indices = vertices.bone_indices[i];

         uint32 packed_indices = 0;

         if (are_flags_set(flags, vbuf_flags::bone_weights)) {
            packed_indices |= static_cast<uint32>(bone_indices[0]) << 16;
            packed_indices |= static_cast<uint32>(bone_indices[1]) << 8;
            packed_indices |= static_cast<uint32>(bone_indices[2]);
            packed_indices |= static_cast<uint32>(bone_indices[0]) << 24;
         }
         else {
            packed_indices |= static_cast<uint32>(bone_indices[0]) << 16;
            packed_indices |= static_cast<uint32>(bone_indices[0]) << 8;
            packed_indices |= static_cast<uint32>(bone_indices[0]);
            packed_indices |= static_cast<uint32>(bone_indices[0]) << 24;
         }

         vbuf.write(packed_indices);
      }

      if (are_flags_set(flags, vbuf_flags::normal)) {
         const float3& normal = vertices.normalSS[i];

         if (are_flags_set(flags, vbuf_flags::normals_compressed)) {
            uint32 packed_normal = 0;

            packed_normal |= static_cast<uint32>(normal.x * 127.5f + 127.5f) << 16;
            packed_normal |= static_cast<uint32>(normal.y * 127.5f + 127.5f) << 8;
            packed_normal |= static_cast<uint32>(normal.z * 127.5f + 127.5f);

            vbuf.write(packed_normal);
         }
         else {
            vbuf.write(normal);
         }
      }

      if (are_flags_set(flags, vbuf_flags::tangents)) {
         const model_segment_vertex_tangents& tangents = vertices.tangents[i];
         const float3& tangent = tangents.tangentSS;
         const float3& bitangent = tangents.bitangentSS;

         if (are_flags_set(flags, vbuf_flags::normals_compressed)) {
            uint32 packed_tangent = 0;

            packed_tangent |= static_cast<uint32>(tangent.x * 127.5f + 127.5f) << 16;
            packed_tangent |= static_cast<uint32>(tangent.y * 127.5f + 127.5f) << 8;
            packed_tangent |= static_cast<uint32>(tangent.z * 127.5f + 127.5f);

            uint32 packed_bitangent = 0;

            packed_bitangent |=
               static_cast<uint32>(bitangent.x * 127.5f + 127.5f) << 16;
            packed_bitangent |=
               static_cast<uint32>(bitangent.y * 127.5f + 127.5f) << 8;
            packed_bitangent |=
               static_cast<uint32>(bitangent.z * 127.5f + 127.5f) << 0;

            vbuf.write(packed_tangent);
            vbuf.write(packed_bitangent);
         }
         else {
            vbuf.write(tangent);
            vbuf.write(bitangent);
         }
      }

      if (are_flags_set(flags, vbuf_flags::color) or
          are_flags_set(flags, vbuf_flags::color_lighting)) {
         vbuf.write(vertices.color[i]);
      }

      if (are_flags_set(flags, vbuf_flags::texcoords)) {
         const float2& texcoords = vertices.texcoords[i];

         if (are_flags_set(flags, vbuf_flags::texcoords_compressed)) {
            vbuf.write(std::array<int16, 2>{
               static_cast<int16>(texcoords.x * 2048.0f),
               static_cast<int16>(texcoords.y * 2048.0f),
            });
         }
         else {
            vbuf.write(texcoords);
         }
      }
   }
}

void write_vbuf(ucfb::writer& vbuf, const model_shadow_vertices& vertices,
                const model& model)
{
   const float3 vertex_compress_sub =
      (model.vertex_box.max + model.vertex_box.min) * 0.5f;
   const float3 vertex_compress_mul = float3{32767.0f, 32767.0f, 32767.0f} * 2.0f /
                                      (model.vertex_box.max - model.vertex_box.min);

   switch (vertices.type) {
   case model_shadow_vertex_type::unskinned: {
      vbuf.write(static_cast<uint32>(vertices.unskinned.size()));
      vbuf.write(int32{12}); // 8 for position, 4 for normal
      vbuf.write(vbuf_flags::position | vbuf_flags::normal |
                 vbuf_flags::position_compressed | vbuf_flags::normals_compressed);

      for (const model_shadow_unskinned_vertex& vertex : vertices.unskinned) {
         const float3 positionCS = floor(
            (vertex.positionSS - vertex_compress_sub) * vertex_compress_mul + 0.5f);

         vbuf.write(std::array<int16, 4>{
            static_cast<int16>(positionCS.x),
            static_cast<int16>(positionCS.y),
            static_cast<int16>(positionCS.z),
            0,
         });

         uint32 packed_normal = 0;

         packed_normal |= vertex.normalSS[0] << 16u;
         packed_normal |= vertex.normalSS[1] << 8u;
         packed_normal |= vertex.normalSS[2];

         vbuf.write(packed_normal);
      }
   } break;
   case model_shadow_vertex_type::hard_skinned: {
      vbuf.write(static_cast<uint32>(vertices.hard_skinned.size()));
      vbuf.write(int32{28}); // 8 for position0, 4 for bone indices, 8 for position1, 8 for position2
      vbuf.write(vbuf_flags::position | vbuf_flags::bone_indices |
                 vbuf_flags::bone_weights | vbuf_flags::shadow_data |
                 vbuf_flags::position_compressed |
                 vbuf_flags::weights_compressed | vbuf_flags::normals_compressed);

      for (const model_shadow_hard_skinned_vertex& vertex : vertices.hard_skinned) {
         const std::array<float3, 3> positionCS = {
            floor((vertex.positionSS[0] - vertex_compress_sub) * vertex_compress_mul + 0.5f),
            floor((vertex.positionSS[1] - vertex_compress_sub) * vertex_compress_mul + 0.5f),
            floor((vertex.positionSS[2] - vertex_compress_sub) * vertex_compress_mul + 0.5f),
         };

         vbuf.write(std::array<int16, 4>{
            static_cast<int16>(positionCS[0].x),
            static_cast<int16>(positionCS[0].y),
            static_cast<int16>(positionCS[0].z),
            0,
         });

         uint32 packed_indices = 0;

         packed_indices |= static_cast<uint32>(vertex.bone_indices[0]) << 16;
         packed_indices |= static_cast<uint32>(vertex.bone_indices[1]) << 8;
         packed_indices |= static_cast<uint32>(vertex.bone_indices[2]);

         vbuf.write(packed_indices);

         vbuf.write(std::array<int16, 4>{
            static_cast<int16>(positionCS[1].x),
            static_cast<int16>(positionCS[1].y),
            static_cast<int16>(positionCS[1].z),
            0,
         });

         vbuf.write(std::array<int16, 4>{
            static_cast<int16>(positionCS[2].x),
            static_cast<int16>(positionCS[2].y),
            static_cast<int16>(positionCS[2].z),
            0,
         });
      }
   } break;
   case model_shadow_vertex_type::soft_skinned: {
      vbuf.write(static_cast<uint32>(vertices.soft_skinned.size()));
      vbuf.write(int32{48}); // 8 for position0, 4 for bone indices, 8 for position1, 8 for position2
      vbuf.write(vbuf_flags::position | vbuf_flags::bone_indices |
                 vbuf_flags::bone_weights | vbuf_flags::shadow_data |
                 vbuf_flags::position_compressed |
                 vbuf_flags::weights_compressed | vbuf_flags::normals_compressed);

      for (const model_shadow_soft_skinned_vertex& vertex : vertices.soft_skinned) {
         for (std::size_t i = 0; i < vertex.positionSS.size(); ++i) {
            const float3 positionCS = floor(
               (vertex.positionSS[i] - vertex_compress_sub) * vertex_compress_mul + 0.5f);

            vbuf.write(std::array<int16, 4>{
               static_cast<int16>(positionCS.x),
               static_cast<int16>(positionCS.y),
               static_cast<int16>(positionCS.z),
               0,
            });

            uint32 packed_weights = 0xff'00'00'00u;

            packed_weights |= static_cast<uint32>(vertex.bone_weights[i][0]) << 16;
            packed_weights |= static_cast<uint32>(vertex.bone_weights[i][1]) << 8;
            packed_weights |= static_cast<uint32>(vertex.bone_weights[i][2]);

            vbuf.write(packed_weights);

            uint32 packed_indices = 0;

            packed_indices |= static_cast<uint32>(vertex.bone_indices[i][0]) << 16;
            packed_indices |= static_cast<uint32>(vertex.bone_indices[i][1]) << 8;
            packed_indices |= static_cast<uint32>(vertex.bone_indices[i][2]);

            vbuf.write(packed_indices);
         }
      }
   } break;
   }
}

void write_skin(ucfb::writer& skin, const model_segment_vertices& vertices)
{
   assert(vertices.bone_indices);

   skin.write(static_cast<uint32>(vertices.vertex_count));
   skin.write(vertices.bone_weights ? uint32{3} : uint32{1});

   if (vertices.bone_weights) {
      for (std::size_t i = 0; i < vertices.vertex_count; ++i) {
         const float3& bone_weights = vertices.bone_weights[i];

         std::array<uint32, 2> unorm_weights =
            {static_cast<uint32>(bone_weights.x * 255.0f + 0.5f),
             static_cast<uint32>(bone_weights.y * 255.0f + 0.5f)};

         if (unorm_weights[0] + unorm_weights[1] > 255) {
            unorm_weights[1] = 255 - unorm_weights[1];
         }

         skin.write(static_cast<uint8>(unorm_weights[0]));
         skin.write(static_cast<uint8>(unorm_weights[1]));
         skin.write(vertices.bone_indices[i]);
      }
   }
   else {
      for (std::size_t i = 0; i < vertices.vertex_count; ++i) {
         skin.write(vertices.bone_indices[i][0]);
      }
   }
}

void write_gshd(ucfb::writer& gshd, const model_shadow& shadow, const model& model)
{
   // INFO
   {
      ucfb::writer info = gshd.write_child("INFO"_id);

      // shadow_skin_type skinn_type;
      // shadow_skin_state skin_state;
      // D3DPRIMITIVETYPE (uint32) primitive_type;
      // uint32 vertex_count;
      // uint32 primitive_count;

      info.write(get_shadow_skin_type(shadow.vertices));
      info.write(get_shadow_skin_state(shadow.vertices));
      info.write(pc_primitive_type::triangle_list);
      info.write(vertex_count(shadow.vertices));
      info.write(static_cast<uint32>(shadow.index_buffer.size()));
   }

   // IBUF
   {
      ucfb::writer ibuf = gshd.write_child("IBUF"_id);

      ibuf.write(static_cast<uint32>(shadow.index_buffer.size() * 3));
      ibuf.write(std::as_bytes(std::span{shadow.index_buffer}));
   }

   // VBUF
   {
      ucfb::writer vbuf = gshd.write_child("VBUF"_id);

      write_vbuf(vbuf, shadow.vertices, model);
   }
}

void write_shdw(ucfb::writer& shdw, const model_shadow& shadow, const model& model)
{
   ucfb::writer info = shdw.write_child("INFO"_id);

   if (shadow.vertices.type != model_shadow_vertex_type::unskinned) {
      // BMAP
      {
         ucfb::writer bmap = shdw.write_child("BMAP"_id);

         bmap.write(static_cast<uint32>(shadow.bone_map.size()));
         bmap.write(std::as_bytes(std::span{shadow.bone_map}));
      }
   }
   else {
      shdw.write_child("BNAM"_id).write(shadow.bone_name);
   }

   // GSHD
   {
      ucfb::writer gshd = shdw.write_child("GSHD"_id);

      write_gshd(gshd, shadow, model);
   }
}

void write_segm(ucfb::writer& segm, const model_segment& segment, const model& model)
{
   // INFO
   {
      ucfb::writer info = segm.write_child("INFO"_id);

      // D3DPRIMITIVETYPE (uint32) primitive_type;
      // uint32 vertex_count;
      // uint32 primitive_count;

      info.write(pc_primitive_type::triangle_list);
      info.write(static_cast<uint32>(segment.vertices.vertex_count));
      info.write(static_cast<uint32>(segment.index_buffer.size()));
   }

   // MTRL
   {
      ucfb::writer mtrl = segm.write_child("MTRL"_id);

      // uint32 mask;
      // uint32 diffuse_color;
      // uint32 specular_color;
      // uint32 specular_exponent;
      // uint32 param0;
      // uint32 param1;
      // string attached_light;

      mtrl.write(segment.material.flags);
      mtrl.write(segment.material.diffuse_color);
      mtrl.write(segment.material.specular_color);
      mtrl.write(segment.material.specular_exponent);
      mtrl.write(segment.material.param0);
      mtrl.write(segment.material.param1);
      mtrl.write(segment.material.attached_light);
   }

   // MNAM
   if (not segment.material.name.empty()) {
      segm.write_child("MNAM"_id).write(segment.material.name);
   }

   segm.write_child("RTYP"_id).write(segment.material.render_type);

   // TNAM
   for (uint32 i = 0; i < segment.material.textures.size(); ++i) {
      ucfb::writer tnam = segm.write_child("TNAM"_id);

      tnam.write(i);
      tnam.write(segment.material.textures[i]);
   }

   // BBOX
   {
      ucfb::writer bbox = segm.write_child("BBOX"_id);

      bbox.write(segment.bboxSS.min);
      bbox.write(segment.bboxSS.max);
   }

   // IBUF
   {
      ucfb::writer ibuf = segm.write_child("IBUF"_id);

      ibuf.write(static_cast<uint32>(segment.index_buffer.size() * 3));
      ibuf.write(std::as_bytes(std::span{segment.index_buffer}));
   }

   // VBUF(s)
   {
      // Compressed
      {
         ucfb::writer vbuf = segm.write_child("VBUF"_id);

         write_vbuf(vbuf, segment.vertices, model,
                    {.position_compressed = true,
                     .weights_compressed = true,
                     .normals_compressed = true,
                     .texcoords_compressed = not model.large_texcoords});
      }

      // Uncompressed
      {
         ucfb::writer vbuf = segm.write_child("VBUF"_id);

         write_vbuf(vbuf, segment.vertices, model,
                    {.position_compressed = false,
                     .weights_compressed = false,
                     .normals_compressed = false,
                     .texcoords_compressed = false});
      }

      // Uncompressed (no tangents)
      if (segment.vertices.tangents) {
         ucfb::writer vbuf = segm.write_child("VBUF"_id);

         write_vbuf(vbuf, segment.vertices, model,
                    {.position_compressed = false,
                     .weights_compressed = false,
                     .normals_compressed = false,
                     .texcoords_compressed = false,
                     .exclude_tangents = true});
      }
   }

   if (segment.vertices.bone_indices) {
      // VDAT
      {
         ucfb::writer vdat = segm.write_child("VDAT"_id);

         write_vbuf(vdat, segment.vertices, model,
                    {.position_compressed = false,
                     .weights_compressed = false,
                     .normals_compressed = false,
                     .texcoords_compressed = false,
                     .exclude_skinning = true,
                     .exclude_tangents = true});
      }

      // SKIN
      {
         ucfb::writer skin = segm.write_child("SKIN"_id);

         write_skin(skin, segment.vertices);
      }

      // BMAP
      {
         ucfb::writer bmap = segm.write_child("BMAP"_id);

         bmap.write(static_cast<uint32>(segment.bone_map.size()));
         bmap.write(std::as_bytes(std::span{segment.bone_map}));
      }
   }
   else {
      segm.write_child("BNAM"_id).write(segment.bone_name);
   }
}

void write_modl(ucfb::writer& modl, const model& model, const model_container& container)
{
   const std::string model_name =
      fmt::format("{}{}", container.name, lod_suffix(model));
   const uint32 collision_vertices =
      container.collision_mesh
         ? static_cast<uint32>(container.collision_mesh->vertices.size())
         : 0;

   modl.write_child("NAME"_id).write(model_name);
   modl.write_child("VRTX"_id).write(collision_vertices);
   modl.write_child("NODE"_id).write(model.node_name);

   // INFO
   {
      ucfb::writer info = modl.write_child("INFO"_id);

      // uint32 pre_inverse_transformed_vertices;
      // uint32 no_projection_lights;
      // uint32 segment_count;
      // uint32 unknown;
      // float3 vertex_box_min;
      // float3 vertex_box_max;
      // float3 visibility_box_min;
      // float3 visibility_box_max;
      // uint32 bone_count;
      // uint32 total_index_count;

      // pc_modelmunge writes total_index_count as the total count of indices in
      // the index buffer.
      //
      // We don't write triangle strips (unlike pc_modelmunge) so it is more
      // accurate for us to write out the total triangle count instead. Else
      // we'd be writing out a far higher value than the game expects and
      // we could cause LOD flickering where pc_modelmunge wouldn't.

      info.write(model.pre_inverse_transformed_vertices ? uint32{1} : uint32{0});
      info.write(model.no_projection_lights ? uint32{1} : uint32{0});
      info.write(static_cast<uint32>(model.segments.size()));
      info.write(uint32{0});
      info.write(model.vertex_box.min);
      info.write(model.vertex_box.max);
      info.write(model.visibility_box.min);
      info.write(model.visibility_box.max);
      info.write(static_cast<uint32>(container.skeleton.bones.size()));
      info.write(model.total_triangle_count);
   }

   for (const model_shadow& shadow : model.shadows) {
      ucfb::writer shdw = modl.write_child("shdw"_id);

      write_shdw(shdw, shadow, model);
   }

   for (const model_segment& segemnt : model.segments) {
      ucfb::writer segm = modl.write_child("segm"_id);

      write_segm(segm, segemnt, model);
   }

   // SPHR
   {
      ucfb::writer sphr = modl.write_child("SPHR"_id);

      sphr.write(model.bounding_sphere.position);
      sphr.write(model.bounding_sphere.radius);
   }
}

void write_gmod(ucfb::writer& gmod, const model_container& container)
{
   gmod.write_child("NAME"_id).write(container.name);

   // INFO
   {
      ucfb::writer info = gmod.write_child("INFO"_id);

      info.write(container.game_model.lod_group);
      info.write(container.game_model.lod_bias);
      info.write(static_cast<uint32>(container.cloths.size()));

      for (const cloth& cloth : container.cloths) info.write(cloth.name);
   }

   for (const model& model : container.models) {
      ucfb::writer lod = gmod.write_child(lod_id(model));

      lod.write(fmt::format("{}{}", container.name, lod_suffix(model)));
      lod.write(model.total_triangle_count);
   }
}

void write_tree(ucfb::writer& tree, const collision_mesh& mesh)
{
   for (const collision_mesh_node& node : mesh.tree) {
      if (node.type == collision_mesh_node_type::branch) {
         tree.write_child("NODE"_id).write(node.bbox);
      }
      else if (node.type == collision_mesh_node_type::leaf) {
         ucfb::writer leaf = tree.write_child("LEAF"_id);

         leaf.write(node.face_index_count);
         leaf.write(node.bbox);

         const std::size_t first_index = node.face_index_begin;
         const std::size_t last_index = first_index + node.face_index_count;

         for (std::size_t i = first_index; i < last_index; ++i) {
            assert(i < mesh.face_indices.size());

            leaf.write(mesh.face_indices[i]);
         }
      }
   }
}

void write_coll(ucfb::writer& coll, const collision_mesh& mesh, std::string_view model_name)
{
   coll.write_child("NAME"_id).write(model_name);

   if (mesh.mask != collision_flags::all) {
      coll.write_child("MASK"_id).write(mesh.mask);
   }

   coll.write_child("NODE"_id).write(mesh.node_name);

   // INFO
   {
      uint32 branch_nodes = 0;
      uint32 leaf_nodes = 0;
      uint32 total_face_vertices = 0;

      for (const collision_mesh_node& node : mesh.tree) {
         if (node.type == collision_mesh_node_type::branch) {
            branch_nodes += 1;
         }
         else if (node.type == collision_mesh_node_type::leaf) {
            leaf_nodes += 1;
         }

         total_face_vertices += node.face_index_count;
      }

      ucfb::writer info = coll.write_child("INFO"_id);

      info.write(static_cast<uint32>(mesh.vertices.size()));
      info.write(branch_nodes);
      info.write(leaf_nodes);
      info.write(total_face_vertices);
      info.write(mesh.bbox);
   }

   coll.write_child("POSI"_id).write(std::as_bytes(std::span{mesh.vertices}));

   // TREE
   {
      ucfb::writer tree = coll.write_child("TREE"_id);

      write_tree(tree, mesh);
   }
}

void write_prim(ucfb::writer& prim, const std::span<const collision_primitive> primitives,
                std::string_view model_name)
{
   // INFO
   {
      ucfb::writer info = prim.write_child("INFO"_id);

      info.write(model_name);
      info.write(static_cast<uint32>(primitives.size()));
   }

   for (const collision_primitive& primitive : primitives) {
      prim.write_child("NAME"_id).write(primitive.name);

      if (primitive.mask != collision_flags::all) {
         prim.write_child("MASK"_id).write(primitive.mask);
      }

      prim.write_child("PRNT"_id).write(primitive.parent);

      // XFRM
      {
         ucfb::writer xfrm = prim.write_child("XFRM"_id);

         for (const float4& row : primitive.transform.rows) {
            xfrm.write(row.x, row.y, row.z);
         }
      }

      // DATA
      {
         ucfb::writer data = prim.write_child("DATA"_id);

         data.write(primitive.shape);
         data.write(primitive.size);
      }
   }
}

void write_clth(ucfb::writer& clth, const cloth& cloth, std::string_view model_name)
{
   clth.write_child("INFO"_id).write(model_name);
   clth.write_child("NAME"_id).write(cloth.name);
   clth.write_child("PRNT"_id).write(cloth.parent);

   // XFRM
   {
      ucfb::writer xfrm = clth.write_child("XFRM"_id);

      for (const float4& row : cloth.transform.rows) {
         xfrm.write(row.x, row.y, row.z);
      }
   }

   // DATA
   {
      ucfb::writer data = clth.write_child("DATA"_id);

      // string texture_name;
      // uint32 vertex_count;
      // float3 positions[vertex_count];
      // float2 texcoords[vertex_count];
      // uint32 fixed_vertices_count;
      // uint32 fixed_vertices_weight_count; // Matches fixed_vertices_count
      // string fixed_vertices_weights[fixed_vertices_weight_count];
      // uint32 triangle_count;
      // uint32 triangles[triangle_count][3];
      // uint32 stretch_constraint_count;
      // uint32 stretch_constraints[stretch_constraint_count][2];
      // uint32 bend_constraint_count;
      // uint32 bend_constraints[bend_constraint_count][2];
      // uint32 cross_constraint_count;
      // uint32 cross_constraints[cross_constraint_count][2];

      data.write(cloth.texture_name);
      data.write(static_cast<uint32>(cloth.vertices.vertex_count));
      data.write(std::as_bytes(
         std::span{cloth.vertices.position.get(), cloth.vertices.vertex_count}));
      data.write(std::as_bytes(std::span{cloth.vertices.texcoords.get(),
                                         cloth.vertices.vertex_count}));

      data.write(cloth.fixed_point_count);
      data.write(static_cast<uint32>(cloth.fixed_weights.size()));

      for (const std::string& bone : cloth.fixed_weights) {
         data.write(bone);
      }

      data.write(static_cast<uint32>(cloth.triangles.size()));
      data.write(std::as_bytes(std::span{cloth.triangles}));

      data.write(static_cast<uint32>(cloth.stretch_constraints.size()));
      data.write(std::as_bytes(std::span{cloth.stretch_constraints}));

      data.write(static_cast<uint32>(cloth.bend_constraints.size()));
      data.write(std::as_bytes(std::span{cloth.bend_constraints}));

      data.write(static_cast<uint32>(cloth.cross_constraints.size()));
      data.write(std::as_bytes(std::span{cloth.cross_constraints}));
   }

   // COLL
   {
      // primitive {
      //    string   parent;
      //    uint32   shape; // 0 = sphere, 1 = cylinder, 2 = box
      //    float3   size;
      //    float3x4 transform;
      // };
      // uint32    primitve_count;
      // primitive primitives[primitve_count];

      ucfb::writer coll = clth.write_child("COLL"_id);

      coll.write(static_cast<uint32>(cloth.collision.size()));

      for (const cloth_collision_primitive& primitive : cloth.collision) {
         coll.write(primitive.parent);
         coll.write(primitive.shape);
         coll.write(primitive.size);

         for (const float4& row : primitive.transform.rows) {
            coll.write(row.x, row.y, row.z);
         }
      }
   }
}

}

void write_model(const io::path& output_file_path, const model_container& container)
{
   try {
      io::output_file out{output_file_path};

      ucfb::writer ucfb{"ucfb"_id, out, {}};

      // skel
      {
         ucfb::writer skel = ucfb.write_child("skel"_id);

         write_skel(skel, container.skeleton, container.name);
      }

      // modl
      for (const model& model : container.models) {
         ucfb::writer modl = ucfb.write_child("modl"_id);

         write_modl(modl, model, container);
      }

      // gmod
      if (not container.game_model.no_game_model and not container.models.empty()) {
         ucfb::writer gmod = ucfb.write_child("gmod"_id);

         write_gmod(gmod, container);
      }

      if (container.collision_mesh) {
         ucfb::writer coll = ucfb.write_child("coll"_id);

         write_coll(coll, *container.collision_mesh, container.name);
      }

      if (not container.collision_primitives.empty()) {
         ucfb::writer prim = ucfb.write_child("prim"_id);

         write_prim(prim, container.collision_primitives, container.name);
      }

      // CLTH
      for (const cloth& cloth : container.cloths) {
         ucfb::writer clth = ucfb.write_child("CLTH"_id);

         write_clth(clth, cloth, container.name);
      }
   }
   catch (io::open_error& error) {
      throw model_error{error.what(), model_ec::write_io_open_error};
   }
   catch (io::error& error) {
      throw model_error{error.what(), model_ec::write_io_generic_error};
   }
}
}