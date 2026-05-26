#include "error.hpp"

#include "assets/msh/error.hpp"

#include <fmt/format.h>

namespace we::munge {

namespace {

auto get_description(const model_ec c) noexcept -> std::string_view
{
   switch (c) {
   case model_ec::msh_read_version_not_supported:
      return R"(MSH_READ_VERSION_NOT_SUPPORTED

The .msh file can not be read as it contains a MSH1 chunk instead of a MSH2 chunk.)";
   case model_ec::msh_read_no_scene:
      return R"(MSH_READ_NO_SCENE

The .msh file was read successfully but contained no scene.

Use an external tool (like Softimage, Blender or a msh viewer) to validate that the file contains a scene.

If it does try re-exporting the .msh file. Then try reading the .msh file again.)";
   case model_ec::msh_read_mndx_missing:
      return R"(MSH_READ_MNDX_MISSING

The model index (MNDX chunk) was missing from a node. The .msh file can not be read correctly without it.)";
   case model_ec::msh_read_mndx_duplicate:
      return R"(MSH_READ_MNDX_DUPLICATE

Two nodes share the same model index (MNDX chunk).)";
   case model_ec::msh_read_envl_entry_out_of_range:
      return R"(MSH_READ_ENVL_ENTRY_OUT_OF_RANGE

An entry in an ENVL chunk references a node with an index greater than any present node.)";
   case model_ec::msh_read_envl_entry_missing_node:
      return R"(MSH_READ_ENVL_ENTRY_MISSING_NODE

An entry in an ENVL chunk references a missing node.)";

   case model_ec::msh_validation_fail_not_empty:
      return R"(MSH_VALIDATION_FAIL_NOT_EMPTY

The .msh file scene contained no nodes.)";
   case model_ec::msh_validation_fail_node_name_unique:
      return R"(MSH_VALIDATION_FAIL_NODE_NAME_UNIQUE

Each node in the .msh file must have a unique name.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_node_type_valid:
      return R"(MSH_VALIDATION_FAIL_NODE_TYPE_VALID

A node has an unknown type.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_node_parents_valid:
      return R"(MSH_VALIDATION_FAIL_NODE_PARENTS_VALID

A node references a missing parent node. 

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_node_parents_noncircular:
      return R"(MSH_VALIDATION_FAIL_NODE_PARENTS_NONCIRCULAR

A node has a circular relationship with an ancestor/parent.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_node_mesh_data:
      return R"(MSH_VALIDATION_FAIL_NODE_MESH_DATA

No node in the .msh file contains mesh data. This includes regular meshes, cloth and 
collision primitives.)";
   case model_ec::msh_validation_fail_geometry_segment_material_index:
      return R"(MSH_VALIDATION_FAIL_GEOMETRY_SEGMENT_MATERIAL_INDEX

A geometry segment references has an invalid material index.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_geometry_segment_attibutes_count_matches:
      return R"(MSH_VALIDATION_FAIL_GEOMETRY_SEGMENT_ATTIBUTES_COUNT_MATCHES

A geometry segment's vertex attributes have mismatches counts. I.e 4 positions and only 3 normals.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_geometry_segment_vertex_count_limit:
      return R"(MSH_VALIDATION_FAIL_GEOMETRY_SEGMENT_VERTEX_COUNT_LIMIT

A geometry segment has too many vertices. .msh files are limited to 32,768 vertices in each geometry segment.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_geometry_segment_triangles_index_valid:
      return R"(MSH_VALIDATION_FAIL_GEOMETRY_SEGMENT_TRIANGLES_INDEX_VALID

A geometry segment's triangle reference invalid vertices.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_geometry_segment_no_nans:
      return R"(MSH_VALIDATION_FAIL_GEOMETRY_SEGMENT_NO_NANS

A vertex position in a geometry segment has a NaN (Not a Number). This is invalid and can cause issues with modelmunge.

If you still have the source files for this mesh try cleaning it of degenerate faces and exporting again.)";
   case model_ec::msh_validation_fail_geometry_segment_weights_bone_indices_valid:
      return R"(MSH_VALIDATION_FAIL_GEOMETRY_SEGMENT_WEIGHTS_BONE_INDICES_VALID

A vertex weight in a geometry segment contains a bone index that is out of range of the bone map/envelope.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_shadow_volume_edges_valid:
      return R"(MSH_VALIDATION_FAIL_SHADOW_VOLUME_EDGES_VALID

The shadow volume half edge list is invalid.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_cloth_attibutes_count_matches:
      return R"(MSH_VALIDATION_FAIL_CLOTH_ATTIBUTES_COUNT_MATCHES

