#pragma once

#include "stack.hpp"
#include "types.hpp"
#include "ui_action.hpp"
#include "world/world.hpp"

#include "imgui/imgui.h"

#include <memory>

namespace we::actions::imgui {

template<typename T, typename U>
inline bool drag_any(const char* label, T* value, float v_speed, U v_min,
                     U v_max, const char* format, ImGuiSliderFlags flags)
{
   if constexpr (std::is_same_v<float, T>) {
      return ImGui::DragFloat(label, value, v_speed, v_min, v_max, format, flags);
   }
   else if constexpr (std::is_same_v<float2, T>) {
      return ImGui::DragFloat2(label, &value->x, v_speed, v_min, v_max, format, flags);
   }
   else if constexpr (std::is_same_v<float3, T>) {
      return ImGui::DragFloat3(label, &value->x, v_speed, v_min, v_max, format, flags);
   }
   else if constexpr (std::is_same_v<float4, T>) {
      return ImGui::DragFloat4(label, &value->x, v_speed, v_min, v_max, format, flags);
   }
   else if constexpr (std::is_same_v<quaternion, T>) {
      return ImGui::DragFloat4(label, &value->x, v_speed, v_min, v_max, format, flags);
   }
   else {
      static_assert(std::is_same_v<void, T>,
                    "Unsupported type!"); // Add more types as needed.
   }
}

}

namespace ImGui {

template<typename Entity, typename T, typename U>
inline bool DragScalarN(const char* label, Entity* object,
                        T Entity::*value_member_ptr, we::actions::stack* action_stack,
                        we::world::world* world, float v_speed, U v_min,
                        U v_max, const char* format, ImGuiSliderFlags flags)
{
   using namespace we;
   using namespace we::actions;
   using namespace we::actions::imgui;

   using edit_action = ui_action<Entity, T>;
   using value_type = T;

   value_type value = object->*value_member_ptr;
   value_type original_value = value;

   bool valued_changed = drag_any(label, &value, v_speed, v_min, v_max, format, flags);

   if (valued_changed) {
      if (edit_action* action =
             dynamic_cast<edit_action*>(action_stack->applied_top());
          action and action->matching(*object, value_member_ptr) and not action->closed) {
         original_value = action->original_value;

         action_stack->revert(*world);
      }

      action_stack->apply(std::make_unique<edit_action>(object->id, value_member_ptr,
                                                        value, original_value),
                          *world);
   }
   else if (ImGui::IsItemDeactivated()) {
      edit_action* action = dynamic_cast<edit_action*>(action_stack->applied_top());

      if (action and action->matching(*object, value_member_ptr)) {
         action->closed = true;
      }
   }

   return valued_changed;
}

template<typename Entity>
inline bool DragFloat3(const char* label, Entity* entity,
                       we::float3 Entity::*value_member_ptr,
                       we::actions::stack* action_stack, we::world::world* world,
                       float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
                       const char* format = "%.3f", ImGuiSliderFlags flags = 0)
{
   return DragScalarN(label, entity, value_member_ptr, action_stack, world,
                      v_speed, v_min, v_max, format, flags);
}

template<typename Entity>
inline bool DragQuat(const char* label, Entity* entity,
                     we::quaternion Entity::*value_member_ptr,
                     we::actions::stack* action_stack, we::world::world* world,
                     float v_speed = 1.0f, const char* format = "%.3f",
                     ImGuiSliderFlags flags = 0)
{
   return DragScalarN(label, entity, value_member_ptr, action_stack, world,
                      v_speed, 0.0f, 1.0f, format, flags);
}

#if 0
inline bool DragFloat3(const char* label, we::world::object* object,
                       we::float3 we::world::object::*value_member_ptr,
                       we::actions::stack* action_stack, we::world::world* world,
                       float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
                       const char* format = "%.3f", ImGuiSliderFlags flags = 0)
{
   using namespace we;
   using namespace we::actions;

   using edit_action = ui_action<world::object, float3>;

   float3 value = object->*value_member_ptr;
   float3 original_value = value;

   bool valued_changed =
      ImGui::DragFloat3(label, &value.x, v_speed, v_min, v_max, format, flags);

   if (valued_changed) {
      if (edit_action* action =
             dynamic_cast<edit_action*>(action_stack->applied_top());
          action and action->matching(*object, value_member_ptr) and not action->closed) {
         original_value = action->original_value;

         action_stack->revert(*world);
      }

      action_stack->apply(std::make_unique<edit_action>(object->id, value_member_ptr,
                                                        value, original_value),
                          *world);
   }
   else if (ImGui::IsItemDeactivated()) {
      edit_action* action = dynamic_cast<edit_action*>(action_stack->applied_top());

      if (action and action->matching(*object, value_member_ptr)) {
         action->closed = true;
      }
   }

   return valued_changed;
}
#endif

}
