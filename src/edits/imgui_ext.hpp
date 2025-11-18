#pragma once

#include "../imgui_ext.hpp"
#include "set_value.hpp"
#include "stack.hpp"
#include "types.hpp"
#include "utility/function_ptr.hpp"
#include "world/interaction_context.hpp"

#include <imgui.h>

// Wrappers for ImGui controls that integrate with the Undo-Redo stack.

namespace we::edits::imgui {

template<typename T>
struct enum_select_option {
   const char* label = "";
   T value = {};
};

template<typename T>
struct edit_flag {
   const char* label = "";
   T bit = {};
};

}

namespace we {
using we::edits::imgui::edit_flag;
using we::edits::imgui::enum_select_option;

namespace world {

struct object_class_library;

}

}

namespace ImGui {

bool Checkbox(const char* label, bool* value,
              we::edits::stack<we::world::edit_context>& edit_stack,
              we::world::edit_context& context) noexcept;

bool DragFloat(const char* label, float* value,
               we::edits::stack<we::world::edit_context>& edit_stack,
               we::world::edit_context& context, float v_speed = 1.0f,
               float v_min = 0.0f, float v_max = 0.0f,
               const char* format = "%.3f", ImGuiSliderFlags flags = 0) noexcept;

bool DragFloat2(const char* label, we::float2* value,
                we::edits::stack<we::world::edit_context>& edit_stack,
                we::world::edit_context& context, float v_speed = 1.0f,
                float v_min = 0.0f, float v_max = 0.0f,
                ImGuiSliderFlags flags = 0) noexcept;

bool DragFloat2XZ(const char* label, we::float2* value,
                  we::edits::stack<we::world::edit_context>& edit_stack,
                  we::world::edit_context& context, float v_speed = 1.0f,
                  float v_min = 0.0f, float v_max = 0.0f,
                  ImGuiSliderFlags flags = 0) noexcept;

bool DragFloat3(const char* label, we::float3* value,
                we::edits::stack<we::world::edit_context>& edit_stack,
                we::world::edit_context& context, float v_speed = 1.0f,
                float v_min = 0.0f, float v_max = 0.0f,
                ImGuiSliderFlags flags = 0) noexcept;

bool DragQuat(const char* label, we::quaternion* value,
              we::edits::stack<we::world::edit_context>& edit_stack,
              we::world::edit_context& context, float v_speed = 0.001f,
              ImGuiSliderFlags flags = 0) noexcept;

bool DragInt(const char* label, we::int32* value,
             we::edits::stack<we::world::edit_context>& edit_stack,
             we::world::edit_context& context, float v_speed = 1.0f,
             we::int32 v_min = 0, we::int32 v_max = 0,
             const char* format = "%d", ImGuiSliderFlags flags = 0) noexcept;

bool DragInt2(const char* label, std::array<we::int32, 2>* value,
              we::edits::stack<we::world::edit_context>& edit_stack,
              we::world::edit_context& context, float v_speed = 1.0f,
              int v_min = 0, int v_max = 0, const char* format = "%d",
              ImGuiSliderFlags flags = 0) noexcept;

bool DragFloatRange2(const char* label, float* value_current_min,
                     float* value_current_max,
                     we::edits::stack<we::world::edit_context>& edit_stack,
                     we::world::edit_context& context, float v_speed = 1.0f,
                     float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f",
                     const char* format_max = NULL, ImGuiSliderFlags flags = 0);

bool SliderInt(const char* label, int* value,
               we::edits::stack<we::world::edit_context>& edit_stack,
               we::world::edit_context& context, int v_min = 0, int v_max = 0,
               const char* format = "%d", ImGuiSliderFlags flags = 0) noexcept;

bool SliderInt(const char* label, we::int8* value,
               we::edits::stack<we::world::edit_context>& edit_stack,
               we::world::edit_context& context, we::int8 v_min = 0,
               we::int8 v_max = 0, const char* format = "%d",
               ImGuiSliderFlags flags = 0) noexcept;

bool ColorEdit3(const char* label, we::float3* value,
                we::edits::stack<we::world::edit_context>& edit_stack,
                we::world::edit_context& context, ImGuiColorEditFlags flags = 0) noexcept;

bool ColorEdit4(const char* label, we::float4* value,
                we::edits::stack<we::world::edit_context>& edit_stack,
                we::world::edit_context& context, ImGuiColorEditFlags flags = 0) noexcept;

bool InputText(const char* label, std::string* str,
               we::edits::stack<we::world::edit_context>& edit_stack,
               we::world::edit_context& context,
               we::function_ptr<void(std::string*) noexcept> edit_filter = nullptr) noexcept;

bool InputTextAutoComplete(
   const char* label, std::string* str,
   we::edits::stack<we::world::edit_context>& edit_stack,
   we::world::edit_context& context,
   we::function_ptr<std::array<std::string_view, 6>() noexcept> fill_entries_callback) noexcept;

bool LayerPick(const char* label, we::int8* layer,
               we::edits::stack<we::world::edit_context>& edit_stack,
               we::world::edit_context& context) noexcept;

bool InputKeyValue(std::vector<we::world::instance_property>* properties,
                   const we::uint32 item_index,
                   we::edits::stack<we::world::edit_context>& edit_stack,
                   we::world::edit_context& context) noexcept;

bool InputKeyValueAutoComplete(
   std::vector<we::world::instance_property>* properties,
   const we::uint32 item_index, we::edits::stack<we::world::edit_context>& edit_stack,
   we::world::edit_context& context,
   we::function_ptr<std::array<std::string_view, 6>() noexcept> fill_entries_callback) noexcept;

bool InputKeyValueWithDelete(std::vector<we::world::path::property>* properties,
                             const we::uint32 item_index, bool* delete_property,
                             we::edits::stack<we::world::edit_context>& edit_stack,
                             we::world::edit_context& context) noexcept;

bool InputKeyValueWithDelete(std::vector<we::world::path::node>* nodes,
                             const we::uint32 node_index,
                             const we::uint32 item_index, bool* delete_property,
                             we::edits::stack<we::world::edit_context>& edit_stack,
                             we::world::edit_context& context) noexcept;

bool DragRotationEuler(const char* label, we::quaternion* quat_rotation,
                       we::float3* euler_rotation,
                       we::edits::stack<we::world::edit_context>& edit_stack,
                       we::world::edit_context& context, float v_speed = 0.001f,
                       ImGuiSliderFlags flags = 0) noexcept;

bool DragBarrierRotation(const char* label, float* value,
                         we::edits::stack<we::world::edit_context>& edit_stack,
                         we::world::edit_context& context, float v_speed = 1.0f,
                         float v_min = 0.0f, float v_max = 0.0f,
                         const char* format = "%.3f",
                         ImGuiSliderFlags flags = 0) noexcept;

bool DragPathNodeRotation(const char* label, std::vector<we::world::path::node>* nodes,
                          const we::uint32 node_index,
                          we::edits::stack<we::world::edit_context>& edit_stack,
                          we::world::edit_context& context, float v_speed = 1.0f,
                          ImGuiSliderFlags flags = 0) noexcept;

bool DragPathNodeRotationEuler(const char* label, we::world::creation_entity* entity,
                               we::edits::stack<we::world::edit_context>& edit_stack,
                               we::world::edit_context& context, float v_speed = 0.001f,
                               ImGuiSliderFlags flags = 0) noexcept;

bool DragPathNodePosition(const char* label, std::vector<we::world::path::node>* nodes,
                          const we::uint32 node_index,
                          we::edits::stack<we::world::edit_context>& edit_stack,
                          we::world::edit_context& context, float v_speed = 0.001f,
                          ImGuiSliderFlags flags = 0) noexcept;

bool DragSectorPoint(const char* label, std::vector<we::float2>* points,
                     const we::uint32 point_index,
                     we::edits::stack<we::world::edit_context>& edit_stack,
                     we::world::edit_context& context, float v_speed = 0.001f,
                     ImGuiSliderFlags flags = 0) noexcept;

template<typename Enum>
inline bool EnumSelect(const char* label, Enum* value,
                       we::edits::stack<we::world::edit_context>& edit_stack,
                       we::world::edit_context& context,
                       std::initializer_list<we::enum_select_option<Enum>> values) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   bool changed = false;

