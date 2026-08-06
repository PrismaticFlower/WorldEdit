#pragma once

#include "billboard_patch_class.hpp"

namespace we::assets::odf {

struct definition;

}

namespace we::world {

enum class leaf_patch_type { sphere, box, vine };

struct leaf_patch_class final : billboard_patch_class {
   explicit leaf_patch_class(const assets::odf::definition& definition) noexcept;

   void update(double delta_time) noexcept override;

   void get_quads(const float4x4& world_matrix, const float3& light_direction,
                  const bool animated,
                  std::span<std::array<billboard_patch_vertex, 4>> out) const noexcept override;

   auto num_particles() const noexcept -> std::size_t override;

   auto height_scale() const noexcept -> float override;

   auto bbox() const noexcept -> const math::bounding_box& override;

   auto texture() const noexcept -> const std::string& override;

private:
   struct random_gen;

   struct particle {
      float3 position;
      float wiggle;
      float size;
      double wiggle_accum;
      int8 variation;
      float darkness;
   };

   math::bounding_box _bbox;
   uint8 _max_falling_leaves = 0;
   uint8 _max_scatter_birds = 0;
   float _radius = 5.0f;
   float _height_scale = 1.0f;
   float _height = 5.0f;
   int _seed = 1; // Doesn't really matter for us.
   leaf_patch_type _type = leaf_patch_type::sphere;
   int _num_particles = 50;
   std::unique_ptr<particle[]> _particles;
   float3 _offset = {0.0f, 0.0f, 0.0f};
   float _min_size = 10.0f;
   float _max_size = 15.0f;
   float _alpha = 0.5f;
   float _max_distance = 100.0f;
   float _cone_height = 0.0f;
   float3 _box_size = {0.0f, 0.0f, 0.0f};
   std::string _texture;
   float _darkness_min = 1.0f;
   float _darkness_max = 1.0f;
   int _num_parts = 4;
   float _vineX = 1.0f;
   float _vineZ = 0.0f;
   int _vine_lengthX = 2;
   int _vine_lengthZ = 4;
   float _vine_spread = 0.6f;
   double _wiggle_speed = 1.2;
   float _wiggle_amount = 0.11f;
   int _num_visible = 0;

   void create_vine_branch(float3 position, float3 vine_vec, int num_particles,
                           int& particle_index, random_gen& random) noexcept;
};

}
