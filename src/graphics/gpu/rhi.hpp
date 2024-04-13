#pragma once

#include "types.hpp"
#include "utility/enum_bitflags.hpp"
#include "utility/implementation_storage.hpp"

#include <array>
#include <concepts>
#include <cstddef>
#include <span>
#include <string_view>

#include <dxgiformat.h>

namespace we::graphics::gpu {

/// Constants ///

constexpr inline uint32 frame_pipeline_length = 2;
constexpr inline uint32 max_root_parameters = 6;
constexpr inline uint32 max_root_static_samplers = 16;
constexpr inline uint32 append_aligned_input_element = 0xffffffffu;
constexpr inline uint32 texture_barrier_all_subresources = 0xffffffffu;
constexpr inline uint32 raw_uav_srv_byte_alignment = 16;
constexpr inline uint32 constant_buffer_data_placement_alignment = 256;
constexpr inline uint32 texture_data_pitch_alignment = 256;
constexpr inline uint32 texture_data_placement_alignment = 512;

/// Enums & Typedefs ///

using gpu_virtual_address = uint64;

enum class gpu_preference {
   unspecified = 0,
   minimum_power = 1,
   high_performance = 2
};

enum class heap_type {
   default_ = 1,
   upload = 2,
   readback = 3,
};

enum class texture_dimension { t_1d = 2, t_2d = 3, t_3d = 4 };

enum class uav_dimension {
   buffer = 1,
   texture1d = 2,
   texture1d_array = 3,
   texture2d = 4,
   texture2d_array = 5,
   texture3d = 8
};

enum class rtv_dimension {
   texture1d = 2,
   texture1d_array = 3,
   texture2d = 4,
   texture2d_array = 5,
   texture2d_ms = 6,
   texture2d_ms_array = 7,
   texture3d = 8
};

enum class dsv_dimension {
   texture1d = 1,
   texture1d_array = 2,
   texture2d = 3,
   texture2d_array = 4,
   texture2d_ms = 5,
   texture2d_ms_array = 6,
};

enum class root_shader_visibility {
   all = 0,
   vertex = 1,
   pixel = 5,
   amplification = 6,
   mesh = 7
};

enum class root_parameter_type {
   _32bit_constants,
   constant_buffer_view,
   shader_resource_view,
   unordered_access_view,
   unordered_access_view_resource_view,

