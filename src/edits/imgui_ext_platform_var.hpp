#pragma once

#include "imgui_ext.hpp"

#include "utility/function_ptr.hpp"

namespace ImGui {

using namespace we;

bool EditPrecipitationType(const char* label,
                           we::world::platform_var<we::world::precipitation_type>* value,
                           we::edits::stack<we::world::edit_context>& edit_stack,
                           we::world::edit_context& context) noexcept;

bool Checkbox(const char* label, we::world::platform_var<bool>* value,
              we::edits::stack<we::world::edit_context>& edit_stack,
              we::world::edit_context& context) noexcept;

bool DragFloat(const char* label, we::world::platform_var<float>* value,
               we::edits::stack<we::world::edit_context>& edit_stack,
               we::world::edit_context& context, float v_speed = 1.0f,
               float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f",
               ImGuiSliderFlags flags = ImGuiSliderFlags_None) noexcept;

bool DragFloat(const char* label, we::world::platform_pc_xb_var<float>* value,
               we::edits::stack<we::world::edit_context>& edit_stack,
               we::world::edit_context& context, float v_speed = 1.0f,
               float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f",
               ImGuiSliderFlags flags = ImGuiSliderFlags_None) noexcept;

bool DragFloat2(const char* label, we::world::platform_var<we::float2>* value,
                we::edits::stack<we::world::edit_context>& edit_stack,
                we::world::edit_context& context, float v_speed = 1.0f,
                float v_min = 0.0f, float v_max = 0.0f) noexcept;

bool DragFloat3(const char* label, we::world::platform_var<we::float3>* value,
                we::edits::stack<we::world::edit_context>& edit_stack,
                we::world::edit_context& context, float v_speed = 1.0f,
                float v_min = 0.0f, float v_max = 0.0f) noexcept;

bool DragFloatRange2(const char* label, we::world::platform_var<we::float2>* value,
                     we::edits::stack<we::world::edit_context>& edit_stack,
                     we::world::edit_context& context, float v_speed = 1.0f,
                     float v_min = 0.0f, float v_max = 0.0f,
                     const char* format_min = "Min: %.3f",
                     const char* format_max = "Max: %.3f",
                     ImGuiSliderFlags flags = ImGuiSliderFlags_None) noexcept;

bool DragFloatRange2(const char* label, we::world::platform_pc_xb_var<we::float2>* value,
                     we::edits::stack<we::world::edit_context>& edit_stack,
                     we::world::edit_context& context, float v_speed = 1.0f,
                     float v_min = 0.0f, float v_max = 0.0f,
                     const char* format_min = "Min: %.3f",
                     const char* format_max = "Max: %.3f",
                     ImGuiSliderFlags flags = ImGuiSliderFlags_None) noexcept;

bool DragInt(const char* label, we::world::platform_var<we::int32>* value,
             we::edits::stack<we::world::edit_context>& edit_stack,
             we::world::edit_context& context, float v_speed = 1.0f,
             we::int32 v_min = 0, we::int32 v_max = 0) noexcept;

bool DragInt2(const char* label,
              we::world::platform_var<std::array<we::int32, 2>>* value,
              we::edits::stack<we::world::edit_context>& edit_stack,
              we::world::edit_context& context, float v_speed = 1.0f,
              we::int32 v_min = 0, we::int32 v_max = 0, const char* format = "%d",
              ImGuiSliderFlags flags = ImGuiSliderFlags_None) noexcept;

bool SliderInt(const char* label, we::world::platform_pc_xb_var<we::int32>* value,
               we::edits::stack<we::world::edit_context>& edit_stack,
               we::world::edit_context& context, we::int32 v_min = 0,
               we::int32 v_max = 0) noexcept;

bool SliderInt(const char* label, we::world::platform_var<we::int32>* value,
               we::edits::stack<we::world::edit_context>& edit_stack,
               we::world::edit_context& context, we::int32 v_min = 0,
               we::int32 v_max = 0, const char* format = "%d",
               ImGuiSliderFlags flags = ImGuiSliderFlags_None) noexcept;

bool ColorEdit3(const char* label, we::world::platform_var<we::float3>* value,
                we::edits::stack<we::world::edit_context>& edit_stack,
                we::world::edit_context& context) noexcept;

bool ColorEdit4(const char* label, we::world::platform_var<we::float4>* value,
                we::edits::stack<we::world::edit_context>& edit_stack,
                we::world::edit_context& context) noexcept;

bool ColorEdit4(const char* label, we::world::platform_pc_xb_var<we::float4>* value,
                we::edits::stack<we::world::edit_context>& edit_stack,
                we::world::edit_context& context) noexcept;

bool InputText(const char* label, we::world::platform_var<std::string>* value,
               we::edits::stack<we::world::edit_context>& edit_stack,
               we::world::edit_context& context) noexcept;

bool CustomWidget(const char* label, we::world::platform_var<std::string>* value,
                  we::edits::stack<we::world::edit_context>& edit_stack,
                  we::world::edit_context& context,
                  we::function_ptr<bool(const char*, std::string*) noexcept> widget) noexcept;

bool EditBumpMap(
   we::world::platform_pc_xb_var<we::world::heat_shimmer::bump_map_t>* value,
   we::edits::stack<we::world::edit_context>& edit_stack,
   we::world::edit_context& context,
   we::function_ptr<bool(const char*, std::string*) noexcept> texture_picker) noexcept;

bool EditHaloRing(const char* label,
                  we::world::platform_var<we::world::sun_flare::halo_ring>* value,
                  we::edits::stack<we::world::edit_context>& edit_stack,
                  we::world::edit_context& context) noexcept;

bool EditAnimatedTextures(const char* label, we::world::water::animated_textures* value,
                          we::edits::stack<we::world::edit_context>& edit_stack,
                          we::world::edit_context& context) noexcept;

bool EditAnimatedTextures(const char* label,
                          we::world::platform_pc_xb_var<we::world::water::animated_textures>* value,
                          we::edits::stack<we::world::edit_context>& edit_stack,
                          we::world::edit_context& context) noexcept;
}