The position and texcoords counts for a cloth segment is mismatched.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_cloth_fixed_weight_count_matches:
      return R"(MSH_VALIDATION_FAIL_CLOTH_FIXED_WEIGHT_COUNT_MATCHES

The fixed weights and fixed indices array sizes for a cloth segment are mismatched. 

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_cloth_fixed_index_valid:
      return R"(MSH_VALIDATION_FAIL_CLOTH_FIXED_INDEX_VALID

A fixed index entry in a cloth segment does not reference a valid cloth vertex.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_cloth_fixed_weight_valid:
      return R"(MSH_VALIDATION_FAIL_CLOTH_FIXED_WEIGHT_VALID

A fixed weight entry in a cloth segment is invalid.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_cloth_triangles_index_valid:
      return R"(MSH_VALIDATION_FAIL_CLOTH_TRIANGLES_INDEX_VALID

A triangle in a cloth segment references an invalid vertex.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_cloth_constraints_valid:
      return R"(MSH_VALIDATION_FAIL_CLOTH_CONSTRAINTS_VALID

A constraint entry in a cloth segment is invalid.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_cloth_collision_parent_valid:
      return R"(MSH_VALIDATION_FAIL_CLOTH_COLLISION_PARENT_VALID

A cloth collision primitive does not reference a valid parent.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message below may have more information and context.)";
   case model_ec::msh_validation_fail_cloth_collision_shape_valid:
      return R"(MSH_VALIDATION_FAIL_CLOTH_COLLISION_SHAPE_VALID

A cloth collision primitive does not have a valid shape.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message below may have more information and context.)";

   case model_ec::msh_ucfb_memory_too_small_minimum:
      return R"(MSH_UCFB_MEMORY_TOO_SMALL_MINIMUM

The size of the the .msh file is too small to be a valid .msh file.)";
   case model_ec::msh_ucfb_memory_too_small_chunk:
      return R"(MSH_UCFB_MEMORY_TOO_SMALL_CHUNK

The .msh file is too small to fit it's root chunk in.)";
   case model_ec::msh_ucfb_chunk_read_child_id_mismatch:
      return R"(MSH_UCFB_CHUNK_READ_CHILD_ID_MISMATCH

This error shouldn't typically be encountered. The Exception Message below may have more information and context but it is likely this indicates memory corruption or a bug in WorldEdit.)";
   case model_ec::msh_ucfb_chunk_read_overrun:
      return R"(MSH_UCFB_CHUNK_READ_OVERRUN

A chunk in the .msh file was too short.

The Exception Message below may have more information and context.)";
   case model_ec::msh_ucfb_strict_reader_id_mismatch:
      return R"(MSH_UCFB_STRICT_READER_ID_MISMATCH

This error shouldn't typically be encountered. The Exception Message below may have more information and context but it is likely this indicates memory corruption or a bug in WorldEdit.)";
   case model_ec::msh_ucfb_unknown:
      return R"(MSH_UCFB_UNKNOWN

An unknow error occured while reading the .msh file's structure.)";

   case model_ec::msh_option_load_bad_keep:
      return R"(MSH_OPTION_LOAD_BAD_KEEP

Bad "-keep" option in .msh.option file. "-keep" must be followed by the name of the node to keep.

i.e "-keep light_attach")";
   case model_ec::msh_option_load_bad_keep_material:
      return R"(MSH_OPTION_LOAD_BAD_KEEP_MATERIAL

Bad "-keepmaterial" option in .msh.option file. "-keepmaterial" must be followed by the name of the node to keep.

i.e "-keepmaterial OverrideTexture")";
   case model_ec::msh_option_load_bad_scale:
      return R"(MSH_OPTION_LOAD_BAD_SCALE

Bad "-scale" option in .msh.option file. "-scale" must be followed by a non-negative float setting the scale to use.

i.e "-scale 0.125")";
   case model_ec::msh_option_load_bad_max_bones:
      return R"(MSH_OPTION_LOAD_BAD_MAX_BONES

Bad "-maxbones" option in .msh.option file. "-scale" must be followed by an unsigned integer setting the max bones per-segment.

i.e "-maxbones 8")";
   case model_ec::msh_option_load_bad_lod_group:
      return R"(MSH_option_load_bad_lod_group

Bad "-lodgroup" option in .msh.option file. "-lodgroup" must be followed by the lodgroup to use.

i.e "-lodgroup bigmodel"

Valid Lod Groups:

model
bigmodel
soldier
hugemodel)";
   case model_ec::msh_option_load_bad_lod_bias:
      return R"(MSH_OPTION_LOAD_BAD_LOD_BIAS

