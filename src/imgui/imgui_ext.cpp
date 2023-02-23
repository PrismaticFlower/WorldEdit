#include "imgui_ext.hpp"
#include "imgui_stdlib.h"

#include "math/quaternion_funcs.hpp"
#include "utility/string_ops.hpp"

#include <cmath>
#include <optional>

namespace ImGui {

namespace {

struct text_callback_autofill_data {
   std::optional<std::array<std::string, 6>>& autocomplete_entries;
   std::add_pointer_t<std::array<std::string, 6>(void*)> fill_entries_callback;
   void* fill_entries_callback_user_data;
};

struct item_widths {
   float one;
   float last;
};

auto get_item_widths(const float components) -> item_widths
{
   const float item_width_full = CalcItemWidth();
   const float item_inner_spacing = GetStyle().ItemInnerSpacing.x;

   const float item_width_one =
      std::fmax(1.0f, std::floor((item_width_full -
                                  (item_inner_spacing) * (components - 1.0f)) /
                                 components));
   const float item_width_last =
      std::fmax(1.0f, std::floor(item_width_full - (item_width_one + item_inner_spacing) *
                                                      (components - 1.0f)));

   return {item_width_one, item_width_last};
}

}

bool DragFloat2(const char* label, we::float2* v, float v_speed, float v_min,
                float v_max, ImGuiSliderFlags flags)
{
   bool value_changed = false;

   const item_widths item_widths = get_item_widths(2.0f);

   BeginGroup();
   PushID(label);
   PushItemWidth(item_widths.last);
   PushItemWidth(item_widths.one);

   value_changed |= DragFloat("##X", &v->x, v_speed, v_min, v_max, "X:%.3f", flags);
   SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
   PopItemWidth();
   value_changed |= DragFloat("##Y", &v->y, v_speed, v_min, v_max, "Y:%.3f", flags);
   PopItemWidth();

   PopID();

   auto label_text =
      we::utility::string::split_first_of_exclusive(std::string_view{label},
                                                    "##");

   if (not label_text.empty()) {
      SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
      TextUnformatted(&label_text[0].front(), &label_text[0].back() + 1);
   }

   EndGroup();

   return value_changed;
}

bool DragFloat3(const char* label, we::float3* v, float v_speed, float v_min,
                float v_max, ImGuiSliderFlags flags)
{
   bool value_changed = false;

   const item_widths item_widths = get_item_widths(3.0f);

   BeginGroup();
   PushID(label);
   PushItemWidth(item_widths.last);
   PushItemWidth(item_widths.one);
   PushItemWidth(item_widths.one);

   value_changed |= DragFloat("##X", &v->x, v_speed, v_min, v_max, "X:%.3f", flags);
   SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
   PopItemWidth();
   value_changed |= DragFloat("##Y", &v->y, v_speed, v_min, v_max, "Y:%.3f", flags);
   SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
   PopItemWidth();
   value_changed |= DragFloat("##Z", &v->z, v_speed, v_min, v_max, "Z:%.3f", flags);
   PopItemWidth();

   PopID();

   auto label_text =
      we::utility::string::split_first_of_exclusive(std::string_view{label},
                                                    "##");

   if (not label_text.empty()) {
      SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
      TextUnformatted(&label_text[0].front(), &label_text[0].back() + 1);
   }

   EndGroup();

   return value_changed;
}

bool DragFloat4(const char* label, we::float4* v, float v_speed, float v_min,
                float v_max, ImGuiSliderFlags flags)
{
   bool value_changed = false;

   const item_widths item_widths = get_item_widths(4.0f);

   BeginGroup();
   PushID(label);
   PushItemWidth(item_widths.last);
   PushItemWidth(item_widths.one);
   PushItemWidth(item_widths.one);
   PushItemWidth(item_widths.one);

   value_changed |= DragFloat("##X", &v->x, v_speed, v_min, v_max, "X:%.3f", flags);
   SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
   PopItemWidth();
   value_changed |= DragFloat("##Y", &v->y, v_speed, v_min, v_max, "Y:%.3f", flags);
   SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
   PopItemWidth();
   value_changed |= DragFloat("##Z", &v->z, v_speed, v_min, v_max, "Z:%.3f", flags);
   SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
   PopItemWidth();
   value_changed |= DragFloat("##W", &v->w, v_speed, v_min, v_max, "W:%.3f", flags);
   PopItemWidth();

   PopID();

   auto label_text =
      we::utility::string::split_first_of_exclusive(std::string_view{label},
                                                    "##");

   if (not label_text.empty()) {
      SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
      TextUnformatted(&label_text[0].front(), &label_text[0].back() + 1);
   }

   EndGroup();

   return value_changed;
}

bool DragQuat(const char* label, we::quaternion* v, float v_speed, float v_min,
              float v_max, ImGuiSliderFlags flags)
{
   bool value_changed = false;

   const item_widths item_widths = get_item_widths(4.0f);

   BeginGroup();
   PushID(label);
   PushItemWidth(item_widths.last);
   PushItemWidth(item_widths.one);
   PushItemWidth(item_widths.one);
   PushItemWidth(item_widths.one);

   value_changed |= DragFloat("##W", &v->w, v_speed, v_min, v_max, "W:%.3f", flags);
   SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
   PopItemWidth();
   value_changed |= DragFloat("##X", &v->x, v_speed, v_min, v_max, "X:%.3f", flags);
   SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
   PopItemWidth();
   value_changed |= DragFloat("##Y", &v->y, v_speed, v_min, v_max, "Y:%.3f", flags);
   SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
   PopItemWidth();
   value_changed |= DragFloat("##Z", &v->z, v_speed, v_min, v_max, "Z:%.3f", flags);
   PopItemWidth();

   PopID();

   auto label_text =
      we::utility::string::split_first_of_exclusive(std::string_view{label},
                                                    "##");

   if (not label_text.empty()) {
      SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
      TextUnformatted(&label_text[0].front(), &label_text[0].back() + 1);
   }

   EndGroup();

   if (value_changed) {
      if (v->w != 0.0f or v->x != 0.0f or v->y != 0.0f or v->z != 0.0f) {
         *v = normalize(*v);
      }
   }

   return value_changed;
}

bool InputTextAutoComplete(const char* label, std::string* value,
                           const std::add_pointer_t<std::array<std::string, 6>(void*)> fill_entries_callback,
                           void* fill_entries_callback_user_data)
{
   std::optional<std::array<std::string, 6>> autocomplete_entries;

   text_callback_autofill_data callback_user_data{autocomplete_entries,
                                                  fill_entries_callback,
                                                  fill_entries_callback_user_data};

   ImGui::BeginGroup();

   bool value_changed = ImGui::InputText(
      label, value,
      ImGuiInputTextFlags_NoUndoRedo | ImGuiInputTextFlags_CallbackCompletion,
      [](ImGuiInputTextCallbackData* data) {
         if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion) {
            text_callback_autofill_data& user_data =
               *static_cast<decltype(callback_user_data)*>(data->UserData);

            user_data.autocomplete_entries = user_data.fill_entries_callback(
               user_data.fill_entries_callback_user_data);

            std::string& autofill = (*user_data.autocomplete_entries)[0];

            if (not autofill.empty()) {
               data->DeleteChars(0, data->BufTextLen);
               data->InsertChars(0, autofill.c_str(),
                                 autofill.c_str() + autofill.size());
            }
         }

         return 0;
      },
      &callback_user_data);

   if (ImGui::IsItemActive()) {
      ImGui::SetNextWindowPos(
         ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y));
      ImGui::SetNextWindowSize(
         ImVec2{ImGui::GetItemRectMax().x - ImGui::GetItemRectMin().x -
                   ImGui::CalcTextSize(label, nullptr, true).x -
                   ImGui::GetStyle().ItemInnerSpacing.x,
                9.0f * ImGui::GetFontSize()});

      ImGui::BeginTooltip();

      if (not autocomplete_entries) {
         autocomplete_entries = fill_entries_callback(fill_entries_callback_user_data);
      }

      if ((*autocomplete_entries)[0].empty()) {
         ImGui::TextUnformatted("No matches.");
      }
      else {
         for (const std::string& asset : *autocomplete_entries) {
            ImGui::TextUnformatted(asset.c_str(), asset.c_str() + asset.size());
         }
      }

      ImGui::EndTooltip();
   }

   ImGui::EndGroup();

   return value_changed;
}

}