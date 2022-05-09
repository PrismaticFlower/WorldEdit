#pragma once

#include "stack.hpp"
#include "types.hpp"
#include "ui_action.hpp"
#include "world/world.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

#include <initializer_list>
#include <memory>
#include <type_traits>

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

struct edit_widget_result {
   bool value_changed;
   bool item_deactivated;
};

template<typename T>
struct enum_select_option {
   const char* label = "";
   T value = {};
};

template<typename Callback, typename T>
concept edit_widget_callback =
   std::is_invocable_r_v<edit_widget_result, Callback, T*>;

// clang-format off
template<typename T>
concept input_key_value_type = requires(T t)
{
   { t.key } -> std::convertible_to<std::string&>;
   { t.value } -> std::convertible_to<std::string&>;
};
// clang-format on

}

namespace we {
using we::actions::imgui::edit_widget_result;
using we::actions::imgui::enum_select_option;
}

namespace ImGui {

// Entity Property Editors

template<typename Entity, typename T>
inline bool EditWithUndo(Entity* object, T Entity::*value_member_ptr,
                         we::actions::stack* action_stack, we::world::world* world,
                         we::actions::imgui::edit_widget_callback<T> auto edit) noexcept
{
   using namespace we;
   using namespace we::actions;
   using namespace we::actions::imgui;

   using edit_action = ui_action<Entity, T>;
   using value_type = T;

   value_type value = object->*value_member_ptr;
   value_type original_value = value;

   auto [valued_changed, item_deactivated] = edit(&value);

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

   if (item_deactivated) {
      edit_action* action = dynamic_cast<edit_action*>(action_stack->applied_top());

      if (action and action->matching(*object, value_member_ptr)) {
         action->closed = true;
      }
   }

   return valued_changed;
}

template<typename Entity>
inline bool Checkbox(const char* label, Entity* entity, bool Entity::*value_member_ptr,
                     we::actions::stack* action_stack, we::world::world* world) noexcept
{
   return EditWithUndo(entity, value_member_ptr, action_stack, world, [=](bool* value) {
      bool value_changed = ImGui::Checkbox(label, value);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool DragFloat(const char* label, Entity* entity, float Entity::*value_member_ptr,
                      we::actions::stack* action_stack, we::world::world* world,
                      float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
                      const char* format = "%.3f", ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, action_stack, world, [=](float* value) {
      bool value_changed =
         ImGui::DragFloat(label, value, v_speed, v_min, v_max, format, flags);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool DragFloat2(const char* label, Entity* entity,
                       we::float2 Entity::*value_member_ptr,
                       we::actions::stack* action_stack, we::world::world* world,
                       float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
                       const char* format = "%.3f", ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, action_stack, world, [=](we::float2* value) {
      bool value_changed =
         ImGui::DragFloat2(label, &value->x, v_speed, v_min, v_max, format, flags);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool DragFloat3(const char* label, Entity* entity,
                       we::float3 Entity::*value_member_ptr,
                       we::actions::stack* action_stack, we::world::world* world,
                       float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
                       const char* format = "%.3f", ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, action_stack, world, [=](we::float3* value) {
      bool value_changed =
         ImGui::DragFloat3(label, &value->x, v_speed, v_min, v_max, format, flags);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool DragQuat(const char* label, Entity* entity,
                     we::quaternion Entity::*value_member_ptr,
                     we::actions::stack* action_stack, we::world::world* world,
                     float v_speed = 1.0f, const char* format = "%.3f",
                     ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, action_stack, world, [=](we::quaternion* value) {
      bool value_changed =
         ImGui::DragFloat4(label, &(*value)[0], v_speed, 0.0f, 1.0f, format, flags);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool SliderInt(const char* label, Entity* entity, int Entity::*value_member_ptr,
                      we::actions::stack* action_stack, we::world::world* world,
                      int v_min = 0, int v_max = 0, const char* format = "%d",
                      ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, action_stack, world, [=](int* value) {
      bool value_changed = ImGui::SliderInt(label, value, v_min, v_max, format, flags);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool ColorEdit3(const char* label, Entity* entity,
                       we::float3 Entity::*value_member_ptr,
                       we::actions::stack* action_stack, we::world::world* world,
                       ImGuiColorEditFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, action_stack, world, [=](we::float3* value) {
      bool value_changed = ImGui::ColorEdit3(label, &value->x, flags);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool InputText(const char* label, Entity* entity,
                      std::string Entity::*value_member_ptr,
                      we::actions::stack* action_stack, we::world::world* world,
                      ImGuiInputTextFlags flags = 0,
                      ImGuiInputTextCallback callback = nullptr,
                      void* user_data = nullptr) noexcept
{
   return EditWithUndo(entity, value_member_ptr, action_stack, world, [=](std::string* value) {
      bool value_changed =
         ImGui::InputText(label, value, flags | ImGuiInputTextFlags_NoUndoRedo,
                          callback, user_data);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool LayerPick(const char* label, Entity* entity,
                      we::actions::stack* action_stack, we::world::world* world) noexcept
{
   return EditWithUndo(entity, &Entity::layer, action_stack, world, [=](int* layer) {
      bool value_changed = false;

      if (ImGui::BeginCombo(label, [&] {
             if (entity->layer >= world->layer_descriptions.size()) return "";

             return world->layer_descriptions[entity->layer].name.c_str();
          }())) {

         for (int i = 0; i < std::ssize(world->layer_descriptions); ++i) {
            if (ImGui::Selectable(world->layer_descriptions[i].name.c_str())) {
               *layer = i;
               value_changed = true;
            }
         }

         ImGui::EndCombo();
      }

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = value_changed};
   });
}

template<typename Entity, typename Enum>
inline bool EnumSelect(const char* label, Entity* entity, Enum Entity::*value_member_ptr,
                       we::actions::stack* action_stack, we::world::world* world,
                       std::initializer_list<we::enum_select_option<Enum>> values) noexcept
{
   return EditWithUndo(entity, value_member_ptr, action_stack, world, [=, &values](Enum* value) {
      bool value_changed = false;

      if (ImGui::BeginCombo(label, [&] {
             for (auto& option : values) {
                if (option.value == *value) return option.label;
             }

             return "";
          }())) {

         for (auto option : values) {
            if (ImGui::Selectable(option.label)) {
               *value = option.value;
               value_changed = true;
            }
         }

         ImGui::EndCombo();
      }

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = value_changed};
   });
}

// Entity Vector/Array Property Editors

template<typename Entity, typename T>
inline bool EditWithUndo(Entity* object, std::vector<T> Entity::*value_member_ptr,
                         const std::size_t item_index,
                         we::actions::stack* action_stack, we::world::world* world,
                         we::actions::imgui::edit_widget_callback<T> auto edit) noexcept
{
   using namespace we;
   using namespace we::actions;
   using namespace we::actions::imgui;

   using edit_action = ui_action_indexed<Entity, T>;
   using value_type = T;

   value_type value = (object->*value_member_ptr)[item_index];
   value_type original_value = value;

   auto [valued_changed, item_deactivated] = edit(&value);

   if (valued_changed) {
      if (edit_action* action =
             dynamic_cast<edit_action*>(action_stack->applied_top());
          action and action->matching(*object, value_member_ptr, item_index) and
          not action->closed) {
         original_value = action->original_value;

         action_stack->revert(*world);
      }

      action_stack->apply(std::make_unique<edit_action>(object->id, value_member_ptr,
                                                        item_index, value, original_value),
                          *world);
   }

   if (item_deactivated) {
      edit_action* action = dynamic_cast<edit_action*>(action_stack->applied_top());

      if (action and action->matching(*object, value_member_ptr, item_index)) {
         action->closed = true;
      }
   }

   return valued_changed;
}

template<typename Entity, we::actions::imgui::input_key_value_type T>
inline bool InputKeyValue(Entity* entity, std::vector<T> Entity::*value_member_ptr,
                          const std::size_t item_index, we::actions::stack* action_stack,
                          we::world::world* world, ImGuiInputTextFlags flags = 0,
                          ImGuiInputTextCallback callback = nullptr,
                          void* user_data = nullptr) noexcept
{
   return EditWithUndo(entity, value_member_ptr, item_index, action_stack, world, [=](T* kv) {
      bool value_changed = ImGui::InputText(kv->key.c_str(), &kv->value,
                                            flags | ImGuiInputTextFlags_NoUndoRedo,
                                            callback, user_data);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

// Path Node Property Editors

template<typename T>
inline bool EditWithUndo(we::world::path* path, const std::size_t node_index,
                         T we::world::path::node::*value_member_ptr,
                         we::actions::stack* action_stack, we::world::world* world,
                         we::actions::imgui::edit_widget_callback<T> auto edit) noexcept
{
   using namespace we;
   using namespace we::actions;
   using namespace we::actions::imgui;

   using edit_action = ui_action_path_node<T>;
   using value_type = T;

   value_type value = path->nodes[node_index].*value_member_ptr;
   value_type original_value = value;

   auto [valued_changed, item_deactivated] = edit(&value);

   if (valued_changed) {
      if (edit_action* action =
             dynamic_cast<edit_action*>(action_stack->applied_top());
          action and action->matching(*path, node_index, value_member_ptr) and
          not action->closed) {
         original_value = action->original_value;

         action_stack->revert(*world);
      }

      action_stack->apply(std::make_unique<edit_action>(path->id, node_index, value_member_ptr,
                                                        value, original_value),
                          *world);
   }

   if (item_deactivated) {
      edit_action* action = dynamic_cast<edit_action*>(action_stack->applied_top());

      if (action and action->matching(*path, node_index, value_member_ptr)) {
         action->closed = true;
      }
   }

   return valued_changed;
}

inline bool DragFloat3(const char* label, we::world::path* entity,
                       const std::size_t node_index,
                       we::float3 we::world::path::node::*value_member_ptr,
                       we::actions::stack* action_stack, we::world::world* world,
                       float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
                       const char* format = "%.3f", ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, node_index, value_member_ptr, action_stack,
                       world, [=](we::float3* value) {
                          bool value_changed =
                             ImGui::DragFloat3(label, &value->x, v_speed, v_min,
                                               v_max, format, flags);

                          return we::edit_widget_result{.value_changed = value_changed,
                                                        .item_deactivated =
                                                           ImGui::IsItemDeactivated()};
                       });
}

inline bool DragQuat(const char* label, we::world::path* entity,
                     const std::size_t node_index,
                     we::quaternion we::world::path::node::*value_member_ptr,
                     we::actions::stack* action_stack, we::world::world* world,
                     float v_speed = 1.0f, const char* format = "%.3f",
                     ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, node_index, value_member_ptr, action_stack,
                       world, [=](we::quaternion* value) {
                          bool value_changed =
                             ImGui::DragFloat4(label, &(*value)[0], v_speed,
                                               0.0f, 1.0f, format, flags);

                          return we::edit_widget_result{.value_changed = value_changed,
                                                        .item_deactivated =
                                                           ImGui::IsItemDeactivated()};
                       });
}

// Path Node Vector/Array Property Editors

template<typename T>
inline bool EditWithUndo(we::world::path* path, const std::size_t node_index,
                         std::vector<T> we::world::path::node::*value_member_ptr,
                         const std::size_t item_index,
                         we::actions::stack* action_stack, we::world::world* world,
                         we::actions::imgui::edit_widget_callback<T> auto edit) noexcept
{
   using namespace we;
   using namespace we::actions;
   using namespace we::actions::imgui;

   using edit_action = ui_action_path_node_indexed<T>;
   using value_type = T;

   value_type value = (path->nodes[node_index].*value_member_ptr)[item_index];
   value_type original_value = value;

   auto [valued_changed, item_deactivated] = edit(&value);

   if (valued_changed) {
      if (edit_action* action =
             dynamic_cast<edit_action*>(action_stack->applied_top());
          action and action->matching(*path, node_index, value_member_ptr, item_index) and
          not action->closed) {
         original_value = action->original_value;

         action_stack->revert(*world);
      }

      action_stack->apply(std::make_unique<edit_action>(path->id, node_index,
                                                        value_member_ptr, item_index,
                                                        value, original_value),
                          *world);
   }

   if (item_deactivated) {
      edit_action* action = dynamic_cast<edit_action*>(action_stack->applied_top());

      if (action and action->matching(*path, node_index, value_member_ptr, item_index)) {
         action->closed = true;
      }
   }

   return valued_changed;
}

inline bool InputKeyValue(we::world::path* entity, const std::size_t node_index,
                          const std::size_t item_index, we::actions::stack* action_stack,
                          we::world::world* world, ImGuiInputTextFlags flags = 0,
                          ImGuiInputTextCallback callback = nullptr,
                          void* user_data = nullptr) noexcept
{
   return EditWithUndo(entity, node_index, &we::world::path::node::properties,
                       item_index, action_stack, world, [=](auto* kv) {
                          bool value_changed =
                             ImGui::InputText(kv->key.c_str(), &kv->value,
                                              flags | ImGuiInputTextFlags_NoUndoRedo,
                                              callback, user_data);

                          return we::edit_widget_result{.value_changed = value_changed,
                                                        .item_deactivated =
                                                           ImGui::IsItemDeactivated()};
                       });
}

}