   if (BeginCombo(label, [&] {
          for (auto& option : values) {
             if (option.value == *value) return option.label;
          }

          return "";
       }())) {

      for (auto option : values) {
         if (Selectable(option.label)) {
            edit_stack.apply(we::edits::make_set_value(value, option.value),
                             context, {.closed = true});
            changed = true;
         }
      }

      EndCombo();
   }

   return changed;
}

template<typename Flags, int count>
inline bool EditFlags(const char* label, Flags* value,
                      we::edits::stack<we::world::edit_context>& edit_stack,
                      we::world::edit_context& context,
                      const we::edit_flag<Flags> (&flags)[count]) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   static_assert(sizeof(Flags) <= sizeof(unsigned int),
                 "Flags size is too large.");

   unsigned int uint_value = static_cast<unsigned int>(*value);

   ExtEditFlag uint_flags[count] = {};

   for (int i = 0; i < count; ++i) {
      uint_flags[i] = {flags[i].label, static_cast<unsigned int>(flags[i].bit)};
   }

   bool changed = EditFlags(label, &uint_value, {&uint_flags[0], count});

   if (changed) {
      edit_stack.apply(we::edits::make_set_value(value, static_cast<Flags>(uint_value)),
                       context, {.closed = true});
   }

   return changed;
}

template<typename Enum>
inline bool EnumSelect(const char* label, Enum* value,
                       std::initializer_list<we::enum_select_option<Enum>> values) noexcept
{
   bool changed = false;

   if (ImGui::BeginCombo(label, [&] {
          for (auto& option : values) {
             if (option.value == *value) return option.label;
          }

          return "";
       }())) {

      for (auto option : values) {
         if (ImGui::Selectable(option.label)) {
            *value = option.value;
            changed = true;
         }
      }

      ImGui::EndCombo();
   }

   return changed;
}

}
