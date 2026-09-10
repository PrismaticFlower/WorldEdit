#include "dust_effect_class.hpp"

#include "utility/random_gen.hpp"

#include "assets/odf/definition.hpp"

#include "math/vector_funcs.hpp"

#include "utility/string_icompare.hpp"

using we::string::iequals;

namespace we::world {

void parse(std::string_view str, float& value)
{
   std::string buffer{str};

   std::sscanf(buffer.c_str(), "%f", &value);
}

void parse(std::string_view str, float3& value)
{
   std::string buffer{str};

   std::sscanf(buffer.c_str(), "%f %f %f", &value.x, &value.y, &value.z);
}

void parse(std::string_view str, float4& value)
{
   std::string buffer{str};

   std::sscanf(buffer.c_str(), "%f %f %f %f", &value.x, &value.y, &value.z,
               &value.w);
}

void parse(std::string_view str, int& value)
{
   std::string buffer{str};

   std::sscanf(buffer.c_str(), "%i", &value);
}

auto pack_position(const float3& v) -> std::array<int16, 3>
{
   // The game uses [-400, 400] for the vertex box for dust effect particles.
   //
   // const float vertex_compress_sub = (400.0f + -400.0f) * 0.5f; // equals 0
   const float vertex_compress_mul = 32767.0f * 2.0f / (400.0f - -400.0f);

   return {static_cast<int16>(v.x * vertex_compress_mul),
           static_cast<int16>(v.y * vertex_compress_mul),
           static_cast<int16>(v.z * vertex_compress_mul)};
}

dust_effect_class::dust_effect_class(const assets::odf::definition& definition) noexcept
{
   for (const assets::odf::property& prop : definition.properties) {
      if (iequals("MinPos", prop.key)) {
         parse(prop.value, _min_position);
      }
      else if (iequals("MaxPos", prop.key)) {
         parse(prop.value, _max_position);
      }
      else if (iequals("MinVel", prop.key)) {
         parse(prop.value, _min_velocity);
      }
      else if (iequals("MaxVel", prop.key)) {
         parse(prop.value, _max_velocity);
      }
      else if (iequals("MinSize", prop.key)) {
         parse(prop.value, _min_size);
      }
      else if (iequals("MaxSize", prop.key)) {
         parse(prop.value, _max_size);
      }
      else if (iequals("MinLifeTime", prop.key)) {
         parse(prop.value, _min_life_time);
      }
      else if (iequals("MaxLifeTime", prop.key)) {
         parse(prop.value, _max_life_time);
      }
      else if (iequals("Alpha", prop.key)) {
         parse(prop.value, _color.w);
      }
      else if (iequals("Color", prop.key)) {
         parse(prop.value, _color);
      }
      else if (iequals("NumParticles", prop.key)) {
         parse(prop.value, _num_particles);

         _num_particles = std::max(_num_particles, 0);
      }
      else if (iequals("MinDistance", prop.key)) {
         parse(prop.value, _min_distance);
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
      else if (iequals("HeightScale", prop.key)) {
         parse(prop.value, _height_scale);
      }
      else if (iequals("Texture", prop.key)) {
         _texture = prop.value;
      }
      else if (iequals("CameraDistance", prop.key)) {
         parse(prop.value, _camera_distance);
      }
   }

   if (_num_particles <= 0) return;

   _particles = std::make_unique<particle[]>(_num_particles);

   for (int i = 0; i < _num_particles; ++i) {
      particle& particle = _particles[i];

      spawn_particle(particle);

      particle.lifespan *= _random.get_float();
   }

   _static_particles = std::make_unique<particle[]>(_num_particles);

   for (int i = 0; i < _num_particles; ++i) {
      particle& particle = _static_particles[i];

      particle = _particles[i];

      if (not update_particle(particle, particle.lifespan * _random.get_float())) {
         spawn_particle(particle);
      }
   }

   _bbox = {
      .min = _min_position - _max_size * 0.5f,
      .max = _max_position + _max_size * 0.5f,
   };

   _bbox.min.y *= _height_scale;
   _bbox.max.y *= _height_scale;
}

void dust_effect_class::update(double dbl_delta_time) noexcept
{
   const float delta_time = static_cast<float>(dbl_delta_time);

   for (int i = 0; i < _num_particles; ++i) {
      particle& particle = _particles[i];

      if (not update_particle(particle, delta_time)) {
         spawn_particle(particle);
      }
   }
}

void dust_effect_class::get_quads(
   const float4x4& world_matrix, [[maybe_unused]] const float3& light_direction,
   const bool animated, std::span<std::array<billboard_patch_vertex, 4>> out) const noexcept
{
   const std::ptrdiff_t output_count =
      std::min(std::ssize(out), std::ptrdiff_t{_num_particles});

   std::span<const particle> particles(animated ? _particles.get()
                                                : _static_particles.get(),
                                       _num_particles);

   const float3 x_axis = {world_matrix[0].x, world_matrix[0].y, world_matrix[0].z};
   const float3 y_axis = {world_matrix[1].x, world_matrix[1].y, world_matrix[1].z};

   uint32 color_base = 0;

   color_base |= static_cast<uint32>(_color.x * 255.0f + 0.5f);
   color_base |= static_cast<uint32>(_color.y * 255.0f + 0.5f) << 8;
   color_base |= static_cast<uint32>(_color.z * 255.0f + 0.5f) << 16;

   for (int particle_index = 0; particle_index < output_count; ++particle_index) {
      const particle& particle = particles[particle_index];

      const uint32 color =
         color_base | static_cast<uint32>(particle.alpha * 255.0f + 0.5f) << 24;

      const float3 x_axis_scaled = x_axis * particle.size;
      const float3 y_axis_scaled = y_axis * particle.size;

      float3 bottom_right =
         (particle.position - y_axis_scaled * 0.5f) - x_axis_scaled * 0.5f;

      std::array<billboard_patch_vertex, 4> quad;

      quad[0].position = pack_position(bottom_right);
      quad[0].normal = color;
      quad[0].texcoords = 0x800;

      quad[1].position = pack_position(bottom_right + x_axis_scaled);
      quad[1].normal = color;
      quad[1].texcoords = 0x8000800;

      quad[2].position = pack_position(bottom_right + y_axis_scaled + x_axis_scaled);
      quad[2].normal = color;
      quad[2].texcoords = 0x8000000;

      quad[3].position = pack_position(bottom_right + y_axis_scaled);
      quad[3].normal = color;
      quad[3].texcoords = 0x0;

      quad[0].darkness = quad[1].darkness = quad[2].darkness = quad[3].darkness = 0xff;

      std::memcpy(&out[particle_index], &quad, sizeof(quad));
   }
}

auto dust_effect_class::num_particles() const noexcept -> std::size_t
{
   return static_cast<std::size_t>(_num_particles);
}

auto dust_effect_class::height_scale() const noexcept -> float
{
   return _height_scale;
}

auto dust_effect_class::bbox() const noexcept -> const math::bounding_box&
{
   return _bbox;
}

auto dust_effect_class::texture() const noexcept -> const std::string&
{
   return _texture;
}

auto dust_effect_class::shader_type() const noexcept -> billboard_shader_type
{
   return billboard_shader_type::unlit_dust_particle;
}

void dust_effect_class::dust_effect_class::spawn_particle(particle& particle) noexcept
{
   const float3 position_randomness = {_random.get_float(), _random.get_float(),
                                       _random.get_float()};
   const float3 velocity_randomness = {_random.get_float(), _random.get_float(),
                                       _random.get_float()};

   particle.position =
      (_max_position - _min_position) * position_randomness + _min_position;
   particle.velocity =
      (_max_velocity - _min_velocity) * velocity_randomness + _min_velocity;
   particle.size = (_max_size - _min_size) * _random.get_float() + _min_size;
   particle.alpha = 0.0f;
   particle.lifespan =
      (_max_life_time - _min_life_time) * _random.get_float() + _min_life_time;
   particle.lifetime = 0.0f;
}

bool dust_effect_class::update_particle(particle& particle, float delta_time) noexcept
{
   particle.lifetime += delta_time;

   if (particle.lifetime > particle.lifespan) {
      return false;
   }

   particle.alpha = _color.w;

   particle.position += particle.velocity * delta_time;

   if (particle.lifespan * 0.3f <= particle.lifetime) {
      if (particle.lifespan - particle.lifetime < particle.lifespan * 0.5f) {
         particle.alpha *=
            (particle.lifespan - particle.lifetime) / (particle.lifespan * 0.5f);
      }
   }
   else {
      particle.alpha *= (particle.lifetime / (particle.lifespan * 0.3f));
   }

   if (_radius_fade_min > 0.0f) {
      const float distance_sq =
         dot(float2{particle.position.x, particle.position.z},
             float2{particle.position.x, particle.position.z});

      if (distance_sq < _radius_fade_min * _radius_fade_min) {
         if (distance_sq > _radius_fade_max * _radius_fade_max) {
            return false;
         }

         const float distance = sqrt(distance_sq);

         particle.alpha *= 1.0f - (distance - _radius_fade_min) /
                                     (_radius_fade_max - _radius_fade_min);
      }
   }

   return true;
}
}