#include "light_class.hpp"

#include "utility/random_gen.hpp"
#include "utility/scanf.hpp"

#include "assets/odf/definition.hpp"

#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include "math/vector_funcs.hpp"

#include <algorithm>
#include <array>
#include <numbers>
#include <vector>

#include <fmt/format.h>

using we::string::iequals;

namespace we::world {

constexpr float light_class_icon_factor = 0.125f;

enum class flicker_type : int32 {
   none = 0,
   strobe = 1,
   strobe_random = 2,
   strobe_fade = 3,
   pulse = 4,
   flicker = 5,
};

struct light_class::impl {
   explicit impl(const assets::odf::definition& definition) noexcept
   {
      std::array<uint8, 3> color = {255, 255, 255};

      for (const assets::odf::property& prop : definition.properties) {
         if (iequals("Color", prop.key)) {
            uint32 r = 255;
            uint32 g = 255;
            uint32 b = 255;
            uint32 a = 255;

            const int scanned_channels = scan(prop.value, r, g, b, a);

            if (scanned_channels == 4) {
               color[0] = static_cast<uint8>((r * a) / 255u);
               color[1] = static_cast<uint8>((g * a) / 255u);
               color[2] = static_cast<uint8>((b * a) / 255u);
            }
            else if (scanned_channels == 3) {
               color[0] = static_cast<uint8>(r);
               color[1] = static_cast<uint8>(g);
               color[2] = static_cast<uint8>(b);
            }
         }
         else if (iequals("ConeLength", prop.key)) {
            if (_fade_length < 0.0f) _fade_length = 0.2f;

            scan(prop.key, _cone_length);

            _cone_length = std::max(_cone_length, 0.0f);
         }
         else if (iequals("ConeWidth", prop.key)) {
            if (_fade_length < 0.0f) _fade_length = 0.2f;

            const int scanned_values = scan(prop.value, _halo_radius, _cone_width);

            if (scanned_values == 2) {
               if (_halo_radius != 0.0f and _cone_width != 0.0f) {
                  _halo_radius = std::abs(_halo_radius);
                  _cone_width = std::abs(_cone_width);
               }
               else {
                  _halo_radius = 0.0f;
                  _cone_width = 0.0f;
               }
            }
            else if (scanned_values == 1) {
               _halo_radius = std::abs(_halo_radius);
               _cone_width = _halo_radius;
            }
         }
         else if (iequals("ConeInitialWidth", prop.key)) {
            if (_fade_length < 0.0f) _fade_length = 0.2f;

            const int scanned_values =
               scan(prop.value, _initial_cone_width_min, _initial_cone_width_max);

            if (scanned_values == 2) {
               if (_initial_cone_width_min != 0.0f and _initial_cone_width_max != 0.0f) {
                  _initial_cone_width_min = std::abs(_initial_cone_width_min);
                  _initial_cone_width_max = std::abs(_initial_cone_width_max);
               }
               else {
                  _initial_cone_width_min = 0.0f;
                  _initial_cone_width_max = 0.0f;
               }
            }
            else if (scanned_values == 1) {
               _initial_cone_width_min = std::abs(_initial_cone_width_min);
               _initial_cone_width_max = _initial_cone_width_min;
            }
         }
         else if (iequals("ConeFadeFactor", prop.key)) {
            if (_fade_length < 0.0f) _fade_length = 0.2f;

            scan(prop.value, _fade_factor);

            _fade_factor = std::clamp(_fade_factor, 0.0f, 1.0f);
         }
         else if (iequals("ConeFadeLength", prop.key)) {
            if (_fade_length < 0.0f) _fade_length = 0.2f;

            scan(prop.value, _fade_length);

            _fade_length = std::clamp(_fade_length, 0.0f, 1.0f);
         }
         else if (iequals("HaloRadius", prop.key)) {
            if (_fade_length < 0.0f) _fade_length = 0.5f;

            if (scan(prop.value, _halo_radius) == 1) {
               _halo_radius = std::abs(_halo_radius);
               _cone_width = _halo_radius;
            }
         }
         else if (iequals("HaloFadeFactor", prop.key)) {
            if (_fade_length < 0.0f) _fade_length = 0.5f;

            scan(prop.value, _fade_factor);

            _fade_factor = std::clamp(_fade_factor, 0.0f, 1.0f);
         }
         else if (iequals("HaloFadeLength", prop.key)) {
            if (_fade_length < 0.0f) _fade_length = 0.5f;

            scan(prop.value, _fade_length);

            _fade_length = std::clamp(_fade_length, 0.0f, 1.0f);
         }
         else if (iequals("FlareIntensity", prop.key)) {
            scan(prop.value, _flare_intensity);

            _flare_intensity = std::clamp(_flare_intensity, 0.0f, 1.0f);
         }
         else if (iequals("BeamIntensity", prop.key)) {
            scan(prop.value, _beam_intensity);

            _beam_intensity = std::clamp(_beam_intensity, 0.0f, 1.0f);
         }
         else if (iequals("FlickerType", prop.key)) {

            if (not prop.value.empty() and prop.value[0] >= '0' and
                prop.value[0] <= '9') {

               int flicker = 0;

               scan(prop.value, flicker);

               switch (flicker) {
               default:
               case 0:
                  _flicker_type = flicker_type::none;
               case 1:
                  _flicker_type = flicker_type::strobe;
               case 2:
                  _flicker_type = flicker_type::strobe_random;
               case 3:
                  _flicker_type = flicker_type::strobe_fade;
               case 4:
                  _flicker_type = flicker_type::pulse;
               case 5:
                  _flicker_type = flicker_type::flicker;
               }
            }
            else {
               if (iequals(prop.value, "Strobe")) {
                  _flicker_type = flicker_type::strobe;
               }
               else if (iequals(prop.value, "StrobeRandom")) {
                  _flicker_type = flicker_type::strobe_random;
               }
               else if (iequals(prop.value, "StrobeFade")) {
                  _flicker_type = flicker_type::strobe_fade;
               }
               else if (iequals(prop.value, "Pulse")) {
                  _flicker_type = flicker_type::pulse;
               }
               else if (iequals(prop.value, "Flicker")) {
                  _flicker_type = flicker_type::flicker;
               }
            }
         }
         else if (iequals("FlickerPeriod", prop.key)) {
            scan(prop.value, _flicker_period_min, _flicker_period_max);
         }
         else if (iequals("FadePeriod", prop.key)) {
            scan(prop.value, _fade_period);
         }
         else if (iequals("OmniRadius", prop.key)) {
            scan(prop.value, _omni_radius);
         }
         else if (iequals("SpotInnerConeAngle", prop.key)) {
            float angle_degrees = _spot_inner_cone_angle;

            scan(prop.value, angle_degrees);

            _spot_inner_cone_angle =
               (angle_degrees * std::numbers::pi_v<float>) / 180.0f;
         }
         else if (iequals("SpotOuterConeAngle", prop.key)) {
            float angle_degrees = _spot_outer_cone_angle;

            scan(prop.value, angle_degrees);

            _spot_outer_cone_angle =
               (angle_degrees * std::numbers::pi_v<float>) / 180.0f;
         }
         else if (iequals("DrawDistance", prop.key)) {
            scan(prop.value, _draw_distance);
         }
         else if (iequals("Static", prop.key)) {
            int value = 0;

            _flags.static_ = scan(prop.value, value) == 1 and value != 0;
         }
         else if (iequals("ShadowCaster", prop.key)) {
            int value = 0;

            _flags.shadow_caster = scan(prop.value, value) == 1 and value != 0;
         }
         else if (iequals("SpecularCaster", prop.key)) {
            int value = 0;

            _flags.specular_caster = scan(prop.value, value) == 1 and value != 0;
         }
         else if (iequals("Bidirectional", prop.key)) {
            int value = 0;

            _flags.bidirectional = scan(prop.value, value) == 1 and value != 0;
         }
         else if (iequals("Type", prop.key)) {
            if (iequals("Spot", prop.value)) {
               _type = light_class_type::spot;
            }
         }
         else if (iequals("ProjectedTexture", prop.key)) {
            std::string_view value = string::trim_leading_whitespace(prop.value);
            auto [texture, frame_count_str] =
               string::split_first_of_exclusive_whitespace(value);
            int frame_count = 0;

            scan(frame_count_str, frame_count);

            if (frame_count <= 0) {
               _textures.emplace_back(texture);
            }
            else {
               for (int i = 0; i < frame_count; ++i) {
                  _textures.emplace_back(fmt::format("{}{}", texture, i));
               }
            }
         }
         else if (iequals("FrameRate", prop.key)) {
            scan(prop.value, _frame_rate);

            _frame_rate = std::max(_frame_rate, 0.0f);
         }
         else if (iequals("Name", prop.key)) {
            _name = prop.value;
         }
         else if (iequals("Synchronize", prop.key)) {
            int value = 0;

            _flags.synchronize = scan(prop.value, value) == 1 and value != 0;
         }
      }

      const std::string_view start_texture =
         not _textures.empty() ? _textures.front() : std::string_view{};

      _color = {color[0] / 255.0f, color[1] / 255.0f, color[2] / 255.0f};
      _light_description = {
         .type = _type,

         .color = _color,
         .fixed_color = _color,

         .range = _omni_radius,
         .cos_half_outer_cone_angle = std::cos(_spot_outer_cone_angle * 0.5f),
         .cos_half_inner_cone_angle = std::cos(_spot_inner_cone_angle * 0.5f),
         .tan_half_outer_cone_angle = std::tan(_spot_outer_cone_angle * 0.5f),

         .flags = _flags,

         .texture = start_texture,
         .fixed_texture = start_texture,
      };
   }

