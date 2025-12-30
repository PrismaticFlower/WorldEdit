#include "scene_io.hpp"

#include "ucfb/writer.hpp"

using namespace we::ucfb::literals;

namespace we::assets::msh {

namespace {

void write_tx_d(ucfb::writer& tx_d, std::string_view name)
{
   if (not name.empty()) {
      tx_d.write(std::as_bytes(std::span{name}));
      tx_d.write('.', 'T', 'G', 'A', '\0');
   }
   else {
      tx_d.write('\0');
   }
}

void write_matd(ucfb::writer& matd, const material& material)
{
   // NAME
   {
      ucfb::writer name = matd.write_child("NAME"_id);

      name.write(material.name);
   }

   // DATA
   {
      ucfb::writer data = matd.write_child("DATA"_id);

      data.write(float4{}); // Diffuse Colour
      data.write(material.specular_color);
      data.write(0.0f);     // Specular Colour Alpha
      data.write(float4{}); // Ambient Colour
      data.write(50.0f);    // Specular Exponent/Decay
   }

   // ATRB
   {
      ucfb::writer atrb = matd.write_child("ATRB"_id);

      atrb.write(material.flags);
      atrb.write(material.rendertype);
      atrb.write(material.data0);
      atrb.write(material.data1);
   }

   // TX0D
   if (not material.textures[0].empty()) {
      ucfb::writer tx0d = matd.write_child("TX0D"_id);

      write_tx_d(tx0d, material.textures[0]);
   }

   // TX1D
   if (not material.textures[1].empty()) {
      ucfb::writer tx1d = matd.write_child("TX1D"_id);

      write_tx_d(tx1d, material.textures[1]);
   }

   // TX2D
   if (not material.textures[2].empty()) {
      ucfb::writer tx2d = matd.write_child("TX2D"_id);

      write_tx_d(tx2d, material.textures[2]);
   }

   // TX3D
   if (not material.textures[3].empty()) {
      ucfb::writer tx3d = matd.write_child("TX3D"_id);

      write_tx_d(tx3d, material.textures[3]);
   }
}

void write_matl(ucfb::writer& matl, const scene& scene)
{
   matl.write(static_cast<int32>(scene.materials.size()));

   for (const material& material : scene.materials) {
      ucfb::writer matd = matl.write_child("MATD"_id);

      write_matd(matd, material);
   }
}

void write_strp(ucfb::writer& strp, const std::span<const std::array<uint16, 3>> triangles)
{
   strp.write(static_cast<int32>(triangles.size() * 3));

   for (const auto& [i0, i1, i2] : triangles) {
      // TODO: Proper strips. This can not be left like this.
      strp.write<uint16, uint16, uint16>(i0 | uint16{0x8000u},
                                         i1 | uint16{0x8000u}, i2);
   }
}

void write_segm(ucfb::writer& segm, const geometry_segment& segment)
{
   // MATI
   {
      ucfb::writer mati = segm.write_child("MATI"_id);

      mati.write(segment.material_index);
   }

   // POSL
   {
      ucfb::writer posl = segm.write_child("POSL"_id);

      posl.write(static_cast<int32>(segment.positions.size()));
      posl.write(std::as_bytes(std::span{segment.positions}));
   }

   // NRML
   if (segment.normals) {
      ucfb::writer nrml = segm.write_child("NRML"_id);

      nrml.write(static_cast<int32>(segment.normals->size()));
      nrml.write(std::as_bytes(std::span{*segment.normals}));
   }

   // UV0L
   if (segment.texcoords) {
      ucfb::writer uv0l = segm.write_child("UV0L"_id);

      uv0l.write(static_cast<int32>(segment.texcoords->size()));

      for (const float2& texcoords : *segment.texcoords) {
         uv0l.write(texcoords.x, 1.0f - texcoords.y);
      }
   }

   // CLRL / CLRB
   if (segment.colors and not segment.colors->empty()) {
      const uint32 start_color = (*segment.colors)[0];
      bool single_color = true;

      for (const uint32 color : *segment.colors) {
         if (start_color != color) {
            single_color = false;

            break;
         }
      }

      if (single_color) {
         ucfb::writer clrb = segm.write_child("CLRB"_id);

         clrb.write(start_color);
      }
      else {
         ucfb::writer clrl = segm.write_child("CLRL"_id);

         clrl.write(static_cast<int32>(segment.colors->size()));
         clrl.write(std::as_bytes(std::span{*segment.colors}));
      }
   }

   // STRP
   {
      ucfb::writer strp = segm.write_child("STRP"_id);

      write_strp(strp, segment.triangles);
   }
}

void write_geom(ucfb::writer& geom, const node& node)
{
   // SEGM
   for (const geometry_segment& segment : node.segments) {
      ucfb::writer segm = geom.write_child("SEGM"_id);

      write_segm(segm, segment);
   }
}

void write_modl(ucfb::writer& modl, const node& node)
{
   // MTYP
   {
      ucfb::writer mtyp = modl.write_child("MTYP"_id);

      mtyp.write(node.type);
   }

   // NAME
   {
      ucfb::writer name = modl.write_child("NAME"_id);

      name.write(node.name);
   }

   // PRNT
   if (node.parent) {
      ucfb::writer prnt = modl.write_child("PRNT"_id);

      prnt.write(*node.parent);
   }

   // FLGS
   {
      ucfb::writer flgs = modl.write_child("FLGS"_id);

      flgs.write(node.hidden ? uint32{1} : uint32{0});
   }

   // TRAN
   if (node.parent) {
      ucfb::writer tran = modl.write_child("TRAN"_id);

      tran.write(float3{1.0f, 1.0f, 1.0f}); // scale, ignored by modelmunge
      tran.write(node.transform.rotation.x, node.transform.rotation.y,
                 node.transform.rotation.z, node.transform.rotation.w);
      tran.write(node.transform.translation);
   }

   // GEOM
   {
      ucfb::writer geom = modl.write_child("GEOM"_id);

      write_geom(geom, node);
   }

   // SWCI
   if (node.collision_primitive) {
      ucfb::writer swci = modl.write_child("SWCI"_id);

      swci.write(node.collision_primitive->shape);
      swci.write(node.collision_primitive->radius);
      swci.write(node.collision_primitive->height);
      swci.write(node.collision_primitive->length);
   }
}

}

void save_scene(const io::path& path, const scene& scene)
{
   // HEDR
   {
      ucfb::writer_root hedr{"HEDR"_id,
                             path,
                             {.child_trail_alignment_padding_included = true}};

      // MSH2
      {
         ucfb::writer msh2 = hedr.write_child("MSH2"_id);

         // MATL
         {
            ucfb::writer matl = msh2.write_child("MATL"_id);

            write_matl(matl, scene);
         }

         // MODL
         for (const node& node : scene.nodes) {
            ucfb::writer modl = msh2.write_child("MODL"_id);

            write_modl(modl, node);
         }
      }

      // CL1L
      {
         [[maybe_unused]] ucfb::writer CL1L = hedr.write_child("CL1L"_id);
      }
   }
}

}
