#include "grass_patch_class.hpp"

#include "assets/odf/definition.hpp"

#include "math/vector_funcs.hpp"

#include "utility/string_icompare.hpp"

#include <bit>
#include <cstdio>

using we::string::iequals;

namespace we::world {

namespace {

void parse(std::string_view str, float& value)
{
   std::string buffer{str};

   std::sscanf(buffer.c_str(), "%f", &value);
}

void parse(std::string_view str, float& x, float& y)
{
   std::string buffer{str};

   std::sscanf(buffer.c_str(), "%f %f", &x, &y);
}

void parse(std::string_view str, int& value)
{
   std::string buffer{str};

   std::sscanf(buffer.c_str(), "%i", &value);
}

auto pack_position(const float3& v) -> std::array<int16, 3>
{
   return {static_cast<int16>(v.x * 655.35f), static_cast<int16>(v.y * 655.35f),
           static_cast<int16>(v.z * 655.35f)};
}

const std::array<uint32, 72> particle_texcoords = {{

   0x7ff0000u, 0x7ff07ffu, 0x0u,       0x7ffu,     0x7ff07ffu, 0x7ff0000u,
   0x7ffu,     0x0u,       0x7ff0000u, 0x7ff07ffu, 0x0u,       0x7ffu,
   0x7ff07ffu, 0x7ff0000u, 0x7ffu,     0x0u,       0x0u,       0x0u,
   0x0u,       0x0u,       0x0u,       0x0u,       0x0u,       0x0u,
   0x7ff0000u, 0x7ff03ffu, 0x4000000u, 0x40003ffu, 0x7ff0400u, 0x7ff07ffu,
   0x4000400u, 0x40007ffu, 0x3ff0000u, 0x3ff03ffu, 0x0u,       0x3ffu,
   0x3ff0400u, 0x3ff07ffu, 0x400u,     0x7ffu,     0x0u,       0x0u,
   0x0u,       0x0u,       0x0u,       0x0u,       0x0u,       0x0u,
   0x3ff0000u, 0x3ff01ffu, 0x0u,       0x1ffu,     0x3ff0200u, 0x3ff03ffu,
   0x200u,     0x3ffu,     0x3ff0400u, 0x3ff05ffu, 0x400u,     0x5ffu,
   0x3ff0600u, 0x3ff07ffu, 0x600u,     0x7ffu,     0x4000000u, 0x40003ffu,
   0x7ff0000u, 0x7ff03ffu, 0x4000400u, 0x40007ffu, 0x7ff0400u, 0x7ff07ffu,
}};

const std::array<float, 144> particle_uncompressed_texcoords = {{
   0.0f,  1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 1.0f,  0.0f, 1.0f,  1.0f, 0.0f,  1.0f,
   1.0f,  0.0f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 1.0f,  0.0f,
   1.0f,  1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f,
   0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f,
   0.0f,  1.0f, 0.5f,  1.0f, 0.0f,  0.5f, 0.5f,  0.5f, 0.5f,  1.0f, 1.0f,  1.0f,
   0.5f,  0.5f, 1.0f,  0.5f, 0.0f,  0.5f, 0.5f,  0.5f, 0.0f,  0.0f, 0.5f,  0.0f,
   0.5f,  0.5f, 1.0f,  0.5f, 0.5f,  0.0f, 1.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f,
   0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f,
   0.0f,  0.5f, 0.25f, 0.5f, 0.0f,  0.0f, 0.25f, 0.0f, 0.25f, 0.5f, 0.5f,  0.5f,
   0.25f, 0.0f, 0.5f,  0.0f, 0.5f,  0.5f, 0.75f, 0.5f, 0.5f,  0.0f, 0.75f, 0.0f,
   0.75f, 0.5f, 1.0f,  0.5f, 0.75f, 0.0f, 1.0f,  0.0f, 0.0f,  0.5f, 0.5f,  0.5f,
   0.0f,  1.0f, 0.5f,  1.0f, 0.5f,  0.5f, 1.0f,  0.5f, 0.5f,  1.0f, 1.0f,  1.0f,
}};

}

struct grass_patch_class::random_gen {
   auto operator()() noexcept -> int32
   {
      const uint32 v = state * 0x19660d + 0x3c6ef35f;
      state = v * 0x19660d + 0x3c6ef35f;

      return v >> 0x10 | state & 0xffff0000;
   }

