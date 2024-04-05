#include "pch.h"

#include "world/utility/region_properties.hpp"

namespace we::world::tests {

TEST_CASE("world utilities get_region_type tests", "[World][Utility]")
{
   CHECK(get_region_type("cp1") == region_type::typeless);
   CHECK(get_region_type("") == region_type::typeless);
   CHECK(get_region_type("soundstream say_hello 1.00000") == region_type::soundstream);
   CHECK(get_region_type("soundstatic say_hello 1.00000") == region_type::soundstatic);
   CHECK(get_region_type("soundspace wow_space") == region_type::soundspace);
   CHECK(get_region_type("soundtrigger nice_region") == region_type::soundtrigger);
   CHECK(get_region_type("foleyfx foley_what_goes_here") == region_type::foleyfx);
   CHECK(get_region_type("shadow4 directional=1.00000 directional1=1.00000 "
                         "colortop=31,248,215 colorbottom=63,92,89") ==
         region_type::shadow);
   CHECK(get_region_type("mapbounds") == region_type::mapbounds);
   CHECK(get_region_type("rumble ClassyRumble SnowStorm") == region_type::rumble);
   CHECK(get_region_type("reflection1") == region_type::reflection);
   CHECK(get_region_type("rainshadow1") == region_type::rainshadow);
   CHECK(get_region_type("danger1") == region_type::danger);
   CHECK(get_region_type("DamageRegion1 damagerate=30") == region_type::damage_region);
   CHECK(get_region_type("AIVis1 crouch=0.5 stand=1.0") == region_type::ai_vis);
   CHECK(get_region_type("colorgrading Config=Colours.clrfx FadeLength=8.0") ==
         region_type::colorgrading);

   CHECK(get_region_type("soundstream1 say_hello 1.00000") == region_type::typeless);
   CHECK(get_region_type("soundstatic1 say_hello 1.00000") == region_type::typeless);
   CHECK(get_region_type("soundspace1 wow_space") == region_type::typeless);
   CHECK(get_region_type("soundtrigger1 nice_region") == region_type::typeless);
   CHECK(get_region_type("foleyfx1 foley_what_goes_here") == region_type::typeless);
   CHECK(get_region_type("mapbounds1") == region_type::typeless);
   CHECK(get_region_type("rumble1 ClassyRumble SnowStorm") == region_type::typeless);
   CHECK(get_region_type("colorgrading1 Config=Colours.clrfx FadeLength=8.0") ==
         region_type::typeless);
}

TEST_CASE("world utilities unpack_region_sound_stream tests",
          "[World][Utility]")
{
   sound_stream_properties properties =
      unpack_region_sound_stream("soundstream amazing_music 1.4");

   CHECK(properties.sound_name == "amazing_music");
   CHECK(properties.min_distance_divisor == 1.4f);
}

TEST_CASE("world utilities pack_region_sound_stream tests", "[World][Utility]")
{
   CHECK(pack_region_sound_stream(
            {.sound_name = "amazing_music", .min_distance_divisor = 1.4f}) ==
         "soundstream amazing_music 1.400");
}

TEST_CASE("world utilities unpack_region_sound_static tests",
          "[World][Utility]")
{
   sound_stream_properties properties =
      unpack_region_sound_static("soundstatic amazing_sound 1.4");

   CHECK(properties.sound_name == "amazing_sound");
   CHECK(properties.min_distance_divisor == 1.4f);
}

TEST_CASE("world utilities pack_region_sound_static tests", "[World][Utility]")
{
   CHECK(pack_region_sound_static(
            {.sound_name = "amazing_sound", .min_distance_divisor = 1.4f}) ==
         "soundstatic amazing_sound 1.400");
}

TEST_CASE("world utilities unpack_region_sound_space tests", "[World][Utility]")
{
   sound_space_properties properties =
      unpack_region_sound_space("soundspace wild_space");

   CHECK(properties.sound_space_name == "wild_space");
}

TEST_CASE("world utilities pack_region_sound_space tests", "[World][Utility]")
{
   CHECK(pack_region_sound_space({.sound_space_name = "wild_space"}) ==
         "soundspace wild_space");
}

TEST_CASE("world utilities unpack_region_sound_trigger tests",
          "[World][Utility]")
{
   sound_trigger_properties properties =
      unpack_region_sound_trigger("soundtrigger AReallyNiceRegion");

   CHECK(properties.region_name == "AReallyNiceRegion");
}

TEST_CASE("world utilities pack_region_sound_trigger tests", "[World][Utility]")
{
   CHECK(pack_region_sound_trigger({.region_name = "AReallyNiceRegion"}) ==
         "soundtrigger AReallyNiceRegion");
}

TEST_CASE("world utilities unpack_region_foley_fx tests", "[World][Utility]")
{
   foley_fx_region_properties properties =
      unpack_region_foley_fx("foleyfx water_fx");

   CHECK(properties.group_id == "water_fx");
}

TEST_CASE("world utilities pack_region_foley_fx tests", "[World][Utility]")
{
   CHECK(pack_region_foley_fx({.group_id = "water_fx"}) == "foleyfx water_fx");
}

TEST_CASE("world utilities unpack_region_shadow tests", "[World][Utility]")
{
   shadow_region_properties properties = unpack_region_shadow(
      "shadow2 directional=1.0 directional1=0.5 "
      "colortop=255,255,255 colorbottom=0,0,0 envmap=reflection");

   CHECK(properties.suffix == "2");
   CHECK(properties.directional0 == 1.0f);
   CHECK(properties.directional1 == 0.5f);
   CHECK(properties.color_top == float3{1.0f, 1.0f, 1.0f});
   CHECK(properties.color_bottom == float3{0.0f, 0.0f, 0.0f});
   CHECK(properties.env_map == "reflection");

   properties =
      unpack_region_shadow("shadow colortop=255,255,255 colorbottom=0,0,0");

   CHECK(properties.suffix == "");
   CHECK(not properties.directional0);
   CHECK(not properties.directional1);
   CHECK(properties.color_top == float3{1.0f, 1.0f, 1.0f});
   CHECK(properties.color_bottom == float3{0.0f, 0.0f, 0.0f});
   CHECK(properties.env_map.empty());
}

TEST_CASE("world utilities pack_region_shadow tests", "[World][Utility]")
{
   CHECK(pack_region_shadow(
            shadow_region_properties{.suffix = "2",
                                     .directional0 = 1.0f,
                                     .directional1 = 0.5f,
                                     .color_top = float3{1.0f, 1.0f, 1.0f},
                                     .color_bottom = float3{0.0f, 0.0f, 0.0f},
                                     .env_map = "reflection"}) ==
         "shadow2 directional=1.000 directional1=0.500 colortop=255,255,255 "
         "colorbottom=0,0,0 envmap=reflection");

   CHECK(pack_region_shadow(
            shadow_region_properties{.color_top = float3{1.0f, 1.0f, 1.0f},
                                     .color_bottom = float3{0.0f, 0.0f, 0.0f}}) ==
         "shadow colortop=255,255,255 colorbottom=0,0,0");
}

TEST_CASE("world utilities unpack_region_rumble tests", "[World][Utility]")
{
   rumble_region_properties properties =
      unpack_region_rumble("rumble ClassyRumble SnowStorm");

   CHECK(properties.rumble_class == "ClassyRumble");
   CHECK(properties.particle_effect == "SnowStorm");
}

TEST_CASE("world utilities pack_region_rumble tests", "[World][Utility]")
{
   CHECK(pack_region_rumble(
            {.rumble_class = "ClassyRumble", .particle_effect = "SnowStorm"}) ==
         "rumble ClassyRumble SnowStorm");
}

TEST_CASE("world utilities unpack_region_damage tests", "[World][Utility]")
{
   damage_region_properties properties = unpack_region_damage(
      "DamageRegion damagerate=10.000 personscale=1.000 animalscale=2.000 "
      "droidscale=3.000 vehiclescale=4.000 buildingscale=5.000 "
      "buildingdeadscale=6.000 buildingunbuiltscale=7.000");

   CHECK(properties.damage_rate == 10.0f);
   CHECK(properties.person_scale == 1.0f);
   CHECK(properties.animal_scale == 2.0f);
   CHECK(properties.droid_scale == 3.0f);
   CHECK(properties.vehicle_scale == 4.0f);
   CHECK(properties.building_scale == 5.0f);
   CHECK(properties.building_dead_scale == 6.0f);
   CHECK(properties.building_unbuilt_scale == 7.0f);

   properties = unpack_region_damage("DamageRegion damagerate=10.000");

   CHECK(properties.damage_rate == 10.0f);
   CHECK(not properties.person_scale);
   CHECK(not properties.animal_scale);
   CHECK(not properties.droid_scale);
   CHECK(not properties.vehicle_scale);
   CHECK(not properties.building_scale);
   CHECK(not properties.building_dead_scale);
   CHECK(not properties.building_unbuilt_scale);
}

TEST_CASE("world utilities pack_region_damage tests", "[World][Utility]")
{
   CHECK(pack_region_damage(damage_region_properties{.damage_rate = 10.0f,
                                                     .person_scale = 1.0f,
                                                     .animal_scale = 2.0f,
                                                     .droid_scale = 3.0f,
                                                     .vehicle_scale = 4.0f,
                                                     .building_scale = 5.0f,
                                                     .building_dead_scale = 6.0f,
                                                     .building_unbuilt_scale = 7.0f}) ==
         "DamageRegion damagerate=10.000 personscale=1.000 animalscale=2.000 "
         "droidscale=3.000 vehiclescale=4.000 buildingscale=5.000 "
         "buildingdeadscale=6.000 buildingunbuiltscale=7.000");

   CHECK(pack_region_damage(damage_region_properties{.damage_rate = 10.0f}) ==
         "DamageRegion damagerate=10.000");
}

TEST_CASE("world utilities unpack_region_ai_vis tests", "[World][Utility]")
{
   ai_vis_region_properties properties =
      unpack_region_ai_vis("AIVis crouch=0.5 stand=0.75");

   CHECK(properties.crouch == 0.5f);
   CHECK(properties.stand == 0.75f);

   properties = unpack_region_ai_vis("AIVis crouch=0.5");

   CHECK(properties.crouch == 0.5f);
   CHECK(not properties.stand);
}

TEST_CASE("world utilities pack_region_ai_vis tests", "[World][Utility]")
{
   CHECK(pack_region_ai_vis({.crouch = 0.5f, .stand = 0.75f}) ==
         "AIVis crouch=0.500 stand=0.750");

   CHECK(pack_region_ai_vis({.crouch = 0.5f}) == "AIVis crouch=0.500");
}

TEST_CASE("world utilities unpack_region_colorgrading tests",
          "[World][Utility]")
{
   colorgrading_region_properties properties = unpack_region_colorgrading(
      "colorgrading Config=AmazingShaderPatchConfig FadeLength=1.000");

   CHECK(properties.config == "AmazingShaderPatchConfig");
   CHECK(properties.fade_length == 1.0f);
}

TEST_CASE("world utilities pack_region_colorgrading tests", "[World][Utility]")
{
   CHECK(pack_region_colorgrading(
            {.config = "AmazingShaderPatchConfig", .fade_length = 1.0f}) ==
         "colorgrading Config=AmazingShaderPatchConfig FadeLength=1.000");
}

}
