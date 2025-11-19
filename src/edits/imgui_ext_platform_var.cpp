#include "imgui_ext_platform_var.hpp"

#include "edits/set_value.hpp"

#include <concepts>

namespace ImGui {

using namespace we;

namespace {

bool EditPrecipitationType(const char* label, world::precipitation_type* value,
                           edits::stack<world::edit_context>& edit_stack,
                           world::edit_context& context) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   bool edited = false;

   if (BeginTable(label, 2, ImGuiTableFlags_SizingStretchSame, {CalcItemWidth(), 0.0f})) {
      TableNextColumn();

      if (RadioButton("Streaks", *value == world::precipitation_type::streaks)) {
         edit_stack.apply(edits::make_set_value(value, world::precipitation_type::streaks),
                          context);

         edited = true;
      }

      TableNextColumn();

      if (RadioButton("Quads", *value == world::precipitation_type::quads)) {
         edit_stack.apply(edits::make_set_value(value, world::precipitation_type::quads),
                          context);

         edited = true;
      }

      EndTable();
   }

   SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

   Text(label);

   return edited;
}

template<typename T>
void ShowSplitPopup(const char* label, world::platform_var<T>* value,
                    edits::stack<world::edit_context>& edit_stack,
                    world::edit_context& context)
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   if (BeginPopupContextItem(label)) {
      if (MenuItem("Split for Each Platform")) {
         edit_stack.apply(edits::make_set_value(value, {.pc = value->pc,
                                                        .ps2 = value->pc,
                                                        .xbox = value->pc,
                                                        .per_platform = true}),
                          context, {.closed = true});
      }

      EndPopup();
   }
}

template<typename T>
void ShowSplitPopup(const char* label, world::platform_pc_xb_var<T>* value,
                    edits::stack<world::edit_context>& edit_stack,
                    world::edit_context& context)
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   if (BeginPopupContextItem(label)) {
      if (MenuItem("Split for Each Platform")) {
         edit_stack.apply(edits::make_set_value(value, {.pc = value->pc,
                                                        .xbox = value->pc,
                                                        .per_platform = true}),
                          context, {.closed = true});
      }

      EndPopup();
   }
}

template<typename T>
void ShowCombinePopup(const char* label, world::platform_var<T>* value,
                      const T& combined_value,
                      edits::stack<world::edit_context>& edit_stack,
                      world::edit_context& context)
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   if (BeginPopupContextItem(label)) {
      if (MenuItem("Combine for Each Platform")) {
         edit_stack.apply(edits::make_set_value(value, {.pc = combined_value,
                                                        .ps2 = combined_value,
                                                        .xbox = combined_value,
                                                        .per_platform = false}),
                          context, {.closed = true});
      }

      EndPopup();
   }
}

template<typename T>
void ShowCombinePopup(const char* label, world::platform_pc_xb_var<T>* value,
                      const T& combined_value,
                      edits::stack<world::edit_context>& edit_stack,
                      world::edit_context& context)
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   if (BeginPopupContextItem(label)) {
      if (MenuItem("Combine for Each Platform")) {
         edit_stack.apply(edits::make_set_value(value, {.pc = combined_value,
                                                        .xbox = combined_value,
                                                        .per_platform = false}),
                          context, {.closed = true});
      }

      EndPopup();
   }
}

template<typename T, typename... Args>
concept widget_function = std::predicate<T, Args...>;

template<typename T, typename... Args>
bool Widget(const char* label, world::platform_var<T>* value,
            edits::stack<world::edit_context>& edit_stack, world::edit_context& context,
            const widget_function<const char*, T*, edits::stack<world::edit_context>&,
                                  world::edit_context&, Args...> auto& widget_fn,
            Args&&... args) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   bool edited = false;

   if (value->per_platform) [[unlikely]] {
      PushID(label);
      BeginGroup();

      edited |= widget_fn("##PC", &value->pc, edit_stack, context,
                          std::forward<Args>(args)...);

      ShowCombinePopup("##PC", value, value->pc, edit_stack, context);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s (PC)", label);

      edited |= widget_fn("##PS2", &value->ps2, edit_stack, context,
                          std::forward<Args>(args)...);

      ShowCombinePopup("##PS2", value, value->ps2, edit_stack, context);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s (PS2)", label);

      edited |= widget_fn("##Xbox", &value->xbox, edit_stack, context,
                          std::forward<Args>(args)...);

      ShowCombinePopup("##Xbox", value, value->xbox, edit_stack, context);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s (Xbox)", label);

      EndGroup();
      PopID();
   }
   else [[likely]] {
      edited |= widget_fn(label, &value->pc, edit_stack, context,
                          std::forward<Args>(args)...);

      ShowSplitPopup(label, value, edit_stack, context);
   }

   return edited;
}