   constexpr auto get_float() noexcept -> float
   {
      const uint32 v = state * 0x19660d + 0x3c6ef35f;
      state = v * 0x19660d + 0x3c6ef35f;

      return std::bit_cast<float>((v >> 0x10 | state & 0xffff0000) >> 9 | 0x3f800000) -
             1.0f;
   }

   uint32 state = 0x94153a94;
};

grass_patch_class::grass_patch_class(const assets::odf::definition& definition) noexcept
{
   for (const assets::odf::property& prop : definition.properties) {

      if (iequals("MinSize", prop.key)) {
         parse(prop.value, _min_size);
      }
      else if (iequals("MaxSize", prop.key)) {
         parse(prop.value, _max_size);
      }
      else if (iequals("Alpha", prop.key)) {
         parse(prop.value, _alpha);
      }
      else if (iequals("NumParticles", prop.key)) {
         parse(prop.value, _num_particles);

         _num_particles = std::max(_num_particles, 0);
      }
      else if (iequals("MaxDistance", prop.key)) {
         parse(prop.value, _max_distance);
      }
      else if (iequals("RadiusFadeMin", prop.key)) {
         parse(prop.value, _radius_fade_min);
      }
      else if (iequals("RadiusFadeMax", prop.key)) {
         parse(prop.value, _radius_fade_max);
      }
      else if (iequals("DarknessMin", prop.key)) {
         parse(prop.value, _darkness_min);
      }
      else if (iequals("DarknessMax", prop.key)) {
         parse(prop.value, _darkness_max);
      }
      else if (iequals("NumParts", prop.key)) {
         parse(prop.value, _num_parts);
      }
      else if (iequals("YOffset", prop.key)) {
         parse(prop.value, _y_offset);
      }
      else if (iequals("Texture", prop.key)) {
         _texture = prop.value;
      }
      else if (iequals("BoxSize", prop.key)) {
         parse(prop.value, _box_sizeX, _box_sizeZ);
      }
      else if (iequals("FlatHeight", prop.key)) {
         parse(prop.value, _flat_height_min, _flat_height_max);
      }
      else if (iequals("FlatSizeMultiplier", prop.key)) {
         parse(prop.value, _flat_size_multiplier);
      }
      else if (iequals("FlatFaceFactor", prop.key)) {
         parse(prop.value, _flat_face_factor);
      }
      else if (iequals("FlatShadowHeight", prop.key)) {
         parse(prop.value, _flat_shadow_height);
      }
      else if (iequals("FlatGrassSwing", prop.key)) {
         _flat_grass_swing = true;
      }
      else if (iequals("FlatCount", prop.key)) {
         parse(prop.value, _flat_count);
      }
      else if (iequals("SkinnyFactor", prop.key)) {
         parse(prop.value, _skinny_factor);
      }
      else if (iequals("MaxSkew", prop.key)) {
         parse(prop.value, _max_skew);
      }
      else if (iequals("TransparentType", prop.key)) {
         _transparent = true;
      }
   }

   if (_num_particles <= 0) return;

   _particles = std::make_unique<particle[]>(_num_particles);

   random_gen random;

   for (int particle_index = 0; particle_index < _num_particles; ++particle_index) {
      float flat_factor = 1.0f;

      if (_box_sizeX > 0.0 or _box_sizeZ > 0.0) {
         _particles[particle_index].position = {(random.get_float() - 0.5f) * _box_sizeX,
                                                0.0f,
                                                (random.get_float() - 0.5f) * _box_sizeZ};
      }
      else {
         do {
            _particles[particle_index].position =
               {(random.get_float() * 2.0f - 1.0f) * _radius_fade_max, 0.0f,
                (random.get_float() * 2.0f - 1.0f) * _radius_fade_max};

            float placement_length = length(_particles[particle_index].position);

            if (placement_length < _radius_fade_min) {
               flat_factor = 1.0;

               break;
            }

            if (_radius_fade_max <= placement_length) continue;

            flat_factor = 1.0f - (placement_length - _radius_fade_min) /
                                    (_radius_fade_max - _radius_fade_min);
         } while (flat_factor == 0.0f);
      }

      _particles[particle_index].size =
         (_max_size - _min_size) * random.get_float() + _min_size;
      _particles[particle_index].darkness = static_cast<uint8>(
         ((_darkness_max - _darkness_min) * random.get_float() + _darkness_min) * 255.0f +
         0.5f);
      _particles[particle_index].swing_accum = random.get_float() * 10.0;
      _particles[particle_index].swing = 0.0f;
      _particles[particle_index].flat = false;
      _particles[particle_index].skew = (random.get_float() - 0.5f) * _max_skew * 2.0f;

      if (_num_parts == 3 or _num_parts > 4) {
         bool flat = false;

         if (_num_parts == 3) {
            _particles[particle_index].variant =
               static_cast<int8>(particle_index % (_flat_count + 2));

            if (_particles[particle_index].variant > 1) {
               _particles[particle_index].variant = 2;
               flat = true;
            }
         }
         else {
            _particles[particle_index].variant =
               static_cast<int8>(particle_index % (_flat_count + 4));

            while (_particles[particle_index].variant > 5) {
               _particles[particle_index].variant -= 2;
            }

            flat = _particles[particle_index].variant > 3;
         }

         _particles[particle_index].position.y = _y_offset;

         if (flat == false) {
            _particles[particle_index].flat_size = _particles[particle_index].size;
            _particles[particle_index].size *= _skinny_factor;
            _particles[particle_index].axis_factor = random.get_float() * 0.2f + 0.8f;
         }
         else {
            _particles[particle_index].flat_size =
               _particles[particle_index].size * _flat_size_multiplier;
            _particles[particle_index].size =
               _particles[particle_index].size * _flat_size_multiplier;
            _particles[particle_index].axis_factor = random.get_float() * 0.1f + 0.05f;
            _particles[particle_index].flat = true;

            _particles[particle_index].position.y +=
               ((_flat_height_max - _flat_height_min) * random.get_float() *
                   (1.0f - flat_factor) +
                _flat_height_min) *
               _min_size;

            while (true) {
               _particles[particle_index].flat_vector = {random.get_float() - 0.5f, 0.0f,
                                                         random.get_float() - 0.5f};

               if (_particles[particle_index].flat_vector.x >= 0.01f) break;
               if (_particles[particle_index].flat_vector.z > 0.01f) break;
            }

            _particles[particle_index].flat_vector =
               normalize(_particles[particle_index].flat_vector);
            _particles[particle_index].flat_vector.y =
               (random.get_float() - 0.5f) * 0.3f;
            _particles[particle_index].flat_vector =
               normalize(_particles[particle_index].flat_vector);
         }
      }
      else {
         _particles[particle_index].axis_factor = 1.0f;
         _particles[particle_index].flat_size = _particles[particle_index].size * 0.9f;
         _particles[particle_index].size = _particles[particle_index].size * 1.1f;
         _particles[particle_index].variant = static_cast<int8>(particle_index & 0x3);
         _particles[particle_index].position.y =
            (1.0f - flat_factor) * _min_size * -0.5f + _y_offset;
      }
   }

   if (_box_sizeX > 0.0 or _box_sizeZ > 0.0) {
      _bbox = {.min = {-_box_sizeX * 0.5f - 0.5f * _max_size, _y_offset,
                       -_box_sizeZ * 0.5f - 0.5f * _max_size},
               .max = {_box_sizeX * 0.5f + 0.5f * _max_size, _max_size + _y_offset,
                       _box_sizeZ * 0.5f + 0.5f * _max_size}};
   }
   else {
      const float min = -_radius_fade_max - 0.5f * _max_size;
      const float max = _radius_fade_max + 0.5f * _max_size;

      _bbox = {.min = {min, _y_offset, min}, .max = {max, _max_size + _y_offset, max}};
   }
}

void grass_patch_class::update(double delta_time) noexcept
{
   for (int i = 0; i < _num_particles; ++i) {
      particle& particle = _particles[i];

      particle.swing_accum += ((i % 5) * 0.2 + 1.0) * delta_time * 0.8;
      particle.swing = static_cast<float>(std::sin(particle.swing_accum));
   }
}

void grass_patch_class::get_quads(
   const float4x4& world_matrix, [[maybe_unused]] const float3& light_direction,
   const bool animated, std::span<std::array<billboard_patch_vertex, 4>> out) const noexcept

{
   const std::ptrdiff_t output_count =
      std::min(std::ssize(out), std::ptrdiff_t{_num_particles});

   const float3 x_axis = {world_matrix[0].x, world_matrix[0].y, world_matrix[0].z};
   float3 z_axis = {world_matrix[2].x, world_matrix[2].y, world_matrix[2].z};

   float3 y_axis = {0.0f, 1.0f, 0.0f};

   const float ZdotY = dot(z_axis, y_axis);

   y_axis = normalize(y_axis - (z_axis * ZdotY * 0.7f));

   std::array<float3, 10> x_axes = {};

   x_axes[0] = z_axis * 0.0f + x_axis * 1.1f;
   x_axes[1] = z_axis * -0.055f + x_axis * 1.045f;
   x_axes[2] = z_axis * 0.11f + x_axis * 0.99f;
   x_axes[3] = z_axis * -0.165f + x_axis * 0.9350001f;
   x_axes[4] = z_axis * 0.22f + x_axis * 0.8800001f;
   x_axes[5] = z_axis * -0.0f + x_axis * 1.1f;
   x_axes[6] = z_axis * 0.055f + x_axis * 1.045f;
   x_axes[7] = z_axis * -0.11f + x_axis * 0.99f;
   x_axes[8] = z_axis * 0.165f + x_axis * 0.9350001f;
   x_axes[9] = z_axis * -0.22f + x_axis * 0.8800001f;

   z_axis = {-world_matrix[2].x, 0.0f, -world_matrix[2].z};

   if (z_axis.x == 0.0f and z_axis.z == 0.0f) {
      z_axis.x = 1.0f;
   }
   else {
      z_axis = normalize(z_axis);
   }

   const uint32 normal_alpha =
      0x7f'ff'7f | (static_cast<uint32>(_alpha * 255.0f + 0.5f) << 24u);

   for (int particle_index = 0; particle_index < output_count; ++particle_index) {
      const particle& particle = _particles[particle_index];

      float3 x_offset;
      float3 yz_offset;
      float3 x_neg_offset;

      if (not particle.flat) {
         const int x_axis_variant = particle_index % 10;

         float swing = 0.0f;

         if (animated) swing = particle.swing * 0.08f - 0.5f;

         x_offset = x_axes[x_axis_variant] * particle.size;

         yz_offset = ((y_axis - z_axis) * particle.axis_factor + z_axis) *
                        particle.flat_size +
                     particle.position + swing * x_offset;

         x_neg_offset = particle.position - x_offset * 0.5f;
      }
      else {
         float swing = 0.0f;

         if (_flat_grass_swing and animated) swing = particle.swing * 0.03f;

         swing += _flat_face_factor;

         float3 z_swing;

         z_swing.x = 0.0f - z_axis.x * swing;
         z_swing.y = 1.0f - z_axis.y * swing;
         z_swing.z = 0.0f - z_axis.z * swing;

         x_offset = cross(particle.flat_vector, z_swing);
         x_offset.y += particle.flat_vector.y;
         x_offset = normalize(x_offset);

         float3 xz_vec = normalize(cross(x_offset, z_swing));

         const float size = particle.size;

         x_offset *= size;

         xz_vec = size * xz_vec * 0.5f;

         yz_offset = (xz_vec + particle.position) - x_offset * 0.5f;

         x_neg_offset = (particle.position - xz_vec) - x_offset * 0.5f;
      }

      float y_size_scale = 0.0f;
      float y_offset = x_neg_offset.y;

      if (particle.position.y < 0.0f) {
         y_size_scale = -(particle.position.y / particle.size);

         y_offset = (yz_offset.y - x_neg_offset.y) * y_size_scale + x_neg_offset.y;
         x_neg_offset.x =
            (yz_offset.x - x_neg_offset.x) * y_size_scale + x_neg_offset.x;
         x_neg_offset.z =
            (yz_offset.z - x_neg_offset.z) * y_size_scale + x_neg_offset.z;
      }

      int texcoords_multiplier = 0;

      if (_num_parts == 4) {
         texcoords_multiplier = 1;
      }
      else if (_num_parts == 6) {
         texcoords_multiplier = 2;
      }

      const uint32 texcoords_index =
         std::min(static_cast<uint32>(particle.variant + texcoords_multiplier * 6),
                  static_cast<uint32>(particle_texcoords.size() / 4 - 1));

      x_neg_offset.y = particle_uncompressed_texcoords[texcoords_index * 8 + 1];

      if (y_size_scale == 0.0f) {
         x_neg_offset.y = (particle_uncompressed_texcoords[texcoords_index * 8 + 5] -
                           x_neg_offset.y) *
                             y_size_scale +
                          x_neg_offset.y;
      }

      const uint32 y_top_texcoord =
         static_cast<uint32>(x_neg_offset.y * 2048.0f + 0.5f) << 16u;

      std::array<billboard_patch_vertex, 4> quad;

      quad[0].position = pack_position(yz_offset);
      quad[0].darkness = particle.darkness;
      quad[0].normal = normal_alpha;
      quad[0].texcoords = particle_texcoords[texcoords_index * 4 + 2];

      quad[1].position = pack_position(x_offset + yz_offset);
      quad[1].darkness = particle.darkness;
      quad[1].normal = normal_alpha;
      quad[1].texcoords = particle_texcoords[texcoords_index * 4 + 3];

      quad[2].position =
         pack_position({x_neg_offset.x + x_offset.x, x_offset.y + y_offset,
                        x_offset.z + x_neg_offset.z});
      quad[2].darkness = particle.darkness;
      quad[2].normal = normal_alpha;
      quad[2].texcoords =
         (particle_texcoords[texcoords_index * 4 + 1] & 0xffffu) | y_top_texcoord;

      quad[3].position = pack_position({x_neg_offset.x, y_offset, x_neg_offset.z});
      quad[3].darkness = particle.darkness;
      quad[3].normal = normal_alpha;
      quad[3].texcoords =
         (particle_texcoords[texcoords_index * 4 + 0] & 0xffffu) | y_top_texcoord;

      std::memcpy(&out[particle_index], &quad, sizeof(quad));
   }
}

auto grass_patch_class::num_particles() const noexcept -> std::size_t
{
   return static_cast<std::size_t>(_num_particles);
}

auto grass_patch_class::height_scale() const noexcept -> float
{
   return 1.0f;
}

auto grass_patch_class::bbox() const noexcept -> const math::bounding_box&
{
   return _bbox;
}

auto grass_patch_class::texture() const noexcept -> const std::string&
{
   return _texture;
}

bool grass_patch_class::is_transparent() const noexcept
{
   return _transparent;
}

}
