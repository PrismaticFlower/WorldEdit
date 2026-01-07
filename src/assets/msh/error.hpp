#pragma once

#include <stdexcept>

namespace we::assets::msh {

enum class read_ec {
   read_version_not_supported,
   read_no_scene,
   read_matl_list_too_short,

   validation_fail_node_name_unique,
   validation_fail_node_type_valid,
   validation_fail_node_parents_valid,
   validation_fail_node_parents_noncircular,
   validation_fail_geometry_segment_material_index,
   validation_fail_geometry_segment_attibutes_count_matches,
   validation_fail_geometry_segment_vertex_count_limit,
   validation_fail_geometry_segment_triangles_index_valid,
   validation_fail_geometry_segment_non_empty,
   validation_fail_geometry_segment_no_nans,
   validation_fail_collision_primitive_shape_valid,

   ucfb_memory_too_small_minimum,
   ucfb_memory_too_small_chunk,
   ucfb_chunk_read_child_id_mismatch,
   ucfb_chunk_read_overrun,
   ucfb_strict_reader_id_mismatch,
   ucfb_unknown,

   option_load_bad_keep,
   option_load_bad_keep_material,
   option_load_bad_scale,
   option_load_bad_max_bones,
   option_load_bad_lod_group,
   option_load_bad_lod_bias,
   option_load_bad_hi_res_shadow,
   option_load_bad_bump,
   option_load_bad_bounding_box_scale,
   option_load_bad_bounding_box_offset,
   option_load_bad_ambient_lighting,
   option_load_io_open_error,
   option_load_io_generic_error,
};

/// @brief Indicates an error munging a msh.
struct read_error : std::runtime_error {
   /// @brief Construct a read_error from a message and error code.
   /// @param message The message.
   /// @param code The error code.
   read_error(const std::string& message, read_ec ec) noexcept
      : std::runtime_error{message}, _code{ec} {};

   /// @brief Construct a read_error from a message and error code.
   /// @param message The message.
   /// @param code The error code.
   read_error(const char* message, read_ec ec) noexcept
      : std::runtime_error{message}, _code{ec} {};

   /// @brief Gets the error code for the error.
   /// @return The error code.
   auto code() const noexcept -> read_ec
   {
      return _code;
   }

private:
   read_ec _code;
};

}