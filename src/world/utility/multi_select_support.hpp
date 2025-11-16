#pragma once

#include "types.hpp"

#include "region_properties.hpp"

#include "../interaction_context.hpp"
#include "../world.hpp"

#include "utility/enum_bitflags.hpp"

#include <optional>

namespace we::world {

enum class multi_select_flags : uint64 {
   // clang-format off
   has_layer                          = 1ull << 0,
   has_rotation                       = 1ull << 1,
   has_position                       = 1ull << 2,
   has_box_size                       = 1ull << 3,
   has_width                          = 1ull << 4,
   has_height                         = 1ull << 5,
   has_radius                         = 1ull << 6,
   has_ai_flags                       = 1ull << 7,
   has_object                         = 1ull << 8,
   has_light                          = 1ull << 9,
   has_light_range                    = 1ull << 10,
   has_light_cone_props               = 1ull << 11,
   has_light_texture_addressing       = 1ull << 12,
   has_light_directional_texture      = 1ull << 13,
   has_light_region                   = 1ull << 14,
   has_light_ps2_blend_mode           = 1ull << 15,
   has_path                           = 1ull << 16,
   has_path_spline_type               = 1ull << 17,
   has_region                         = 1ull << 18,
   has_region_sound_stream            = 1ull << 19,
   has_region_sound_static            = 1ull << 20,
   has_region_sound_space             = 1ull << 21,
   has_region_sound_trigger           = 1ull << 22,
   has_region_foley_fx                = 1ull << 23,
   has_region_shadow                  = 1ull << 24,
   has_region_rumble                  = 1ull << 25,
   has_region_damage                  = 1ull << 26,
   has_region_ai_vis                  = 1ull << 27,
   has_region_colorgrading            = 1ull << 28,
   has_sector                         = 1ull << 29,
   has_portal                         = 1ull << 30,
   has_hintnode                       = 1ull << 31,
   has_hintnode_command_post          = 1ull << 32,
   has_hintnode_primary_stance        = 1ull << 33,
   has_hintnode_secondary_stance      = 1ull << 34,
   has_hintnode_mode                  = 1ull << 35,
   has_hintnode_radius                = 1ull << 36,
   has_barrier                        = 1ull << 37,
   has_planning_hub                   = 1ull << 38,
   has_planning_hub_branch_weights    = 1ull << 39,
   has_planning_connection            = 1ull << 40,
   has_boundary                       = 1ull << 41,
   has_measurement                    = 1ull << 42,
   has_block                          = 1ull << 43,
   has_block_segments                 = 1ull << 44,
   has_block_flat_shading             = 1ull << 45,
   has_block_texture_loops            = 1ull << 46,
   has_block_step_properties          = 1ull << 47,
   has_block_ring_properties          = 1ull << 48,
   has_block_beveled_box_properties   = 1ull << 49,
   has_block_arch_properties          = 1ull << 50,
   // clang-format on

   all = ~0ull,
   none = 0,
};

constexpr bool marked_as_enum_bitflag(multi_select_flags)
{
   return true;
}

struct multi_select_properties {
   template<typename T>
   struct value {
      void integrate(const T& new_value) noexcept
      {
         if (_count > 0) {
            if (_value != new_value) _value = std::nullopt;
         }
         else {
            _value = new_value;
         }

         _count += 1;
      }

      bool is_different() const noexcept
      {
         return not _value.has_value();
      }

      auto value_or(const T& fallback) const noexcept -> T
      {
         return _value.value_or(fallback);
      }

      auto count() const noexcept -> uint32
      {
         return _count;
      }

   private:
      uint32 _count = 0;
      std::optional<T> _value;
   };

   struct value_split_float2 {
      void integrate(const float2& value) noexcept
      {
         x.integrate(value.x);
         y.integrate(value.y);
      }

      auto count() const noexcept -> uint32
      {
         return std::max(x.count(), y.count());
      }

      value<float> x;
      value<float> y;
   };

   struct value_split_float3 {
      void integrate(const float3& value) noexcept
      {
         x.integrate(value.x);
         y.integrate(value.y);
         z.integrate(value.z);
      }

      auto count() const noexcept -> uint32
      {
         return std::max(std::max(x.count(), y.count()), z.count());
      }

      value<float> x;
      value<float> y;
      value<float> z;
   };

