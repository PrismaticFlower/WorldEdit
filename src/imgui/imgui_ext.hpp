#pragma once

#include "types.hpp"

#include "imgui.h"

#include <span>
#include <string>
#include <type_traits>

#include <absl/container/inlined_vector.h>

namespace ImGui {

struct ExtEditFlag {
   const char* label = "";
   unsigned int bit = 0;
};

bool DragFloat2(const char* label, we::float2* v, float v_speed = 1.0f,
                float v_min = 0.0f, float v_max = 0.0f, ImGuiSliderFlags flags = 0);

bool DragFloat2XZ(const char* label, we::float2* v, float v_speed = 1.0f,
                  float v_min = 0.0f, float v_max = 0.0f, ImGuiSliderFlags flags = 0);

bool DragFloat3(const char* label, we::float3* v, float v_speed = 1.0f,
                float v_min = 0.0f, float v_max = 0.0f, ImGuiSliderFlags flags = 0);

bool DragFloat4(const char* label, we::float4* v, float v_speed = 1.0f,
                float v_min = 0.0f, float v_max = 0.0f, ImGuiSliderFlags flags = 0);

bool DragQuat(const char* label, we::quaternion* v, float v_speed = 0.001f,
              float v_min = 0.0f, float v_max = 0.0f, ImGuiSliderFlags flags = 0);

bool EditFlags(const char* label, unsigned int* value,
               std::span<const ExtEditFlag> flags);

bool InputText(const char* label, absl::InlinedVector<char, 256>* buffer,
               ImGuiInputTextFlags flags = 0,
               ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

bool InputTextAutoComplete(
   const char* label, absl::InlinedVector<char, 256>* buffer,
   const std::add_pointer_t<std::array<std::string_view, 6>(void*)> fill_entries_callback,
   void* fill_entries_callback_user_data);

}