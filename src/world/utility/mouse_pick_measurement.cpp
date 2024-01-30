#include "mouse_pick_measurement.hpp"
#include "math/matrix_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <fmt/core.h>
#include <imgui.h>

namespace we::world {

auto mouse_pick(const float2 mouse_position, const float2 viewport_size,
                const float4x4 view_projection_matrix,
                std::span<const measurement> measurements) noexcept
   -> std::optional<measurement_id>
{
   for (const measurement& measurement : measurements) {
      if (measurement.hidden) continue;

      const float3 centreWS = (measurement.start + measurement.end) * 0.5f;
      const float4 centrePS = view_projection_matrix * float4{centreWS, 1.0f};
      const float2 centreNDC = {centrePS.x / centrePS.w, centrePS.y / centrePS.w};

      if (centreNDC.x > 1.0f or centreNDC.x < -1.0f or centreNDC.y > 1.0f or
          centreNDC.y < -1.0f) {
         continue;
      }

      const float2 positionRT = {(centreNDC.x + 1.0f) * viewport_size.x * 0.5f,
                                 (1.0f - centreNDC.y) * viewport_size.y * 0.5f};

      std::array<char, 128> text;

      const char* const text_end =
         fmt::format_to_n(text.data(), text.size(), "{:.2f}m",
                          distance(measurement.start, measurement.end))
            .out;

      const ImVec2 text_size = ImGui::CalcTextSize(text.data(), text_end);
      const float2 text_size_half = {text_size.x * 0.5f, text_size.y * 0.5f};

      const float2 text_minRT = positionRT - text_size_half;
      const float2 text_maxRT = positionRT + text_size_half;

      if (mouse_position.x >= text_minRT.x and mouse_position.x < text_maxRT.x and
          mouse_position.y >= text_minRT.y and mouse_position.y < text_maxRT.y) {
         return measurement.id;
      }
   }

   return std::nullopt;
}

}