Bad "-lodbias" option in .msh.option file. "-lodbias" must be followed by a non-negative float setting the lod bias.

i.e "-lodbias 2.0")";
   case model_ec::msh_option_load_bad_hi_res_shadow:
      return R"(MSH_OPTION_LOAD_BAD_HI_RES_SHADOW

Bad "-hiresshadow" option in .msh.option file. When "-hiresshadow" is followed by an argument it must be an unsigned integer setting the LOD to use for the shadow.

i.e "-hiresshadow 1")";
   case model_ec::msh_option_load_bad_bump:
      return R"(MSH_OPTION_LOAD_BAD_BUMP

Bad "-bump" option in .msh.option file. "-bump" must be followed by the name of a texture to add a bump map to.

i.e "-bump test_main_01")";
   case model_ec::msh_option_load_bad_bounding_box_scale:
      return R"(MSH_OPTION_LOAD_BAD_BOUNDING_BOX_SCALE

Bad "-boundingboxscale" option in .msh.option file. "-boundingboxscale" must be followed by a non-negative float setting the scale for the bounding sphere.

i.e "-boundingboxscale 2.0")";
   case model_ec::msh_option_load_bad_bounding_box_offset:
      return R"(MSH_OPTION_LOAD_BAD_BOUNDING_BOX_OFFSET

Base "-boundingboxoffsetx", "-boundingboxoffsety", "-boundingboxoffsetz" or "-boundingboxoffsetnz". These options must be followed by a non-negative float setting the offset for the bounding sphere.

i.e "-boundingboxoffsety 2.0")";
   case model_ec::msh_option_load_bad_ambient_lighting:
      return R"(MSH_OPTION_LOAD_BAD_AMBIENT_LIGHTING

Bad "-ambientlighting" option in .msh.option file. "-ambientlighting" must be followed by a string in quotes of the form "r=[-1, 1] g=[-1, 1] b=[-1, 1]".

i.e "-ambientlighting "r=1 g=-0.5 b=2.0"")";
   case model_ec::msh_option_load_bad_attach_light:
      return R"(MSH_OPTION_LOAD_BAD_ATTACH_LIGHT

Bad "-attachlight" option in .msh.option file. "-attachlight" must be followed by a quoted string with the name of a the node to attach the light to and the name of the light to attach.

i.e "-attachlight "blinking_node_01 blinking_light_01"")";
   case model_ec::msh_option_load_io_open_error:
      return R"(MSH_OPTION_LOAD_IO_OPEN_ERROR

Failed to open the .msh.option file.

The Exception Message below may have more information and context.)";
   case model_ec::msh_option_load_io_generic_error:
      return R"(MSH_OPTION_LOAD_IO_GENERIC_ERROR

Failed to read the .msh.option file.

The Exception Message may have more information.)";
   case model_ec::msh_unknown:
      return R"(MSH_UNKNOWN

An unknown error occured while reading the .msh file.)";

   case model_ec::skeleton_too_many_bones:
      return R"(SKELETON_TOO_MANY_BONES

Resulting skeleton from .msh bones and kept nodes has too many bones.)";

   case model_ec::model_split_segment_unskinned:
      return R"(MODEL_SPLIT_SEGMENT_UNSKINNED

Attempt to split an unskinned segment.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.)";
   case model_ec::model_split_segment_bad_vertex_count:
      return R"(MODEL_SPLIT_SEGMENT_BAD_VERTEX_COUNT

A segment has 0 vertices after being split.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.)";
   case model_ec::model_split_segment_unexpected_failure:
      return R"(MODEL_SPLIT_SEGMENT_UNEXPECTED_FAILURE

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.)";

   case model_ec::model_generate_tangents_missing_inputs:
      return R"(MODEL_GENERATE_TANGENTS_MISSING_INPUTS

A mesh segment is missing vertex attributes or triangles needed to generate tangents for normal mapping.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.)";
   case model_ec::model_generate_tangents_unknown_failure:
      return R"(MODEL_GENERATE_TANGENTS_UNKNOWN_FAILURE

This indicates an unexpected failure in generating mesh tangents for normal mapping. Please report this along with the .msh you're trying to munge)";
   case model_ec::model_generate_tangents_bad_vertex_count:
      return R"(MODEL_GENERATE_TANGENTS_BAD_VERTEX_COUNT

A segment unexpected has 0 vertices after generating mesh tangents.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.)";

   case model_ec::model_optimize_merge_segments_bad_vertex_count:
      return R"(MODEL_OPTIMIZE_MERGE_SEGMENTS_BAD_VERTEX_COUNT

