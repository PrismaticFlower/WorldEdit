#pragma once

#include "types.hpp"

#include "imgui.h"

#include <span>
#include <string>
#include <type_traits>

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

bool InputTextAutoComplete(const char* label, std::string* value,
                           const std::add_pointer_t<std::array<std::string, 6>(void*)> fill_entries_callback,
                           void* fill_entries_callback_user_data);

bool EditFlags(const char* label, unsigned int* value,
               std::span<const ExtEditFlag> flags);

}