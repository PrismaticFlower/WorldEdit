#pragma once

#include "model.hpp"

#include "../../feedback.hpp"

#include "assets/msh/scene.hpp"

#include <span>

namespace we::munge {

struct build_shadow_segments_context {
   uint32 max_bones = 15;

   const io::path& path;
   munge_feedback& feedback;
};

struct build_shadow_volume_vertices {
   std::size_t vertex_count = 0;

   std::unique_ptr<float3[]> positionSS;
   std::unique_ptr<float3[]> positionLS;
};

auto build_shadow_segments(const model_segment& segment,
                           const build_shadow_segments_context& context)
   -> std::vector<model_shadow>;

auto build_shadow_segments(std::span<const assets::msh::shadow_volume_half_edge> half_edges,
                           build_shadow_volume_vertices vertices,
                           const std::string_view bone_name)
   -> std::vector<model_shadow>;

}