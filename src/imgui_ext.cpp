#include "imgui_ext.hpp"

#include "math/quaternion_funcs.hpp"
#include "utility/string_ops.hpp"

#include <cmath>
#include <optional>

// From imgui_internal.h, forward declared to avoid accidentally taking a dependency on other internals.
// This is a special exception, as without the auto complete window is... hard.
struct ImGuiWindow;

namespace ImGui {

// From imgui_internal.h, see forward declaration of ImGuiWindow.
ImGuiWindow* GetCurrentWindow();
IMGUI_API void BringWindowToDisplayFront(ImGuiWindow* window);

namespace {

struct text_callback_autofill_data {
   std::optional<std::array<std::string_view, 6>>& autocomplete_entries;
   int autocomplete_index = 0;
   std::add_pointer_t<std::array<std::string_view, 6>(void*)> fill_entries_callback;
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
      we::string::split_first_of_exclusive(std::string_view{label}, "##");

   if (not label_text[0].empty()) {
      SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
      TextUnformatted(&label_text[0].front(), &label_text[0].back() + 1);
   }

   EndGroup();

   return value_changed;
}

bool DragFloat2XZ(const char* label, we::float2* v, float v_speed, float v_min,
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
   value_changed |= DragFloat("##Z", &v->y, v_speed, v_min, v_max, "Z:%.3f", flags);
   PopItemWidth();

   PopID();

   auto label_text =
      we::string::split_first_of_exclusive(std::string_view{label}, "##");

   if (not label_text[0].empty()) {
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
      we::string::split_first_of_exclusive(std::string_view{label}, "##");

   if (not label_text[0].empty()) {
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
      we::string::split_first_of_exclusive(std::string_view{label}, "##");

   if (not label_text[0].empty()) {
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
      we::string::split_first_of_exclusive(std::string_view{label}, "##");

   if (not label_text[0].empty()) {
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

bool EditFlags(const char* label, unsigned int* value, std::span<const ExtEditFlag> flags)
{
   bool value_changed = false;

   const float item_width = CalcItemWidth();

   ImGui::BeginGroup();

   ImGui::SeparatorText(label);

   for (const auto& flag : flags) {
      if (ImGui::Selectable(flag.label, *value & flag.bit, 0, {item_width, 0.0f})) {
         *value ^= flag.bit;

         value_changed |= true;
      }
   }

   ImGui::EndGroup();

   return value_changed;
}

bool InputText(const char* label, absl::InlinedVector<char, 256>* buffer,
               ImGuiInputTextFlags flags, ImGuiInputTextCallback callback,
               void* user_data)
{
   IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);

   flags |= ImGuiInputTextFlags_CallbackResize;

   struct callback_user_data {
      absl::InlinedVector<char, 256>* buffer;
      ImGuiInputTextCallback chain_callback;
      void* chain_callback_user_data;
   };

   callback_user_data cb_user_data;
   cb_user_data.buffer = buffer;
   cb_user_data.chain_callback = callback;
   cb_user_data.chain_callback_user_data = user_data;

   const auto resize_callback = [](ImGuiInputTextCallbackData* data) -> int {
      callback_user_data* user_data = (callback_user_data*)data->UserData;
      if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
         absl::InlinedVector<char, 256>* buffer = user_data->buffer;

         buffer->resize(data->BufTextLen + 1); // + 1 for null terminator
         data->Buf = buffer->data();
      }
      else if (user_data->chain_callback) {
         data->UserData = user_data->chain_callback_user_data;
         return user_data->chain_callback(data);
      }
      return 0;
   };

   buffer->push_back('\0');

   const bool edited = InputText(label, buffer->data(), buffer->capacity() + 1,
                                 flags, resize_callback, &cb_user_data);

   buffer->pop_back();

   return edited;
}

bool InputTextAutoComplete(
   const char* label, absl::InlinedVector<char, 256>* buffer,
   const std::add_pointer_t<std::array<std::string_view, 6>(void*)> fill_entries_callback,
   void* fill_entries_callback_user_data)
{

   ImGui::PushID(label);

   ImGui::BeginGroup();

   const ImGuiID storage_id = ImGui::GetID("auto_complete_index");

   int selected_index = ImGui::GetStateStorage()->GetInt(storage_id, 0);

   std::optional<std::array<std::string_view, 6>> autocomplete_entries;

   text_callback_autofill_data callback_user_data{autocomplete_entries, selected_index,
                                                  fill_entries_callback,
                                                  fill_entries_callback_user_data};

   bool value_changed = ImGui::InputText(
      label, buffer, ImGuiInputTextFlags_CallbackCompletion,
      [](ImGuiInputTextCallbackData* data) {
         if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion) {
            text_callback_autofill_data& user_data =
               *static_cast<decltype(callback_user_data)*>(data->UserData);

            user_data.autocomplete_entries = user_data.fill_entries_callback(
               user_data.fill_entries_callback_user_data);

            std::string_view autofill =
               (*user_data.autocomplete_entries)[user_data.autocomplete_index];

            if (not autofill.empty()) {
               data->DeleteChars(0, data->BufTextLen);
               data->InsertChars(0, autofill.data(),
                                 autofill.data() + autofill.size());
            }
         }

         return 0;
      },
      &callback_user_data);

   if (ImGui::IsItemActive()) {
      if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) and selected_index > 0) {
         selected_index -= 1;
      }

      if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) and selected_index < 6) {
         selected_index += 1;
      }