   paramters_end
};

enum class render_target_blend {
   disabled,            // overwrite with src
   premult_alpha_blend, // src + (dest * (1.0 - src_alpha))
   additive_blend,      // src + dest
   alpha_belnd          // (src * src_alpha) + (dest * (1.0 - src_alpha))
};

enum class cull_mode {
   none = 1,
   front = 2, // Counter Clockwise
   back = 3,  // Clockwise
};

enum class comparison_func {
   never = 1,
   less = 2,
   equal = 3,
   less_equal = 4,
   greater = 5,
   not_equal = 6,
   greater_equal = 7,
   always = 8
};

enum class stencil_op {
   keep = 1,
   zero = 2,
   replace = 3,
   incr_sat = 4,
   decr_sat = 5,
   invert = 6,
   incr = 7,
   decr = 8
};

enum class primitive_type {
   point = 1,
   line = 2,
   triangle = 3,
};

enum class primitive_topology {
   undefined = 0,
   pointlist = 1,
   linelist = 2,
   trianglelist = 4
};

enum class barrier_layout : uint32 {
   undefined = 0xffffffff,
   common = 0,
   present = 0,
   generic_read,
   render_target,
   unordered_access,
   depth_stencil_write,
   depth_stencil_read,
   shader_resource,
   copy_source,
   copy_dest,
   resolve_source,
   resolve_dest,
   shading_rate_source,
   video_decode_read,
   video_decode_write,
   video_process_read,
   video_process_write,
   video_encode_read,
   video_encode_write,
   direct_queue_common,
   direct_queue_generic_read,
   direct_queue_unordered_access,
   direct_queue_shader_resource,
   direct_queue_copy_source,
   direct_queue_copy_dest,
   compute_queue_common,
   compute_queue_generic_read,
   compute_queue_unordered_access,
   compute_queue_shader_resource,
   compute_queue_copy_source,
   compute_queue_copy_dest,
   video_queue_common
};

enum class barrier_sync : uint32 {
   none = 0,
   all = 0x1,
   draw = 0x2,
   input_assembler = 0x4,
   vertex_shading = 0x8,
   pixel_shading = 0x10,
   depth_stencil = 0x20,
   render_target = 0x40,
   compute_shading = 0x80,
   raytracing = 0x100,
   copy = 0x200,
   resolve = 0x400,
   execute_indirect = 0x800,
   predication = 0x800,
   all_shading = 0x1000,
   non_pixel_shading = 0x2000,
   emit_raytracing_acceleration_structure_postbuild_info = 0x4000,
   clear_unordered_access_view = 0x8000,
   video_decode = 0x100000,
   video_process = 0x200000,
   video_encode = 0x400000,
   build_raytracing_acceleration_structure = 0x800000,
   copy_raytracing_acceleration_structure = 0x1000000,
   split = 0x80000000
};

constexpr bool marked_as_enum_bitflag(barrier_sync) noexcept
{
   return true;
}

enum class barrier_access : uint32 {
   common = 0,
   vertex_buffer = 0x1,
   constant_buffer = 0x2,
   index_buffer = 0x4,
   render_target = 0x8,
   unordered_access = 0x10,
   depth_stencil_write = 0x20,
   depth_stencil_read = 0x40,
   shader_resource = 0x80,
   stream_output = 0x100,
   indirect_argument = 0x200,
   predication = 0x200,
   copy_dest = 0x400,
   copy_source = 0x800,
   resolve_dest = 0x1000,
   resolve_source = 0x2000,
   raytracing_acceleration_structure_read = 0x4000,
   raytracing_acceleration_structure_write = 0x8000,
   shading_rate_source = 0x10000,
   video_decode_read = 0x20000,
   video_decode_write = 0x40000,
   video_process_read = 0x80000,
   video_process_write = 0x100000,
   video_encode_read = 0x200000,
   video_encode_write = 0x400000,
   no_access = 0x80000000
};

constexpr bool marked_as_enum_bitflag(barrier_access) noexcept
{
   return true;
}

enum class texture_barrier_flags { none = 0, discard = 0x1 };

constexpr bool marked_as_enum_bitflag(texture_barrier_flags) noexcept
{
   return true;
}

enum class legacy_resource_state {
   common = 0,
   vertex_and_constant_buffer = 0x1,
   index_buffer = 0x2,
   render_target = 0x4,
   unordered_access = 0x8,
   depth_write = 0x10,
   depth_read = 0x20,
   non_pixel_shader_resource = 0x40,
   pixel_shader_resource = 0x80,
   stream_out = 0x100,
   indirect_argument = 0x200,
   copy_dest = 0x400,
   copy_source = 0x800,
   resolve_dest = 0x1000,
   resolve_source = 0x2000,
   raytracing_acceleration_structure = 0x400000,
   shading_rate_source = 0x1000000,
   generic_read = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
   all_shader_resource = (0x40 | 0x80),
   present = 0,
   predication = 0x200,
};

constexpr bool marked_as_enum_bitflag(legacy_resource_state) noexcept
{
   return true;
}

enum class filter {
   point = 0,
   bilinear = 0x14,
   trilinear = 0x15,
   anisotropic = 0x55,

   comparison_point = 0x80,
   comparison_bilinear = 0x94,
   comparison_trilinear = 0x95,
   comparison_anisotropic = 0xd5,

   minimum_point = 0x100,
   minimum_bilinear = 0x114,
   minimum_trilinear = 0x115,
   minimum_anisotropic = 0x155,

   maximum_point = 0x180,
   maximum_bilinear = 0x194,
   maximum_trilinear = 0x195,
   maximum_anisotropic = 0x1d5
};

enum class texture_address_mode {
   wrap = 1,
   mirror = 2,
   clamp = 3,
   border = 4,
   mirror_once = 5
};

enum class static_border_color {
   transparent_black = 0,
   opaque_black = 1,
   opaque_white = 2,
   opaque_black_uint = 3,
   opaque_white_uint = 4
};

enum class comparison_mode {
   equal = 3,
   not_equal = 6,
   less = 2,
   less_equal = 4,
   greater = 5,
   greater_equal = 7,
   always = 8,
};

struct memory_range {
   std::size_t begin = 0;
   std::size_t end = 0;
};

struct rect {
   uint32 left;
   uint32 top;
   uint32 right;
   uint32 bottom;
};

struct box {
   uint32 left;
   uint32 top;
   uint32 front;
   uint32 right;
   uint32 bottom;
   uint32 back;
};

// Internal forward declares.

struct device;
struct command_queue;

struct command_queue_init;
struct command_list_state;
struct command_queue_state;
struct swap_chain_state;
struct device_state;

/// GPU Data Handles ///

enum class resource_handle : std::uintptr_t {};
constexpr inline resource_handle null_resource_handle = resource_handle{0};

enum class rtv_handle : std::uintptr_t {};
constexpr inline auto null_rtv_handle = rtv_handle{0};

enum class dsv_handle : std::uintptr_t {};
constexpr inline auto null_dsv_handle = dsv_handle{0};

struct resource_view {
   uint32 index = 0xffffffffu;

