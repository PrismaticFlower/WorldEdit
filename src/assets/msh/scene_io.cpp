
#include "scene_io.hpp"
#include "../option_file.hpp"
#include "io/read_file.hpp"
#include "ucfb/reader.hpp"
#include "utility/string_icompare.hpp"
#include "validate_scene.hpp"

#include <charconv>
#include <numeric>
#include <stdexcept>

#include <fmt/core.h>

#pragma warning(disable : 4063) // case is not a valid value for switch of enum

using namespace we::ucfb::literals;
using namespace std::literals;

namespace we::assets::msh {

namespace {

auto from_msh_space(float3 vec) -> float3
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

auto read_geom(ucfb::reader_strict<"GEOM"_id> geom) -> std::vector<geometry_segment>
{
   std::vector<geometry_segment> segments;
   segments.reserve(4);

   while (geom) {
      auto child = geom.read_child();

      switch (child.id()) {
      case "SEGM"_id:
         segments.emplace_back(read_segm(ucfb::reader_strict<"SEGM"_id>{child}));
         continue;
      }
   }

   return segments;
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

auto read_modl(ucfb::reader_strict<"MODL"_id> modl) -> node
{
   node node;

   while (modl) {
      auto child = modl.read_child();

      switch (child.id()) {
      case "MTYP"_id:
         node.type = child.read<node_type>();
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
         node.segments = read_geom(ucfb::reader_strict<"GEOM"_id>{child});
         continue;
      case "SWCI"_id:
         node.collision_primitive = read_swci(ucfb::reader_strict<"SWCI"_id>{child});
         continue;
      }
   }

   return node;
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
         throw std::runtime_error{fmt::format(
            ".msh file material list (MATL) ended after {} materials but the "
            "declared count was {}.",
            i, count)};
      }

      materials.emplace_back(read_matd(matl.read_child_strict<"MATD"_id>()));
   }

   return materials;
}

auto read_msh2(ucfb::reader_strict<"MSH2"_id> msh2) -> scene
{
   scene scene;

   while (msh2) {
      auto child = msh2.read_child();

      switch (child.id()) {
      case "MATL"_id:
         scene.materials = read_matl(ucfb::reader_strict<"MATL"_id>{child});
         continue;
      case "MODL"_id:
         scene.nodes.emplace_back(read_modl(ucfb::reader_strict<"MODL"_id>{child}));
         continue;
      }
   }

   return scene;
}

}

auto read_scene(const std::span<const std::byte> bytes) -> scene
{
   ucfb::reader_strict<"HEDR"_id> hedr{bytes};

   while (hedr) {
      auto msh_ = hedr.read_child();

      if (msh_.id() == "MSH1"_id) {
         throw std::runtime_error{"Version 1 .msh files are not supported."};
      }
      else if (msh_.id() != "MSH2"_id) {
         continue;
      }

      scene result = read_msh2(ucfb::reader_strict<"MSH2"_id>{msh_});

      validate_scene(result);

      return result;
   }

   throw std::runtime_error{".msh file contained no scene."};
}

auto read_scene(const io::path& path) -> scene
{
   auto file = io::read_file_to_bytes(path);
   auto scene = read_scene(file);

   if (auto option_path = io::path{path} += ".option"sv; io::exists(option_path)) {
      scene.options = read_scene_options(option_path);
   }

   return scene;
}

auto read_scene_options(const io::path& path) -> options
{
   options results;

   for (auto& opt : parse_options(io::read_file_to_string(path))) {
      if (string::iequals(opt.name, "-bump"sv)) {
         results.normal_maps.assign(opt.arguments.begin(), opt.arguments.end());
      }
      else if (string::iequals(opt.name, "-additiveemissive"sv)) {
         results.additive_emissive = true;
      }
      else if (string::iequals(opt.name, "-vertexlighting"sv)) {
         results.vertex_lighting = true;
      }
      else if (string::iequals(opt.name, "-ambientlighting"sv) and
               not opt.arguments.empty()) {
         float3 ambient_lighting = {};

         const std::string_view red = string::split_first_of_exclusive(
            string::split_first_of_exclusive(opt.arguments[0], "r=")[1], " ")[0];
         const std::string_view green = string::split_first_of_exclusive(
            string::split_first_of_exclusive(opt.arguments[0], "g=")[1], " ")[0];
         const std::string_view blue = string::split_first_of_exclusive(
            string::split_first_of_exclusive(opt.arguments[0], "b=")[1], " ")[0];

         if (std::from_chars(red.data(), red.data() + red.size(),
                             ambient_lighting.x)
                .ec != std::error_code{}) {
            ambient_lighting.x = 0.0f;
         }

         if (std::from_chars(green.data(), green.data() + green.size(),
                             ambient_lighting.y)
                .ec != std::error_code{}) {
            ambient_lighting.y = 0.0f;
         }

         if (std::from_chars(blue.data(), blue.data() + blue.size(),
                             ambient_lighting.z)
                .ec != std::error_code{}) {
            ambient_lighting.z = 0.0f;
         }

         results.ambient_lighting = ambient_lighting;
      }
      else if (string::iequals(opt.name, "-scale"sv) and not opt.arguments.empty()) {
         if (std::from_chars(opt.arguments[0].data(),
                             opt.arguments[0].data() + opt.arguments[0].size(),
                             results.scale)
                .ec != std::error_code{}) {
            results.scale = 1.0f;
         }
      }
   }

   return results;
}

}