      if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
         if (not autocomplete_entries) {
            autocomplete_entries =
               fill_entries_callback(fill_entries_callback_user_data);
         }

         buffer->assign((*autocomplete_entries)[selected_index].begin(),
                        (*autocomplete_entries)[selected_index].end());

         value_changed = true;
      }
   }

   const bool input_text_deactivated = ImGui::IsItemDeactivated();

   if (ImGui::IsItemActivated()) ImGui::OpenPopup("##autocomplete-entries");

   {
      ImGui::SetNextWindowPos(
         ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y));
      ImGui::SetNextWindowSize(
         ImVec2{ImGui::GetItemRectMax().x - ImGui::GetItemRectMin().x -
                   ImGui::CalcTextSize(label, nullptr, true).x -
                   ImGui::GetStyle().ItemInnerSpacing.x,
                9.0f * ImGui::GetFontSize()});

      if (ImGui::BeginPopup("##autocomplete-entries",
                            ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration |
                               ImGuiWindowFlags_NoFocusOnAppearing |
                               ImGuiWindowFlags_NoSavedSettings)) {
         ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

         if (not autocomplete_entries) {
            autocomplete_entries =
               fill_entries_callback(fill_entries_callback_user_data);
         }

         if ((*autocomplete_entries)[0].empty()) {
            ImGui::TextUnformatted("No matches.");
         }
         else {
            int highest_entry = 0;

            for (int i = 0; i < autocomplete_entries->size(); ++i) {
               const std::string_view asset = (*autocomplete_entries)[i];

               if (asset.empty()) break;

               ImGui::PushID(i);

               if (ImGui::Selectable("##selectable", i == selected_index)) {
                  buffer->assign(asset.begin(), asset.end());

                  value_changed = true;
               }
               ImGui::SameLine();
               ImGui::TextUnformatted(asset.data(), asset.data() + asset.size());

               ImGui::PopID();

               highest_entry = i;
            }

            if (selected_index > highest_entry) selected_index = 0;
         }

         if (input_text_deactivated and not ImGui::IsWindowFocused()) {
            ImGui::CloseCurrentPopup();
         }

         ImGui::EndPopup();
      }
   }

   if (input_text_deactivated) selected_index = 0;

   ImGui::GetStateStorage()->SetInt(storage_id, selected_index);

   ImGui::EndGroup();

   ImGui::PopID();

   return value_changed;
}

}