   constexpr bool operator==(const resource_view&) const noexcept = default;
};

constexpr inline auto invalid_resource_view = resource_view{.index = 0xffffffffu};

enum class root_signature_handle : std::uintptr_t {};
constexpr inline auto null_root_signature_handle = root_signature_handle{0};

enum class pipeline_handle : std::uintptr_t {};
constexpr inline auto null_pipeline_handle = pipeline_handle{0};

enum class query_heap_handle : std::uintptr_t {};
constexpr inline auto null_query_heap_handle = query_heap_handle{0};

enum class command_allocator_handle : std::uintptr_t {};
constexpr inline auto null_command_allocator_handle = command_allocator_handle{0};

/// Pipeline Desc Structures ///

struct root_parameter {
   root_parameter_type type = root_parameter_type::paramters_end;
   uint32 shader_register = 0;
   uint32 register_space = 0;

   /// Number of 32-bit constants for root_parameter_type::_32bit_constants
   uint32 values_count = 0;

   root_shader_visibility visibility = root_shader_visibility::all;
};

struct root_signature_flags {
   bool allow_input_assembler_input_layout : 1 = false;
   bool deny_vertex_shader_root_access : 1 = false;
   bool deny_pixel_shader_root_access : 1 = false;
   bool deny_amplification_shader_root_access : 1 = false;
   bool deny_mesh_shader_root_access : 1 = false;
};

struct static_sampler_desc {
   filter filter = filter::point;

   union {
      texture_address_mode address_u, address = texture_address_mode::wrap;
   };

   texture_address_mode address_v = address;
   texture_address_mode address_w = address;
   float mip_lod_bias = 0.0f;
   uint32 max_anisotropy = 1;
   comparison_func comparison = comparison_func::never;
   static_border_color border_color = static_border_color::transparent_black;

   uint32 shader_register = 0;
   root_shader_visibility visibility = root_shader_visibility::all;
};

struct root_signature_desc {
   std::array<root_parameter, max_root_parameters> parameters;
   std::span<const static_sampler_desc> samplers;

   root_signature_flags flags;

   std::string_view debug_name;
};

struct blend_state_desc {
   bool alpha_to_coverage_enabled = false;
   bool independent_blend_enabled = false;

   std::array<render_target_blend, 8> render_target{};
};

struct rasterizer_state_desc {
   cull_mode cull_mode = cull_mode::back;

   int32 depth_bias = 0;

   bool antialiased_lines = false;
   bool conservative_raster = false;
};

struct stencil_op_desc {
   stencil_op fail_op = stencil_op::keep;
   stencil_op depth_fail_op = stencil_op::keep;
   stencil_op pass_op = stencil_op::keep;
   comparison_func func = comparison_func::always;
};

struct depth_stencil_state_desc {
   bool depth_test_enabled = false;
   comparison_func depth_test_func = comparison_func::less_equal;
   bool write_depth = true;
   bool stencil_enabled = false;
   uint8 stencil_read_mask = 0xffu;
   uint8 stencil_write_mask = 0xffu;
   stencil_op_desc stencil_front_face;
   stencil_op_desc stencil_back_face;
};

struct input_element_desc {
   const char* semantic_name = "";
   uint32 semantic_index = 0;
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   uint32 input_slot = 0;
   uint32 aligned_byte_offset = append_aligned_input_element;
};

struct graphics_pipeline_desc {
   root_signature_handle root_signature;
   std::span<const std::byte> vs_bytecode;
   std::span<const std::byte> ps_bytecode;
   std::span<const std::byte> gs_bytecode;

   blend_state_desc blend_state{};
   rasterizer_state_desc rasterizer_state{};
   depth_stencil_state_desc depth_stencil_state{};
   std::span<const input_element_desc> input_layout;
   primitive_type primitive_type = primitive_type::triangle;

   uint32 render_target_count = 0;
   std::array<DXGI_FORMAT, 8> rtv_formats{};
   DXGI_FORMAT dsv_format = DXGI_FORMAT_UNKNOWN;

   uint32 sample_count = 1;

   std::string_view debug_name;
};

struct compute_pipeline_desc {
   root_signature_handle root_signature;
   std::span<const std::byte> cs_bytecode;

   std::string_view debug_name;
};

/// Resource Desc Structures ///

struct buffer_flags {
   bool allow_unordered_access : 1 = false;
};

struct texture_flags {
   bool allow_render_target : 1 = false;
   bool allow_depth_stencil : 1 = false;
   bool allow_unordered_access : 1 = false;
   bool deny_shader_resource : 1 = false;
};

struct texture_clear_value {
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

   union {
      float4 color = {0.0f, 0.0f, 0.0f, 0.0f};