   void update(double dbl_delta_time) noexcept
   {
      if (_textures.size() > 1) {
         _total_time += dbl_delta_time;

         const std::size_t texture_index =
            static_cast<std::size_t>(static_cast<int>(_total_time * _frame_rate)) %
            _textures.size();

         _light_description.texture = _textures[texture_index];
      }

      const float delta_time = static_cast<float>(dbl_delta_time);

      switch (_flicker_type) {
      case flicker_type::none: {
         return;
      } break;
      case flicker_type::strobe: {
         _flicker_time -= delta_time;

         if (_flicker_time > 0.0f) return;

         if (_strength > 0.0f) {
            _strength = 0.0f;
            _flicker_time = _flicker_period_max;
         }
         else {
            _strength = 1.0f;
            _flicker_time = _flicker_period_min;
         }
      } break;
      case flicker_type::strobe_random: {
         _flicker_time -= delta_time;

         if (_flicker_time <= 0.0) {
            _flicker_time = _flicker_period_min * _random.get_float();
            _strength = 1.0f;
         }
         else if (_flicker_time < 0.25f) {
            _strength = (0.25f - _flicker_time) * 4.0f;
         }
      } break;
      case flicker_type::strobe_fade: {
         _flicker_time += delta_time;

         const float flicker_range = _flicker_period_max + _flicker_period_min;

         if (flicker_range < _flicker_time) {
            _flicker_time = std::fmod(_flicker_time, flicker_range);
         }

         if (_flicker_period_min < _flicker_time) {
            _strength = 0.0f;
         }
         else if (_flicker_time < _fade_period) {
            _strength = _flicker_time / _fade_period;
         }
         else if (_flicker_period_min < _fade_period + _flicker_time) {
            _strength = (_flicker_period_min - _flicker_time) / _fade_period;
         }
         else {
            _strength = 1.0f;
         }
      } break;
      case flicker_type::pulse: {
         _flicker_time += delta_time;

         if (_flicker_period_min < _flicker_time) {
            _flicker_time = std::fmod(_flicker_time, _flicker_period_min);
         }

         const float half_flicker_period = _flicker_period_min * 0.5f;

         if (_flicker_time <= half_flicker_period) {
            _strength = _flicker_time / half_flicker_period;
         }
         else {
            _strength = (_flicker_period_min - _flicker_time) / half_flicker_period;
         }
      } break;
      case flicker_type::flicker: {
         _flicker_time += delta_time;

         const float randomness = _random.get_float() * 0.5f + 0.5f;

         if (_flicker_period_min <= _flicker_time) {
            _strength = randomness;
            _flicker_time = 0.0f;
         }
         else {
            const float factor = (_flicker_time * 0.4f) / _flicker_period_min;
            _strength = (1.0f - factor) * _strength + factor * randomness;
         }
      } break;
      }

      if (_strength >= 1.0f) {
         _light_description.color = _color;
      }
      else {
         _light_description.color = _color * _strength;
      }
   }

   auto light_description() const noexcept -> const light_class_light_description&
   {
      return _light_description;
   }

   auto textures() const noexcept -> std::span<const std::string>
   {
      return _textures;
   }

   auto world_icon_size() const noexcept -> float
   {
      return std::max(_light_description.range * light_class_icon_factor, 1.0f);
   }

private:
   light_class_light_description _light_description;

   float _flicker_time = 0.0f;
   float _strength = 0.0f;
   random_gen _random;

   double _total_time = 0.0;

   float3 _color;
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

   light_class_flags _flags;

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

auto light_class::world_icon_size() const noexcept -> float
{
   return _impl->world_icon_size();
}

auto light_class_light_description::positionWS(const float4x4& world_from_object) noexcept
   -> float3
{
   return {world_from_object[3].x, world_from_object[3].y, world_from_object[3].z};
}

auto light_class_light_description::directionWS(const float4x4& world_from_object) noexcept
   -> float3
{
   return {world_from_object[2].x, world_from_object[2].y, world_from_object[2].z};
}

}