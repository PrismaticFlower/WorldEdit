#pragma once

#include "billboard_patch_class.hpp"

#include "utility/random_gen.hpp"

namespace we::assets::odf {

struct definition;

}

namespace we::world {

struct dust_effect_class final : billboard_patch_class {
   explicit dust_effect_class(const assets::odf::definition& definition) noexcept;

   void update(double delta_time) noexcept override;

   void get_quads(const float4x4& world_matrix, const float3& light_direction,
                  const bool animated,
                  std::span<std::array<billboard_patch_vertex, 4>> out) const noexcept override;

   auto num_particles() const noexcept -> std::size_t override;

   auto height_scale() const noexcept -> float override;

   auto bbox() const noexcept -> const math::bounding_box& override;

   auto texture() const noexcept -> const std::string& override;

   auto shader_type() const noexcept -> billboard_shader_type override;

private:
   struct particle {
      float3 position;
      float3 velocity;
      float size;
      float alpha;
      float lifespan;
      float lifetime;
   };

   math::bounding_box _bbox;

   int _num_particles = 50;
   std::unique_ptr<particle[]> _particles;
   std::unique_ptr<particle[]> _static_particles;

   float3 _min_position = {-20.0f, -1.0f, -20.0f};
   float3 _max_position = {20.0f, 1.0f, 20.0f};
   float3 _min_velocity = {-3.0f, -1.0f, -3.0f};
   float3 _max_velocity = {3.0f, 1.0f, 3.0f};

   float _min_size = 10.0f;
   float _max_size = 15.0f;

   float _min_life_time = 2.0f;
   float _max_life_time = 3.0f;

   float4 _color = {1.0f, 1.0f, 1.0f, 1.0f};

   float _min_distance = 0.0f;
   float _max_distance = 0.0f;

   float _radius_fade_min = -1.0f;
   float _radius_fade_max = -1.0f;

   float _height_scale = 0.2f;

   float _camera_distance = -1.0f;

   std::string _texture;

   random_gen _random;

   void spawn_particle(particle& particle) noexcept;

   bool update_particle(particle& particle, float delta_time) noexcept;
};

}
