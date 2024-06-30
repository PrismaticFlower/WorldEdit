# cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=DEBUG -DDIR_VCPKG_ROOT:PATH="<vcpkg-path>" -DVCPKG_MANIFEST_INSTALL:PATH="<vcpkg-path>/installed" -DVCPKG_TARGET_TRIPLET:STRING="x64-windows-static" -DCMAKE_TOOLCHAIN_FILE:PATH="<vcpkg-path>/scripts/buildsystems/vcpkg.cmake" -Wno-dev
project(WorldEdit)


#
# CMake
#

cmake_minimum_required(VERSION 3.23)


#
# Paths
#

set(DIR_ROOT "${PROJECT_SOURCE_DIR}")
set(DIR_LIBRARY "${DIR_ROOT}/lib")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${DIR_LIBRARY}/deb")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${DIR_LIBRARY}/deb")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${DIR_ROOT})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${DIR_LIBRARY}/rel")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${DIR_LIBRARY}/rel")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${DIR_ROOT})

set(DIR_VCPKG_INSTALL "${DIR_VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}")
set(DIR_VCPKG_INCLUDE "${DIR_VCPKG_INSTALL}/include")
set(DIR_VCPKG_LIBRARY "${DIR_VCPKG_INSTALL}/lib")
set(DIR_VCPKG_LIBRARY_DEBUG "${DIR_VCPKG_INSTALL}/debug/lib")


if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    set(LIBRARY_DIRECTORIES
            ${DIR_VCPKG_LIBRARY}
            ${DIR_VCPKG_LIBRARY}/deb)
else()
    set(LIBRARY_DIRECTORIES
            ${DIR_VCPKG_LIBRARY}
            ${DIR_VCPKG_LIBRARY}/rel)
endif()


