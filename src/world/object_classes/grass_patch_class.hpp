#pragma once

#include "billboard_patch_class.hpp"

namespace we::assets::odf {

struct definition;

}

namespace we::world {

struct grass_patch_class final : billboard_patch_class {
   explicit grass_patch_class(const assets::odf::definition& definition) noexcept;

   void update(double delta_time) noexcept override;

   void get_quads(const float4x4& world_matrix, const float3& light_direction,
                  const bool animated,
                  std::span<std::array<billboard_patch_vertex, 4>> out) const noexcept override;

   auto num_particles() const noexcept -> std::size_t override;

   auto height_scale() const noexcept -> float override;

   auto bbox() const noexcept -> const math::bounding_box& override;

   auto texture() const noexcept -> const std::string& override;

   bool is_transparent() const noexcept override;

private:
   struct random_gen;

   struct particle {
      float3 position;
      float3 flat_vector;
      float size;
      double swing_accum;
      float swing;
      int8 variant;
      uint8 darkness;
      float flat_size;
      float axis_factor;
      float skew;
      bool flat;
   };

   math::bounding_box _bbox;
   int _num_particles = 50;
   std::unique_ptr<particle[]> _particles;
   float _min_size = 10.0f;
   float _max_size = 15.0f;
   float _y_offset = 0.0f;
   float _alpha = 0.5f;
   float _max_distance = 100.0f;
   std::string _texture = "grass";
   float _radius_fade_min = -1.0f;
   float _radius_fade_max = -1.0f;
   float _darkness_min = 1.0f;
   float _darkness_max = 1.0f;
   int _num_parts = 4;
   float _box_sizeX = -1.0f;
   float _box_sizeZ = -1.0f;
   float _flat_height_min = 0.2f;
   float _flat_height_max = 0.2f;
   float _flat_size_multiplier = 2.0f;
   float _flat_face_factor = 0.5f;
   float _max_skew = 0.1f;
   float _flat_shadow_height = 0.0f;
   int _flat_count = 6;
   float _skinny_factor = 0.3f;
   bool _flat_grass_swing = false;
   bool _transparent = false;
};

}