template<typename T, typename... Args>
bool Widget(const char* label, world::platform_pc_xb_var<T>* value,
            edits::stack<world::edit_context>& edit_stack, world::edit_context& context,
            const widget_function<const char*, T*, edits::stack<world::edit_context>&,
                                  world::edit_context&, Args...> auto& widget_fn,
            Args&&... args) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   bool edited = false;

   if (value->per_platform) [[unlikely]] {
      PushID(label);
      BeginGroup();

      edited |= widget_fn("##PC", &value->pc, edit_stack, context,
                          std::forward<Args>(args)...);

      ShowCombinePopup("##PC", value, value->pc, edit_stack, context);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s (PC)", label);

      edited |= widget_fn("##Xbox", &value->xbox, edit_stack, context,
                          std::forward<Args>(args)...);

      ShowCombinePopup("##Xbox", value, value->xbox, edit_stack, context);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s (Xbox)", label);

      EndGroup();
      PopID();
   }
   else [[likely]] {
      edited |= widget_fn(label, &value->pc, edit_stack, context,
                          std::forward<Args>(args)...);

      ShowSplitPopup(label, value, edit_stack, context);
   }

   return edited;
}

}

bool EditPrecipitationType(const char* label,
                           world::platform_var<world::precipitation_type>* value,
                           edits::stack<world::edit_context>& edit_stack,
                           world::edit_context& context) noexcept
{
   return Widget(label, value, edit_stack, context,
                 [](const char* label, world::precipitation_type* value,
                    edits::stack<world::edit_context>& edit_stack,
                    world::edit_context& context) {
                    return EditPrecipitationType(label, value, edit_stack, context);
                 });
}

bool Checkbox(const char* label, world::platform_var<bool>* value,
              edits::stack<world::edit_context>& edit_stack,
              world::edit_context& context) noexcept
{
   return Widget(label, value, edit_stack, context,
                 [](const char* label, bool* value,
                    edits::stack<world::edit_context>& edit_stack,
                    world::edit_context& context) {
                    return Checkbox(label, value, edit_stack, context);
                 });
}

bool DragFloat(const char* label, world::platform_var<float>* value,
               edits::stack<world::edit_context>& edit_stack,
               world::edit_context& context, float v_speed, float v_min,
               float v_max, const char* format, ImGuiSliderFlags flags) noexcept
{
   return Widget(
      label, value, edit_stack, context,
      [](const char* label, float* value, edits::stack<world::edit_context>& edit_stack,
         world::edit_context& context, float v_speed, float v_min, float v_max,
         const char* format, ImGuiSliderFlags flags) {
         return DragFloat(label, value, edit_stack, context, v_speed, v_min,
                          v_max, format, flags);
      },
      v_speed, v_min, v_max, format, flags);
}

bool DragFloat(const char* label, world::platform_pc_xb_var<float>* value,
               edits::stack<world::edit_context>& edit_stack,
               world::edit_context& context, float v_speed, float v_min,
               float v_max, const char* format, ImGuiSliderFlags flags) noexcept
{
   return Widget(
      label, value, edit_stack, context,
      [](const char* label, float* value, edits::stack<world::edit_context>& edit_stack,
         world::edit_context& context, float v_speed, float v_min, float v_max,
         const char* format, ImGuiSliderFlags flags) {
         return DragFloat(label, value, edit_stack, context, v_speed, v_min,
                          v_max, format, flags);
      },
      v_speed, v_min, v_max, format, flags);
}

bool DragFloat2(const char* label, world::platform_var<float2>* value,
                edits::stack<world::edit_context>& edit_stack,
                world::edit_context& context, float v_speed, float v_min,
                float v_max) noexcept
{
   return Widget(
      label, value, edit_stack, context,
      [](const char* label, float2* value, edits::stack<world::edit_context>& edit_stack,
         world::edit_context& context, float v_speed, float v_min, float v_max) {
         return DragFloat2(label, value, edit_stack, context, v_speed, v_min, v_max);
      },
      v_speed, v_min, v_max);
}

bool DragFloat3(const char* label, world::platform_var<float3>* value,
                edits::stack<world::edit_context>& edit_stack,
                world::edit_context& context, float v_speed, float v_min,
                float v_max) noexcept
{
   return Widget(
      label, value, edit_stack, context,
      [](const char* label, float3* value, edits::stack<world::edit_context>& edit_stack,
         world::edit_context& context, float v_speed, float v_min, float v_max) {
         return DragFloat3(label, value, edit_stack, context, v_speed, v_min, v_max);
      },
      v_speed, v_min, v_max);
}

