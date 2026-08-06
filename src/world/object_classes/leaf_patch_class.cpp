#include "leaf_patch_class.hpp"

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

void parse(std::string_view str, float3& value)
{
   std::string buffer{str};

   std::sscanf(buffer.c_str(), "%f %f %f", &value.x, &value.y, &value.z);
}

void parse(std::string_view str, double& value)
{
   float intermediate = static_cast<float>(value);

   std::string buffer{str};

   std::sscanf(buffer.c_str(), "%f", &intermediate);

   value = intermediate;
}

void parse(std::string_view str, int& value)
{
   std::string buffer{str};

   std::sscanf(buffer.c_str(), "%i", &value);
}

void parse(std::string_view str, int& x, int& y)
{
   std::string buffer{str};

   std::sscanf(buffer.c_str(), "%i %i", &x, &y);
}

void parse(std::string_view str, uint8& value)
{
   int intermediate = value;

   std::string buffer{str};

   std::sscanf(buffer.c_str(), "%i", &intermediate);

   value = static_cast<uint8>(intermediate);
}

auto pack_position(const float3& v) -> std::array<int16, 3>
{
   return {static_cast<int16>(v.x * 655.35f), static_cast<int16>(v.y * 655.35f),
           static_cast<int16>(v.z * 655.35f)};
}

const std::array<uint32, 32> particle_texcoords = {{
   0x7ff0000, 0x7ff07ff, 0x0,       0x7ff,     0x7ff07ff, 0x7ff0000, 0x7ff,
   0x0,       0x7ff0000, 0x7ff07ff, 0x0,       0x7ff,     0x7ff07ff, 0x7ff0000,
   0x7ff,     0x0,       0x7ff0000, 0x7ff03ff, 0x4000000, 0x40003ff, 0x7ff0400,
   0x7ff07ff, 0x4000400, 0x40007ff, 0x3ff0000, 0x3ff03ff, 0x0,       0x3ff,
   0x3ff0400, 0x3ff07ff, 0x400,     0x7ff,
}};

}

