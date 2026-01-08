
#include "scene_io.hpp"
#include "error.hpp"
#include "utility/string_icompare.hpp"
#include "validate_scene.hpp"

#include "../option_file.hpp"

#include "io/error.hpp"
#include "io/read_file.hpp"

#include "ucfb/reader.hpp"

#include <charconv>
#include <numeric>

#include <fmt/core.h>

#pragma warning(disable : 4063) // case is not a valid value for switch of enum

using we::string::iequals;
using namespace we::ucfb::literals;
using namespace std::literals;

namespace we::assets::msh {

namespace {

auto from_msh_space(const float3& vec) -> const float3&
{
   return vec;
}

auto from_msh_space(float2 vec) -> float2
{
   return {vec.x, 1.0f - vec.y};
}

auto from_msh_space(uint32 color) -> uint32
{
   return color;
}

auto from_msh_space(const std::array<vertex_weight, 4>& weights)
   -> const std::array<vertex_weight, 4>&
{
   return weights;
}

auto count_triangles_in_strips(const std::vector<std::vector<uint16>>& strips) noexcept
   -> std::size_t
{
   return std::accumulate(strips.cbegin(), strips.cend(), std::size_t{0},
                          [](std::size_t count, const std::vector<uint16>& strip) {
                             return (strip.size() * 3 - 2) + count;
                          });
}

bool is_degenerate_triangle(const std::array<uint16, 3> triangle) noexcept
{
   return triangle[0] == triangle[1] || triangle[0] == triangle[2] ||
          triangle[1] == triangle[2];
}

auto triangle_strips_to_lists(const std::vector<std::vector<uint16>>& strips)
   -> std::vector<std::array<uint16, 3>>
{
   std::vector<std::array<uint16, 3>> triangles;
   triangles.reserve(count_triangles_in_strips(strips));

   for (const auto& strip : strips) {
      if (strip.size() < 3) continue;

      for (auto i = 0; i < (strip.size() - 2); ++i) {
         const bool even = i % 2 == 0;
         const auto tri = even ? std::array{strip[i], strip[i + 1], strip[i + 2]}
                               : std::array{strip[i + 2], strip[i + 1], strip[i]};

         if (is_degenerate_triangle(tri)) continue;

         triangles.emplace_back(tri);
      }
   }

   return triangles;
}

bool is_valid_collision_primitive_shape(const collision_primitive_shape shape) noexcept
{
   switch (shape) {
   case collision_primitive_shape::sphere:
   case collision_primitive_shape::cylinder:
   case collision_primitive_shape::box:
      return true;
   }

   return false;
}

void remap_bone_maps(scene& scene, const std::vector<uint32>& node_index)
{
   uint32 max_node = 0;

   for (uint32 index : node_index) max_node = std::max(max_node, index);

   std::vector<uint32> remap;
   remap.resize(max_node + 1, 0xff'ff'ff'ffu);

   for (uint32 new_index = 0; new_index < node_index.size(); ++new_index) {
      remap[node_index[new_index]] = new_index;
   }

   for (node& node : scene.nodes) {
      for (uint32& bone_index : node.bone_map) {
         if (bone_index > remap.size()) {
            throw read_error{".msh file ENVL entry out of valid range!",
                             read_ec::read_envl_entry_out_of_range};
         }
         else if (remap[bone_index] == 0xff'ff'ff'ffu) {
            throw read_error{".msh file ENVL entry references missing node!",
                             read_ec::read_envl_entry_missing_node};
         }

         bone_index = remap[bone_index];
      }
   }
}

auto read_swci(ucfb::reader_strict<"SWCI"_id> swci) -> collision_primitive
{
   collision_primitive primitive;

   const auto shape = swci.read<collision_primitive_shape>();

   // The game treats any invalid shape (and some stock assets DO have invalid shapes) as a sphere.
   primitive.shape = is_valid_collision_primitive_shape(shape)
                        ? shape
                        : collision_primitive_shape::sphere;
   primitive.radius = swci.read<float>();
   primitive.height = swci.read<float>();
   primitive.length = swci.read<float>();

   return primitive;
}

auto read_strp(ucfb::reader_strict<"STRP"_id> strp)
   -> std::vector<std::array<uint16, 3>>
{
   const auto count = strp.read<int32>();

   if (count < 3) return {};

   std::vector<std::vector<uint16>> strips;
   strips.push_back({static_cast<uint16>(strp.read<uint16>() & 0x7fff),
                     static_cast<uint16>(strp.read<uint16>() & 0x7fff)});

   for (int i = 2; i < count; ++i) {
      auto index = strp.read<uint16>();

      if (index & 0x8000) {
         strips.push_back({static_cast<uint16>(index & 0x7fff),
                           static_cast<uint16>(strp.read<uint16>() & 0x7fff)});

         i += 1;
         continue;
      }

      strips.back().push_back(index);
   }

   return triangle_strips_to_lists(strips);
}

template<typename Type>
auto read_vertex_atrb(ucfb::reader reader) -> std::vector<Type>
{
   const auto count = reader.read<int32>();

   std::vector<Type> result;
   result.reserve(count);

   for (int i = 0; i < count; ++i) {
      result.emplace_back(from_msh_space(reader.read<Type>()));
   }

   return result;
}

auto read_segm(ucfb::reader_strict<"SEGM"_id> segm) -> geometry_segment
{
   geometry_segment segment;
   std::optional<uint32> segment_color;

   while (segm) {
      auto child = segm.read_child();

      switch (child.id()) {
      case "MATI"_id:
         segment.material_index = child.read<int32>();
         continue;
      case "POSL"_id:
         segment.positions = read_vertex_atrb<float3>(child);
         continue;
      case "WGHT"_id:
         segment.weights = read_vertex_atrb<std::array<vertex_weight, 4>>(child);
         continue;
      case "NRML"_id:
         segment.normals = read_vertex_atrb<float3>(child);
         continue;
      case "UV0L"_id:
         segment.texcoords = read_vertex_atrb<float2>(child);
         continue;
      case "CLRL"_id:
         segment.colors = read_vertex_atrb<uint32>(child);
         continue;
      case "CLRB"_id:
         segment_color = child.read<uint32>();
         continue;
      case "STRP"_id:
         segment.triangles = read_strp(ucfb::reader_strict<"STRP"_id>{child});
         continue;
      }
   }

   if (segment_color and not segment.colors) {
      segment.colors.emplace(segment.positions.size(), *segment_color);
   }

   return segment;
}

void read_geom(ucfb::reader_strict<"GEOM"_id> geom, node& node_out)
{
   std::vector<geometry_segment> segments;
   segments.reserve(4);

   std::vector<uint32> bone_map;

   while (geom) {
      auto child = geom.read_child();

      switch (child.id()) {
      case "SEGM"_id:
         segments.emplace_back(read_segm(ucfb::reader_strict<"SEGM"_id>{child}));
         continue;
      case "ENVL"_id:
         bone_map = read_vertex_atrb<uint32>(child);
         continue;
      }
   }

   node_out.segments = std::move(segments);
   node_out.bone_map = std::move(bone_map);
}

auto read_tran(ucfb::reader_strict<"TRAN"_id> tran) -> transform
{
   transform transform;

   tran.read<float3>(); // scale, ignored by modelmunge
   float4 rotation = tran.read<float4>();

   transform.rotation = quaternion{rotation.w, rotation.x, rotation.y, rotation.z};
   transform.translation = tran.read<float3>();

   return transform;
}

void read_modl(ucfb::reader_strict<"MODL"_id> modl,
               std::vector<node>& nodes_out, std::vector<uint32>& node_index_out)
{
   node node;
   std::optional<uint32> node_index;

   while (modl) {
      auto child = modl.read_child();

      switch (child.id()) {
      case "MTYP"_id:
         node.type = child.read<node_type>();
         continue;
      case "MNDX"_id:
         node_index = child.read<uint32>();
         continue;
      case "NAME"_id:
         node.name = child.read_string();
         continue;
      case "PRNT"_id:
         node.parent = child.read_string();
         continue;
      case "FLGS"_id:
         node.hidden = (child.read<uint32>() & 0x1) != 0;
         continue;
      case "TRAN"_id:
         node.transform = read_tran(ucfb::reader_strict<"TRAN"_id>{child});
         continue;
      case "GEOM"_id:
         read_geom(ucfb::reader_strict<"GEOM"_id>{child}, node);
         continue;
      case "SWCI"_id:
         node.collision_primitive = read_swci(ucfb::reader_strict<"SWCI"_id>{child});
         continue;
      }
   }

   if (not node_index) {
      modl.reset_head();

      throw read_error{fmt::format(".msh file MODL chunk missing MNDX!\n\n{}",
                                   modl.trace()),
                       read_ec::read_mndx_missing};
   }

   nodes_out.push_back(std::move(node));
   node_index_out.push_back(*node_index);
}

auto read_txnd(ucfb::reader txnd) -> std::string
{
   auto name = txnd.read_string();

   if (const auto ext_offset = name.find_last_of('.');
       ext_offset != std::string_view::npos) {
      return std::string{name.substr(0, ext_offset)};
   }

   return std::string{name};
}

auto read_matd(ucfb::reader_strict<"MATD"_id> matd) -> material
{
   material material;

   while (matd) {
      auto child = matd.read_child();

      switch (child.id()) {
      case "NAME"_id:
         material.name = child.read_string();
         continue;
      case "DATA"_id:
         child.read<float4>(); // Diffuse Colour, seams to get ignored by modelmunge
         material.specular_color = child.read<float3>();
         child.read<float>(); // Specular Colour Alpha, effectively just padding
         child.read<float4>(); // Ambient Colour, ignored by modelmunge and Zero(?)
         child.read<float>(); // Specular Exponent/Decay (Gets ignored by RedEngine in SWBFII for all known materials)
         continue;
      case "ATRB"_id:
         material.flags = child.read<material_flags>();
         material.rendertype = child.read<rendertype>();
         material.data0 = child.read<uint8>();
         material.data1 = child.read<uint8>();
         continue;
      case "TX0D"_id:
         material.textures[0] = read_txnd(child);
         continue;
      case "TX1D"_id:
         material.textures[1] = read_txnd(child);
         continue;
      case "TX2D"_id:
         material.textures[2] = read_txnd(child);
         continue;
      case "TX3D"_id:
         material.textures[3] = read_txnd(child);
         continue;
      }
   }

   return material;
}

auto read_matl(ucfb::reader_strict<"MATL"_id> matl) -> std::vector<material>
{
   const auto count = matl.read<int32>();

   std::vector<material> materials;
   materials.reserve(count);

   for (int i = 0; i < count; ++i) {
      if (!matl) {
         throw read_error{
            fmt::format(".msh file material list (MATL) ended after "
                        "{} materials but the "
                        "declared count was {}.",
                        i, count),
            read_ec::read_matl_list_too_short};
      }

      materials.emplace_back(read_matd(matl.read_child_strict<"MATD"_id>()));
   }

   return materials;
}

auto read_msh2(ucfb::reader_strict<"MSH2"_id> msh2) -> scene
{
   scene scene;
   std::vector<uint32> node_index;

   while (msh2) {
      auto child = msh2.read_child();

      switch (child.id()) {
      case "MATL"_id:
         scene.materials = read_matl(ucfb::reader_strict<"MATL"_id>{child});
         continue;
      case "MODL"_id:
         read_modl(ucfb::reader_strict<"MODL"_id>{child}, scene.nodes, node_index);
         continue;
      }
   }

   remap_bone_maps(scene, node_index);

   return scene;
}

void read_scene_option(const option& opt, scene_options& out)
{
   if (iequals(opt.name, "-keep"sv)) {
      if (opt.arguments.empty()) {
         throw read_error{"Invalid -keep option.", read_ec::option_load_bad_keep};
      }

      out.keep_nodes.append_range(opt.arguments);
   }
   else if (iequals(opt.name, "-keepall"sv)) {
      out.keep_all = true;
   }
   else if (iequals(opt.name, "-keepmaterial"sv)) {
      if (opt.arguments.empty()) {
         throw read_error{"Invalid -keepmaterial option.",
                          read_ec::option_load_bad_keep_material};
      }

      out.keep_materials.append_range(opt.arguments);
   }
   else if (iequals(opt.name, "-righthanded"sv)) {
      out.left_handed = false;
   }
   else if (iequals(opt.name, "-lefthanded"sv)) {
      out.left_handed = true;
   }
   else if (iequals(opt.name, "-scale"sv)) {
      float scale = 1.0f;

      if (opt.arguments.empty() or
          std::from_chars(opt.arguments[0].data(),
                          opt.arguments[0].data() + opt.arguments[0].size(), scale)
                .ec != std::errc{}) {
         throw read_error{"Invalid -scale option.", read_ec::option_load_bad_scale};
      }

      out.scale = scale;
   }
   else if (iequals(opt.name, "-maxbones"sv)) {
      uint32 max_bones = 15;

      if (opt.arguments.empty() or
          std::from_chars(opt.arguments[0].data(),
                          opt.arguments[0].data() + opt.arguments[0].size(), max_bones)
                .ec != std::errc{}) {
         throw read_error{"Invalid -maxbones option.",
                          read_ec::option_load_bad_max_bones};
      }

      out.max_bones = max_bones;
   }
   else if (iequals(opt.name, "-lodgroup"sv)) {
      if (opt.arguments.empty()) {
         throw read_error{"Invalid -lodgroup option.",
                          read_ec::option_load_bad_lod_group};
      }

      if (iequals(opt.arguments[0], "model")) {
         out.lod_group = lod_group::model;
      }
      else if (iequals(opt.arguments[0], "bigmodel")) {
         out.lod_group = lod_group::big_model;
      }
      else if (iequals(opt.arguments[0], "soldier")) {
         out.lod_group = lod_group::soldier;
      }
      else if (iequals(opt.arguments[0], "hugemodel")) {
         out.lod_group = lod_group::huge_model;
      }
      else {
         throw read_error{"Invalid -lodgroup option.",
                          read_ec::option_load_bad_lod_group};
      }
   }
   else if (iequals(opt.name, "-lodbias"sv)) {
      float lod_bias = 1.0f;

      if (opt.arguments.empty() or
          std::from_chars(opt.arguments[0].data(),
                          opt.arguments[0].data() + opt.arguments[0].size(), lod_bias)
                .ec != std::errc{}) {
         throw read_error{"Invalid -lodbias option.", read_ec::option_load_bad_lod_bias};
      }

      out.lod_bias = lod_bias;
   }
   else if (iequals(opt.name, "-nocollision"sv)) {
      out.no_collision = true;
   }
   else if (iequals(opt.name, "-nogamemodel"sv)) {
      out.no_game_model = true;
   }
   else if (iequals(opt.name, "-hiresshadow"sv)) {
      out.high_res_shadow = 0;

      if (not opt.arguments.empty()) {
         uint8 high_res_shadow_lod = 0;

         if (std::from_chars(opt.arguments[0].data(),
                             opt.arguments[0].data() + opt.arguments[0].size(),
                             high_res_shadow_lod)
                .ec != std::errc{}) {
            throw read_error{"Invalid -hiresshadow option.",
                             read_ec::option_load_bad_hi_res_shadow};
         }

         out.high_res_shadow_lod = high_res_shadow_lod;
      }
   }
   else if (iequals(opt.name, "-shadowon"sv)) {
      out.shadow_on = true;
   }
   else if (iequals(opt.name, "-softskinshadow"sv)) {
      out.soft_skin_shadow = true;
   }
   else if (iequals(opt.name, "-hardskinonly"sv)) {
      out.hard_skin_only = true;
   }
   else if (iequals(opt.name, "-softskin"sv)) {
      out.soft_skin = true;
   }
   else if (iequals(opt.name, "-donotmergeskins"sv)) {
      out.do_not_merge_skins = true;
   }
   else if (iequals(opt.name, "-vertexlighting"sv)) {
      out.vertex_lighting = true;
   }
   else if (iequals(opt.name, "-additiveemissive"sv)) {
      out.additive_emissive = true;
   }
   else if (iequals(opt.name, "-bump"sv)) {
      if (opt.arguments.empty()) {
         throw read_error{"Invalid -bump option.", read_ec::option_load_bad_bump};
      }

      out.normal_maps.append_range(opt.arguments);
   }
   else if (iequals(opt.name, "-boundingboxscale"sv)) {
      float bounding_box_scale = 1.0f;

      if (opt.arguments.empty() or
          std::from_chars(opt.arguments[0].data(),
                          opt.arguments[0].data() + opt.arguments[0].size(),
                          bounding_box_scale)
                .ec != std::errc{}) {
         throw read_error{"Invalid -boundingboxscale option.",
                          read_ec::option_load_bad_bounding_box_scale};
      }

      out.bounding_box_scale = bounding_box_scale;
   }
   else if (iequals(opt.name, "-boundingboxoffsetx"sv)) {
      float offset = 0.0f;

      if (opt.arguments.empty() or
          std::from_chars(opt.arguments[0].data(),
                          opt.arguments[0].data() + opt.arguments[0].size(), offset)
                .ec != std::errc{}) {
         throw read_error{"Invalid -boundingboxoffsetx option.",
                          read_ec::option_load_bad_bounding_box_offset};
      }

      out.bounding_box_offset.x = offset;
   }
   else if (iequals(opt.name, "-boundingboxoffsety"sv)) {
      float offset = 0.0f;

      if (opt.arguments.empty() or
          std::from_chars(opt.arguments[0].data(),
                          opt.arguments[0].data() + opt.arguments[0].size(), offset)
                .ec != std::errc{}) {
         throw read_error{"Invalid -boundingboxoffsety option.",
                          read_ec::option_load_bad_bounding_box_offset};
      }

      out.bounding_box_offset.y = offset;
   }
   else if (iequals(opt.name, "-boundingboxoffsetz"sv)) {
      float offset = 0.0f;

      if (opt.arguments.empty() or
          std::from_chars(opt.arguments[0].data(),
                          opt.arguments[0].data() + opt.arguments[0].size(), offset)
                .ec != std::errc{}) {
         throw read_error{"Invalid -boundingboxoffsety option.",
                          read_ec::option_load_bad_bounding_box_offset};
      }

      out.bounding_box_offset.z = offset;
   }
   else if (iequals(opt.name, "-boundingboxoffsetnz"sv)) {
      float offset = 0.0f;

      if (opt.arguments.empty() or
          std::from_chars(opt.arguments[0].data(),
                          opt.arguments[0].data() + opt.arguments[0].size(), offset)
                .ec != std::errc{}) {
         throw read_error{"Invalid -boundingboxoffsetnz option.",
                          read_ec::option_load_bad_bounding_box_offset};
      }

      out.bounding_box_offset.z = -offset;
   }
   else if (iequals(opt.name, "-kcollision"sv)) {
      out.k_collision = true;
   }
   else if (iequals(opt.name, "-donotmergecollision"sv)) {
      out.do_not_merge_collision = true;
   }
   else if (iequals(opt.name, "-removeverticesonmerge"sv)) {
      out.remove_vertices_on_merge = true;
   }
   else if (iequals(opt.name, "-ambientlighting"sv)) {
      if (opt.arguments.empty()) {
         throw read_error{"Invalid -ambientlighting option.",
                          read_ec::option_load_bad_ambient_lighting};
      }

      float3 ambient_lighting = {};

      const std::string_view red = string::split_first_of_exclusive(
         string::split_first_of_exclusive(opt.arguments[0], "r=")[1], " ")[0];
      const std::string_view green = string::split_first_of_exclusive(
         string::split_first_of_exclusive(opt.arguments[0], "g=")[1], " ")[0];
      const std::string_view blue = string::split_first_of_exclusive(
         string::split_first_of_exclusive(opt.arguments[0], "b=")[1], " ")[0];

      if (red.empty()) {
         throw read_error{"Invalid -ambientlighting option.",
                          read_ec::option_load_bad_ambient_lighting};
      }
      else if (green.empty()) {
         throw read_error{"Invalid -ambientlighting option.",
                          read_ec::option_load_bad_ambient_lighting};
      }
      else if (blue.empty()) {
         throw read_error{"Invalid -ambientlighting option.",
                          read_ec::option_load_bad_ambient_lighting};
      }

      if (std::from_chars(red.data(), red.data() + red.size(), ambient_lighting.x)
             .ec != std::error_code{}) {
         throw read_error{"Invalid -ambientlighting option.",
                          read_ec::option_load_bad_ambient_lighting};
      }

      if (std::from_chars(green.data(), green.data() + green.size(),
                          ambient_lighting.y)
             .ec != std::error_code{}) {
         throw read_error{"Invalid -ambientlighting option.",
                          read_ec::option_load_bad_ambient_lighting};
      }

      if (std::from_chars(blue.data(), blue.data() + blue.size(),
                          ambient_lighting.z)
             .ec != std::error_code{}) {
         throw read_error{"Invalid -ambientlighting option.",
                          read_ec::option_load_bad_ambient_lighting};
      }

      out.ambient_lighting = ambient_lighting;
   }
}

auto load_scene_options(const io::path& path, const options& directory_options) -> scene_options
{
   options file_options;

   try {
      file_options = parse_options(io::read_file_to_string(path));
   }
   catch (io::open_error& e) {
      if (e.code() != io::open_error_code::file_not_found) {
         throw read_error{e.what(), read_ec::option_load_io_open_error};
      }
   }
   catch (io::error& e) {
      throw read_error{e.what(), read_ec::option_load_io_generic_error};
   }

   scene_options results;

   for (const std::span<const assets::option>& options :
        {std::span<const assets::option>{directory_options},
         std::span<const assets::option>{file_options}}) {
      for (const option& opt : options) read_scene_option(opt, results);
   }

   return results;
}

}

auto read_scene(const std::span<const std::byte> bytes) -> scene
{
   try {
      ucfb::reader_strict<"HEDR"_id> hedr{bytes, {.aligned_children = false}};

      while (hedr) {
         auto msh_ = hedr.read_child();

         if (msh_.id() == "MSH1"_id) {
            throw read_error{"Version 1 .msh files are not supported.",
                             read_ec::read_version_not_supported};
         }
         else if (msh_.id() != "MSH2"_id) {
            continue;
         }

         scene result = read_msh2(ucfb::reader_strict<"MSH2"_id>{msh_});

         validate_scene(result);

         return result;
      }
   }
   catch (ucfb::read_error& e) {
      switch (e.code()) {
      case ucfb::read_ec::memory_too_small_minimum:
         throw read_error{e.what(), read_ec::ucfb_memory_too_small_minimum};
      case ucfb::read_ec::memory_too_small_chunk:
         throw read_error{e.what(), read_ec::ucfb_memory_too_small_chunk};
      case ucfb::read_ec::chunk_read_child_id_mismatch:
         throw read_error{e.what(), read_ec::ucfb_chunk_read_child_id_mismatch};
      case ucfb::read_ec::chunk_read_overrun:
         throw read_error{e.what(), read_ec::ucfb_chunk_read_overrun};
      case ucfb::read_ec::strict_reader_id_mismatch:
         throw read_error{e.what(), read_ec::ucfb_strict_reader_id_mismatch};
      }

      throw read_error{e.what(), read_ec::ucfb_unknown};
   }

   throw read_error{".msh file contained no scene.", read_ec::read_no_scene};
}

auto load_scene(const io::path& path, const options& directory_options) -> scene
{
   auto file = io::read_file_to_bytes(path);
   auto scene = read_scene(file);

   scene.options =
      load_scene_options(io::path{path} += ".option"sv, directory_options);

   return scene;
}
}