bool DragFloatRange2(const char* label, we::world::platform_var<we::float2>* value,
                     we::edits::stack<we::world::edit_context>& edit_stack,
                     we::world::edit_context& context, float v_speed,
                     float v_min, float v_max, const char* format_min,
                     const char* format_max, ImGuiSliderFlags flags) noexcept
{
   return Widget(
      label, value, edit_stack, context,
      [](const char* label, float2* value, edits::stack<world::edit_context>& edit_stack,
         world::edit_context& context, float v_speed, float v_min, float v_max,
         const char* format_min, const char* format_max, ImGuiSliderFlags flags) {
         return DragFloatRange2(label, &value->x, &value->y, edit_stack, context,
                                v_speed, v_min, v_max, format_min, format_max, flags);
      },
      v_speed, v_min, v_max, format_min, format_max, flags);
}

bool DragFloatRange2(const char* label, we::world::platform_pc_xb_var<we::float2>* value,
                     we::edits::stack<we::world::edit_context>& edit_stack,
                     we::world::edit_context& context, float v_speed,
                     float v_min, float v_max, const char* format_min,
                     const char* format_max, ImGuiSliderFlags flags) noexcept
{
   return Widget(
      label, value, edit_stack, context,
      [](const char* label, float2* value, edits::stack<world::edit_context>& edit_stack,
         world::edit_context& context, float v_speed, float v_min, float v_max,
         const char* format_min, const char* format_max, ImGuiSliderFlags flags) {
         return DragFloatRange2(label, &value->x, &value->y, edit_stack, context,
                                v_speed, v_min, v_max, format_min, format_max, flags);
      },
      v_speed, v_min, v_max, format_min, format_max, flags);
}

bool DragInt(const char* label, world::platform_var<int32>* value,
             edits::stack<world::edit_context>& edit_stack,
             world::edit_context& context, float v_speed, int32 v_min, int32 v_max) noexcept
{
   return Widget(
      label, value, edit_stack, context,
      [](const char* label, int32* value, edits::stack<world::edit_context>& edit_stack,
         world::edit_context& context, float v_speed, int32 v_min, int32 v_max) {
         return DragInt(label, value, edit_stack, context, v_speed, v_min, v_max);
      },
      v_speed, v_min, v_max);
}

bool DragInt2(const char* label, world::platform_var<std::array<int32, 2>>* value,
              edits::stack<world::edit_context>& edit_stack,
              world::edit_context& context, float v_speed, int32 v_min,
              int32 v_max, const char* format, ImGuiSliderFlags flags) noexcept
{
   return Widget(
      label, value, edit_stack, context,
      [](const char* label, std::array<int32, 2>* value,
         edits::stack<world::edit_context>& edit_stack,
         world::edit_context& context, float v_speed, int32 v_min, int32 v_max,
         const char* format, ImGuiSliderFlags flags) {
         return DragInt2(label, value, edit_stack, context, v_speed, v_min,
                         v_max, format, flags);
      },
      v_speed, v_min, v_max, format, flags);
}

bool SliderInt(const char* label, world::platform_pc_xb_var<int32>* value,
               edits::stack<world::edit_context>& edit_stack,
               world::edit_context& context, int32 v_min, int32 v_max) noexcept
{
   return Widget(
      label, value, edit_stack, context,
      [](const char* label, int32* value, edits::stack<world::edit_context>& edit_stack,
         world::edit_context& context, int32 v_min, int32 v_max) {
         return SliderInt(label, value, edit_stack, context, v_min, v_max);
      },
      v_min, v_max);
}

bool SliderInt(const char* label, world::platform_var<int32>* value,
               edits::stack<world::edit_context>& edit_stack,
               world::edit_context& context, int32 v_min, int32 v_max,
               const char* format, ImGuiSliderFlags flags) noexcept
{
   return Widget(
      label, value, edit_stack, context,
      [](const char* label, int32* value,
         edits::stack<world::edit_context>& edit_stack, world::edit_context& context,
         int32 v_min, int32 v_max, const char* format, ImGuiSliderFlags flags) {
         return SliderInt(label, value, edit_stack, context, v_min, v_max,
                          format, flags);
      },
      v_min, v_max, format, flags);
}