      struct depth_stencil {
         float depth = 1.0f;
         uint8 stencil = 0x0;
      } depth_stencil;
   };
};

struct buffer_desc {
   uint64 size = 0;
   buffer_flags flags{};

   std::string_view debug_name;
};

struct texture_desc {
   texture_dimension dimension = texture_dimension::t_1d;
   texture_flags flags = {};

   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   uint32 width = 1;
   uint32 height = 1;
   uint32 depth = 1;
   uint32 mip_levels = 1;
   uint32 array_size = 1;
   uint32 sample_count = 1;

   texture_clear_value optimized_clear_value = {.format = DXGI_FORMAT_UNKNOWN};

   std::string_view debug_name;
};

/// Resource View Desc Structures ///

enum class shader_component_mapping : uint32 {
   from_memory_component_0 = 0,
   from_memory_component_1 = 1,
   from_memory_component_2 = 2,
   from_memory_component_3 = 3,
   force_value_0 = 4,
   force_value_1 = 5
};

struct shader_4_component_mapping {
   shader_component_mapping component_0 : 3 =
      shader_component_mapping::from_memory_component_0;
   shader_component_mapping component_1 : 3 =
      shader_component_mapping::from_memory_component_1;
   shader_component_mapping component_2 : 3 =
      shader_component_mapping::from_memory_component_2;
   shader_component_mapping component_3 : 3 =
      shader_component_mapping::from_memory_component_3;
};

struct shader_resource_view_desc {
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   shader_4_component_mapping shader_4_component_mapping = {};

   struct {
      uint32 first_element = 0;
      uint32 number_elements = 0;
      uint32 structure_byte_stride = 0;
      bool raw_buffer = false;
   } buffer;

   bool texture_cube_view = false;
};

struct unordered_access_view_desc {
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   uav_dimension dimension = uav_dimension::buffer;

   union {
      struct buffer {
         uint32 first_element = 0;
         uint32 number_elements = 0;
         uint32 structure_byte_stride = 0;
         bool raw_buffer = true;

         /// @brief The resource handle for a counter for the UAV. Maybe null.
         resource_handle counter_resource = null_resource_handle;
         uint64 counter_offset_in_bytes = 0;
      } buffer = {};

      struct texture1d_uav {
         uint32 mip_slice = 0;
      } texture1d;

      struct texture1d_array_uav {
         uint32 mip_slice = 0;
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
      } texture1d_array;

      struct texture2d_uav {
         uint32 mip_slice = 0;
      } texture2d;

      struct texture2d_array_uav {
         uint32 mip_slice = 0;
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
      } texture2d_array;

      struct texture3d_uav {
         uint32 mip_slice = 0;
         uint32 first_w_slice = 0;
         uint32 w_size = 0;
      } texture3d;
   };
};

struct render_target_view_desc {
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   rtv_dimension dimension = rtv_dimension::texture2d;

   union {
      struct texture1d_rtv {
         uint32 mip_slice = 0;
      } texture1d;

      struct texture1d_array_rtv {
         uint32 mip_slice = 0;
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
      } texture1d_array;

      struct texture2d_rtv {
         uint32 mip_slice = 0;
      } texture2d;

      struct texture2d_array_rtv {
         uint32 mip_slice = 0;
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
      } texture2d_array;

      struct texture2d_ms_rtv {
      } texture2d_ms;

      struct texture2d_ms_array_rtv {
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
      } texture2d_ms_array;

      struct texture3d_rtv {
         uint32 mip_slice = 0;
         uint32 first_w_slice = 0;
         uint32 w_size = 0;
      } texture3d;
   };
};

struct depth_stencil_view_desc {
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   dsv_dimension dimension = dsv_dimension::texture2d;

   union {
      struct texture1d_dsv {
         uint32 mip_slice = 0;
      } texture1d;

      struct texture1d_array_dsv {
         uint32 mip_slice = 0;
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
      } texture1d_array;

      struct texture2d_dsv {
         uint32 mip_slice = 0;
      } texture2d;

      struct texture2d_array_dsv {
         uint32 mip_slice = 0;
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
      } texture2d_array;

      struct texture2d_ms_dsv {
         uint32 mip_slice = 0;
      } texture2d_ms;

      struct texture2d_ms_array_dsv {
         uint32 mip_slice = 0;
         uint32 first_array_slice = 0;
         uint32 array_size = 0;
      } texture2d_ms_array;
   };
};

/// Swapchain Desc Structure ///

struct swap_chain_desc {
   using window_handle = void*;

   /// @brief HWND for the swap chain. Can not be left nullptr. The size of the swap chain will be derived from the window size.
   window_handle window = nullptr;

   /// @brief The format to use for the swap chain buffers.
   DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM;

