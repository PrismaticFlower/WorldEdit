#pragma once

#include "math/quaternion_funcs.hpp"
#include "stack.hpp"
#include "types.hpp"
#include "ui_action.hpp"
#include "world/interaction_context.hpp"
#include "world/world.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_ext.hpp"
#include "imgui/imgui_stdlib.h"

#include <array>
#include <initializer_list>
#include <memory>
#include <numbers>
#include <optional>
#include <type_traits>
#include <utility>

// Wrappers for ImGui controls that integrate with the Undo-Redo stack.

namespace we::edits::imgui {

struct edit_widget_result {
   bool value_changed;
   bool item_deactivated;
};

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

template<typename Callback, typename T>
concept edit_widget_callback =
   std::is_invocable_r_v<edit_widget_result, Callback, T*>;

template<typename Callback, typename T, typename U>
concept edit_widget_double_callback =
   std::is_invocable_r_v<edit_widget_result, Callback, T*, U*>;

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
using we::edits::imgui::edit_flag;
using we::edits::imgui::edit_widget_result;
using we::edits::imgui::enum_select_option;
}

namespace ImGui {

// Entity Property Editors

template<typename Entity, typename T>
inline bool EditWithUndo(const Entity* object, T Entity::*value_member_ptr,
                         we::edits::stack<we::world::edit_context>* edit_stack,
                         we::world::edit_context* context,
                         we::edits::imgui::edit_widget_callback<T> auto editor) noexcept
{
   using namespace we;
   using namespace we::edits;
   using namespace we::edits::imgui;

   using edit_type = ui_edit<Entity, T>;
   using value_type = T;

   value_type value = object->*value_member_ptr;
   value_type original_value = value;

   auto [valued_changed, item_deactivated] = editor(&value);

   if (valued_changed) {
      edit_stack->apply(std::make_unique<edit_type>(object->id, value_member_ptr,
                                                    value, original_value),
                        *context);
   }

   if (item_deactivated) edit_stack->close_last();

   return valued_changed;
}

template<typename Entity>
inline bool Checkbox(const char* label, const Entity* entity,
                     bool Entity::*value_member_ptr,
                     we::edits::stack<we::world::edit_context>* edit_stack,
                     we::world::edit_context* context) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](bool* value) {
      bool value_changed = ImGui::Checkbox(label, value);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool DragFloat(const char* label, const Entity* entity,
                      float Entity::*value_member_ptr,
                      we::edits::stack<we::world::edit_context>* edit_stack,
                      we::world::edit_context* context, float v_speed = 1.0f,
                      float v_min = 0.0f, float v_max = 0.0f,
                      const char* format = "%.3f", ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](float* value) {
      bool value_changed =
         ImGui::DragFloat(label, value, v_speed, v_min, v_max, format, flags);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool DragFloat2(const char* label, const Entity* entity,
                       we::float2 Entity::*value_member_ptr,
                       we::edits::stack<we::world::edit_context>* edit_stack,
                       we::world::edit_context* context, float v_speed = 1.0f,
                       float v_min = 0.0f, float v_max = 0.0f,
                       ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](we::float2* value) {
      bool value_changed =
         ImGui::DragFloat2(label, value, v_speed, v_min, v_max, flags);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool DragFloat2XZ(const char* label, const Entity* entity,
                         we::float2 Entity::*value_member_ptr,
                         we::edits::stack<we::world::edit_context>* edit_stack,
                         we::world::edit_context* context, float v_speed = 1.0f,
                         float v_min = 0.0f, float v_max = 0.0f,
                         ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](we::float2* value) {
      bool value_changed =
         ImGui::DragFloat2XZ(label, value, v_speed, v_min, v_max, flags);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool DragFloat3(const char* label, const Entity* entity,
                       we::float3 Entity::*value_member_ptr,
                       we::edits::stack<we::world::edit_context>* edit_stack,
                       we::world::edit_context* context, float v_speed = 1.0f,
                       float v_min = 0.0f, float v_max = 0.0f,
                       ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](we::float3* value) {
      bool value_changed =
         ImGui::DragFloat3(label, value, v_speed, v_min, v_max, flags);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool DragQuat(const char* label, const Entity* entity,
                     we::quaternion Entity::*value_member_ptr,
                     we::edits::stack<we::world::edit_context>* edit_stack,
                     we::world::edit_context* context, float v_speed = 0.001f,
                     ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](we::quaternion* value) {
      bool value_changed = ImGui::DragQuat(label, value, v_speed, 0.0f, 0.0f, flags);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool SliderInt(const char* label, const Entity* entity,
                      int Entity::*value_member_ptr,
                      we::edits::stack<we::world::edit_context>* edit_stack,
                      we::world::edit_context* context, int v_min = 0, int v_max = 0,
                      const char* format = "%d", ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](int* value) {
      bool value_changed = ImGui::SliderInt(label, value, v_min, v_max, format, flags);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool SliderInt(const char* label, const Entity* entity,
                      we::int8 Entity::*value_member_ptr,
                      we::edits::stack<we::world::edit_context>* edit_stack,
                      we::world::edit_context* context, int v_min = 0, int v_max = 0,
                      const char* format = "%d", ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](we::int8* value) {
      int int_value = *value;

      bool value_changed =
         ImGui::SliderInt(label, &int_value, v_min, v_max, format, flags);

      if (value_changed) *value = static_cast<we::int8>(int_value);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool ColorEdit3(const char* label, const Entity* entity,
                       we::float3 Entity::*value_member_ptr,
                       we::edits::stack<we::world::edit_context>* edit_stack,
                       we::world::edit_context* context,
                       ImGuiColorEditFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](we::float3* value) {
      bool value_changed = ImGui::ColorEdit3(label, &value->x, flags);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity, typename T>
inline bool InputText(const char* label, const Entity* entity,
                      T Entity::*value_member_ptr,
                      we::edits::stack<we::world::edit_context>* edit_stack,
                      we::world::edit_context* context,
                      std::invocable<T*> auto edit_filter) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](std::string* value) {
      bool value_changed = ImGui::InputText(label, value);

      if (value_changed) edit_filter(value);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity, typename T, typename Fill>
inline bool InputTextAutoComplete(const char* label, const Entity* entity,
                                  T Entity::*value_member_ptr,
                                  we::edits::stack<we::world::edit_context>* edit_stack,
                                  we::world::edit_context* context,
                                  Fill fill_entries_callback) noexcept
   requires std::is_invocable_r_v<std::array<std::string, 6>, Fill>
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [&](T* value) {
      bool value_changed = ImGui::InputTextAutoComplete(
         label, value,
         [](void* user_data) { return (*static_cast<Fill*>(user_data))(); },
         static_cast<void*>(&fill_entries_callback));
      bool is_deactivated = ImGui::IsItemDeactivated();

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = is_deactivated};
   });
}

template<typename Entity>
inline bool LayerPick(const char* label, const Entity* entity,
                      we::edits::stack<we::world::edit_context>* edit_stack,
                      we::world::edit_context* context) noexcept
{
   return EditWithUndo(entity, &Entity::layer, edit_stack, context, [=](int* layer) {
      bool value_changed = false;

      if (ImGui::BeginCombo(label, [&] {
             if (entity->layer >= context->world.layer_descriptions.size())
                return "";

             return context->world.layer_descriptions[entity->layer].name.c_str();
          }())) {

         for (int i = 0; i < std::ssize(context->world.layer_descriptions); ++i) {
            if (ImGui::Selectable(context->world.layer_descriptions[i].name.c_str())) {
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
inline bool EnumSelect(const char* label, const Entity* entity,
                       Enum Entity::*value_member_ptr,
                       we::edits::stack<we::world::edit_context>* edit_stack,
                       we::world::edit_context* context,
                       std::initializer_list<we::enum_select_option<Enum>> values) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=, &values](Enum* value) {
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

template<typename Entity, typename Flags>
inline bool EditFlags(const char* label, const Entity* entity,
                      Flags Entity::*value_member_ptr,
                      we::edits::stack<we::world::edit_context>* edit_stack,
                      we::world::edit_context* context,
                      std::initializer_list<we::edit_flag<Flags>> flags) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=, &flags](Flags* value) {
      unsigned int uint_value = std::to_underlying(*value);

      std::array<ImGui::ExtEditFlag, 32> uint_flags;

      for (std::size_t i = 0; i < (flags.size() > 32 ? 32 : flags.size()); ++i) {
         auto& flag = flags.begin()[i];

         uint_flags[i] = {flag.label, static_cast<unsigned int>(flag.bit)};
      }

      bool value_changed =
         ImGui::EditFlags(label, &uint_value, {uint_flags.data(), flags.size()});

      if (value_changed) {
         *value = static_cast<Flags>(uint_value);
      }

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = value_changed};
   });
}

inline bool DragBarrierRotation(const char* label, we::world::barrier* barrier,
                                we::edits::stack<we::world::edit_context>* edit_stack,
                                we::world::edit_context* context,
                                float v_speed = 1.0f, float v_min = 0.0f,
                                float v_max = 0.0f, const char* format = "%.3f",
                                ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(barrier, &we::world::barrier::rotation_angle, edit_stack,
                       context, [=](float* value) {
                          float degrees = *value * 180.0f / std::numbers::pi_v<float>;

                          const bool value_changed =
                             ImGui::DragFloat(label, &degrees, v_speed, v_min,
                                              v_max, format, flags);

                          if (value_changed) {
                             *value = degrees / 180.0f * std::numbers::pi_v<float>;
                          }

                          return we::edit_widget_result{.value_changed = value_changed,
                                                        .item_deactivated =
                                                           ImGui::IsItemDeactivated()};
                       });
}

// Entity Vector/Array Property Editors

template<typename Entity, typename T>
inline bool EditWithUndo(const Entity* entity, std::vector<T> Entity::*value_member_ptr,
                         const std::size_t item_index,
                         we::edits::stack<we::world::edit_context>* edit_stack,
                         we::world::edit_context* context,
                         we::edits::imgui::edit_widget_callback<T> auto editor) noexcept
{
   using namespace we;
   using namespace we::edits;
   using namespace we::edits::imgui;

   using edit_type = ui_edit_indexed<Entity, T>;
   using value_type = T;

   value_type value = (entity->*value_member_ptr)[item_index];
   value_type original_value = value;

   auto [valued_changed, item_deactivated] = editor(&value);

   if (valued_changed) {
      edit_stack->apply(std::make_unique<edit_type>(entity->id, value_member_ptr,
                                                    item_index, value, original_value),
                        *context);
   }

   if (item_deactivated) edit_stack->close_last();

   return valued_changed;
}

template<typename Entity, we::edits::imgui::input_key_value_type T>
inline bool InputKeyValue(const Entity* entity, std::vector<T> Entity::*value_member_ptr,
                          const std::size_t item_index,
                          we::edits::stack<we::world::edit_context>* edit_stack,
                          we::world::edit_context* context,
                          ImGuiInputTextFlags flags = 0,
                          ImGuiInputTextCallback callback = nullptr,
                          void* user_data = nullptr) noexcept
{
   return EditWithUndo(entity, value_member_ptr, item_index, edit_stack, context, [=](T* kv) {
      bool value_changed =
         ImGui::InputText(kv->key.c_str(), &kv->value, flags, callback, user_data);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity, we::edits::imgui::input_key_value_type T, typename Fill>
inline bool InputKeyValueAutoComplete(const Entity* entity,
                                      std::vector<T> Entity::*value_member_ptr,
                                      const std::size_t item_index,
                                      we::edits::stack<we::world::edit_context>* edit_stack,
                                      we::world::edit_context* context,
                                      const Fill& fill_entries_callback) noexcept
{
   return EditWithUndo(entity, value_member_ptr, item_index, edit_stack, context, [=](T* kv) {
      using string_type = decltype(kv->value);

      std::optional<std::array<string_type, 6>> autocomplete_entries;

      std::pair<const Fill&, decltype(autocomplete_entries)&>
         callback_userdata{fill_entries_callback, autocomplete_entries};

      ImGui::BeginGroup();

      bool value_changed = ImGui::InputText(
         kv->key.c_str(), &kv->value, ImGuiInputTextFlags_CallbackCompletion,
         [](ImGuiInputTextCallbackData* data) {
            if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion) {
               auto& user_data =
                  *static_cast<decltype(callback_userdata)*>(data->UserData);

               user_data.second.emplace(user_data.first());

               string_type& autofill = (*user_data.second)[0];

               if (not autofill.empty()) {
                  data->DeleteChars(0, data->BufTextLen);
                  data->InsertChars(0, autofill.c_str(),
                                    autofill.c_str() + autofill.size());
               }
            }

            return 0;
         },
         &callback_userdata);

      bool is_deactivated = ImGui::IsItemDeactivated();

      if (ImGui::IsItemActive()) {
         ImGui::SetNextWindowPos(
            ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y));
         ImGui::SetNextWindowSize(
            ImVec2{ImGui::GetItemRectMax().x - ImGui::GetItemRectMin().x -
                      ImGui::CalcTextSize(kv->key.c_str(), nullptr, true).x -
                      ImGui::GetStyle().ItemInnerSpacing.x,
                   9.0f * ImGui::GetFontSize()});

         ImGui::BeginTooltip();

         if (not autocomplete_entries) {
            autocomplete_entries.emplace(fill_entries_callback());
         }

         if ((*autocomplete_entries)[0].empty()) {
            ImGui::TextUnformatted("No matches.");
         }
         else {
            for (const string_type& asset : *autocomplete_entries) {
               ImGui::TextUnformatted(asset.c_str(), asset.c_str() + asset.size());
            }
         }

         ImGui::EndTooltip();
      }

      ImGui::EndGroup();

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = is_deactivated};
   });
}

// Path Node Property Editors

template<typename T>
inline bool EditWithUndo(we::world::path* path, const std::size_t node_index,
                         T we::world::path::node::*value_member_ptr,
                         we::edits::stack<we::world::edit_context>* edit_stack,
                         we::world::edit_context* context,
                         we::edits::imgui::edit_widget_callback<T> auto editor) noexcept
{
   using namespace we;
   using namespace we::edits;
   using namespace we::edits::imgui;

   using edit_type = ui_edit_path_node<T>;
   using value_type = T;

   value_type value = path->nodes[node_index].*value_member_ptr;
   value_type original_value = value;

   auto [valued_changed, item_deactivated] = editor(&value);

   if (valued_changed) {
      edit_stack->apply(std::make_unique<edit_type>(path->id, node_index, value_member_ptr,
                                                    value, original_value),
                        *context);
   }

   if (item_deactivated) edit_stack->close_last();

   return valued_changed;
}

inline bool DragFloat3(const char* label, we::world::path* entity,
                       const std::size_t node_index,
                       we::float3 we::world::path::node::*value_member_ptr,
                       we::edits::stack<we::world::edit_context>* edit_stack,
                       we::world::edit_context* context, float v_speed = 1.0f,
                       float v_min = 0.0f, float v_max = 0.0f,
                       const char* format = "%.3f", ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, node_index, value_member_ptr, edit_stack,
                       context, [=](we::float3* value) {
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
                     we::edits::stack<we::world::edit_context>* edit_stack,
                     we::world::edit_context* context, float v_speed = 0.01f,
                     const char* format = "%.4f", ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, node_index, value_member_ptr, edit_stack,
                       context, [=](we::quaternion* value) {
                          bool value_changed =
                             ImGui::DragFloat4(label, &(*value).w, v_speed,
                                               0.0f, 0.0f, format, flags);

                          if (value_changed) {
                             *value = normalize(*value);
                          }

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
                         we::edits::stack<we::world::edit_context>* edit_stack,
                         we::world::edit_context* context,
                         we::edits::imgui::edit_widget_callback<T> auto editor) noexcept
{
   using namespace we;
   using namespace we::edits;
   using namespace we::edits::imgui;

   using edit_type = ui_edit_path_node_indexed<T>;
   using value_type = T;

   value_type value = (path->nodes[node_index].*value_member_ptr)[item_index];
   value_type original_value = value;

   auto [valued_changed, item_deactivated] = editor(&value);

   if (valued_changed) {
      edit_stack->apply(std::make_unique<edit_type>(path->id, node_index,
                                                    value_member_ptr, item_index,
                                                    value, original_value),
                        *context);
   }

   if (item_deactivated) edit_stack->close_last();

   return valued_changed;
}

inline bool InputKeyValue(we::world::path* entity, const std::size_t node_index,
                          const std::size_t item_index,
                          we::edits::stack<we::world::edit_context>* edit_stack,
                          we::world::edit_context* context,
                          ImGuiInputTextFlags flags = 0,
                          ImGuiInputTextCallback callback = nullptr,
                          void* user_data = nullptr) noexcept
{
   return EditWithUndo(entity, node_index, &we::world::path::node::properties,
                       item_index, edit_stack, context, [=](auto* kv) {
                          bool value_changed =
                             ImGui::InputText(kv->key.c_str(), &kv->value,
                                              flags, callback, user_data);

                          return we::edit_widget_result{.value_changed = value_changed,
                                                        .item_deactivated =
                                                           ImGui::IsItemDeactivated()};
                       });
}

// Entity Creation Editors

template<typename Fill>
inline bool InputTextAutoComplete(const char* label, std::string* value,
                                  Fill fill_entries_callback) noexcept
   requires std::is_invocable_r_v<std::array<std::string, 6>, Fill>
{
   return ImGui::InputTextAutoComplete(
      label, value,
      [](void* user_data) { return (*static_cast<Fill*>(user_data))(); },
      static_cast<void*>(&fill_entries_callback));
}

inline bool LayerPick(const char* label, int* layer, we::world::world* world) noexcept
{
   bool value_changed = false;

   if (ImGui::BeginCombo(label, [&] {
          if (*layer >= world->layer_descriptions.size()) return "";

          return world->layer_descriptions[*layer].name.c_str();
       }())) {

      for (int i = 0; i < std::ssize(world->layer_descriptions); ++i) {
         if (ImGui::Selectable(world->layer_descriptions[i].name.c_str())) {
            *layer = i;
            value_changed = true;
         }
      }

      ImGui::EndCombo();
   }

   return value_changed;
}

template<typename Enum>
inline bool EnumSelect(const char* label, Enum* value,
                       std::initializer_list<we::enum_select_option<Enum>> values) noexcept
{
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

   return value_changed;
}

// Creation Entity Editors

template<typename Entity, typename T>
inline bool EditWithUndo(we::world::creation_entity* entity, T Entity::*value_member_ptr,
                         we::edits::stack<we::world::edit_context>* edit_stack,
                         we::world::edit_context* context,
                         we::edits::imgui::edit_widget_callback<T> auto editor) noexcept
{
   using namespace we;
   using namespace we::edits;
   using namespace we::edits::imgui;

   using edit_type = ui_creation_edit<Entity, T>;
   using value_type = T;

   value_type value = std::get_if<Entity>(entity)->*value_member_ptr;
   value_type original_value = value;

   auto [valued_changed, item_deactivated] = editor(&value);

   if (valued_changed) {
      edit_stack->apply(std::make_unique<edit_type>(value_member_ptr, value, original_value),
                        *context);
   }

   if (item_deactivated) edit_stack->close_last();

   return valued_changed;
}

template<typename Entity, typename T, typename U>
inline bool EditWithUndo(we::world::creation_entity* entity, T Entity::*value_member_ptr,
                         U we::world::edit_context::*meta_value_member_ptr,
                         we::edits::stack<we::world::edit_context>* edit_stack,
                         we::world::edit_context* context,
                         we::edits::imgui::edit_widget_double_callback<T, U> auto editor) noexcept
{
   using namespace we;
   using namespace we::edits;
   using namespace we::edits::imgui;

   using edit_type = ui_creation_edit_with_meta<Entity, T, U>;
   using value_type = T;
   using meta_value_type = U;

   value_type value = std::get_if<Entity>(entity)->*value_member_ptr;
   value_type original_value = value;
   meta_value_type meta_value = context->*meta_value_member_ptr;
   meta_value_type meta_original_value = meta_value;

   auto [valued_changed, item_deactivated] = editor(&value, &meta_value);

   if (valued_changed) {
      edit_stack->apply(std::make_unique<edit_type>(value_member_ptr, value,
                                                    original_value, meta_value_member_ptr,
                                                    meta_value, meta_original_value),
                        *context);
   }

   if (item_deactivated) edit_stack->close_last();

   return valued_changed;
}

template<typename Entity, typename T>
inline bool InputText(const char* label, we::world::creation_entity* entity,
                      T Entity::*value_member_ptr,
                      we::edits::stack<we::world::edit_context>* edit_stack,
                      we::world::edit_context* context,
                      std::invocable<T*> auto edit_filter) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](T* value) {
      bool value_changed = ImGui::InputText(label, value);

      if (value_changed) {
         edit_filter(value);
      }

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity, typename T, typename Fill>
inline bool InputTextAutoComplete(const char* label, we::world::creation_entity* entity,
                                  T Entity::*value_member_ptr,
                                  we::edits::stack<we::world::edit_context>* edit_stack,
                                  we::world::edit_context* context,
                                  Fill fill_entries_callback) noexcept
   requires std::is_invocable_r_v<std::array<std::string, 6>, Fill>
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [&](std::string* value) {
      bool value_changed = ImGui::InputTextAutoComplete(
         label, value,
         [](void* user_data) { return (*static_cast<Fill*>(user_data))(); },
         static_cast<void*>(&fill_entries_callback));
      bool is_deactivated = ImGui::IsItemDeactivated();

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = is_deactivated};
   });
}

template<typename Entity>
inline bool LayerPick(const char* label, we::world::creation_entity* entity,
                      we::edits::stack<we::world::edit_context>* edit_stack,
                      we::world::edit_context* context) noexcept
{
   return EditWithUndo(entity, &Entity::layer, edit_stack, context, [=](int* layer) {
      bool value_changed = false;

      if (ImGui::BeginCombo(label, [&] {
             if (*layer >= context->world.layer_descriptions.size()) return "";

             return context->world.layer_descriptions[*layer].name.c_str();
          }())) {

         for (int i = 0; i < std::ssize(context->world.layer_descriptions); ++i) {
            if (ImGui::Selectable(context->world.layer_descriptions[i].name.c_str())) {
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

template<typename Entity>
inline bool DragRotationEuler(const char* label, we::world::creation_entity* entity,
                              we::quaternion Entity::*value_member_ptr,
                              we::float3 we::world::edit_context::*meta_value_member_ptr,
                              we::edits::stack<we::world::edit_context>* edit_stack,
                              we::world::edit_context* context)
{
   return EditWithUndo(entity, value_member_ptr, meta_value_member_ptr,
                       edit_stack, context,
                       [=](we::quaternion* rotation, we::float3* rotation_euler) {
                          bool value_changed =
                             ImGui::DragFloat3(label, rotation_euler);

                          if (value_changed) {
                             *rotation = make_quat_from_euler(
                                *rotation_euler * std::numbers::pi_v<float> / 180.0f);
                          }

                          return we::edit_widget_result{.value_changed = value_changed,
                                                        .item_deactivated =
                                                           ImGui::IsItemDeactivated()};
                       });
}

template<typename Entity>
inline bool DragFloat(const char* label, we::world::creation_entity* entity,
                      float Entity::*value_member_ptr,
                      we::edits::stack<we::world::edit_context>* edit_stack,
                      we::world::edit_context* context, float v_speed = 1.0f,
                      float v_min = 0.0f, float v_max = 0.0f,
                      const char* format = "%.3f", ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](float* value) {
      return we::edit_widget_result{.value_changed =
                                       ImGui::DragFloat(label, value, v_speed, v_min,
                                                        v_max, format, flags),
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool DragFloat2(const char* label, we::world::creation_entity* entity,
                       we::float2 Entity::*value_member_ptr,
                       we::edits::stack<we::world::edit_context>* edit_stack,
                       we::world::edit_context* context, float v_speed = 1.0f,
                       float v_min = 0.0f, float v_max = 0.0f,
                       ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](we::float2* value) {
      return we::edit_widget_result{.value_changed =
                                       ImGui::DragFloat2(label, value, v_speed,
                                                         v_min, v_max, flags),
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool DragFloat3(const char* label, we::world::creation_entity* entity,
                       we::float3 Entity::*value_member_ptr,
                       we::edits::stack<we::world::edit_context>* edit_stack,
                       we::world::edit_context* context)
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](we::float3* value) {
      return we::edit_widget_result{.value_changed = ImGui::DragFloat3(label, value),
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool DragFloat2XZ(const char* label, we::world::creation_entity* entity,
                         we::float2 Entity::*value_member_ptr,
                         we::edits::stack<we::world::edit_context>* edit_stack,
                         we::world::edit_context* context, float v_speed = 1.0f,
                         float v_min = 0.0f, float v_max = 0.0f,
                         ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](we::float2* value) {
      return we::edit_widget_result{.value_changed =
                                       ImGui::DragFloat2XZ(label, value, v_speed,
                                                           v_min, v_max, flags),
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool DragQuat(const char* label, we::world::creation_entity* entity,
                     we::quaternion Entity::*value_member_ptr,
                     we::edits::stack<we::world::edit_context>* edit_stack,
                     we::world::edit_context* context)
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](we::quaternion* rotation) {
      return we::edit_widget_result{.value_changed = ImGui::DragQuat(label, rotation),
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool SliderInt(const char* label, we::world::creation_entity* entity,
                      int Entity::*value_member_ptr,
                      we::edits::stack<we::world::edit_context>* edit_stack,
                      we::world::edit_context* context, int v_min = 0, int v_max = 0,
                      const char* format = "%d", ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](int* value) {
      bool value_changed = ImGui::SliderInt(label, value, v_min, v_max, format, flags);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool SliderInt(const char* label, we::world::creation_entity* entity,
                      we::int8 Entity::*value_member_ptr,
                      we::edits::stack<we::world::edit_context>* edit_stack,
                      we::world::edit_context* context, int v_min = 0, int v_max = 0,
                      const char* format = "%d", ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](we::int8* value) {
      int int_value = *value;

      bool value_changed =
         ImGui::SliderInt(label, &int_value, v_min, v_max, format, flags);

      if (value_changed) *value = static_cast<we::int8>(int_value);

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool ColorEdit3(const char* label, we::world::creation_entity* entity,
                       we::float3 Entity::*value_member_ptr,
                       we::edits::stack<we::world::edit_context>* edit_stack,
                       we::world::edit_context* context,
                       ImGuiColorEditFlags flags = 0) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](we::float3* value) {
      return we::edit_widget_result{.value_changed =
                                       ImGui::ColorEdit3(label, &value->x, flags),
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity>
inline bool Checkbox(const char* label, we::world::creation_entity* entity,
                     bool Entity::*value_member_ptr,
                     we::edits::stack<we::world::edit_context>* edit_stack,
                     we::world::edit_context* context)
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=](bool* value) {
      return we::edit_widget_result{.value_changed = ImGui::Checkbox(label, value),
                                    .item_deactivated = ImGui::IsItemDeactivated()};
   });
}

template<typename Entity, typename Enum>
inline bool EnumSelect(const char* label, we::world::creation_entity* entity,
                       Enum Entity::*value_member_ptr,
                       we::edits::stack<we::world::edit_context>* edit_stack,
                       we::world::edit_context* context,
                       std::initializer_list<we::enum_select_option<Enum>> values) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=, &values](Enum* value) {
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

template<typename Entity, typename Flags>
inline bool EditFlags(const char* label, we::world::creation_entity* entity,
                      Flags Entity::*value_member_ptr,
                      we::edits::stack<we::world::edit_context>* edit_stack,
                      we::world::edit_context* context,
                      std::initializer_list<we::edit_flag<Flags>> flags) noexcept
{
   return EditWithUndo(entity, value_member_ptr, edit_stack, context, [=, &flags](Flags* value) {
      unsigned int uint_value = std::to_underlying(*value);

      std::array<ImGui::ExtEditFlag, 32> uint_flags;

      for (std::size_t i = 0; i < (flags.size() > 32 ? 32 : flags.size()); ++i) {
         auto& flag = flags.begin()[i];

         uint_flags[i] = {flag.label, static_cast<unsigned int>(flag.bit)};
      }

      bool value_changed =
         ImGui::EditFlags(label, &uint_value, {uint_flags.data(), flags.size()});

      if (value_changed) {
         *value = static_cast<Flags>(uint_value);
      }

      return we::edit_widget_result{.value_changed = value_changed,
                                    .item_deactivated = value_changed};
   });
}

inline bool DragBarrierRotation(const char* label, we::world::creation_entity* entity,
                                we::edits::stack<we::world::edit_context>* edit_stack,
                                we::world::edit_context* context,
                                float v_speed = 1.0f, float v_min = 0.0f,
                                float v_max = 0.0f, const char* format = "%.3f",
                                ImGuiSliderFlags flags = 0) noexcept
{
   return EditWithUndo(entity, &we::world::barrier::rotation_angle, edit_stack,
                       context, [=](float* value) {
                          float degrees = *value * 180.0f / std::numbers::pi_v<float>;

                          const bool value_changed =
                             ImGui::DragFloat(label, &degrees, v_speed, v_min,
                                              v_max, format, flags);

                          if (value_changed) {
                             *value = degrees / 180.0f * std::numbers::pi_v<float>;
                          }

                          return we::edit_widget_result{.value_changed = value_changed,
                                                        .item_deactivated =
                                                           ImGui::IsItemDeactivated()};
                       });
}

// Creation Entity Path Node Editors

template<typename T>
inline bool EditWithUndoPathNode(we::world::creation_entity* entity,
                                 T we::world::path::node ::*value_member_ptr,
                                 we::edits::stack<we::world::edit_context>* edit_stack,
                                 we::world::edit_context* context,
                                 we::edits::imgui::edit_widget_callback<T> auto editor) noexcept
{
   using namespace we;
   using namespace we::edits;
   using namespace we::edits::imgui;

   using edit_type = ui_creation_path_node_edit<T>;
   using value_type = T;

   value_type value = std::get_if<we::world::path>(entity)->nodes[0].*value_member_ptr;
   value_type original_value = value;

   auto [valued_changed, item_deactivated] = editor(&value);

   if (valued_changed) {
      edit_stack->apply(std::make_unique<edit_type>(value_member_ptr, value, original_value),
                        *context);
   }

   if (item_deactivated) edit_stack->close_last();

   return valued_changed;
}

inline bool EditWithUndoPathNode(
   we::world::creation_entity* entity,
   we::quaternion we::world::path::node::*value_member_ptr,
   we::float3 we::world::edit_context::*meta_value_member_ptr,
   we::edits::stack<we::world::edit_context>* edit_stack,
   we::world::edit_context* context,
   we::edits::imgui::edit_widget_double_callback<we::quaternion, we::float3> auto editor) noexcept
{
   using namespace we;
   using namespace we::edits;
   using namespace we::edits::imgui;

   using edit_type = ui_creation_path_node_edit_with_meta<quaternion, float3>;
   using value_type = quaternion;
   using meta_value_type = float3;

   value_type value = std::get_if<we::world::path>(entity)->nodes[0].*value_member_ptr;
   value_type original_value = value;
   meta_value_type meta_value = context->*meta_value_member_ptr;
   meta_value_type meta_original_value = meta_value;

   auto [valued_changed, item_deactivated] = editor(&value, &meta_value);

   if (valued_changed) {
      edit_stack->apply(std::make_unique<edit_type>(value_member_ptr, value,
                                                    original_value, meta_value_member_ptr,
                                                    meta_value, meta_original_value),
                        *context);
   }

   if (item_deactivated) edit_stack->close_last();

   return valued_changed;
}

inline bool DragRotationEulerPathNode(const char* label,
                                      we::world::creation_entity* entity,
                                      we::float3 we::world::edit_context::*meta_value_member_ptr,
                                      we::edits::stack<we::world::edit_context>* edit_stack,
                                      we::world::edit_context* context)
{
   return EditWithUndoPathNode(
      entity, &we::world::path::node::rotation, meta_value_member_ptr, edit_stack,
      context, [=](we::quaternion* rotation, we::float3* rotation_euler) {
         bool value_changed = ImGui::DragFloat3(label, rotation_euler);

         if (value_changed) {
            *rotation = make_quat_from_euler(*rotation_euler *
                                             std::numbers::pi_v<float> / 180.0f);
         }

         return we::edit_widget_result{.value_changed = value_changed,
                                       .item_deactivated = ImGui::IsItemDeactivated()};
      });
}

inline bool DragQuatPathNode(const char* label, we::world::creation_entity* entity,
                             we::edits::stack<we::world::edit_context>* edit_stack,
                             we::world::edit_context* context)
{
   return EditWithUndoPathNode(entity, &we::world::path::node::rotation,
                               edit_stack, context, [=](we::quaternion* rotation) {
                                  return we::edit_widget_result{
                                     .value_changed = ImGui::DragQuat(label, rotation),
                                     .item_deactivated = ImGui::IsItemDeactivated()};
                               });
}

inline bool DragFloat3PathNode(const char* label, we::world::creation_entity* entity,
                               we::edits::stack<we::world::edit_context>* edit_stack,
                               we::world::edit_context* context)
{
   return EditWithUndoPathNode(entity, &we::world::path::node::position,
                               edit_stack, context, [=](we::float3* value) {
                                  return we::edit_widget_result{
                                     .value_changed = ImGui::DragFloat3(label, value),
                                     .item_deactivated = ImGui::IsItemDeactivated()};
                               });
}

// Sector Point Editors

inline bool DragSectorPoint(const char* label, we::world::creation_entity* entity,
                            we::edits::stack<we::world::edit_context>* edit_stack,
                            we::world::edit_context* context,
                            float v_speed = 1.0f, float v_min = 0.0f,
                            float v_max = 0.0f, ImGuiSliderFlags flags = 0) noexcept
{
   using namespace we;
   using namespace we::edits;
   using namespace we::edits::imgui;

   using edit_type = ui_creation_sector_point_edit;

   float2 value = std::get_if<we::world::sector>(entity)->points[0];
   float2 original_value = value;

   const bool value_changed =
      ImGui::DragFloat2XZ(label, &value, v_speed, v_min, v_max, flags);
   const bool item_deactivated = ImGui::IsItemDeactivated();

   if (value_changed) {
      edit_stack->apply(std::make_unique<edit_type>(value, original_value), *context);
   }

   if (item_deactivated) edit_stack->close_last();

   return value_changed;
}

}