Attempt to merge segments with 0 total vertices.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.)";
   case model_ec::model_optimize_vertex_cache_bad_triangle_count:
      return R"(MODEL_OPTIMIZE_VERTEX_CACHE_BAD_TRIANGLE_COUNT

Attempt to optimize vertex buffer of mesh segment with no triangles.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.)";
   case model_ec::model_optimize_vertex_cache_bad_vertex_count:
      return R"(MODEL_OPTIMIZE_VERTEX_CACHE_BAD_VERTEX_COUNT

Attempt to optimize empty vertex buffer.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.)";
   case model_ec::model_optimize_vertex_fetch_reorder_bad_vertex_count:
      return R"(MODEL_OPTIMIZE_VERTEX_FETCH_REORDER_BAD_VERTEX_COUNT

A vertex buffer has 0 vertices after optimization. This should be impossible.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.)";

   case model_ec::model_shadow_mesh_missing_inputs:
      return R"(MODEL_SHADOW_MESH_MISSING_INPUTS

Attempt to build shadow mesh without vertex positions.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.)";
   case model_ec::model_shadow_mesh_unexpected_failure:
      return R"(MODEL_SHADOW_MESH_UNEXPECTED_FAILURE

Unexpected failure while building final shadow mesh buffers.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.)";

   case model_ec::collision_mesh_bad_vertex_count:
      return R"(COLLISION_MESH_BAD_VERTEX_COUNT

Attempt to munge a collision mesh with no vertices.)";
   case model_ec::collision_mesh_bad_triangle_count:
      return R"(COLLISION_MESH_BAD_TRIANGLE_COUNT

Attempt to munge a collision mesh with no triangles)";
   case model_ec::collision_mesh_too_many_vertices:
      return R"(COLLISION_MESH_TOO_MANY_VERTICES

Attempt to munge a collison mesh with too many vertices.)";
   case model_ec::collision_mesh_simplified_face_invalid:
      return R"(COLLISION_MESH_SIMPLIFIED_FACE_INVALID

The mesh simplifer for collision has returned an invalid face (one with 0 vertices).

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.)";

   case model_ec::req_write_io_open_error:
      return R"(REQ_WRITE_IO_OPEN_ERROR

Failed to open .model.req file for output! Make sure the munge out directory is accessible and that nothing currently has the output .model.req file open.)";
   case model_ec::req_write_io_generic_error:
      return R"(REQ_WRITE_IO_GENERIC_ERROR

An unexpected IO error has occured while writing the model's .model.req file. The Exception Message below may have more information and context.)";

   case model_ec::write_io_open_error:
      return R"(WRITE_IO_OPEN_ERROR

Failed to open .model file for output.

The Exception Message below may have more information and context.)";
   case model_ec::write_io_generic_error:
      return R"(WRITE_IO_GENERIC_ERROR

Failed to write to .model file.

The Exception Message below may have more information and context.)";
   }

   return "Unknown Error";
}

}

model_error::model_error(const assets::msh::read_error& e) noexcept
   : model_error{
        e.what(), [](assets::msh::read_ec ec) {
           using assets::msh::read_ec;

           switch (ec) {
           case read_ec::read_version_not_supported:
              return model_ec::msh_read_version_not_supported;
           case read_ec::read_no_scene:
              return model_ec::msh_read_no_scene;
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
           case read_ec::option_load_bad_attach_light:
              return model_ec::msh_option_load_bad_attach_light;
           case read_ec::option_load_io_open_error:
              return model_ec::msh_option_load_io_open_error;
           case read_ec::option_load_io_generic_error:
              return model_ec::msh_option_load_io_generic_error;
           }

           return model_ec::msh_unknown;
        }(e.code())}
{
}

auto get_descriptive_message(const model_error& e) noexcept -> std::string
{
   return fmt::format("{}\n\nException Message: {}", get_description(e.code()),
                      e.what());
}

auto get_descriptive_message(const model_wc c) noexcept -> std::string_view
{
   switch (c) {
   case model_wc::bone_map_used_unkept_node:
      return R"(BONE_MAP_USED_UNKEPT_NODE

An envelope/bone map uses a node that is not a bone and has not been explicitly kept (using "-keep"/"-keepall").

This is valid but the entry in the bone map will instead point towards an unnamed bone with an identity transform. It is unlikely to be 
what you want.)";

   case model_wc::material_invalid_rendertype:
      return R"(MATERIAL_INVALID_RENDERTYPE

A material uses an invalid rendertype. The material should still work ingame but is unlikely to be what you want.)";

   case model_wc::missing_keep_node:
      return R"(MISSING_KEEP_NODE