   /// @brief The format to use for the swap chain RTVs.
   DXGI_FORMAT format_rtv = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

   /// @brief Number of buffers in the swap chain.
   uint32 buffer_count = 2;

   /// @brief Maximum frame latency of the swap chain. Only has effect if frame_latency_waitable is true. https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_3/nf-dxgi1_3-idxgiswapchain2-setmaximumframelatency
   uint32 maximum_frame_latency = 1;

   /// @brief When true allows callling swap_chain::wait_for_ready to wait on the
   /// swap chain's frame latency waitable object. https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_3/nf-dxgi1_3-idxgiswapchain2-getframelatencywaitableobject
   bool frame_latency_waitable = true;

   /// @brief When true allows calling swap_chain::present with allow_tearing set to true.
   bool allow_tearing = false;
};

/// Barriers Structures ///

struct barrier_subresource_range {
   uint32 index_or_first_mip_level = texture_barrier_all_subresources;
   uint32 num_mip_levels = 0;
   uint32 first_array_slice = 0;
   uint32 num_array_slices = 0;
   uint32 first_plane = 0;
   uint32 num_planes = 0;
};

struct global_barrier {
   barrier_sync sync_before;
   barrier_sync sync_after;
   barrier_access access_before;
   barrier_access access_after;
};

struct texture_barrier {
   barrier_sync sync_before;
   barrier_sync sync_after;
   barrier_access access_before;
   barrier_access access_after;
   barrier_layout layout_before;
   barrier_layout layout_after;
   resource_handle resource;
   barrier_subresource_range subresources{};
   texture_barrier_flags flags = texture_barrier_flags::none;
};

struct buffer_barrier {
   barrier_sync sync_before;
   barrier_sync sync_after;
   barrier_access access_before;
   barrier_access access_after;
   resource_handle resource;
   uint64 offset = 0;
   uint64 size = UINT64_MAX;
};

/// Legacy Barrier Structures ///

struct legacy_resource_transition_barrier {
   resource_handle resource;
   uint32 subresource = texture_barrier_all_subresources;
   legacy_resource_state state_before;
   legacy_resource_state state_after;
};

struct legacy_resource_aliasing_barrier {
   resource_handle resource_before;
   resource_handle resource_after;
};

struct legacy_resource_uav_barrier {
   resource_handle resource;
};

/// Command List Structures and Definitions ///

struct command_list_desc {
   std::string_view debug_name = "";
};

struct texture_copy_buffer_footprint {
   uint64 offset = 0;
   DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
   uint32 width = 1;
   uint32 height = 1;
   uint32 depth = 1;
   uint32 row_pitch = 0;
};

struct index_buffer_view {
   gpu_virtual_address buffer_location = 0;
   uint32 size_in_bytes = 0;
   constexpr static DXGI_FORMAT format = DXGI_FORMAT_R16_UINT;
};

struct vertex_buffer_view {
   gpu_virtual_address buffer_location = 0;
   uint32 size_in_bytes = 0;
   uint32 stride_in_bytes = 0;
};

struct viewport {
   float top_left_x = 0.0f;
   float top_left_y = 0.0f;
   float width;
   float height;
   float min_depth = 0.0f;
   float max_depth = 1.0f;
};

struct depth_stencil_clear_flags {
   bool clear_depth : 1 = true;
   bool clear_stencil : 1 = true;
};

/// Command List Definitions ///

struct command_list {
   void close();

   void reset();

   void clear_state();

   /// Constructors/Destructor ///

   command_list();

   ~command_list();

   command_list(command_list&& other) noexcept;

   auto operator=(command_list&& other) noexcept -> command_list&;

   command_list(const command_list& other) noexcept = delete;

   auto operator=(const command_list& other) noexcept -> command_list& = delete;

protected:
   void reset_common();

   friend device;
   friend command_queue;

   implementation_storage<command_list_state, 216> state;
};

struct copy_command_list : command_list {
   void deferred_barrier(const std::span<const global_barrier> barriers);

   void deferred_barrier(const global_barrier& barrier);

   void deferred_barrier(const std::span<const texture_barrier> barriers);

   void deferred_barrier(const texture_barrier& barrier);

   void deferred_barrier(const std::span<const buffer_barrier> barriers);

   void deferred_barrier(const buffer_barrier& barrier);

   void deferred_barrier(const std::span<const legacy_resource_transition_barrier> barriers);

   void deferred_barrier(const legacy_resource_transition_barrier& barrier);

   void deferred_barrier(const std::span<const legacy_resource_aliasing_barrier> barriers);

   void deferred_barrier(const legacy_resource_aliasing_barrier& barrier);

   void deferred_barrier(const std::span<const legacy_resource_uav_barrier> barriers);

