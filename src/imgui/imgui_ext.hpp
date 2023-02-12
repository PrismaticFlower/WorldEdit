#pragma once

#include "types.hpp"

#include "imgui.h"

namespace ImGui {

bool DragFloat2(const char* label, we::float2* v, float v_speed = 1.0f,
                float v_min = 0.0f, float v_max = 0.0f, ImGuiSliderFlags flags = 0);

bool DragFloat3(const char* label, we::float3* v, float v_speed = 1.0f,
                float v_min = 0.0f, float v_max = 0.0f, ImGuiSliderFlags flags = 0);

bool DragFloat4(const char* label, we::float4* v, float v_speed = 1.0f,
                float v_min = 0.0f, float v_max = 0.0f, ImGuiSliderFlags flags = 0);

bool DragQuat(const char* label, we::quaternion* v, float v_speed = 0.001f,
              float v_min = 0.0f, float v_max = 0.0f, ImGuiSliderFlags flags = 0);

}