   struct value_ai_flags {
      void integrate(const ai_path_flags new_flags) noexcept
      {
         if (_count > 0) {
            if (_soldier != are_flags_set(new_flags, ai_path_flags::soldier)) {
               _soldier = std::nullopt;
            }
            if (_hover != are_flags_set(new_flags, ai_path_flags::hover)) {
               _hover = std::nullopt;
            }
            if (_small != are_flags_set(new_flags, ai_path_flags::small)) {
               _small = std::nullopt;
            }
            if (_medium != are_flags_set(new_flags, ai_path_flags::medium)) {
               _medium = std::nullopt;
            }
            if (_huge != are_flags_set(new_flags, ai_path_flags::huge)) {
               _huge = std::nullopt;
            }
            if (_flyer != are_flags_set(new_flags, ai_path_flags::flyer)) {
               _flyer = std::nullopt;
            }
         }
         else {
            _soldier = are_flags_set(new_flags, ai_path_flags::soldier);
            _hover = are_flags_set(new_flags, ai_path_flags::hover);
            _small = are_flags_set(new_flags, ai_path_flags::small);
            _medium = are_flags_set(new_flags, ai_path_flags::medium);
            _huge = are_flags_set(new_flags, ai_path_flags::huge);
            _flyer = are_flags_set(new_flags, ai_path_flags::flyer);
         }

         _count += 1;
      }

      bool is_soldier_different() const noexcept
      {
         return not _soldier.has_value();
      }

      bool is_hover_different() const noexcept
      {
         return not _hover.has_value();
      }

      bool is_small_different() const noexcept
      {
         return not _small.has_value();
      }

      bool is_medium_different() const noexcept
      {
         return not _medium.has_value();
      }

      bool is_huge_different() const noexcept
      {
         return not _huge.has_value();
      }

      bool is_flyer_different() const noexcept
      {
         return not _flyer.has_value();
      }

      auto value_or_default() const noexcept -> ai_path_flags
      {
         ai_path_flags flags = ai_path_flags::none;

         if (not _soldier or *_soldier) flags |= ai_path_flags::soldier;
         if (not _hover or *_hover) flags |= ai_path_flags::hover;
         if (not _small or *_small) flags |= ai_path_flags::small;
         if (not _medium or *_medium) flags |= ai_path_flags::medium;
         if (not _huge or *_huge) flags |= ai_path_flags::huge;
         if (not _flyer or *_flyer) flags |= ai_path_flags::flyer;

         return flags;
      }

      auto count() const noexcept -> uint32
      {
         return _count;
      }

   private:
      uint32 _count = 0;
      std::optional<bool> _soldier;
      std::optional<bool> _hover;
      std::optional<bool> _small;
      std::optional<bool> _medium;
      std::optional<bool> _huge;
      std::optional<bool> _flyer;
   };

   struct value_stance_flags {
      void integrate(const stance_flags new_flags) noexcept
      {
         if (_count > 0) {
            if (_stand != are_flags_set(new_flags, stance_flags::stand)) {
               _stand = std::nullopt;
            }
            if (_crouch != are_flags_set(new_flags, stance_flags::crouch)) {
               _crouch = std::nullopt;
            }
            if (_prone != are_flags_set(new_flags, stance_flags::prone)) {
               _prone = std::nullopt;
            }
            if (_left != are_flags_set(new_flags, stance_flags::left)) {
               _left = std::nullopt;
            }
            if (_right != are_flags_set(new_flags, stance_flags::right)) {
               _right = std::nullopt;
            }
         }
         else {
            _stand = are_flags_set(new_flags, stance_flags::stand);
            _crouch = are_flags_set(new_flags, stance_flags::crouch);
            _prone = are_flags_set(new_flags, stance_flags::prone);
            _left = are_flags_set(new_flags, stance_flags::left);
            _right = are_flags_set(new_flags, stance_flags::right);
         }

         _count += 1;
      }

      bool is_stand_different() const noexcept
      {
         return not _stand.has_value();
      }

      bool is_crouch_different() const noexcept
      {
         return not _crouch.has_value();
      }

      bool is_prone_different() const noexcept
      {
         return not _prone.has_value();
      }

      bool is_left_different() const noexcept
      {
         return not _left.has_value();
      }

      bool is_right_different() const noexcept
      {
         return not _right.has_value();
      }

      auto value_or_default() const noexcept -> stance_flags
      {
         stance_flags flags = stance_flags::none;

         if (not _stand or *_stand) flags |= stance_flags::stand;
         if (not _crouch or *_crouch) flags |= stance_flags::crouch;
         if (not _prone or *_prone) flags |= stance_flags::prone;
         if (not _left or *_left) flags |= stance_flags::left;
         if (not _right or *_right) flags |= stance_flags::right;

         return flags;
      }

      auto count() const noexcept -> uint32
      {
         return _count;
      }

   private:
      uint32 _count = 0;

      std::optional<bool> _stand = false;
      std::optional<bool> _crouch = false;
      std::optional<bool> _prone = false;
      std::optional<bool> _left = false;
      std::optional<bool> _right = false;
   };