   void deferred_barrier(const legacy_resource_uav_barrier& barrier);

   void flush_barriers();

   void copy_buffer_region(const resource_handle dst_buffer,
                           const uint64 dst_offset, const resource_handle src_buffer,
                           const uint64 src_offset, const uint64 num_bytes);

   void copy_buffer_to_texture(resource_handle dst_texture, const uint32 dst_subresource,
                               const uint32 dst_x, const uint32 dst_y,
                               const uint32 dst_z, resource_handle src_buffer,
                               const texture_copy_buffer_footprint& src_footprint,
                               const box* const src_box = nullptr);

   void copy_texture_to_buffer(resource_handle dst_buffer,
                               const texture_copy_buffer_footprint& dst_footprint,
                               const uint32 dst_x, const uint32 dst_y,
                               const uint32 dst_z, resource_handle src_texture,
                               const uint32 src_subresource,
                               const box* const src_box = nullptr);

   void copy_texture_region(const resource_handle dst_texture,
                            const uint32 dst_subresource, const uint32 dst_x,
                            const uint32 dst_y, const uint32 dst_z,
                            const resource_handle src_texture,
                            const uint32 src_subresource,
                            const box* const src_box = nullptr);

   void copy_resource(resource_handle dst_resource, resource_handle src_resource);

   void write_buffer_immediate(const gpu_virtual_address address, const uint32 value);
};

struct compute_command_list : copy_command_list {
   void dispatch(const uint32 thread_group_count_x, const uint32 thread_group_count_y,
                 const uint32 thread_group_count_z);

   void set_pipeline_state(const pipeline_handle pipeline);

   void set_compute_root_signature(const root_signature_handle root_signature);

   void set_compute_32bit_constant(const uint32 parameter_index, const uint32 constant,
                                   const uint32 dest_offset_in_32bit_values);

   void set_compute_32bit_constants(const uint32 parameter_index,
                                    const std::span<const std::byte> constants,
                                    const uint32 dest_offset_in_32bit_values);

   void set_compute_cbv(const uint32 parameter_index,
                        const gpu_virtual_address buffer_location);

   void set_compute_srv(const uint32 parameter_index,
                        const gpu_virtual_address buffer_location);

   void set_compute_uav(const uint32 parameter_index,
                        const gpu_virtual_address buffer_location);

   void set_compute_uav_resource_view(const uint32 parameter_index,
                                      const resource_view view);

   void discard_resource(const resource_handle resource);

   void query_timestamp(const query_heap_handle query_heap, const uint32 query_index);

   void resolve_query_timestamp(const query_heap_handle query_heap,
                                const uint32 start_query, const uint32 query_count,
                                const resource_handle destination_buffer,
                                const uint32 aligned_destination_buffer_offset);

   friend device;
};

struct graphics_command_list : compute_command_list {
   void draw_instanced(const uint32 vertex_count_per_instance,
                       const uint32 instance_count, const uint32 start_vertex_location,
                       const uint32 start_instance_location);

   void draw_indexed_instanced(const uint32 index_count_per_instance,
                               const uint32 instance_count,
                               const uint32 start_index_location,
                               const int32 base_vertex_location,
                               const uint32 start_instance_location);

   void resolve_subresource(resource_handle dst_resource,
                            const uint32 dst_subresource, resource_handle src_resource,
                            const uint32 src_subresource, const DXGI_FORMAT format);

   void set_graphics_root_signature(const root_signature_handle root_signature);

   void set_graphics_32bit_constant(const uint32 parameter_index, const uint32 constant,
                                    const uint32 dest_offset_in_32bit_values);

   void set_graphics_32bit_constants(const uint32 parameter_index,
                                     const std::span<const std::byte> constants,
                                     const uint32 dest_offset_in_32bit_values);

   void set_graphics_cbv(const uint32 parameter_index,
                         const gpu_virtual_address buffer_location);

   void set_graphics_srv(const uint32 parameter_index,
                         const gpu_virtual_address buffer_location);

   void set_graphics_uav(const uint32 parameter_index,
                         const gpu_virtual_address buffer_location);

   void set_graphics_uav_resource_view(const uint32 parameter_index,
                                       const resource_view view);

   void ia_set_primitive_topology(const primitive_topology primitive_topology);

   void ia_set_index_buffer(const index_buffer_view& view);

   void ia_set_vertex_buffers(const uint32 start_slot,
                              const std::span<const vertex_buffer_view> views);

   void ia_set_vertex_buffers(const uint32 start_slot, const vertex_buffer_view& view);

   void rs_set_viewports(const std::span<const viewport> viewports);

   void rs_set_viewports(const viewport& viewport);

   void rs_set_scissor_rects(const std::span<const rect> scissor_rects);

