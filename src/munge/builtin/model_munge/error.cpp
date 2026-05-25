#include "error.hpp"

#include "assets/msh/error.hpp"

namespace we::munge {

model_error::model_error(const assets::msh::read_error& e) noexcept
   : model_error{
        e.what(), [](assets::msh::read_ec ec) {
           using assets::msh::read_ec;

           switch (ec) {
           case read_ec::read_version_not_supported:
              return model_ec::msh_read_version_not_supported;
           case read_ec::read_no_scene:
              return model_ec::msh_read_no_scene;
           case read_ec::read_matl_list_too_short:
              return model_ec::msh_read_matl_list_too_short;
           case read_ec::read_mndx_missing:
              return model_ec::msh_read_mndx_missing;
           case read_ec::read_mndx_duplicate:
              return model_ec::msh_read_mndx_duplicate;
           case read_ec::read_envl_entry_out_of_range:
              return model_ec::msh_read_envl_entry_out_of_range;
           case read_ec::read_envl_entry_missing_node:
              return model_ec::msh_read_envl_entry_missing_node;

           case read_ec::validation_fail_not_empty:
              return model_ec::msh_validation_fail_not_empty;
           case read_ec::validation_fail_node_name_unique:
              return model_ec::msh_validation_fail_node_name_unique;
           case read_ec::validation_fail_node_type_valid:
              return model_ec::msh_validation_fail_node_type_valid;
           case read_ec::validation_fail_node_parents_valid:
              return model_ec::msh_validation_fail_node_parents_valid;
           case read_ec::validation_fail_node_parents_noncircular:
              return model_ec::msh_validation_fail_node_parents_noncircular;
           case read_ec::validation_fail_node_mesh_data:
              return model_ec::msh_validation_fail_node_mesh_data;
           case read_ec::validation_fail_geometry_segment_material_index:
              return model_ec::msh_validation_fail_geometry_segment_material_index;
           case read_ec::validation_fail_geometry_segment_attibutes_count_matches:
              return model_ec::msh_validation_fail_geometry_segment_attibutes_count_matches;
           case read_ec::validation_fail_geometry_segment_vertex_count_limit:
              return model_ec::msh_validation_fail_geometry_segment_vertex_count_limit;
           case read_ec::validation_fail_geometry_segment_triangles_index_valid:
              return model_ec::msh_validation_fail_geometry_segment_triangles_index_valid;
           case read_ec::validation_fail_geometry_segment_no_nans:
              return model_ec::msh_validation_fail_geometry_segment_no_nans;
           case read_ec::validation_fail_geometry_segment_weights_bone_indices_valid:
              return model_ec::msh_validation_fail_geometry_segment_weights_bone_indices_valid;
           case read_ec::validation_fail_shadow_volume_edges_valid:
              return model_ec::msh_validation_fail_shadow_volume_edges_valid;
           case read_ec::validation_fail_cloth_attibutes_count_matches:
              return model_ec::msh_validation_fail_cloth_attibutes_count_matches;
           case read_ec::validation_fail_cloth_fixed_weight_count_matches:
              return model_ec::msh_validation_fail_cloth_fixed_weight_count_matches;
           case read_ec::validation_fail_cloth_fixed_index_valid:
              return model_ec::msh_validation_fail_cloth_fixed_index_valid;
           case read_ec::validation_fail_cloth_fixed_weight_valid:
              return model_ec::msh_validation_fail_cloth_fixed_weight_valid;
           case read_ec::validation_fail_cloth_triangles_index_valid:
              return model_ec::msh_validation_fail_cloth_triangles_index_valid;
           case read_ec::validation_fail_cloth_constraints_valid:
              return model_ec::msh_validation_fail_cloth_constraints_valid;
           case read_ec::validation_fail_cloth_collision_parent_valid:
              return model_ec::msh_validation_fail_cloth_collision_parent_valid;
           case read_ec::validation_fail_cloth_collision_shape_valid:
              return model_ec::msh_validation_fail_cloth_collision_shape_valid;
           case read_ec::validation_fail_collision_primitive_shape_valid:
              return model_ec::msh_validation_fail_collision_primitive_shape_valid;

           case read_ec::ucfb_memory_too_small_minimum:
              return model_ec::msh_ucfb_memory_too_small_minimum;
           case read_ec::ucfb_memory_too_small_chunk:
              return model_ec::msh_ucfb_memory_too_small_chunk;
           case read_ec::ucfb_chunk_read_child_id_mismatch:
              return model_ec::msh_ucfb_chunk_read_child_id_mismatch;
           case read_ec::ucfb_chunk_read_overrun:
              return model_ec::msh_ucfb_chunk_read_overrun;
           case read_ec::ucfb_strict_reader_id_mismatch:
              return model_ec::msh_ucfb_strict_reader_id_mismatch;
           case read_ec::ucfb_unknown:
              return model_ec::msh_ucfb_unknown;

           case read_ec::option_load_bad_keep:
              return model_ec::msh_option_load_bad_keep;
           case read_ec::option_load_bad_keep_material:
              return model_ec::msh_option_load_bad_keep_material;
           case read_ec::option_load_bad_scale:
              return model_ec::msh_option_load_bad_scale;
           case read_ec::option_load_bad_max_bones:
              return model_ec::msh_option_load_bad_max_bones;
           case read_ec::option_load_bad_lod_group:
              return model_ec::msh_option_load_bad_lod_group;
           case read_ec::option_load_bad_lod_bias:
              return model_ec::msh_option_load_bad_lod_bias;
           case read_ec::option_load_bad_hi_res_shadow:
              return model_ec::msh_option_load_bad_hi_res_shadow;
           case read_ec::option_load_bad_bump:
              return model_ec::msh_option_load_bad_bump;
           case read_ec::option_load_bad_bounding_box_scale:
              return model_ec::msh_option_load_bad_bounding_box_scale;
           case read_ec::option_load_bad_bounding_box_offset:
              return model_ec::msh_option_load_bad_bounding_box_offset;
           case read_ec::option_load_bad_ambient_lighting:
              return model_ec::msh_option_load_bad_ambient_lighting;
           case read_ec::option_load_io_open_error:
              return model_ec::msh_option_load_io_open_error;
           case read_ec::option_load_io_generic_error:
              return model_ec::msh_option_load_io_generic_error;
           }

           return model_ec::msh_unknown;
        }(e.code())}
{
}

auto get_descriptive_message(const model_wc c) noexcept -> std::string_view
{
   switch (c) {
   case model_wc::bone_map_used_unkept_node:
      return "Somebody lazy didn't write the warning message! Unkept bone used "
             "in ENVL chunk or something.";
   case model_wc::material_invalid_rendertype:
      return "Somebody lazy didn't write the warning message! Invalid "
             "rendertype or something.";
   }

   return "Unknown Error";
}

}