bool ColorEdit3(const char* label, world::platform_var<float3>* value,
                edits::stack<world::edit_context>& edit_stack,
                world::edit_context& context) noexcept
{
   return Widget(label, value, edit_stack, context,
                 [](const char* label, float3* value,
                    edits::stack<world::edit_context>& edit_stack,
                    world::edit_context& context) {
                    return ColorEdit3(label, value, edit_stack, context);
                 });
}

bool ColorEdit4(const char* label, world::platform_var<float4>* value,
                edits::stack<world::edit_context>& edit_stack,
                world::edit_context& context) noexcept
{
   return Widget(label, value, edit_stack, context,
                 [](const char* label, float4* value,
                    edits::stack<world::edit_context>& edit_stack,
                    world::edit_context& context) {
                    return ColorEdit4(label, value, edit_stack, context);
                 });
}

bool ColorEdit4(const char* label, world::platform_pc_xb_var<float4>* value,
                edits::stack<world::edit_context>& edit_stack,
                world::edit_context& context) noexcept
{
   return Widget(label, value, edit_stack, context,
                 [](const char* label, float4* value,
                    edits::stack<world::edit_context>& edit_stack,
                    world::edit_context& context) {
                    return ColorEdit4(label, value, edit_stack, context);
                 });
}

bool InputText(const char* label, world::platform_var<std::string>* value,
               edits::stack<world::edit_context>& edit_stack,
               world::edit_context& context) noexcept
{
   return Widget(label, value, edit_stack, context,
                 [](const char* label, std::string* value,
                    edits::stack<world::edit_context>& edit_stack,
                    world::edit_context& context) {
                    return InputText(label, value, edit_stack, context);
                 });
}

bool CustomWidget(const char* label, world::platform_var<std::string>* value,
                  edits::stack<world::edit_context>& edit_stack,
                  world::edit_context& context,
                  function_ptr<bool(const char*, std::string*) noexcept> widget) noexcept
{
   return Widget(
      label, value, edit_stack, context,
      [](const char* label, std::string* value,
         [[maybe_unused]] edits::stack<world::edit_context>& edit_stack,
         [[maybe_unused]] world::edit_context& context,
         function_ptr<bool(const char*, std::string*) noexcept> widget) {
         return widget(label, value);
      },
      widget);
}

bool EditBumpMap(world::platform_pc_xb_var<world::heat_shimmer::bump_map_t>* value,
                 edits::stack<world::edit_context>& edit_stack,
                 world::edit_context& context,
                 function_ptr<bool(const char*, std::string*) noexcept> texture_picker) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   bool edited = false;

   PushID("Bump Map");

   if (value->per_platform) [[unlikely]] {
      BeginGroup();

      PushID("PC");
      BeginGroup();

      edited |= texture_picker("", &value->pc.name);

      SameLine(0.0f, 0.0f);

      Text("Bump Map (PC)");

      edited |= DragFloat2("##tile", &value->pc.tiling);

      SameLine(0.0f, 0.0f);

      Text("Bump Map Tiling (PC)");

      EndGroup();
      PopID();

      ShowCombinePopup("##PC", value, value->xbox, edit_stack, context);

      PushID("Xbox");
      BeginGroup();

      edited |= texture_picker("", &value->xbox.name);

      SameLine(0.0f, 0.0f);

      Text("Bump Map (Xbox)");

      edited |= DragFloat2("##tile", &value->xbox.tiling);

      SameLine(0.0f, 0.0f);

      Text("Bump Map Tiling (Xbox)");

      EndGroup();
      PopID();

      ShowCombinePopup("##Xbox", value, value->xbox, edit_stack, context);

      EndGroup();
   }
   else [[likely]] {
      BeginGroup();

      edited |= texture_picker("Bump Map", &value->pc.name);
      edited |= DragFloat2("Bump Map Tiling", &value->xbox.tiling);

      EndGroup();

      ShowSplitPopup("Split Bump Map", value, edit_stack, context);
   }

   PopID();

   return edited;
}