   void rs_set_scissor_rects(const rect& scissor_rect);

   void om_set_blend_factor(const float4& blend_factor);

   void om_set_stencil_ref(const uint32 stencil_ref);

   void om_set_render_targets(const std::span<const rtv_handle> render_targets);

   void om_set_render_targets(const std::span<const rtv_handle> render_targets,
                              const dsv_handle depth_stencil);

   void om_set_render_targets(const rtv_handle render_target);

   void om_set_render_targets(const rtv_handle render_target,
                              const dsv_handle depth_stencil);

   void om_set_render_targets(const dsv_handle depth_stencil);

   void clear_depth_stencil_view(const dsv_handle depth_stencil,
                                 const depth_stencil_clear_flags flags,
                                 const float depth, const uint8 stencil,
                                 const rect* const rect = nullptr);

   void clear_render_target_view(const rtv_handle rendertarget, const float4& color,
                                 const rect* const rect = nullptr);

   friend device;
};

/// Command Queue Definitions ///

/// @brief Ecapsulates a ID3D12CommandQueue with a high(er) level interface.
struct command_queue {
   /// @brief Submits command lists for execution to the queue.
   /// @param command_lists The command lists to execute.
   void execute_command_lists(std::derived_from<command_list> auto&... command_lists)
   {
      std::array command_lists_array{static_cast<command_list*>(&command_lists)...};

      execute_command_lists(command_lists_array);
   }

   /// @brief Submits command lists for execution to the queue.
   /// @param command_lists The command lists to execute.
   void execute_command_lists(std::span<command_list*> command_lists);

   /// @brief Insert a fence on this queue to synchronize with another queue.
   /// @param other The queue to synchronize with.
   void sync_with(command_queue& other);

   /// @brief Wait for all currently submited work to this queue to finish before returning.
   void wait_for_idle();

   void release_root_signature(root_signature_handle root_signature);

   void release_pipeline(pipeline_handle pipeline);

   void release_resource(resource_handle resource);

   void release_resource_view(resource_view resource_view);

   void release_render_target_view(rtv_handle render_target_view);

   void release_depth_stencil_view(dsv_handle depth_stencil_view);

   void release_query_heap(query_heap_handle query_heap);

   void release_command_allocator(command_allocator_handle command_allocator);

   auto get_timestamp_frequency() -> uint64;

private:
   friend device;

   command_queue();

   explicit command_queue(const command_queue_init& init);

   ~command_queue();

   command_queue(const command_queue& other) noexcept = delete;

   auto operator=(const command_queue& other) noexcept -> command_queue& = delete;

   command_queue(command_queue&& other) noexcept = delete;

   auto operator=(command_queue&& other) noexcept -> command_queue& = delete;

   implementation_storage<command_queue_state, 136> state;
};

/// Swapchain Definitions ///

struct current_backbuffer {
   resource_handle resource;
   rtv_handle rtv;
};

struct swap_chain {
   /// @brief Wait for the swap chain to be ready for rendering. Or does nothing if swap_chain_desc::frame_latency_waitable is false.
   void wait_for_ready();

   /// @brief Present the frame.
   /// @param allow_tearing Allow tearing/disable V-Sync.
   void present(bool allow_tearing);

   /// @brief Resize the swap chain buffers in response to a window size change.
   /// @param new_width The new width.
   /// @param new_height The new height.
   void resize(uint32 new_width, uint32 new_height);

   /// @brief Gets the back buffer to use for rendering the current frame.
   /// @return The back buffer to draw into for the current frame. The handles remain owned by the swap chain.
   auto current_back_buffer() -> current_backbuffer;

   /// @brief Gets the current width of the back buffer.
   auto width() -> uint32;

   /// @brief Gets the current height of the back buffer.
   auto height() -> uint32;

   /// Constructors/Destructor ///

   swap_chain();

   ~swap_chain();

   swap_chain(swap_chain&& other) noexcept;

   auto operator=(swap_chain&& other) noexcept -> swap_chain&;

   swap_chain(const swap_chain& other) noexcept = delete;

   auto operator=(const swap_chain& other) noexcept -> swap_chain& = delete;

private:
   friend device;

   implementation_storage<swap_chain_state, 144> state;
};

/// Device Structures ///

struct device_desc {
   /// @brief The preferred type of GPU to select.
   gpu_preference gpu_preference = gpu_preference::unspecified;

   /// @brief Enable D3D12 debug layer.
   bool enable_debug_layer = false;

   /// @brief Enable GPU based validation from the D3D12 debug layer.
   bool enable_gpu_based_validation = false;

   /// @brief Enable state tracking when GPU based validation is enabled.
   bool enable_gpu_based_validation_state_tracking = false;

   /// @brief Force usage of legacy barriers.
   bool force_legacy_barriers = false;