struct leaf_patch_class::random_gen {
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

leaf_patch_class::leaf_patch_class(const assets::odf::definition& definition) noexcept
{
   for (const assets::odf::property& prop : definition.properties) {
      if (iequals("MaxFallingLeaves", prop.key)) {
         parse(prop.value, _max_falling_leaves);

         _max_falling_leaves = std::min(_max_falling_leaves, uint8{50});
      }
      else if (iequals("MaxScatterBirds", prop.key)) {
         parse(prop.value, _max_scatter_birds);

         _max_scatter_birds = std::min(_max_scatter_birds, uint8{5});
      }
      else if (iequals("Radius", prop.key)) {
         parse(prop.value, _radius);
      }
      else if (iequals("HeightScale", prop.key)) {
         parse(prop.value, _height_scale);
      }
      else if (iequals("Height", prop.key)) {
         parse(prop.value, _height);
      }
      else if (iequals("Seed", prop.key)) {
         parse(prop.value, _seed);
      }
      else if (iequals("NumParticles", prop.key)) {
         parse(prop.value, _num_particles);

         _num_particles = std::max(_num_particles, 0);
      }
      else if (iequals("Offset", prop.key)) {
         parse(prop.value, _offset);
      }
      else if (iequals("MinSize", prop.key)) {
         parse(prop.value, _min_size);
      }
      else if (iequals("MaxSize", prop.key)) {
         parse(prop.value, _max_size);
      }
      else if (iequals("Alpha", prop.key)) {
         parse(prop.value, _alpha);
      }
      else if (iequals("MaxDistance", prop.key)) {
         parse(prop.value, _max_distance);
      }
      else if (iequals("ConeHeight", prop.key)) {
         parse(prop.value, _cone_height);
      }
      else if (iequals("BoxSize", prop.key)) {
         parse(prop.value, _box_size);

         _type = leaf_patch_type::box;
      }
      else if (iequals("Texture", prop.key)) {
         _texture = prop.value;
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
      else if (iequals("Vine", prop.key)) {
         parse(prop.value, _vineX, _vineZ);

         _type = leaf_patch_type::vine;
      }
      else if (iequals("VineLength", prop.key)) {
         parse(prop.value, _vine_lengthX, _vine_lengthZ);
      }
      else if (iequals("VineSpread", prop.key)) {
         parse(prop.value, _vine_spread);
      }
      else if (iequals("WiggleSpeed", prop.key)) {
         parse(prop.value, _wiggle_speed);
      }
      else if (iequals("WiggleAmount", prop.key)) {
         parse(prop.value, _wiggle_amount);
      }
      else if (iequals("NumVisible", prop.key)) {
         parse(prop.value, _num_visible);
      }
   }

   if (_num_particles == 0) return;

   _particles = std::make_unique<particle[]>(_num_particles);

   const float inv_height_scale = 1.0f / _height_scale;

   random_gen random = {
      .state = ((_seed ^ 0xc71c2830) * 0x19660d + 0x3c6ef35f) * 0x19660d + 0x3c6ef35f};

   uint32 sphere_attempts = 0;

   _bbox = {.min = {100.0f, 100.0f, 100.0f}, .max = {-100.0f, -100.0f, -100.0f}};

   if (_type == leaf_patch_type::vine) {
      int particle_index = 0;

      create_vine_branch({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, _num_particles,
                         particle_index, random);
   }

   for (int i = 0; i < _num_particles; ++i) {
      if (_type != leaf_patch_type::vine) {
         if (_type == leaf_patch_type::box) {
            float3 position;

            position.z = random.get_float();
            position.y = random.get_float();
            position.x = random.get_float();

            _particles[i].position = (position - 0.5f) * _box_size;
         }
         else {
            // The game doesn't actually put a limit but I'm going to for sanity's sake.
            const std::size_t max_attempts = 0x10000;

            const float radius_sq = _radius * _radius;

            for (std::size_t attmept = 0; attmept < max_attempts; ++attmept) {
               float3 position;

               position.z = random.get_float() * _radius * 1.5f;
               position.y = random.get_float() * _radius;
               position.x = random.get_float() * _radius * 1.5f;

               _particles[i].position = position;

               if (sphere_attempts < 2) {
                  _particles[i].position.x = -_particles[i].position.x;
               }
               if (sphere_attempts == 1 or sphere_attempts == 2) {
                  _particles[i].position.z = -_particles[i].position.z;
               }

               sphere_attempts = (sphere_attempts + 1) & 0x3;

               if (radius_sq > dot(_particles[i].position, _particles[i].position)) {
                  break;
               }
            }

            _particles[i].position.y *= (inv_height_scale * _height) / _radius;

            if (0.0f < _cone_height and
                _height - _cone_height < _particles[i].position.y) {
               const float height_offset_div_cone =
                  (_height - _particles[i].position.y) / _cone_height;

               _particles[i].position.x *= height_offset_div_cone;
               _particles[i].position.z *= height_offset_div_cone;
            }
         }

         const float random_size = random.get_float();

         _particles[i].size = random_size * (_max_size - _min_size) + _min_size;
      }

      const float darkness_range = _darkness_max - _darkness_min;
      const float darkness_randomness = random.get_float();
      const float darkness = darkness_randomness * darkness_range * 0.5f + _darkness_min;

#if 0
      // Keeping this here for reference but this is done in get_quads

      const float3 light_dirWS = ...;

      const float NdotL = dot(normalize(_particles[i].position), light_dirWS);
      const float lighting = darkness - NdotL * darkness_range * 0.5f;

      _particles[i].color.x = lighting;
      _particles[i].color.y = lighting;
      _particles[i].color.z = lighting;
      _particles[i].color.w = _alpha;
#else
      _particles[i].darkness = darkness;
#endif

      _particles[i].wiggle_accum = random.get_float() * 100.0f;
      _particles[i].variation = i & 0x3;

      _particles[i].position.x += _offset.x;
      _particles[i].position.y += _offset.y * inv_height_scale;
      _particles[i].position.z += _offset.z;

      const float half_particle_size = _particles[i].size * 0.5f;

      _bbox.min =
         min(_bbox.min, {_particles[i].position.x - half_particle_size,
                         (_particles[i].position.y - half_particle_size) * _height_scale,
                         _particles[i].position.z - half_particle_size});
      _bbox.max =
         max(_bbox.max, {_particles[i].position.x + half_particle_size,
                         (_particles[i].position.y + half_particle_size) * _height_scale,
                         _particles[i].position.z + half_particle_size});
   }
}

void leaf_patch_class::create_vine_branch(float3 position, float3 vine_vec,
                                          int num_particles, int& particle_index,
                                          random_gen& random) noexcept
{
   int remaining_num_particles = num_particles - 1;

   if (-1 < remaining_num_particles) {
      float size_factor = (remaining_num_particles + 1.0f) * 0.25f;

      if (1.0f <= size_factor) {
         size_factor = 1.0f;
      }

      float size =
         size_factor * (random.get_float() * (_max_size - _min_size) + _min_size);

      assert(particle_index < _num_particles);

      _particles[particle_index].position = position;
      _particles[particle_index].size = size;

      particle_index += 1;

      int span_length = static_cast<int>(
         random.get_float() * (_vine_lengthZ - _vine_lengthX) + _vine_lengthX);

      if (remaining_num_particles <= span_length) {
         span_length = remaining_num_particles;
      }

      for (int i = span_length; i != 0; --i) {
         vine_vec.z += (random.get_float() - random.get_float()) * _vineZ * 0.5f;
         vine_vec.y += 0.2f;
         vine_vec.x += (random.get_float() - random.get_float()) * _vineX * 0.5f;

         float3 vine_dir = normalize(vine_vec);
         float3 vine_offset = vine_dir * size * _vine_spread;

         position += vine_offset;

         size_factor = (remaining_num_particles + 1.0f) * 0.25f;

         if (1.0f <= size_factor) {
            size_factor = 1.0f;
         }

         size = (size_factor *
                 (random.get_float() * (_max_size - _min_size) + _min_size));

         assert(particle_index < _num_particles);

         _particles[particle_index].position = position;
         _particles[particle_index].size = size;

         particle_index += 1;

         vine_vec = vine_dir;

         remaining_num_particles = remaining_num_particles - 1;
      }

      if (0 < remaining_num_particles) {
         float3 vine_split_vec;

         vine_split_vec.z = (random.get_float() - random.get_float()) * _vineZ;
         vine_split_vec.y = 0.0f;
         vine_split_vec.x = (random.get_float() - random.get_float()) * _vineX;

         float3 branch_vec = lerp(vine_vec, vine_split_vec, 0.8f);
         float3 branch_dir = normalize(branch_vec);

         int branch_length = static_cast<int>((random.get_float() * 0.5 + 0.25) *
                                              remaining_num_particles);

         float3 branch_offset = branch_dir * size * _vine_spread;

         create_vine_branch(branch_offset + position, branch_dir, branch_length,
                            particle_index, random);

         vine_split_vec.z = (random.get_float() - random.get_float()) * _vineZ;
         vine_split_vec.y = 0.0f;
         vine_split_vec.x = (random.get_float() - random.get_float()) * _vineX;

         branch_vec = lerp(vine_vec, vine_split_vec, 0.8f);
         branch_dir = normalize(branch_vec);

         branch_offset = branch_dir * size * _vine_spread;

         create_vine_branch(branch_offset + position, branch_dir,
                            remaining_num_particles - branch_length,
                            particle_index, random);
      }
   }
}

void leaf_patch_class::update(double delta_time) noexcept
{
   if (_wiggle_speed == 0.0 or _wiggle_amount == 0.0f) return;

   for (int i = 0; i < _num_particles; ++i) {
      particle& particle = _particles[i];

      particle.wiggle_accum += _wiggle_speed * delta_time;
      particle.wiggle =
         static_cast<float>(std::sin(particle.wiggle_accum)) * _wiggle_amount;
   }
}

void leaf_patch_class::get_quads(const float4x4& world_matrix,
                                 const float3& light_direction, const bool animated,
                                 std::span<std::array<billboard_patch_vertex, 4>> out) const noexcept

{
   const std::ptrdiff_t output_count =
      std::min(std::ssize(out), std::ptrdiff_t{_num_particles});

   const float3 x_axis = {world_matrix[0].x, world_matrix[0].y, world_matrix[0].z};
   const float3 y_axis = {world_matrix[1].x, world_matrix[1].y, world_matrix[1].z};
   const float3 z_axis = {world_matrix[2].x, world_matrix[2].y, world_matrix[2].z};

   std::array<float3, 10> x_axes = {};

   for (std::size_t i = 0; i < x_axes.size(); ++i) {
      const float permutation = (1.0f - (i & 1) * 2) * (i % 5) * 0.3f;

      x_axes[i] = normalize(x_axis + z_axis * permutation);
   }

   const bool num_parts_is_4 = _num_parts == 4;
   const float darkness_range = _darkness_max - _darkness_min;

   for (int particle_index = 0; particle_index < output_count; ++particle_index) {
      const particle& particle = _particles[particle_index];
      const float3& position = particle.position;
      const float wiggle = animated ? particle.wiggle : 0.0f;
      const uint32 variation = particle.variation;
      const float size = particle.size;

      const float NdotL = dot(normalize(position), light_direction);
      const float darkness = particle.darkness - NdotL * darkness_range * 0.5f;

      const float3 offset = x_axes[particle_index % 10] * size;
      const float3 particle_z_axis = position + z_axis * 0.1f;

      const float wiggle_offset = wiggle - 0.5f;

      const float3 position_top = y_axis * size * 0.5f;

      const float3 position_top_left =
         position_top + particle_z_axis + offset * wiggle_offset;
      const float3 position_bottom_right =
         (particle_z_axis - position_top) - offset * 0.5f;

      const uint32 texcoords_index =
         std::min(static_cast<uint32>(variation & 0xff) + num_parts_is_4 * 4, 28u);

      const uint8 darkness_u8 = static_cast<uint8>(darkness * 255.0f + 0.5f);

      std::array<billboard_patch_vertex, 4> quad;

      quad[0].position = pack_position(position_top_left);
      quad[0].darkness = darkness_u8;
      // quad[0].normal = normalize(quad[0].position); // Kept for posterity and clarity.
      quad[0].texcoords = particle_texcoords[texcoords_index * 4 + 2];

      quad[1].position = pack_position(position_top_left + offset);
      quad[1].darkness = darkness_u8;
      // quad[1].normal = normalize(quad[1].position);
      quad[1].texcoords = particle_texcoords[texcoords_index * 4 + 3];

      quad[2].position = pack_position(position_bottom_right + offset);
      quad[2].darkness = darkness_u8;
      // quad[2].normal = normalize(quad[2].position);
      quad[2].texcoords = particle_texcoords[texcoords_index * 4 + 1];

      quad[3].position = pack_position(position_bottom_right);
      quad[3].darkness = darkness_u8;
      // quad[3].normal = normalize(quad[3].position);
      quad[3].texcoords = particle_texcoords[texcoords_index * 4];

      std::memcpy(&out[particle_index], &quad, sizeof(quad));
   }
}

auto leaf_patch_class::num_particles() const noexcept -> std::size_t
{
   return static_cast<std::size_t>(_num_particles);
}

auto leaf_patch_class::height_scale() const noexcept -> float
{
   return _height_scale;
}

auto leaf_patch_class::bbox() const noexcept -> const math::bounding_box&
{
   return _bbox;
}

auto leaf_patch_class::texture() const noexcept -> const std::string&
{
   return _texture;
}

}