bool EditHaloRing(const char* label,
                  world::platform_var<world::sun_flare::halo_ring>* value,
                  edits::stack<world::edit_context>& edit_stack,
                  world::edit_context& context) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   bool edited = false;

   PushID(label);
   BeginGroup();

   if (value->per_platform) [[unlikely]] {
      BeginGroup();

      edited |= DragFloat("##SIZE_PC", &value->pc.size, edit_stack, context,
                          1.0f, 0.0f, 1e10f);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s Halo Ring Size (PC)", label);

      edited |= ColorEdit4("##COLOR_PC", &value->pc.color, edit_stack, context);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s Halo Ring Color (PC)", label);

      EndGroup();

      ShowCombinePopup("##PC", value, value->pc, edit_stack, context);

      BeginGroup();

      edited |= DragFloat("##SIZE_PS2", &value->ps2.size, edit_stack, context,
                          1.0f, 0.0f, 1e10f);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s Halo Ring Size (PS2)", label);

      edited |= ColorEdit4("##COLOR_PS2", &value->ps2.color, edit_stack, context);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s Halo Ring Color (PS2)", label);

      EndGroup();

      ShowCombinePopup("##PS2", value, value->ps2, edit_stack, context);

      BeginGroup();

      edited |= DragFloat("##SIZE_Xbox", &value->xbox.size, edit_stack, context,
                          1.0f, 0.0f, 1e10f);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s Halo Ring Size (Xbox)", label);

      edited |= ColorEdit4("##COLOR_Xbox", &value->xbox.color, edit_stack, context);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s Halo Ring Color (Xbox)", label);

      EndGroup();

      ShowCombinePopup("##Xbox", value, value->xbox, edit_stack, context);
   }
   else [[likely]] {
      BeginGroup();

      edited |= DragFloat("##SIZE", &value->pc.size, edit_stack, context, 1.0f,
                          0.0f, 1e10f);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s Halo Ring Size", label);

      edited |= ColorEdit4("##COLOR", &value->pc.color, edit_stack, context);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s Halo Ring Color", label);

      EndGroup();

      ShowSplitPopup(label, value, edit_stack, context);
   }

   EndGroup();
   PopID();

   return edited;
}

bool EditAnimatedTextures(const char* label, we::world::water::animated_textures* value,
                          we::edits::stack<we::world::edit_context>& edit_stack,
                          we::world::edit_context& context) noexcept
{
   bool edited = false;

   BeginGroup();
   PushID(label);

   edited |= InputText("##prefix", &value->prefix, edit_stack, context);

   SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

   Text("%s Textures Prefix", label);

   edited |= SliderInt("##count", &value->count, edit_stack, context, 1, 50,
                       "%d", ImGuiSliderFlags_AlwaysClamp);

   SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

   Text("%s Textures Count", label);

   edited |= DragFloat("##framerate", &value->framerate, edit_stack, context,
                       0.25f, 1.0f, 250.0f);

   SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

   Text("%s Textures Framerate", label);

   PopID();
   EndGroup();

   return edited;
}

bool EditAnimatedTextures(const char* label,
                          we::world::platform_pc_xb_var<we::world::water::animated_textures>* value,
                          we::edits::stack<we::world::edit_context>& edit_stack,
                          we::world::edit_context& context) noexcept
{
   IM_ASSERT(value);
   IM_ASSERT(context.is_memory_valid(value));

   bool edited = false;

   PushID(label);
   BeginGroup();

   if (value->per_platform) [[unlikely]] {
      BeginGroup();

      PushID("PC");
      BeginGroup();

      edited |= InputText("##prefix", &value->pc.prefix, edit_stack, context);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s Textures Prefix (PC)", label);

      edited |= SliderInt("##count", &value->pc.count, edit_stack, context, 1,
                          50, "%d", ImGuiSliderFlags_AlwaysClamp);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s Textures Count (PC)", label);

      edited |= DragFloat("##framerate", &value->pc.framerate, edit_stack,
                          context, 0.25f, 1.0f, 250.0f);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s Textures Framerate (PC)", label);

      EndGroup();
      PopID();

      ShowCombinePopup("##PC", value, value->xbox, edit_stack, context);

      PushID("Xbox");
      BeginGroup();

      edited |= InputText("##prefix", &value->pc.prefix, edit_stack, context);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s Textures Prefix (Xbox)", label);

      edited |= SliderInt("##count", &value->pc.count, edit_stack, context, 1,
                          50, "%d", ImGuiSliderFlags_AlwaysClamp);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s Textures Count (Xbox)", label);

      edited |= DragFloat("##framerate", &value->pc.framerate, edit_stack,
                          context, 0.25f, 1.0f, 250.0f);

      SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

      Text("%s Textures Framerate (Xbox)", label);

      EndGroup();
      PopID();

      ShowCombinePopup("##Xbox", value, value->xbox, edit_stack, context);

      EndGroup();
   }
   else [[likely]] {
      edited |= EditAnimatedTextures(label, &value->pc, edit_stack, context);

      ShowSplitPopup(label, value, edit_stack, context);
   }

   EndGroup();
   PopID();

   return edited;
}

}