   /// @brief Force the device to report SM 6.6 as unsupported.
   bool force_no_shader_model_6_6 = false;

   /// @brief Force the device to report open existing heap as unsupported.
   bool force_no_open_existing_heap = false;
};

/// Device Definitions ///

struct device {
private:
   implementation_storage<device_state, 496> state;

public:
   command_queue direct_queue;
   command_queue compute_queue;
   command_queue copy_queue;
   command_queue background_copy_queue;

   /// @brief Return the index to use for buffered resources (exluding swap chains) this frame.
   /// @return The index in the range [0, frame_pipeline_length - 1].
   auto frame_index() -> uint64;

   /// @brief Perform necessary new frame tasks and synchronization.
   void new_frame();

   /// @brief Perform necessary end-of-frame tasks and synchronization.
   void end_frame();

   /// @brief Wait for all currently submited work to the command queues to finish before returning
   void wait_for_idle();

   /// Command List Functions ///

   [[nodiscard]] auto create_copy_command_list(const command_list_desc& desc)
      -> copy_command_list;

   [[nodiscard]] auto create_compute_command_list(const command_list_desc& desc)
      -> compute_command_list;

   [[nodiscard]] auto create_graphics_command_list(const command_list_desc& desc)
      -> graphics_command_list;

   /// Pipeline Functions ///

   [[nodiscard]] auto create_root_signature(const root_signature_desc& desc)
      -> root_signature_handle;

   [[nodiscard]] auto create_graphics_pipeline(const graphics_pipeline_desc& desc)
      -> pipeline_handle;

   [[nodiscard]] auto create_compute_pipeline(const compute_pipeline_desc& desc)
      -> pipeline_handle;

   /// Resource Functions ///

   [[nodiscard]] auto create_buffer(const buffer_desc& desc,
                                    const heap_type heap_type) -> resource_handle;

   [[nodiscard]] auto create_texture(const texture_desc& desc,
                                     const barrier_layout initial_resource_layout,
                                     const legacy_resource_state legacy_initial_resource_state)
      -> resource_handle;

   [[nodiscard]] auto open_existing_buffer(const void* address, const buffer_desc& desc)
      -> resource_handle;

   [[nodiscard]] auto get_gpu_virtual_address(resource_handle resource)
      -> gpu_virtual_address;

   [[nodiscard]] auto map(resource_handle resource, uint32 subresource,
                          memory_range read_range) -> void*;

   void unmap(resource_handle resource, uint32 subresource, memory_range write_range);

   /// Resource View Functions ///

   [[nodiscard]] auto create_shader_resource_view(resource_handle resource,
                                                  const shader_resource_view_desc& desc)
      -> resource_view;

   [[nodiscard]] auto create_unordered_access_view(resource_handle resource,
                                                   const unordered_access_view_desc& desc)
      -> resource_view;

   [[nodiscard]] auto create_render_target_view(resource_handle resource,
                                                const render_target_view_desc& desc)
      -> rtv_handle;

   [[nodiscard]] auto create_depth_stencil_view(resource_handle resource,
                                                const depth_stencil_view_desc& desc)
      -> dsv_handle;

   /// Query Functions ///

   [[nodiscard]] auto create_timestamp_query_heap(const uint32 count)
      -> query_heap_handle;

   /// Swapchain Functions ///

   [[nodiscard]] auto create_swap_chain(const swap_chain_desc& desc) -> swap_chain;

   /// Unsynced Immediate Release Functions ///

   void release_root_signature(root_signature_handle root_signature);

   void release_pipeline(pipeline_handle pipeline);

   void release_resource(resource_handle resource);

   void release_resource_view(resource_view resource_view);

   void release_render_target_view(rtv_handle render_target_view);

   void release_depth_stencil_view(dsv_handle depth_stencil_view);

   void release_query_heap(query_heap_handle query_heap);

   /// Feature Test Functions ///

   [[nodiscard]] bool supports_enhanced_barriers() const noexcept;

   [[nodiscard]] bool supports_shader_barycentrics() const noexcept;

   [[nodiscard]] bool supports_conservative_rasterization() const noexcept;

   [[nodiscard]] bool supports_shader_model_6_6() const noexcept;

   [[nodiscard]] bool supports_open_existing_heap() const noexcept;

   /// Constructors/Destructor ///

   device(const device_desc& desc);

   ~device();

   device(const device& other) noexcept = delete;

   auto operator=(const device& other) noexcept -> device& = delete;

   device(device&& other) noexcept = delete;

   auto operator=(device&& other) noexcept -> device& = delete;
};

}

namespace we::graphics {

// No one wants to read gpu::gpu_virtual_address

using gpu::gpu_virtual_address;

}