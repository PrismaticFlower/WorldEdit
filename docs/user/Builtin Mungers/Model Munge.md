# Model Munge

WorldEdit's builtin model munge supports most of the PC options listed in "misc_documentation.doc" from the modtools and then some. It is currently only 
used when munging for PC. It ranges from being slightly faster to drastically faster than `pc_ModelMunge`, depending on if you manage hit one of the pathological worst cases in `pc_ModelMunge`'s collision handling.

In addition to being a complete ModelMunge replacment it has extra features too such as:

- Performing basic optimization of the index buffers of a mesh using [meshoptimizer](https://github.com/zeux/meshoptimizer), so not only can it munge a little faster, it's output may be drawn slightly faster too.
- Better Diagnostics when an error happens, including [Warnings](#warnings) to help catch issues that don't prevent a munge but are likely indicative of mistakes.
- Optional support for large texture coordinates/UVs (outside the range [-16, 15.999512]) using a new `-largetexcoords` option.


## Compatibility Differences with Modtools' Model Munge

### No Support for Triangle Strips 
Triangle lists are written out instead. This is not a big issue as conventional wisdom seems to suggest GPUs are faster with triangle lists (and have been for a while) anyway.

Furthermore models exported using XSIZETools do not have true triangle strips generated and are better served being exported to .model files with triangle lists.

### Normal Mapping Tangents
Normal mapping tangents are generated using [MikkTSpace](https://github.com/mmikk/MikkTSpace). As a result they're unlikely to exactly match what `pc_ModelMunge` would produce. They are however high quality.


### Bounding Box and Bounding Sphere
In some rare cases I have not been able to get the Visbility Box and Bounding Sphere to line up 100% with what `pc_ModelMunge` produces. Tests ingame (using the `RenderAABB` and `RenderSphere` commands) suggest this will not be an issue as in all tested cases the box and sphere continue to contain the mesh, in some cases more optimially than the `pc_ModelMunge` output.

### `-scale` 
The `-scale` option has not been tested exhaustively exact compatibility with `pc_ModelMunge`. However it is implemented simply and robustly and is likely to always do what you want. Before any other processing is done all of the .msh file's positions, translations and sizes are scaled by the provided factor.

### No `CSHD` Chunk
The `CSHD` chunk (which I suspect stands for "CPU Shadow") is not written out. To the best of my knowledge the data from this chunk is never meaningfully used. It just wastes space.

Though you will get a "No shadow data!" message in BFront2.log if you launch the game with `/fixedfunction`. But even if the chunk was there the game never draws shadows under `/fixedfunction` anyway.

## Node Naming Scheme
The names of nodes in .msh files can have meaning. Below is a list of what model munge recognizes and a description of what they do. Some are case sensitive and some aren't. Ones marked as "Contains" simply need the string to appear anywhere in the name for the effects and ones marked as "Prefix" need the name to to be at the start for it to have an effect.

| Name | Type | Case Sensitive | Description |
|------|------|----------------|-------------|
| lod2 | Contains | No | Marks the node as being part of the LOD1 model. Also forces the node to included even if it is hidden. |
| lod3 | Contains | No | Marks the node as being part of the LOD2 model. Also forces the node to included even if it is hidden. |
| lowres | Contains | No | Marks the node as being part of the low res/LOWD model. Also forces the node to included even if it is hidden. |
| lowrez | Contains | No | Marks the node as being part of the low res/LOWD model. Also forces the node to included even if it is hidden. |
| p_ | Prefix | Yes | Marks the node as being a collision primitive. |
| hp_ | Prefix | Yes | Marks the node as being a hard point. Hard points are included in the skeleton without needing `-keep` options. |
| collision | Prefix | No | Marks the node as being a collision mesh. |
| terraincutter | Prefix | No | Marks the node as being a terrain cutter mesh. This does nothing in model munge except tell it to skip the node. |
| sv_ | Prefix | Yes | Marks the node as being a shadow volume. This is not mutally exclusive with the node being a regular mesh though, unless hidden it will also be used as such. |

## Shadow Volumes
Nodes being turned into shadow volumes (either through the "sv_" prefix or "-hiresshadow") should be closed. If they are not the munger will try and close the mesh. It will often succeed but if it fails it has to fallback to treating each triangle in the mesh as an individual shadow volume, this produces a shadow volume that will technically work but will be very wasteful. If you get the [MODEL_SHADOW_MESH_CLOSE_STILL_OPEN_AFTER_FILL](#MODEL_SHADOW_MESH_CLOSE_STILL_OPEN_AFTER_FILL) warning you strongly encouraged to edit your intended shadow volume mesh and make sure it is closed.

This does not apply to shadow volumes from "SHDW" chunks as they are already closed by nature. Furthermore they will munge the fastest as there is no need to construct the half edge structure for them as that is what they already store.

## Supported Options

| Option | Arg | Description |
|--------|-----|-------------|
| -keep | \<node list> | Keep the listed nodes as bones in the skeleton. |
| -keepall |  | Keep all nodes as bones in the skeleton. |
| -keepmaterial | \<node list> | Give the materials in listed nodes a name matching the node's name. Used for override texture support. |
| -scale | \<scale> | Scale the model by the given factor. |
| -maxbones | \<maxbones> | Sets the max bones per skinned segment. Defaults to and clamped to 15 on PC. |
| -lodgroup | \<lodgroup> | Sets the LOD group for the model. Can be `model`, `bigmodel`, `soldier` or `hugemodel`. |
| -lodbias | \<lodbias> | Sets the LOD bias for the model. |
| -nocollision | | Turn off munging collision meshes for the model. Does not affect collision primitives. |
| -nogamemodel | | Disable saving the "game model" information for the model. Typically used on first person models and prevents a model from being used by an entity normally. |
| -hiresshadow  | \<lod> (optional) | Generate a shadow volume for the model from the supplied LOD, falling back to the closest one if it's missing. If no LOD is specified it defaults to 0. |
| -softskin | | Turn on soft skinning for the model. In BF2 this also requires a mod like Shader Patch for it to look any different ingame. |
| -softskinshadow   | | Enable soft skinning for the shadow volume.  |
| -vertexlighting | | Treat any vertex colours as baked on static lighting. |
| -additiveemissive | | Mark emissive materials in the model as also having additive transparency. |
| -bump | \<diffuse map list> | Converts materials using one of the listed diffuse maps to be normal mapped materials. The normal map will be named `{diffuse_map}_bump`. |
| -boundingboxscale | \<scale> | Scale the bounding sphere (not box, it's misnamed) for the model. |
| -boundingboxoffsetx | \<offset> | Offset the bounding sphere (not box, it's misnamed) for the model along the positive X axis. |
| -boundingboxoffsety | \<offset> | Offset the bounding sphere (not box, it's misnamed) for the model along the positive Y axis. |
| -boundingboxoffsetz | \<offset> | Offset the bounding sphere (not box, it's misnamed) for the model along the positive Z axis. |
| -boundingboxoffsetnz | \<offset> | Offset the bounding sphere (not box, it's misnamed) for the model along the negative Z axis. | 
| -donotmergecollision | | Disable simplification of collision meshes. In `pc_ModelMunge` this can be needed to stop it merging polygons that shouldn't be merged or from just taking __way__ too long to munge. WorldEdit's model munge however is strict with what it will simplify. It will only merge coplanar faces together, will always preserve the mesh's silhouette and doesn't have the same worst case performance as `pc_ModelMunge`. As a result this option is unlikely to be of much use when not using `pc_ModelMunge`. |
| -noprojectionlights | | Turn off projection lights for the model. |
| -ambientlighting | "r=[-1, 1] g=[-1, 1] b=[-1, 1]" | Adjust the vertex colours of the model. The vertex colours are normalized and then have the supplied values added to them so passing `-ambientlighting "r=-1 g=0.5 b=0"` will change a vertex colour of `R: 128 G: 0 B: 64` to `R: 0 G: 128 B: 64`. | 
| -attachlight | \<node light list> | Attach a light to a node's material, allowing the game to sync the material's brightness to that of the light. i.e. `-attachlight "blinking_node_01 blinking_light_01"`.  Crucially this does not create a light, the light must already exist in the world by other means. |
| -largetexcoords | | Enables the use of floating point texture coordinates in the model's vertex buffers. Allows having texture coordinates outside the range [-16, 15.999512]. Increases munged model size and causes incompatibility with Shader Patch versions earlier than v1.10, if that matters to you. |

Note: The `-righthanded` option listed in "misc_documentation.doc" is unsupported.

## Errors
Below is a list of errors the model munger may produce.

### MSH_READ_VERSION_NOT_SUPPORTED

The .msh file can not be read as it contains a MSH1 chunk instead of a MSH2 chunk.

### MSH_READ_MNDX_MISSING

The model index (MNDX chunk) was missing from a node. The .msh file can not be read correctly without it.

### MSH_READ_MNDX_DUPLICATE

Two nodes share the same model index (MNDX chunk).

### MSH_READ_ENVL_ENTRY_OUT_OF_RANGE

An entry in an ENVL chunk references a node with an index greater than any present node.

### MSH_READ_ENVL_ENTRY_MISSING_NODE

An entry in an ENVL chunk references a missing node.

### MSH_VALIDATION_FAIL_NOT_EMPTY

The .msh file scene contained no nodes.

### MSH_VALIDATION_FAIL_NODE_NAME_UNIQUE

Each node in the .msh file must have a unique name.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_NODE_TYPE_VALID

A node has an unknown type.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_NODE_PARENTS_VALID

A node references a missing parent node. 

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_NODE_PARENTS_NONCIRCULAR

A node has a circular relationship with an ancestor/parent.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_NODE_MESH_DATA

No node in the .msh file contains mesh data. This includes regular meshes, cloth and collision primitives.

### MSH_VALIDATION_FAIL_GEOMETRY_SEGMENT_MATERIAL_INDEX

A geometry segment references has an invalid material index.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_GEOMETRY_SEGMENT_ATTIBUTES_COUNT_MATCHES

A geometry segment's vertex attributes have mismatches counts. I.e 4 positions and only 3 normals.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_GEOMETRY_SEGMENT_VERTEX_COUNT_LIMIT

A geometry segment has too many vertices. .msh files are limited to 32,768 vertices in each geometry segment.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_GEOMETRY_SEGMENT_TRIANGLES_INDEX_VALID

A geometry segment's triangle reference invalid vertices.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_GEOMETRY_SEGMENT_NO_NANS

A vertex position in a geometry segment has a NaN (Not a Number). This is invalid and can cause issues with modelmunge.

If you still have the source files for this mesh try cleaning it of degenerate faces and exporting again.

### MSH_VALIDATION_FAIL_GEOMETRY_SEGMENT_WEIGHTS_BONE_INDICES_VALID

A vertex weight in a geometry segment contains a bone index that is out of range of the bone map/envelope.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_SHADOW_VOLUME_EDGES_VALID

The shadow volume half edge list is invalid.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_CLOTH_ATTIBUTES_COUNT_MATCHES

The position and texcoords counts for a cloth segment is mismatched.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_CLOTH_FIXED_INDEX_VALID

A fixed index entry in a cloth segment does not reference a valid cloth vertex.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_CLOTH_FIXED_WEIGHT_VALID

A fixed weight entry in a cloth segment is invalid.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_CLOTH_TRIANGLES_INDEX_VALID

A triangle in a cloth segment references an invalid vertex.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_CLOTH_CONSTRAINTS_VALID

A constraint entry in a cloth segment is invalid.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_CLOTH_COLLISION_PARENT_VALID

A cloth collision primitive does not reference a valid parent.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message printed with the error may have more information and context.

### MSH_VALIDATION_FAIL_CLOTH_COLLISION_SHAPE_VALID

A cloth collision primitive does not have a valid shape.

You might able to use an external tool (like Softimage or Blender) to repair the file.

The Exception Message printed with the error may have more information and context.


### MSH_UCFB_MEMORY_TOO_SMALL_MINIMUM

The size of the the .msh file is too small to be a valid .msh file.

### MSH_UCFB_MEMORY_TOO_SMALL_CHUNK

The .msh file is too small to fit it's root chunk in.

### MSH_UCFB_CHUNK_READ_CHILD_ID_MISMATCH

This error shouldn't typically be encountered. The Exception Message printed with the error may have more information and context but it is likely this indicates memory corruption or a bug in WorldEdit.

### MSH_UCFB_CHUNK_READ_OVERRUN

A chunk in the .msh file was too short.

The Exception Message printed with the error may have more information and context.

### MSH_UCFB_STRICT_READER_ID_MISMATCH

This error shouldn't typically be encountered. The Exception Message printed with the error may have more information and context but it is likely this indicates memory corruption or a bug in WorldEdit.

### MSH_UCFB_UNKNOWN

An unknow error occured while reading the .msh file's structure.

### MSH_OPTION_LOAD_BAD_SCALE

Bad "-scale" option in .msh.option file. "-scale" must be followed by a non-negative float setting the scale to use.

i.e "-scale 0.125"

### MSH_OPTION_LOAD_BAD_MAX_BONES

Bad "-maxbones" option in .msh.option file. "-scale" must be followed by an unsigned integer setting the max bones per-segment.

i.e "-maxbones 8"

### MSH_option_load_bad_lod_group

Bad "-lodgroup" option in .msh.option file. "-lodgroup" must be followed by the lodgroup to use.

i.e "-lodgroup bigmodel"

Valid Lod Groups:

model
bigmodel
soldier
hugemodel

### MSH_OPTION_LOAD_BAD_LOD_BIAS

Bad "-lodbias" option in .msh.option file. "-lodbias" must be followed by a non-negative float setting the lod bias.

i.e "-lodbias 2.0"

### MSH_OPTION_LOAD_BAD_HI_RES_SHADOW

Bad "-hiresshadow" option in .msh.option file. When "-hiresshadow" is followed by an argument it must be an unsigned integer setting the LOD to use for the shadow.

i.e "-hiresshadow 1"

### MSH_OPTION_LOAD_BAD_BOUNDING_BOX_SCALE

Bad "-boundingboxscale" option in .msh.option file. "-boundingboxscale" must be followed by a non-negative float setting the scale for the bounding sphere.

i.e "-boundingboxscale 2.0"

### MSH_OPTION_LOAD_BAD_BOUNDING_BOX_OFFSET

Base "-boundingboxoffsetx", "-boundingboxoffsety", "-boundingboxoffsetz" or "-boundingboxoffsetnz". These options must be followed by a non-negative float setting the offset for the bounding sphere.

i.e "-boundingboxoffsety 2.0"

### MSH_OPTION_LOAD_BAD_AMBIENT_LIGHTING

Bad "-ambientlighting" option in .msh.option file. "-ambientlighting" must be followed by a string in quotes of the form "r=[-1, 1] g=[-1, 1] b=[-1, 1]".

i.e "-ambientlighting "r=1 g=-0.5 b=2.0""

### MSH_OPTION_LOAD_BAD_ATTACH_LIGHT

Bad "-attachlight" option in .msh.option file. "-attachlight" must be followed by a quoted string with the name of a the node to attach the light to and the name of the light to attach.

i.e "-attachlight "blinking_node_01 blinking_light_01""

### MSH_OPTION_LOAD_IO_OPEN_ERROR

Failed to open the .msh.option file.

The Exception Message printed with the error may have more information and context.

### MSH_OPTION_LOAD_IO_GENERIC_ERROR

Failed to read the .msh.option file.

The Exception Message may have more information.

### MSH_UNKNOWN

An unknown error occured while reading the .msh file.

### SKELETON_TOO_MANY_BONES

Resulting skeleton from .msh bones and kept nodes has too many bones.

### MODEL_SPLIT_SEGMENT_UNSKINNED

Attempt to split an unskinned segment.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.

### MODEL_SPLIT_SEGMENT_BAD_VERTEX_COUNT

A segment has 0 vertices after being split.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.

### MODEL_SPLIT_SEGMENT_UNEXPECTED_FAILURE

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.

### MODEL_GENERATE_TANGENTS_MISSING_INPUTS

A mesh segment is missing vertex attributes or triangles needed to generate tangents for normal mapping.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.

### MODEL_GENERATE_TANGENTS_UNKNOWN_FAILURE

This indicates an unexpected failure in generating mesh tangents for normal mapping. Please report this along with the .msh you're trying to munge

### MODEL_GENERATE_TANGENTS_BAD_VERTEX_COUNT

A segment unexpected has 0 vertices after generating mesh tangents.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.

### MODEL_OPTIMIZE_MERGE_SEGMENTS_BAD_VERTEX_COUNT

Attempt to merge segments with 0 total vertices.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.

### MODEL_OPTIMIZE_VERTEX_CACHE_BAD_TRIANGLE_COUNT

Attempt to optimize vertex buffer of mesh segment with no triangles.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.

### MODEL_OPTIMIZE_VERTEX_CACHE_BAD_VERTEX_COUNT

Attempt to optimize empty vertex buffer.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.

### MODEL_OPTIMIZE_VERTEX_FETCH_REORDER_BAD_VERTEX_COUNT

A vertex buffer has 0 vertices after optimization. This should be impossible.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.

### MODEL_SHADOW_MESH_MISSING_INPUTS

Attempt to build shadow mesh without vertex positions.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.

### MODEL_SHADOW_MESH_UNEXPECTED_FAILURE

Unexpected failure while building final shadow mesh buffers.

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.

### COLLISION_MESH_BAD_VERTEX_COUNT

Attempt to munge a collision mesh with no vertices.

### COLLISION_MESH_BAD_TRIANGLE_COUNT

Attempt to munge a collision mesh with no triangles

### COLLISION_MESH_TOO_MANY_VERTICES

Attempt to munge a collison mesh with too many vertices.

### COLLISION_MESH_SIMPLIFIED_FACE_INVALID

The mesh simplifer for collision has returned an invalid face (one with 0 vertices).

This is an internal error and likely indicates a bug. Please report this along with the .msh you're trying to munge.

### REQ_WRITE_IO_OPEN_ERROR

Failed to open .model.req file for output! Make sure the munge out directory is accessible and that nothing currently has the output .model.req file open.

### REQ_WRITE_IO_GENERIC_ERROR

An unexpected IO error has occured while writing the model's .model.req file. The Exception Message printed with the error may have more information and context.

### WRITE_IO_OPEN_ERROR

Failed to open .model file for output.

The Exception Message printed with the error may have more information and context.

### WRITE_IO_GENERIC_ERROR

Failed to write to .model file.

The Exception Message printed with the error may have more information and context.

## Warnings
Below is a list of warnings the munger may produce.

### BONE_MAP_USED_UNKEPT_NODE

An envelope/bone map uses a node that is not a bone and has not been explicitly kept (using "-keep"/"-keepall").

This is valid but the entry in the bone map will instead point towards an unnamed bone with an identity transform. It is unlikely to be 
what you want.

### MATERIAL_INVALID_RENDERTYPE

A material uses an invalid rendertype. The material should still work ingame but is unlikely to be what you want.

### MISSING_KEEP_NODE

The node named in a "-keep" option does not exist in the .msh file. This could indicate a typo.

### MISSING_KEEP_MATERIAL

The node named in a "-keepmaterial" option does not exist in the .msh file. This could indicate a typo.

### MISSING_ATTACH_LIGHT

The node named in a "-attachlight" option does not exist in the .msh file. This could indicate a typo.

### MISSING_BUMP_MAP

The texture named in a "-bump" option is not used by any material. This could indicate a typo.

### POSSIBLE_TYPO_HP

A possible typo has been detected. Only nodes that start with lowercase "hp_" are treated as hardpoints, nodes that start with "HP_", "hP_" or "Hp_" are not.

If you want the node to be a hardpoint you should rename it or use "-keep" in the .msh.option file on it."

### POSSIBLE_TYPO_SHADOW_VOLUME

A possible typo has been detected. Only nodes that start with lowercase "sv_" are treated meshes to be converted to shadow volumes, nodes that start with "SV_", "sV_" or "Sv_" are not.

If you want the node to be a shadow volume you should rename it.

### UNHIDDEN_SHADOW_VOLUME

A shadow volume node is not hidden. The node will also be treated as a regular mesh.

If you want the node to only be a shadow volume you should hide it.

### MODEL_MESH_LARGE_TEXCOORDS

A vertex has large texture coordinates (outside the range [-16, 15.999512]). Consider using "-largetexcoords" to opt-in to floating point texture coordinates.

### MODEL_SHADOW_MESH_DISCARDED_DEGENERATE_TRIANGLE

A degenerate triangle has been discarded from an input mesh while constructing a shadow mesh.

### MODEL_SHADOW_MESH_MERGED_NONIDENTICAL_VERTICES

Merged non-identical vertices while constructing a shadow mesh. This can typically be safely ignored but you may want to simplify your input mesh.

### MODEL_SHADOW_MESH_MERGED_NONIDENTICAL_VERTICES_BONE_WEIGHTS

Merged vertices with non-identical bone indices while constructing a shadow mesh. This is rare and may indicate you should simplify your input mesh.

### MODEL_SHADOW_MESH_MERGED_NONIDENTICAL_VERTICES_BONE_INDICES

Merged vertices with non-identical bone weights while constructing a shadow mesh. This is rare and may indicate you should simplify your input mesh.

### MODEL_SHADOW_MESH_NON_MANIFOLD_EDGE_DETECTED

A non-manifold edge was detected while constructing a shadow mesh. You should make sure your input mesh is closed.

### MODEL_SHADOW_MESH_CLOSE_UNABLE_TO_LOOP_BACK_TO_START

Unable to gather holes needed to fill mesh holes. You should make sure your input mesh is closed.

### MODEL_SHADOW_MESH_CLOSE_STILL_OPEN_AFTER_FILL

The shadow mesh is still open after attempting to close holes. Falling back to brute force shadow mesh construction (this produces much more expensive shadow volumes at runtime). You should make sure your input mesh is closed to fix this.

### COLLISION_MESH_PARENT_NOT_ROOT

A collision mesh has been parented to a bone that is not the root bone. This is likely not intentional as ingame collision meshes can not be attached to bones.

### COLLISION_MESH_HAS_VERTEX_WEIGHTS

A collision mesh has vertex weights. This is likely not intentional as collision meshes can not be animated.

### COLLISION_PRIMITIVE_WRONG_PREFIX

A collision primitive must start with lowercase "p_". This node will be skipped.

### COLLISION_PRIMITIVE_MISSING_DATA

A node has the prefix for a collision primitive but contains to prmitive data.

### COLLISION_INVALID_FLAG

A node has an unexpected collision flag. Defaulting to all flags.

Valid Flags:
   s
   v
   b
   t
   o
   f

### CLOTH_FIXED_WEIGHT_UNKEPT_NODE

A cloth fixed weight entry references as unkept node. It is possible this will crash ingame. Either use -keep in the .msh.option file to force the node to be kept or edit the .msh file itself and make sure the referenced node is setup as a bone.
