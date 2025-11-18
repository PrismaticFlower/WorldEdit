#pragma once

namespace we::settings {

struct ui {
   float extra_scaling = 1.0f;
   float gizmo_scale = 1.0f;

   bool hide_entity_hover_tooltips = false;
   bool hide_extra_light_properties = true;
   bool hide_extra_effects_properties = true;

   bool operator==(const ui&) const noexcept = default;
};

}