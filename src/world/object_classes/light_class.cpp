#include "light_class.hpp"

#include <array>
#include <vector>

namespace we::world {

enum class flicker_type { none };

struct light_class::impl {
   explicit impl(const assets::odf::definition& definition) noexcept;

   void update(double delta_time) noexcept;

   auto light_description() const noexcept -> const light_class_light_description&
   {
      return _light_description;
   }

   auto textures() const noexcept -> std::span<const std::string>
   {
      return _textures;
   }

private:
   light_class_light_description _light_description;

   std::array<uint8, 4> _color = {255, 255, 255, 255};
   light_class_type _type = light_class_type::point;

   flicker_type _flicker_type = flicker_type::none;
   float _flicker_period_min = 0.0f;
   float _flicker_period_max = 0.0f;
   float _fade_period = 1.0f;

   float _frame_rate = 0.0f;

   std::vector<std::string> _textures;

   float _omni_radius = 10.0f;
   float _spot_inner_cone_angle = 0.7853982f;
   float _spot_outer_cone_angle = 1.570796f;

   float _cone_length = 0.0f;
   float _cone_width = 0.0f;
   float _initial_cone_width_min = 1.0f;
   float _initial_cone_width_max = 1.0f;
   float _halo_radius = 0.0f;
   float _fade_length = -1.0f;
   float _fade_factor = 0.2f;
   float _flare_intensity = 0.0f;
   float _beam_intensity = 1.0f;
   float _draw_distance = 200.0f;

   std::string _name;
};

light_class::light_class(const assets::odf::definition& definition) noexcept
   : _impl{definition}
{
}

light_class::~light_class() = default;

void light_class::update(double delta_time) noexcept
{
   _impl->update(delta_time);
}

auto light_class::light_description() const noexcept
   -> const light_class_light_description&
{
   return _impl->light_description();
}

auto light_class::textures() const noexcept -> std::span<const std::string>
{
   return _impl->textures();
}

}