The node named in a "-keep" option does not exist in the .msh file. This could indicate a typo.)";
   case model_wc::missing_keep_material:
      return R"(MISSING_KEEP_MATERIAL

The node named in a "-keepmaterial" option does not exist in the .msh file. This could indicate a typo.)";
   case model_wc::missing_bump_map:
      return R"(MISSING_BUMP_MAP

The texture named in a "-bump" option is not used by any material. This could indicate a typo.)";

   case model_wc::possible_typo_hp:
      return R"(POSSIBLE_TYPO_HP

A possible typo has been detected. Only nodes that start with lowercase "hp_" are treated as hardpoints, nodes that start with "HP_", "hP_" or "Hp_" are not.

If you want the node to be a hardpoint you should rename it or use "-keep" in the .msh.option file on it.")";
   case model_wc::possible_typo_shadowvolume:
      return R"(POSSIBLE_TYPO_SHADOWVOLUME

A possible typo has been detected. Only nodes that start with lowercase "sv_" are treated meshes to be converted to shadow volumes, nodes that start with "SV_", "sV_" or "Sv_" are not.

If you want the node to be a shadow volume you should rename it.)";

   case model_wc::model_shadow_mesh_discarded_degenerate_triangle:
      return R"(MODEL_SHADOW_MESH_DISCARDED_DEGENERATE_TRIANGLE

A degenerate triangle has been discarded from an input mesh while constructing a shadow mesh.)";
   case model_wc::model_shadow_mesh_merged_nonidentical_vertices:
      return R"(MODEL_SHADOW_MESH_MERGED_NONIDENTICAL_VERTICES

Merged non-identical vertices while constructing a shadow mesh. This can typically be safely ignored but you may want to simplify your input mesh.)";
   case model_wc::model_shadow_mesh_merged_nonidentical_vertices_bone_weights:
      return R"(MODEL_SHADOW_MESH_MERGED_NONIDENTICAL_VERTICES_BONE_WEIGHTS

Merged vertices with non-identical bone indices while constructing a shadow mesh. This is rare and may indicate you should simplify your input mesh.)";
   case model_wc::model_shadow_mesh_merged_nonidentical_vertices_bone_indices:
      return R"(MODEL_SHADOW_MESH_MERGED_NONIDENTICAL_VERTICES_BONE_INDICES

Merged vertices with non-identical bone weights while constructing a shadow mesh. This is rare and may indicate you should simplify your input mesh.)";
   case model_wc::model_shadow_mesh_non_manifold_edge_detected:
      return R"(MODEL_SHADOW_MESH_NON_MANIFOLD_EDGE_DETECTED

A non-manifold edge was detected while constructing a shadow mesh. You should make sure your input mesh is closed.)";
   case model_wc::model_shadow_mesh_close_unable_to_loop_back_to_start:
      return R"(MODEL_SHADOW_MESH_CLOSE_UNABLE_TO_LOOP_BACK_TO_START

Unable to gather holes needed to fill mesh holes. You should make sure your input mesh is closed.)";
   case model_wc::model_shadow_mesh_close_still_open_after_fill:
      return R"(MODEL_SHADOW_MESH_CLOSE_STILL_OPEN_AFTER_FILL

The shadow mesh is still open after attempting to close holes. Falling back to brute force shadow mesh construction (this produces much more expensive shadow volumes at runtime). You should make sure your input mesh is closed to fix this.)";

   case model_wc::collision_mesh_parent_transformed:
      return R"(COLLISION_MESH_PARENT_TRANSFORMED

A collision mesh has been parented to a bone without an identity transform. This is likely not intentional as ingame collision meshes will not have parents.)";
   case model_wc::collision_mesh_has_vertex_weights:
      return R"(COLLISION_MESH_HAS_VERTEX_WEIGHTS

A collision mesh has vertex weights. This is likely not intentional as collision meshes can not be animated.)";
   case model_wc::collision_primitive_wrong_prefix:
      return R"(COLLISION_PRIMITIVE_WRONG_PREFIX

A collision primitive must start with lowercase "p_". This node will be skipped.)";
   case model_wc::collision_primitive_missing_data:
      return R"(COLLISION_PRIMITIVE_MISSING_DATA

A node has the prefix for a collision primitive but contains to prmitive data.)";
   case model_wc::collision_invalid_flag:
      return R"(COLLISION_INVALID_FLAG

A node has an unexpected collision flag. Defaulting to all flags.

Valid Flags:
   s
   v
   b
   t
   o
   f
)";
   }

   return "Unknown Warning";
}

}