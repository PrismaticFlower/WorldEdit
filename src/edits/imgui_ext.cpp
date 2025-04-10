#include "imgui_ext.hpp"
#include "../imgui_ext.hpp"
#include "math/quaternion_funcs.hpp"
#include "set_class_name.hpp"
#include "set_value.hpp"

#include <cassert>
#include <numbers>

using namespace we;

namespace ImGui {

bool Checkbox(const char* label, bool* value,
              we::edits::stack<we::world::edit_context>& edit_stack,
              we::world::edit_context& context) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   bool edit_value = *value;

   const bool changed = Checkbox(label, &edit_value);

   if (changed) {
      edit_stack.apply(edits::make_set_value(value, edit_value), context);
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

bool DragFloat(const char* label, float* value,
               we::edits::stack<we::world::edit_context>& edit_stack,
               we::world::edit_context& context, float v_speed, float v_min,
               float v_max, const char* format, ImGuiSliderFlags flags) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   float edit_value = *value;

   const bool changed =
      DragFloat(label, &edit_value, v_speed, v_min, v_max, format, flags);

   if (changed) {
      edit_stack.apply(edits::make_set_value(value, edit_value), context);
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

bool DragFloat2(const char* label, float2* value,
                we::edits::stack<we::world::edit_context>& edit_stack,
                we::world::edit_context& context, float v_speed, float v_min,
                float v_max, ImGuiSliderFlags flags) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   float2 edit_value = *value;

   const bool changed = DragFloat2(label, &edit_value, v_speed, v_min, v_max, flags);

   if (changed) {
      edit_stack.apply(edits::make_set_value(value, edit_value), context);
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

bool DragFloat2XZ(const char* label, float2* value,
                  we::edits::stack<we::world::edit_context>& edit_stack,
                  we::world::edit_context& context, float v_speed, float v_min,
                  float v_max, ImGuiSliderFlags flags) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   float2 edit_value = *value;

   const bool changed = DragFloat2XZ(label, &edit_value, v_speed, v_min, v_max, flags);

   if (changed) {
      edit_stack.apply(edits::make_set_value(value, edit_value), context);
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

bool DragFloat3(const char* label, float3* value,
                we::edits::stack<we::world::edit_context>& edit_stack,
                we::world::edit_context& context, float v_speed, float v_min,
                float v_max, ImGuiSliderFlags flags) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   float3 edit_value = *value;

   const bool changed = DragFloat3(label, &edit_value, v_speed, v_min, v_max, flags);

   if (changed) {
      edit_stack.apply(edits::make_set_value(value, edit_value), context);
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

bool DragQuat(const char* label, quaternion* value,
              we::edits::stack<we::world::edit_context>& edit_stack,
              we::world::edit_context& context, float v_speed,
              ImGuiSliderFlags flags) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   quaternion edit_value = *value;

   const bool changed = DragQuat(label, &edit_value, v_speed, 0.0f, 0.0f, flags);

   if (changed) {
      edit_stack.apply(edits::make_set_value(value, edit_value), context);
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

bool DragInt2(const char* label, std::array<we::int32, 2>* value,
              we::edits::stack<we::world::edit_context>& edit_stack,
              we::world::edit_context& context, float v_speed, int v_min,
              int v_max, const char* format, ImGuiSliderFlags flags) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   std::array<we::int32, 2> edit_value = *value;

   const bool changed =
      DragInt2(label, edit_value.data(), v_speed, v_min, v_max, format, flags);

   if (changed) {
      edit_stack.apply(edits::make_set_value(value, edit_value), context);
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

bool DragFloatRange2(const char* label, float* value_current_min,
                     float* value_current_max,
                     we::edits::stack<we::world::edit_context>& edit_stack,
                     we::world::edit_context& context, float v_speed,
                     float v_min, float v_max, const char* format,
                     const char* format_max, ImGuiSliderFlags flags)
{
   IM_ASSERT(value_current_min);
   IM_ASSERT(context.is_memory_valid(value_current_min));
   IM_ASSERT(value_current_max);
   IM_ASSERT(context.is_memory_valid(value_current_max));

   float edit_value_current_min = *value_current_min;
   float edit_value_current_max = *value_current_max;

   const bool changed =
      DragFloatRange2(label, &edit_value_current_min, &edit_value_current_max,
                      v_speed, v_min, v_max, format, format_max, flags);

   if (changed) {
      if (edit_value_current_min != *value_current_min) {
         edit_stack.apply(edits::make_set_value(value_current_min, edit_value_current_min),
                          context);
      }
      else if (edit_value_current_max != *value_current_max) {
         edit_stack.apply(edits::make_set_value(value_current_max, edit_value_current_max),
                          context);
      }
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

bool SliderInt(const char* label, int* value,
               we::edits::stack<we::world::edit_context>& edit_stack,
               we::world::edit_context& context, int v_min, int v_max,
               const char* format, ImGuiSliderFlags flags) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   int edit_value = *value;

   const bool changed = SliderInt(label, &edit_value, v_min, v_max, format, flags);

   if (changed) {
      edit_stack.apply(edits::make_set_value(value, edit_value), context);
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

bool SliderInt(const char* label, int8* value,
               we::edits::stack<we::world::edit_context>& edit_stack,
               we::world::edit_context& context, int8 v_min, int8 v_max,
               const char* format, ImGuiSliderFlags flags) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   int8 edit_value = *value;

   const bool changed = SliderScalar(label, ImGuiDataType_S8, &edit_value,
                                     &v_min, &v_max, format, flags);

   if (changed) {
      edit_stack.apply(edits::make_set_value(value, edit_value), context);
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

bool ColorEdit3(const char* label, we::float3* value,
                we::edits::stack<we::world::edit_context>& edit_stack,
                we::world::edit_context& context, ImGuiColorEditFlags flags) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   float3 edit_value = *value;

   const bool changed = ColorEdit3(label, &edit_value.x, flags);

   if (changed) {
      edit_stack.apply(edits::make_set_value(value, edit_value), context);
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

bool ColorEdit4(const char* label, we::float4* value,
                we::edits::stack<we::world::edit_context>& edit_stack,
                we::world::edit_context& context, ImGuiColorEditFlags flags) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   float4 edit_value = *value;

   const bool changed = ColorEdit4(label, &edit_value.x, flags);

   if (changed) {
      edit_stack.apply(edits::make_set_value(value, edit_value), context);
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

bool InputText(const char* label, std::string* str,
               we::edits::stack<we::world::edit_context>& edit_stack,
               we::world::edit_context& context,
               we::function_ptr<void(std::string*) noexcept> edit_filter) noexcept
{
   IM_ASSERT(str);
   IM_ASSERT(context.is_memory_valid(str));

   absl::InlinedVector<char, 256> buffer{str->begin(), str->end()};

   const bool changed = InputText(label, &buffer);
   const bool deactivated = IsItemDeactivated();

   if (changed) {
      const std::string_view new_value_view{buffer.data(), buffer.size()};

      std::string new_value{new_value_view};

      if (edit_filter) edit_filter(&new_value);

      edit_stack.apply(edits::make_set_value(str, std::move(new_value)), context);
   }

   if (deactivated) edit_stack.close_last();

   return changed;
}

bool InputTextAutoComplete(
   const char* label, std::string* str,
   we::edits::stack<we::world::edit_context>& edit_stack,
   we::world::edit_context& context,
   we::function_ptr<std::array<std::string_view, 6>() noexcept> fill_entries_callback) noexcept
{
   IM_ASSERT(str);
   IM_ASSERT(context.is_memory_valid(str));

   absl::InlinedVector<char, 256> buffer{str->begin(), str->end()};

   const bool changed = InputTextAutoComplete(label, &buffer, fill_entries_callback);
   const bool deactivated = IsItemDeactivated();

   if (changed) {
      const std::string_view new_value_view{buffer.data(), buffer.size()};

      edit_stack.apply(edits::make_set_value(str, std::string{new_value_view}), context);
   }

   if (deactivated) edit_stack.close_last();

   return changed;
}

bool LayerPick(const char* label, we::int8* layer,
               we::edits::stack<we::world::edit_context>& edit_stack,
               we::world::edit_context& context) noexcept
{
   IM_ASSERT(layer);
   IM_ASSERT(context.is_memory_valid(layer));

   bool changed = false;

   if (ImGui::BeginCombo(label, [&] {
          if (*layer >= context.world.layer_descriptions.size()) return "";

          return context.world.layer_descriptions[*layer].name.c_str();
       }())) {

      for (we::int8 i = 0; i < std::ssize(context.world.layer_descriptions); ++i) {
         if (ImGui::Selectable(context.world.layer_descriptions[i].name.c_str())) {
            edit_stack.apply(edits::make_set_value(layer, i), context,
                             {.closed = true});
            changed = true;
         }
      }

      ImGui::EndCombo();
   }

   return changed;
}

bool InputKeyValue(std::vector<we::world::instance_property>* properties,
                   const we::uint32 item_index,
                   we::edits::stack<we::world::edit_context>& edit_stack,
                   we::world::edit_context& context) noexcept
{
   IM_ASSERT(properties);
   IM_ASSERT(context.is_memory_valid(properties));
   IM_ASSERT(item_index < properties->size());

   const world::instance_property& property = (*properties)[item_index];

   absl::InlinedVector<char, 256> buffer{property.value.begin(), property.value.end()};

   const bool changed = InputText(property.key.c_str(), &buffer);
   const bool deactivated = IsItemDeactivated();

   if (changed) {
      const std::string_view new_value_view{buffer.data(), buffer.size()};

      std::string new_value{new_value_view};

      edit_stack.apply(edits::make_set_vector_value(properties, item_index,
                                                    &world::instance_property::value,
                                                    std::move(new_value)),
                       context);
   }

   if (deactivated) edit_stack.close_last();

   return changed;
}

bool InputKeyValueAutoComplete(
   std::vector<we::world::instance_property>* properties,
   const we::uint32 item_index, we::edits::stack<we::world::edit_context>& edit_stack,
   we::world::edit_context& context,
   we::function_ptr<std::array<std::string_view, 6>() noexcept> fill_entries_callback) noexcept
{
   IM_ASSERT(properties);
   IM_ASSERT(context.is_memory_valid(properties));
   IM_ASSERT(item_index < properties->size());

   const world::instance_property& property = (*properties)[item_index];

   absl::InlinedVector<char, 256> buffer{property.value.begin(), property.value.end()};

   const bool changed =
      InputTextAutoComplete(property.key.c_str(), &buffer, fill_entries_callback);
   const bool deactivated = IsItemDeactivated();

   if (changed) {
      const std::string_view new_value_view{buffer.data(), buffer.size()};

      std::string new_value{new_value_view};

      edit_stack.apply(edits::make_set_vector_value(properties, item_index,
                                                    &world::instance_property::value,
                                                    std::move(new_value)),
                       context);
   }

   if (deactivated) edit_stack.close_last();

   return changed;
}

bool InputKeyValueWithDelete(std::vector<we::world::path::property>* properties,
                             const we::uint32 item_index, bool* delete_property,
                             we::edits::stack<we::world::edit_context>& edit_stack,
                             we::world::edit_context& context) noexcept
{
   IM_ASSERT(properties);
   IM_ASSERT(context.is_memory_valid(properties));
   IM_ASSERT(item_index < properties->size());

   const world::path::property& property = (*properties)[item_index];

   absl::InlinedVector<char, 256> buffer{property.value.begin(), property.value.end()};

   const bool changed =
      InputTextWithClose(property.key.c_str(), &buffer, delete_property);
   const bool deactivated = IsItemDeactivated();

   if (changed) {
      const std::string_view new_value_view{buffer.data(), buffer.size()};

      std::string new_value{new_value_view};

      edit_stack.apply(edits::make_set_vector_value(properties, item_index,
                                                    &world::path::property::value,
                                                    std::move(new_value)),
                       context);
   }

   if (deactivated) edit_stack.close_last();

   return changed;
}

bool InputKeyValueWithDelete(std::vector<we::world::path::node>* nodes,
                             const we::uint32 node_index,
                             const we::uint32 item_index, bool* delete_property,
                             we::edits::stack<we::world::edit_context>& edit_stack,
                             we::world::edit_context& context) noexcept
{
   IM_ASSERT(nodes);
   IM_ASSERT(context.is_memory_valid(nodes));
   IM_ASSERT(node_index < nodes->size());
   IM_ASSERT(item_index < (*nodes)[node_index].properties.size());

   const world::path::property& property = (*nodes)[node_index].properties[item_index];

   absl::InlinedVector<char, 256> buffer{property.value.begin(), property.value.end()};

   const bool changed =
      InputTextWithClose(property.key.c_str(), &buffer, delete_property);
   const bool deactivated = IsItemDeactivated();

   if (changed) {
      const std::string_view new_value_view{buffer.data(), buffer.size()};

      std::string new_value{new_value_view};

      edit_stack.apply(edits::make_set_path_node_property_value(nodes, node_index, item_index,
                                                                std::move(new_value)),
                       context);
   }

   if (deactivated) edit_stack.close_last();

   return changed;
}

bool DragRotationEuler(const char* label, we::quaternion* quat_rotation,
                       we::float3* euler_rotation,
                       we::edits::stack<we::world::edit_context>& edit_stack,
                       we::world::edit_context& context, float v_speed,
                       ImGuiSliderFlags flags) noexcept
{
   IM_ASSERT(quat_rotation);
   IM_ASSERT(context.is_memory_valid(quat_rotation));
   IM_ASSERT(euler_rotation);
   IM_ASSERT(context.is_memory_valid(euler_rotation));

   float3 edit_value = *euler_rotation;

   const bool changed = DragFloat3(label, &edit_value, v_speed, 0.0f, 0.0f, flags);
   const bool deactivated = IsItemDeactivated();

   if (changed) {
      edit_stack.apply(edits::make_set_multi_value(quat_rotation,
                                                   make_quat_from_euler(
                                                      edit_value *
                                                      std::numbers::pi_v<float> / 180.0f),
                                                   euler_rotation, edit_value),
                       context);

      ;
   }

   if (deactivated) edit_stack.close_last();

   return changed;
}

bool DragBarrierRotation(const char* label, float* value,
                         we::edits::stack<we::world::edit_context>& edit_stack,
                         we::world::edit_context& context, float v_speed,
                         float v_min, float v_max, const char* format,
                         ImGuiSliderFlags flags) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   float edit_value = *value * 180.0f / std::numbers::pi_v<float>;

   const bool changed =
      ImGui::DragFloat(label, &edit_value, v_speed, v_min, v_max, format, flags);

   if (changed) {
      edit_stack.apply(edits::make_set_value(value, edit_value / 180.0f *
                                                       std::numbers::pi_v<float>),
                       context);
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

bool DragPathNodeRotation(const char* label, std::vector<we::world::path::node>* nodes,
                          const we::uint32 node_index,
                          we::edits::stack<we::world::edit_context>& edit_stack,
                          we::world::edit_context& context, float v_speed,
                          ImGuiSliderFlags flags) noexcept
{
   IM_ASSERT(nodes);
   IM_ASSERT(context.is_memory_valid(nodes));
   IM_ASSERT(node_index < nodes->size());

   quaternion edit_value = (*nodes)[node_index].rotation;

   const bool changed = DragQuat(label, &edit_value, v_speed, 0.0f, 0.0f, flags);

   if (changed) {
      edit_stack.apply(edits::make_set_vector_value(nodes, node_index,
                                                    &world::path::node::rotation,
                                                    edit_value),
                       context);
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

bool DragPathNodeRotationEuler(const char* label, we::world::creation_entity* entity,
                               we::edits::stack<we::world::edit_context>& edit_stack,
                               we::world::edit_context& context, float v_speed,
                               ImGuiSliderFlags flags) noexcept
{
   IM_ASSERT(entity);
   IM_ASSERT(context.is_memory_valid(entity));
   IM_ASSERT(entity->is<world::path>());
   IM_ASSERT(entity->get<world::path>().nodes.size() >= 1);

   float3 edit_value = context.euler_rotation;

   const bool changed = DragFloat3(label, &edit_value, v_speed, 0.0f, 0.0f, flags);
   const bool deactivated = IsItemDeactivated();

   if (changed) {
      edit_stack.apply(edits::make_set_creation_path_node_location(
                          make_quat_from_euler(
                             edit_value * std::numbers::pi_v<float> / 180.0f),
                          entity->get<world::path>().nodes[0].position, edit_value),
                       context);
   }

   if (deactivated) edit_stack.close_last();

   return changed;
}

bool DragPathNodePosition(const char* label, std::vector<we::world::path::node>* nodes,
                          const we::uint32 node_index,
                          we::edits::stack<we::world::edit_context>& edit_stack,
                          we::world::edit_context& context, float v_speed,
                          ImGuiSliderFlags flags) noexcept
{
   IM_ASSERT(nodes);
   IM_ASSERT(context.is_memory_valid(nodes));
   IM_ASSERT(node_index < nodes->size());

   float3 edit_value = (*nodes)[node_index].position;

   const bool changed = DragFloat3(label, &edit_value, v_speed, 0.0f, 0.0f, flags);

   if (changed) {
      edit_stack.apply(edits::make_set_vector_value(nodes, node_index,
                                                    &world::path::node::position,
                                                    edit_value),
                       context);
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

bool DragSectorPoint(const char* label, std::vector<we::float2>* points,
                     const we::uint32 point_index,
                     we::edits::stack<we::world::edit_context>& edit_stack,
                     we::world::edit_context& context, float v_speed,
                     ImGuiSliderFlags flags) noexcept
{
   IM_ASSERT(points);
   IM_ASSERT(context.is_memory_valid(points));
   IM_ASSERT(point_index < points->size());

   float2 edit_value = (*points)[point_index];

   const bool changed = DragFloat2XZ(label, &edit_value, v_speed, 0.0f, 0.0f, flags);

   if (changed) {
      edit_stack.apply(edits::make_set_vector_value(points, point_index, edit_value),
                       context);
   }

   if (IsItemDeactivated()) {
      edit_stack.close_last();
   }

   return changed;
}

}
