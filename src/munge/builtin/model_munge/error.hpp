#pragma once

#include <stdexcept>

namespace we::assets::msh {
struct read_error;
}

namespace we::munge {

enum class model_ec {
   msh_read_version_not_supported,
   msh_read_no_scene,
   msh_read_mndx_missing,
   msh_read_mndx_duplicate,
   msh_read_envl_entry_out_of_range,
   msh_read_envl_entry_missing_node,

   msh_validation_fail_not_empty,
   msh_validation_fail_node_name_unique,
   msh_validation_fail_node_type_valid,
   msh_validation_fail_node_parents_valid,
   msh_validation_fail_node_parents_noncircular,
   msh_validation_fail_node_mesh_data,
   msh_validation_fail_geometry_segment_material_index,
   msh_validation_fail_geometry_segment_attibutes_count_matches,
   msh_validation_fail_geometry_segment_vertex_count_limit,
   msh_validation_fail_geometry_segment_triangles_index_valid,
   msh_validation_fail_geometry_segment_no_nans,
   msh_validation_fail_geometry_segment_weights_bone_indices_valid,
   msh_validation_fail_shadow_volume_edges_valid,
   msh_validation_fail_cloth_attibutes_count_matches,
   msh_validation_fail_cloth_fixed_index_valid,
   msh_validation_fail_cloth_fixed_weight_valid,
   msh_validation_fail_cloth_triangles_index_valid,
   msh_validation_fail_cloth_constraints_valid,
   msh_validation_fail_cloth_collision_parent_valid,
   msh_validation_fail_cloth_collision_shape_valid,

   msh_ucfb_memory_too_small_minimum,
   msh_ucfb_memory_too_small_chunk,
   msh_ucfb_chunk_read_child_id_mismatch,
   msh_ucfb_chunk_read_overrun,
   msh_ucfb_strict_reader_id_mismatch,
   msh_ucfb_unknown,

   msh_option_load_bad_scale,
   msh_option_load_bad_max_bones,
   msh_option_load_bad_lod_group,
   msh_option_load_bad_lod_bias,
   msh_option_load_bad_hi_res_shadow,
   msh_option_load_bad_bounding_box_scale,
   msh_option_load_bad_bounding_box_offset,
   msh_option_load_bad_ambient_lighting,
   msh_option_load_bad_attach_light,
   msh_option_load_io_open_error,
   msh_option_load_io_generic_error,

   msh_unknown,

   skeleton_too_many_bones,

   model_split_segment_unskinned,
   model_split_segment_bad_vertex_count,
   model_split_segment_unexpected_failure,

   model_generate_tangents_missing_inputs,
   model_generate_tangents_unknown_failure,
   model_generate_tangents_bad_vertex_count,

   model_optimize_merge_segments_bad_vertex_count,
   model_optimize_vertex_cache_bad_triangle_count,
   model_optimize_vertex_cache_bad_vertex_count,
   model_optimize_vertex_fetch_reorder_bad_vertex_count,

   model_shadow_mesh_missing_inputs,
   model_shadow_mesh_unexpected_failure,

   collision_mesh_bad_vertex_count,
   collision_mesh_bad_triangle_count,
   collision_mesh_too_many_vertices,
   collision_mesh_simplified_face_invalid,

   req_write_io_open_error,
   req_write_io_generic_error,

   write_io_open_error,
   write_io_generic_error,
};

/// @brief Indicates an error munging a model.
struct model_error : public std::runtime_error {
   /// @brief Construct an model_error from a message and error code.
   /// @param message The message.
   /// @param code The error code.
   model_error(const std::string& message, model_ec ec) noexcept
      : std::runtime_error{message}, _code{ec} {};

   /// @brief Construct an model_error from a message and error code.
   /// @param message The message.
   /// @param code The error code.
   model_error(const char* message, model_ec ec) noexcept
      : std::runtime_error{message}, _code{ec} {};

   /// @brief Construct an model_error from a assets::msh::read_error.
   /// @param e The exception to convert to a model_error.
   model_error(const assets::msh::read_error& e) noexcept;

   /// @brief Gets the error code for the error.
   /// @return The error code.
   auto code() const noexcept -> model_ec
   {
      return _code;
   }

private:
   model_ec _code;
};

auto get_descriptive_message(const model_error& e) noexcept -> std::string;

enum class model_wc {
   bone_map_used_unkept_node,

   material_invalid_rendertype,

   missing_keep_node,
   missing_keep_material,
   missing_attach_light,
   missing_bump_map,

   possible_typo_hp,
   possible_typo_shadow_volume,

   unhidden_shadow_volume,

   model_shadow_mesh_discarded_degenerate_triangle,
   model_shadow_mesh_merged_nonidentical_vertices,
   model_shadow_mesh_merged_nonidentical_vertices_bone_weights,
   model_shadow_mesh_merged_nonidentical_vertices_bone_indices,
   model_shadow_mesh_non_manifold_edge_detected,
   model_shadow_mesh_close_unable_to_loop_back_to_start,
   model_shadow_mesh_close_still_open_after_fill,

   collision_mesh_parent_transformed,
   collision_mesh_has_vertex_weights,
   collision_primitive_wrong_prefix,
   collision_primitive_missing_data,
   collision_invalid_flag,
};

auto get_descriptive_message(const model_wc c) noexcept -> std::string_view;

}