#
# Compiler
#

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API 2182bf5c-ef0d-489a-91da-49dbc3090d2a)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") # Clang
    #
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") # GCC
    set(COMPILER_FLAGS "-Wall -Wextra -ansi -pedantic")
    set(COMPILER_FLAGS_DEBUG "-MTd")
    set(COMPILER_FLAGS_RELEASE "-O3 -DNDEBUG -MT")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") # MSVC
    set(COMPILER_FLAGS "")
    set(COMPILER_FLAGS_DEBUG "/Zi /Ob0 /Od /RTC1 /MTd")
    set(COMPILER_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG /MT")
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMPILER_FLAGS} ${COMPILER_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${COMPILER_FLAGS} ${COMPILER_FLAGS_RELEASE}")

add_compile_definitions(_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING)
add_compile_definitions(GLM_FORCE_SILENT_WARNINGS)
add_compile_definitions(NOMINMAX)
add_compile_definitions(WIN32_LEAN_AND_MEAN)
add_compile_definitions(WINVER=0x0A00)
add_compile_definitions(_WIN32_WINNT=0x0A00)

add_compile_definitions(_CONSOLE)

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    add_compile_definitions(_DEBUG)
endif()

#
# Linker & Dependencies
#

link_directories(${DIR_LIBRARY} ${LIBRARY_DIRECTORIES})

find_package(absl CONFIG REQUIRED)
find_package(directxmath CONFIG REQUIRED)
find_package(directxtex CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(mimalloc CONFIG REQUIRED)
find_package(wil CONFIG REQUIRED)


#
# Includes
#

include_directories(src)
include_directories(third_party/dxc/inc)
include_directories(third_party/imgui)
include_directories(third_party/Fast-BVH/include)
include_directories(${DIR_VCPKG_INCLUDE}/directx)


#
# Sources
#

set(SRC_ALLOCATORS
        "src/allocators/aligned_allocator.hpp"
    )

set(SRC_ASSETS
        "src/assets/config/io.hpp"
        "src/assets/config/key_node.cpp"
        "src/assets/config/key_node.hpp"
        "src/assets/config/values.hpp"
        "src/assets/config/io.cpp"
        "src/assets/msh/mikktspace/mikktspace.h"
        "src/assets/msh/mikktspace/mikktspace.c"
        "src/assets/msh/flat_model_bvh.cpp"
        "src/assets/msh/flat_model_bvh.hpp"
        "src/assets/msh/generate_tangents.cpp"
        "src/assets/msh/generate_tangents.hpp"
        "src/assets/msh/material.hpp"
        "src/assets/msh/scene.cpp"
        "src/assets/msh/scene.hpp"
        "src/assets/msh/scene_io.cpp"
        "src/assets/msh/scene_io.hpp"
        "src/assets/msh/validate_scene.cpp"
        "src/assets/msh/validate_scene.hpp"
        "src/assets/msh/default_missing_scene.cpp"
        "src/assets/msh/default_missing_scene.hpp"
        "src/assets/msh/flat_model.cpp"
        "src/assets/msh/flat_model.hpp"
        "src/assets/odf/default_object_class_definition.cpp"
        "src/assets/odf/default_object_class_definition.hpp"
        "src/assets/odf/definition.hpp"
        "src/assets/odf/definition_io.cpp"
        "src/assets/odf/definition_io.hpp"
        "src/assets/odf/properties.cpp"
        "src/assets/odf/properties.hpp"
        "src/assets/req/io.hpp"
        "src/assets/req/requirement_list.hpp"
        "src/assets/req/io.cpp"
        "src/assets/sky/io.hpp"
        "src/assets/sky/sky.hpp"
        "src/assets/sky/io.cpp"
        "src/assets/terrain/terrain_io.cpp"
        "src/assets/terrain/terrain_io.hpp"
        "src/assets/terrain/terrain.hpp"
        "src/assets/terrain/terrain_collision.cpp"
        "src/assets/terrain/terrain_collision.hpp"
        "src/assets/texture/texture_format.hpp"
        "src/assets/texture/texture_io.cpp"
        "src/assets/texture/texture_io.hpp"
        "src/assets/texture/texture_transforms.cpp"
        "src/assets/texture/texture_transforms.hpp"
        "src/assets/texture/texture.cpp"
        "src/assets/texture/texture.hpp"
        "src/assets/texture/texture_format.cpp"
        "src/assets/asset_ref.hpp"
        "src/assets/asset_stable_string.hpp"
        "src/assets/asset_state.hpp"
        "src/assets/asset_traits.cpp"
        "src/assets/asset_traits.hpp"
        "src/assets/option_file.cpp"
        "src/assets/option_file.hpp"
        "src/assets/asset_libraries.cpp"
        "src/assets/asset_libraries.hpp"
        "src/assets/asset_ref.cpp"
        )

set(SRC_ASYNC
        "src/async/thread_pool.cpp"
        "src/async/thread_pool.hpp"
        "src/async/wait_all.hpp"
        "src/async/for_each.hpp"
        "src/async/get_all.hpp"
        )

set(SRC_CONTAINER
        "src/container/paged_stack.hpp"
        "src/container/ring_set.hpp"
        "src/container/slim_bitset.hpp"
        "src/container/dynamic_array_2d.hpp"
        "src/container/enum_array.hpp"
        )

set(SRC_EDITS
        "src/edits/edit.hpp"
        "src/edits/game_mode_link_layer.cpp"
        "src/edits/game_mode_link_layer.hpp"
        "src/edits/game_mode_unlink_layer.cpp"
        "src/edits/game_mode_unlink_layer.hpp"
        "src/edits/imgui_ext.hpp"
        "src/edits/insert_entity.cpp"
        "src/edits/insert_entity.hpp"
        "src/edits/insert_node.cpp"
        "src/edits/insert_node.hpp"
        "src/edits/insert_point.cpp"
        "src/edits/insert_point.hpp"
        "src/edits/set_value.cpp"
        "src/edits/set_value.hpp"
        "src/edits/stack.hpp"
        "src/edits/ui_action.hpp"
        "src/edits/add_game_mode.cpp"
        "src/edits/add_game_mode.hpp"
        "src/edits/add_layer.cpp"
        "src/edits/add_layer.hpp"
        "src/edits/add_property.cpp"
        "src/edits/add_property.hpp"
        "src/edits/add_world_req_entry.cpp"
        "src/edits/add_world_req_entry.hpp"
        "src/edits/add_world_req_list.cpp"
        "src/edits/add_world_req_list.hpp"
        "src/edits/bundle.cpp"
        "src/edits/bundle.hpp"
        "src/edits/creation_entity_set.cpp"
        "src/edits/creation_entity_set.hpp"
        "src/edits/delete_entity.cpp"
        "src/edits/delete_entity.hpp"
        "src/edits/delete_game_mode.cpp"
        "src/edits/delete_game_mode.hpp"
        "src/edits/delete_layer.cpp"
        "src/edits/delete_layer.hpp"
        "src/edits/delete_world_req_entry.cpp"
        "src/edits/delete_world_req_entry.hpp"
        "src/edits/delete_world_req_list.cpp"
        "src/edits/delete_world_req_list.hpp"
        )

set(SRC_GRAPHICS
        "src/graphics/gpu/detail/descriptor_heap.hpp"
        "src/graphics/gpu/detail/error.hpp"
        "src/graphics/gpu/detail/handle_packing.hpp"
        "src/graphics/gpu/detail/release_queue.hpp"
        "src/graphics/gpu/detail/command_allocator_pool.hpp"
        "src/graphics/gpu/detail/descriptor_allocator.hpp"
        "src/graphics/gpu/rhi.cpp"
        "src/graphics/gpu/rhi.hpp"
        "src/graphics/gpu/exception.hpp"
        "src/graphics/gpu/resource.hpp"
        "src/graphics/cull_objects.hpp"
        "src/graphics/dynamic_buffer_allocator.cpp"
        "src/graphics/dynamic_buffer_allocator.hpp"
        "src/graphics/frustum.cpp"
        "src/graphics/frustum.hpp"
        "src/graphics/geometric_shapes.cpp"
        "src/graphics/geometric_shapes.hpp"
        "src/graphics/imgui_renderer.cpp"
        "src/graphics/imgui_renderer.hpp"
        "src/graphics/light_clusters.cpp"
        "src/graphics/light_clusters.hpp"
        "src/graphics/material.cpp"
        "src/graphics/material.hpp"
        "src/graphics/meta_draw_batcher.cpp"
        "src/graphics/meta_draw_batcher.hpp"
        "src/graphics/model.cpp"
        "src/graphics/model.hpp"
        "src/graphics/model_manager.cpp"
        "src/graphics/model_manager.hpp"
        "src/graphics/pipeline_library.cpp"
        "src/graphics/pipeline_library.hpp"
        "src/graphics/profiler.cpp"
        "src/graphics/profiler.hpp"
        "src/graphics/renderer.cpp"
        "src/graphics/renderer.hpp"
        "src/graphics/root_signature_library.cpp"
        "src/graphics/root_signature_library.hpp"
        "src/graphics/sampler_list.hpp"
        "src/graphics/shader_library.cpp"
        "src/graphics/shader_library.hpp"
        "src/graphics/shader_list.cpp"
        "src/graphics/shader_list.hpp"
        "src/graphics/shadow_camera.cpp"
        "src/graphics/shadow_camera.hpp"
        "src/graphics/sky.cpp"
        "src/graphics/sky.hpp"
        "src/graphics/terrain.cpp"
        "src/graphics/terrain.hpp"
        "src/graphics/texture_manager.cpp"
        "src/graphics/texture_manager.hpp"
        "src/graphics/world_mesh_list.hpp"
        "src/graphics/camera.cpp"
        "src/graphics/camera.hpp"
        "src/graphics/copy_command_list_pool.cpp"
        "src/graphics/copy_command_list_pool.hpp"
        "src/graphics/cull_objects.cpp"
        )

set(SRC_IO
        "src/io/read_file.cpp"
        "src/io/read_file.hpp"
        "src/io/error.hpp"
        "src/io/output_file.cpp"
        "src/io/output_file.hpp"
        )

set(SRC_MATH
        "src/math/matrix_funcs.cpp"
        "src/math/matrix_funcs.hpp"
        "src/math/quaternion_funcs.hpp"
        "src/math/vector_funcs.hpp"
        "src/math/align.hpp"
        "src/math/bounding_box.cpp"
        "src/math/bounding_box.hpp"
        "src/math/intersectors.hpp"
        )

set(SRC_SETTINGS
        "src/settings/io.cpp"
        "src/settings/io.hpp"
        "src/settings/preferences.cpp"
        "src/settings/preferences.hpp"
        "src/settings/settings.cpp"
        "src/settings/settings.hpp"
        "src/settings/ui.hpp"
        "src/settings/camera.hpp"
        "src/settings/graphics.hpp"
        )

set(SRC_UCFB
        "src/ucfb/reader.hpp"
        )

set(SRC_UTILITY
        "src/utility/float16_packing.cpp"
        "src/utility/float16_packing.hpp"
        "src/utility/implementation_storage.hpp"
        "src/utility/look_for.hpp"
        "src/utility/make_from_bytes.hpp"
        "src/utility/make_range.hpp"
        "src/utility/os_execute.cpp"
        "src/utility/os_execute.hpp"
        "src/utility/overload.hpp"
        "src/utility/srgb_conversion.hpp"
        "src/utility/stopwatch.hpp"
        "src/utility/string_icompare.cpp"
        "src/utility/string_icompare.hpp"
        "src/utility/string_ops.cpp"
        "src/utility/string_ops.hpp"
        "src/utility/binary_reader.hpp"
        "src/utility/com_ptr.hpp"
        "src/utility/command_line.hpp"
        "src/utility/enum_bitflags.hpp"
        "src/utility/event.hpp"
        "src/utility/event_listener.hpp"
        "src/utility/file_pickers.cpp"
        "src/utility/file_pickers.hpp"
        "src/utility/file_watcher.cpp"
        "src/utility/file_watcher.hpp"
        )

set(SRC_WORLD
        "src/world/utility/object_properties.cpp"
        "src/world/utility/object_properties.hpp"
        "src/world/utility/path_properties.cpp"
        "src/world/utility/path_properties.hpp"
        "src/world/utility/raycast.cpp"
        "src/world/utility/raycast.hpp"
        "src/world/utility/region_properties.cpp"
        "src/world/utility/region_properties.hpp"
        "src/world/utility/sector_fill.cpp"
        "src/world/utility/sector_fill.hpp"
        "src/world/utility/snapping.cpp"
        "src/world/utility/snapping.hpp"
        "src/world/utility/world_utilities.cpp"
        "src/world/utility/world_utilities.hpp"
        "src/world/utility/boundary_nodes.cpp"
        "src/world/utility/boundary_nodes.hpp"
        "src/world/utility/hintnode_traits.cpp"
        "src/world/utility/hintnode_traits.hpp"
        "src/world/utility/make_command_post_linked_entities.cpp"
        "src/world/utility/make_command_post_linked_entities.hpp"
        "src/world/barrier.hpp"
        "src/world/boundary.hpp"
        "src/world/game_mode_description.hpp"
        "src/world/global_lights.hpp"
        "src/world/hintnode.hpp"
        "src/world/id.hpp"
        "src/world/interaction_context.cpp"
        "src/world/interaction_context.hpp"
        "src/world/layer_description.hpp"
        "src/world/light.hpp"
        "src/world/object.hpp"
        "src/world/object_class.cpp"
        "src/world/object_class.hpp"
        "src/world/object_class_library.cpp"
        "src/world/object_class_library.hpp"
        "src/world/object_instance_property.hpp"
        "src/world/path.hpp"
        "src/world/planning.hpp"
        "src/world/portal.hpp"
        "src/world/region.hpp"
        "src/world/requirement_list.hpp"
        "src/world/sector.hpp"
        "src/world/terrain.hpp"
        "src/world/tool_visualizers.hpp"
        "src/world/world.hpp"
        "src/world/world_io_load.cpp"
        "src/world/world_io_load.hpp"
        "src/world/world_io_save.cpp"
        "src/world/world_io_save.hpp"
        "src/world/active_elements.hpp"
        "src/world/ai_path_flags.hpp"
        )

SET(SRC_ROOT
        "src/gizmo.cpp"
        "src/gizmo.hpp"
        "src/hotkeys.cpp"
        "src/hotkeys.hpp"
        "src/hotkeys_io.cpp"
        "src/hotkeys_io.hpp"
        "src/imgui_ext.cpp"
        "src/imgui_ext.hpp"
        "src/key.cpp"
        "src/key.hpp"
        "src/lowercase_string.hpp"
        "src/output_stream.cpp"
        "src/output_stream.hpp"
        "src/types.hpp"
        "src/commands.cpp"
        "src/commands.hpp"
        )

SET(SOURCES_IMGUI
        "third_party/imgui/backends/imgui_impl_win32.cpp"
        "third_party/imgui/misc/cpp/imgui_stdlib.cpp"
        "third_party/imgui/misc/freetype/imgui_freetype.cpp"
        "third_party/imgui/imgui_internal.h"
        "third_party/imgui/imgui.cpp"
        "third_party/imgui/imgui_demo.cpp"
        "third_party/imgui/imgui_draw.cpp"
        "third_party/imgui/imgui_tables.cpp"
        "third_party/imgui/imgui_widgets.cpp"
        )

SET(SOURCES_APP
        "app/src/dx_agility_sdk_exports.cpp"
        "app/src/main.cpp"
        "app/src/new_delete_override.cpp"
        "app/src/resource.h"
        "app/src/world_edit.hpp"
        "app/src/world_edit_commands.cpp"
        "app/src/world_edit_ui.cpp"
        "app/src/world_edit.cpp"
        ${SOURCES_IMGUI}
        )

SET(SOURCES_LIB
        ${SRC_ALLOCATORS}
        ${SRC_ASSETS}
        ${SRC_ASYNC}
        ${SRC_CONTAINER}
        ${SRC_EDITS}
        ${SRC_GRAPHICS}
        ${SRC_IO}
        ${SRC_MATH}
        ${SRC_SETTINGS}
        ${SRC_UCFB}
        ${SRC_UTILITY}
        ${SRC_WORLD}
        ${SRC_ROOT}
        ${SOURCES_IMGUI}
    )


#
# Filters
#

source_group("allocators" FILES ${SRC_ALLOCATORS})
source_group("assets" FILES ${SRC_ASSETS})
source_group("async" FILES ${SRC_ASYNC})
source_group("container" FILES ${SRC_CONTAINER})
source_group("edits" FILES ${SRC_EDITS})
source_group("graphics" FILES ${SRC_GRAPHICS})
source_group("io" FILES ${SRC_IO})
source_group("math" FILES ${SRC_MATH})
source_group("settings" FILES ${SRC_SETTINGS})
source_group("ucfb" FILES ${SRC_UCFB})
source_group("utility" FILES ${SRC_UTILITY})
source_group("world" FILES ${SRC_WORLD})
source_group("root" FILES ${SRC_ROOT})


#
# Builds
#

# Build and link 'WorldEdit' library
add_library(${PROJECT_NAME}_library ${SOURCES_LIB})
target_link_libraries(${PROJECT_NAME}_library PRIVATE
        absl::algorithm
        absl::base
        absl::debugging
        absl::flat_hash_map
        absl::flags
        absl::memory
        absl::meta
        absl::numeric
        absl::random_random
        absl::strings
        absl::synchronization
        absl::time
        absl::utility
    )
target_link_libraries(${PROJECT_NAME}_library PRIVATE Microsoft::DirectXMath)
target_link_libraries(${PROJECT_NAME}_library PRIVATE Microsoft::DirectXTex)
target_link_libraries(${PROJECT_NAME}_library PRIVATE fmt::fmt)
target_link_libraries(${PROJECT_NAME}_library PRIVATE Freetype::Freetype)
target_link_libraries(${PROJECT_NAME}_library PRIVATE mimalloc mimalloc-static)
target_link_libraries(${PROJECT_NAME}_library PRIVATE WIL::WIL)
target_link_libraries(${PROJECT_NAME}_library PRIVATE dxcompiler.lib
        d3d12.lib
        dxgi.lib
        kernel32.lib
        user32.lib
        gdi32.lib
        winspool.lib
        comdlg32.lib
        advapi32.lib
        shell32.lib
        ole32.lib
        oleaut32.lib
        uuid.lib
        odbc32.lib
        odbccp32.lib
        dxguid.lib)

# Build and link 'WorldEdit' binary
add_executable(${PROJECT_NAME} ${SOURCES_APP})
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_library)
target_sources(${PROJECT_NAME} PRIVATE app/src/resource.rc)


#
# Info
#

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    message("Build with: ${CMAKE_CXX_FLAGS_DEBUG} ${COMPILER_FLAGS} ${COMPILER_FLAGS_DEBUG}")
else()
    message("Build with: ${CMAKE_CXX_FLAGS_RELEASE} ${COMPILER_FLAGS} ${COMPILER_FLAGS_RELEASE}")
endif()