   template<typename T>
   struct value_properties {
      void integrate(std::span<const T> new_properties) noexcept
      {
         if (_count > 0) {
            if (_properties.size() != new_properties.size()) {
               _properties = {};
            }
            else {
               for (std::size_t i = 0; i < new_properties.size(); ++i) {
                  if (_properties[i].key != new_properties[i].key) {
                     _properties = {};

                     return;
                  }
                  else if (_properties[i].value != new_properties[i].value) {
                     _shared_values[i] = false;
                  }
               }
            }
         }
         else if (not new_properties.empty()) {
            _properties = new_properties;
            _shared_values.resize(new_properties.size(), true);
         }

         _count += 1;
      }

      bool has_common_keys() const noexcept
      {
         return not _properties.empty();
      }

      bool is_different_value(const std::size_t i) const noexcept
      {
         if (i >= _shared_values.size()) return false;

         return not _shared_values[i];
      }

      auto properties_or_default() const noexcept -> std::span<const T>
      {
         return _properties;
      }

      auto count() const noexcept -> uint32
      {
         return _count;
      }

   private:
      uint32 _count = 0;
      std::span<const T> _properties;
      std::vector<bool> _shared_values;
   };

   value<int8> layer;
   value<quaternion> rotation;
   value_split_float3 position;

   value_split_float3 box_size;
   value<float> width;
   value<float> height;
   value<float> radius;
   value_ai_flags ai_flags;

   struct object {
      value<std::string_view> class_name;
      value<int> team;
      value_properties<instance_property> instance_properties;
   } object;

   struct light {
      value<float3> color;
      value<bool> static_;
      value<bool> shadow_caster;
      value<bool> specular_caster;
      value<light_type> type;

      value<float> range;

      value<float> inner_cone_angle;
      value<float> outer_cone_angle;

      value<std::string_view> texture;
      value<texture_addressing> texture_addressing;

      value_split_float2 directional_texture_tiling;
      value_split_float2 directional_texture_offset;

      value<quaternion> region_rotation;

      value<ps2_blend_mode> ps2_blend_mode;
      value<bool> bidirectional;
   } light;

   struct path {
      value<path_type> type;
      value<path_spline_type> spline_type;
      value_properties<we::world::path::property> properties;
      value_properties<we::world::path::property> node_properties;
   } path;

   struct region {
      value<region_type> type;
      value<std::string_view> description;
      value<region_shape> shape;

      struct sound_properties {
         value<std::string> sound_name;
         value<float> min_distance_divisor;
      };

      sound_properties sound_stream;
      sound_properties sound_static;

      struct sound_space {
         value<std::string> sound_space_name;
      } sound_space;

      struct sound_trigger {
         value<std::string> region_name;
      } sound_trigger;

      struct foley_fx {
         value<std::string> group_id;
      } foley_fx;

      struct shadow {
         value<std::string> suffix;

         value<float> directional0;
         value<float> directional1;

         value<float3> color_top;
         value<float3> color_bottom;

         value<std::string> env_map;
      } shadow;

      struct rumble {
         value<std::string> rumble_class;
         value<std::string> particle_effect;
      } rumble;

      struct damage {
         value<float> damage_rate;
         value<float> person_scale;
         value<float> animal_scale;
         value<float> droid_scale;
         value<float> vehicle_scale;
         value<float> building_scale;
         value<float> building_dead_scale;
         value<float> building_unbuilt_scale;
      } damage;

      struct ai_vis {
         value<float> crouch;
         value<float> stand;
      } ai_vis;

      struct colorgrading {
         value<std::string> config;
         value<float> fade_length;
      } colorgrading;
   } region;

   struct sector {
      value<float> base;
   } sector;

   struct portal {
      value<std::string_view> sector1;
      value<std::string_view> sector2;
   } portal;

   struct hintnode {
      value<hintnode_type> type;
      value<std::string> command_post;
      value_stance_flags primary_stance;
      value_stance_flags secondary_stance;
      value<hintnode_mode> mode;
      value<float> radius;
   } hintnode;

   struct barrier {
      value<float> rotation_angle;
      value_split_float2 size;
   } barrier;

   struct planning_connection {
      value<bool> jump;
      value<bool> jet_jump;
      value<bool> one_way;
      value<bool> dynamic;
      value<int8> dynamic_group;
   } planning_connection;

   struct block {
      value<float> step_height;
      value<float> first_step_offset;
      value<uint16> segments;
      value<bool> flat_shading;
      value<float> texture_loops;

      struct ring {
         value<float> inner_radius;
         value<float> outer_radius;
      } ring;

      struct beveled_box {
         value<float> amount;
         value<bool> bevel_top;
         value<bool> bevel_sides;
         value<bool> bevel_bottom;
      } beveled_box;

      struct arch {
         value<float> crown_length;
         value<float> crown_height;
         value<float> curve_height;
         value<float> span_length;
      } arch;
   } block;
};

}